#include <string.h>
#include "eui_serial_transport.h"

void
crc16(uint8_t data, uint16_t *crc)
{
    *crc  = (uint8_t)(*crc >> 8) | (*crc << 8);
    *crc ^= data;
    *crc ^= (uint8_t)(*crc & 0xff) >> 4;
    *crc ^= (*crc << 8) << 4;
    *crc ^= ((*crc & 0xff) << 4) << 1;
}

uint8_t
encode_packet_simple(   callback_uint8_t    output_function, 
                        eui_pkt_settings_t  *settings, 
                        const char          *msg_id, 
                        uint16_t            payload_len, 
                        void*               payload )
{
    //just call the full one with default ack# and offset values
    eui_header_t expanded_header;

    expanded_header.internal   = settings->internal;
    expanded_header.response   = settings->response;
    expanded_header.type       = settings->type;
    expanded_header.acknum     = 0;
    expanded_header.offset     = 0;
    expanded_header.id_len     = strlen(msg_id);
    expanded_header.data_len   = payload_len;

    return encode_packet( output_function, &expanded_header, msg_id, 0x00, payload );
}

uint8_t encode_header( eui_header_t *header, uint8_t *buffer )
{
    uint8_t bytes_written = 0;

    if(header && buffer)
    {
        // Header is 3 bytes
        // payload length 10b, internal 1b, offset 1b, idlen 4b, response 1b, acknum 3b
        buffer[bytes_written] = header->data_len & 0xFF;
        bytes_written++;
        
        buffer[bytes_written] |= (header->data_len & 0xC0) >> 6;
        buffer[bytes_written] |= (header->type     & 0x0F) << 2;
        buffer[bytes_written] |= header->internal  << 6;
        buffer[bytes_written] |= header->offset    << 7;
        bytes_written++;

        buffer[bytes_written] |= (header->id_len & 0x0F);
        buffer[bytes_written] |= header->response  << 4;
        buffer[bytes_written] |= header->acknum    << 5;
        bytes_written++;
    }

    return bytes_written;
}

uint8_t encode_framing( uint8_t *buffer, uint16_t buf_size )
{
    uint16_t previous_null = 0;

    for( uint16_t i = 1; i < buf_size; i++ )
    {
        uint8_t bytes_since = i - previous_null;

        if( buffer[i] == 0x00 )
        {
            buffer[previous_null] = bytes_since;
            previous_null = i;
        }
        // else
        // {
            // if(bytes_since == 0xFE)
            // {
                // buffer[previous_null] = 0xFF;
                // previous_null = i++;
            // }
        // }
    }

    buffer[0] = 0x00;

    return 0;
}
uint8_t
encode_packet(  callback_uint8_t    out_char, 
                eui_header_t        *header, 
                const char          *msg_id, 
                uint16_t            offset, 
                void*               payload )
{
    if(out_char)  //todo ASSERT if not valid?
    {  
        uint8_t pk_tmp[1 + PACKET_BASE_SIZE + MESSAGEID_SIZE + PAYLOAD_SIZE_MAX ] = { 0 };
        uint16_t pk_i = 2; //leave room for the 0x00 and framing byte

        // write header bytes into the buffer
        pk_i += encode_header( header, &pk_tmp[pk_i] );

        //message ID
        memcpy( &pk_tmp[pk_i], msg_id, header->id_len );
        pk_i += header->id_len;

        //offset address
#ifndef EUI_CONF_OFFSETS_DISABLED
        if( header->offset )
        {
            memcpy( &pk_tmp[pk_i], &offset, sizeof(offset) );
            pk_i += sizeof(offset);
        }
#endif

        //payload data
        memcpy( &pk_tmp[pk_i], payload + offset, header->data_len );
        pk_i += header->data_len;

        //calculate CRC
        uint16_t outbound_crc = 0xFFFF;

        for( uint16_t i = 2; i < pk_i; i++ )
        {
            crc16( pk_tmp[i], &outbound_crc );
        }

        //write result of CRC into buffer
        memcpy( &pk_tmp[pk_i], &outbound_crc, sizeof(outbound_crc) );
        pk_i += sizeof(outbound_crc);
        
        //Apply Consistent Overhead Byte Stuffing (COBS) for framing/sync
        encode_framing( pk_tmp, pk_i+1);    //+1 to account for null byte at end

        //write data
        for( uint16_t i = 0; i < pk_i; i++ )
        {
            out_char( pk_tmp[i] );
        }
    }

    return 0;
}


/*
    If byte 0x00 seen, cancel all parsing
    first byte is COBS offset, bytes falling on the offset location are 0x00,
    crc the actual data (COB corrected)
    then process the packet
        header 3 bytes
        msg identifier (1 to 15 bytes)
        offset 2 bytes optional
        data 0 to 1024 bytes
        CRC 2 bytes
*/
uint8_t
decode_packet(uint8_t byte_in, eui_packet_t *p_link_in)
{
    if( 0x00 == byte_in )
    {
        //reset
        p_link_in->parser.state = 0;
        p_link_in->crc_in = 0xFFFF;
    }
    else
    {
        if( 0x01 < p_link_in->parser.frame_offset )
        {
            //we are now one byte closer to the next offset
            p_link_in->parser.frame_offset -= 1;
        }
        else
        {
            //offset has expired, this inbound byte should be the next data framing byte
            p_link_in->parser.frame_offset = byte_in;
            byte_in = 0x00; //replace with pre-COBS byte.
        }

        //CRC data up to the packet's CRC
        // todo work out a way to remove this check
        if( p_link_in->parser.state < exp_crc_b1 && p_link_in->parser.state > exp_frame_offset )   
        {
            crc16(byte_in, &(p_link_in->crc_in)); 
        }
     
        return parse_decoded_packet( byte_in, p_link_in);
    }

    return parser_idle;
}

uint8_t
parse_decoded_packet(uint8_t byte_in, eui_packet_t *p_link_in)
{
    uint8_t parse_status = 0;

    //parse the byte into the inbound packet buffers
    switch( p_link_in->parser.state )
    {
        case exp_frame_offset:
            //first byte is the first offset
            //todo fix/rework this poor handling of 'offset with no underlying data'
            p_link_in->parser.state = exp_header_b1;
        break;

        case exp_header_b1:
            p_link_in->header.data_len = byte_in;
            p_link_in->parser.state = exp_header_b2;
        break;

        case exp_header_b2:
            //the 'last' two length bits = first 2b of this byte
            p_link_in->header.data_len |= ((uint16_t)byte_in << 8) & 0x0300; 
            //shift 2 and mask 4 for type
            p_link_in->header.type      = (byte_in >> 2) & 0x0F;
            p_link_in->header.internal  = (byte_in >> 6) & 1;
            p_link_in->header.offset    = (byte_in >> 7) & 1;
            
            p_link_in->parser.state     = exp_header_b3;
        break;

        case exp_header_b3:
            //mask lowest 4
            p_link_in->header.id_len    = (byte_in     ) & 0x0F;
            p_link_in->header.response  = (byte_in >> 4) & 1;
            p_link_in->header.acknum    = (byte_in >> 5);
            
            p_link_in->parser.state     = exp_message_id;
        break;   
        
        case exp_message_id:
            //Bytes are messageID until we hit the length specified in the header
            p_link_in->msgid_in[p_link_in->parser.id_bytes_in] = byte_in;
            p_link_in->parser.id_bytes_in++;

            //we've read the number of message ID bytes specified by the header
            if( p_link_in->parser.id_bytes_in >= p_link_in->header.id_len )
            {
                //terminate msgID string if shorter than max size
                if( p_link_in->parser.id_bytes_in < MESSAGEID_SIZE )
                {
                    p_link_in->msgid_in[p_link_in->parser.id_bytes_in] = '\0';
                }

                //start reading in the offset or data based on header guide
                if( p_link_in->header.offset )
                {
#ifndef EUI_CONF_OFFSETS_DISABLED
                    p_link_in->parser.state = exp_offset_b1;
#else
                    //add a error code for 'doesn't support offsets
                    parse_status = parser_error;
#endif
                }
                else
                {
                    if(p_link_in->header.data_len)
                    {
                        p_link_in->parser.state = exp_data;            
                    }
                    else
                    {
                        p_link_in->parser.state = exp_crc_b1;            
                    }
                }
            }
        break;

#ifndef EUI_CONF_OFFSETS_DISABLED
        case exp_offset_b1:
            //ingest first byte
            p_link_in->offset_in    = (uint16_t)byte_in << 8;
            p_link_in->parser.state = exp_offset_b2;
        break;

        case exp_offset_b2:
            //ingest second offset byte
            p_link_in->offset_in    |= byte_in;
            p_link_in->parser.state  = exp_data;
        break;
#endif
        
        case exp_data:
            //we know the payload length, parse until we've eaten those bytes
            p_link_in->data_in[p_link_in->parser.data_bytes_in] = byte_in;
            p_link_in->parser.data_bytes_in++;

            //prepare for the crc data, we've seen all the data we were expecting (or can hold)
            if( (p_link_in->parser.data_bytes_in >= p_link_in->header.data_len) 
                || (p_link_in->parser.data_bytes_in >= PAYLOAD_SIZE_MAX) )
            {
                p_link_in->parser.state = exp_crc_b1;
            }
        break;
        
        case exp_crc_b1:
        {
            //check the inbound byte against the corresponding CRC byte
            uint8_t crc_low_byte = (p_link_in->crc_in & 0xFF);
            if( byte_in == crc_low_byte )
            {
                p_link_in->parser.state = exp_crc_b2;        
            }
            else  //first byte didn't match CRC, fail early
            {
                parse_status = parser_error;
            }
        }
        break;

        case exp_crc_b2:
            if( byte_in == (p_link_in->crc_in >> 8) )
            {
                parse_status = parser_complete;  
            }
            else
            {
                parse_status = parser_error;
            }

        break;

        default:
            //shouldn't have unexpected parser state
            parse_status = parser_error;
        break;
    }

    return parse_status;
} 