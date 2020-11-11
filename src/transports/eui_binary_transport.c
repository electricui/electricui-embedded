/* Copyright (c) 2016-2020 Electric UI
 * MIT Licenced - see LICENCE for details.
 *
 * 
 */

#include <string.h>
#include "eui_binary_transport.h"
#include "../eui_utilities.h"

uint8_t
encode_packet_simple(   callback_data_out_t output_function,
                        eui_pkt_settings_t  *settings,
                        const char          *msg_id,
                        uint16_t            payload_len,
                        void*               payload )
{
    // Call the full encode with default ack# and offset values
    eui_header_t expanded_header;

    expanded_header.internal   = settings->internal;
    expanded_header.response   = settings->response;
    expanded_header.type       = settings->type;
    expanded_header.acknum     = 0;
    expanded_header.offset     = 0;
    expanded_header.id_len     = strlen(msg_id);
    expanded_header.data_len   = payload_len;

    return encode_packet( output_function, &expanded_header, msg_id, 0x0000, payload );
}

uint8_t
encode_header( eui_header_t *header, uint8_t *buffer )
{
    // Header is 3 bytes total, inserted directly into the outgoing buffer
    uint8_t bytes_written = 0;

    if( header && buffer )
    {
        // payload length - 10b
        buffer[bytes_written]  = (uint8_t)( (uint16_t)header->data_len & 0xFFu );
        bytes_written++;
        buffer[bytes_written] |= (uint8_t)( (uint16_t)header->data_len >> 8u );

        // Payload Type - 4b
        buffer[bytes_written] |= (uint8_t)( ((uint8_t)header->type & 0x0Fu) << 2u );

        // Internal message - 1b
        buffer[bytes_written] |= (uint8_t)( (uint8_t)header->internal << 6u );

        // Offset message flag - 1b
        buffer[bytes_written] |= (uint8_t)( (uint8_t)header->offset << 7u );
        bytes_written++;

        // Message ID length - 4b
        buffer[bytes_written] |= (uint8_t)( (uint8_t)header->id_len & 0x0Fu );

        // Response Flag - 1b
        buffer[bytes_written] |= (uint8_t)( (uint8_t)header->response << 4u );

        // Ack number - 3b
        buffer[bytes_written] |= (uint8_t)( (uint8_t)header->acknum << 5u );
        bytes_written++;
    }

    return bytes_written;
}

uint8_t
encode_framing( uint8_t *buffer, uint16_t buf_size )
{
    uint16_t previous_null = 0;

    for( uint16_t i = 1; i < buf_size; i++ )
    {
        uint8_t bytes_since = i - previous_null;

        if( buffer[i] == 0x00u )
        {
            buffer[previous_null] = bytes_since;
            previous_null = i;
        } 
        else if( bytes_since == 0xFFu )
        {
            buffer[previous_null] = 0xFFu;

            // Ripple the buffer of data back one byte to make room
            // This 'extra' new byte is now the offset
            for( uint16_t j = 1; j < (buf_size-i); j++ )
            {
                buffer[ buf_size-j ] = buffer[ buf_size-j-1 ];
            }

            buffer[i] = 0xEEu;
            previous_null = i;
        }
    }

    buffer[0] = 0x00;

    return 0;
}

uint8_t
encode_packet(  callback_data_out_t out_char,
                eui_header_t        *header,
                const char          *msg_id,
                uint16_t            offset,
                void*               payload )
{
    uint8_t status = EUI_OUTPUT_ERROR;

    if( out_char && header && msg_id && payload )
    {  
        uint8_t pk_tmp[ 1 + PACKET_BASE_SIZE + EUI_MAX_MSGID_SIZE + PAYLOAD_SIZE_MAX + 4 ] = { 0 };
        uint16_t pk_i = 2; // Leave room for the 0x00 and framing offset byte

        // Write header bytes into the buffer
        pk_i += encode_header( header, &pk_tmp[pk_i] );

        // Message ID
        memcpy( &pk_tmp[pk_i], msg_id, header->id_len );
        pk_i += (uint8_t)header->id_len;

#ifndef EUI_CONF_OFFSETS_DISABLED
        if( header->offset )
        {
            memcpy( &pk_tmp[pk_i], &offset, sizeof(offset) );
            pk_i += sizeof(offset);
        }
#endif

        // Payload data copy
        memcpy( &pk_tmp[pk_i], (uint8_t *)payload + offset, header->data_len );
        pk_i += (uint16_t)header->data_len;

        // Calculate and write CRC
        uint16_t outbound_crc = 0xFFFFu;
        for( uint16_t i = 2; i < pk_i; i++ )
        {
            crc16( pk_tmp[i], &outbound_crc );
        }

        memcpy( &pk_tmp[pk_i], &outbound_crc, sizeof(outbound_crc) );
        pk_i += sizeof(outbound_crc);
        
        // Apply Consistent Overhead Byte Stuffing (COBS) for framing/sync
        pk_i += 1;  // +1 to account for null byte at end
        encode_framing( pk_tmp, pk_i);    

        out_char( pk_tmp, pk_i );
    
        status = EUI_OUTPUT_OK;
    }

    return status;
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
    uint8_t status = EUI_PARSER_IDLE;
    
    if( 0x00u == byte_in )
    {
        //reset
        p_link_in->parser.state = 0u;
        p_link_in->crc_in = 0xFFFFu;
    }
    else
    {
        if( 0x01 < p_link_in->parser.frame_offset )
        {
            // One byte closer to the next offset
            p_link_in->parser.frame_offset -= 1u;
        }
        else
        {
            // Offset has expired, this inbound byte should be the next data framing byte
            p_link_in->parser.frame_offset = byte_in;
            byte_in = 0x00u; // Replace with pre-COBS byte.
        }

        // CRC data up to the packet's CRC
        if( (exp_crc_b1 > (uint8_t)p_link_in->parser.state)
            && (exp_frame_offset < (uint8_t)p_link_in->parser.state) )
        {
            crc16( byte_in, &(p_link_in->crc_in)) ; 
        }
     
        status = parse_decoded_packet( byte_in, p_link_in );
    }

    return status;
}

uint8_t
parse_decoded_packet( uint8_t byte_in, eui_packet_t *p_link_in )
{
    uint8_t parse_status = EUI_PARSER_IDLE;

    // Parse the byte into the inbound packet buffers
    switch( (uint8_t)p_link_in->parser.state )
    {
        case exp_frame_offset:
            // First byte is the first offset
            p_link_in->parser.state = exp_header_b1;
        break;

        case exp_header_b1:
            p_link_in->header.data_len = byte_in;
            p_link_in->parser.state = exp_header_b2;
        break;

        case exp_header_b2:
            // The 'last' two length bits at start of this byte
            p_link_in->header.data_len |= (uint16_t)((uint16_t)byte_in << 8u) & 0x0300u;
            p_link_in->header.type      = (uint8_t)(byte_in >> 2u) & 0x0Fu;
            p_link_in->header.internal  = (uint8_t)(byte_in >> 6u) & 0x01u;
            p_link_in->header.offset    = (uint8_t)(byte_in >> 7u) & 0x01u;
            
            p_link_in->parser.state     = exp_header_b3;
        break;

        case exp_header_b3:
            p_link_in->header.id_len    = (uint8_t)(byte_in      ) & 0x0Fu;
            p_link_in->header.response  = (uint8_t)(byte_in >> 4u) & 0x01u;
            p_link_in->header.acknum    = (uint8_t)(byte_in >> 5u);
            
            p_link_in->parser.state     = exp_message_id;
        break;   
        
        case exp_message_id:
            // Bytes are messageID until we hit the length specified in the header
            p_link_in->id_in[p_link_in->parser.id_bytes_in] = byte_in;
            p_link_in->parser.id_bytes_in++;

            if( (uint8_t)p_link_in->parser.id_bytes_in >= (uint8_t)p_link_in->header.id_len )
            {
                // Null-terminate msgID string if shorter than max size
                p_link_in->id_in[p_link_in->parser.id_bytes_in] = '\0';
                
                // Start reading in the offset or data based on header guide
                if( p_link_in->header.offset )
                {
#ifndef EUI_CONF_OFFSETS_DISABLED
                    p_link_in->parser.state = exp_offset_b1;
#else
                    parse_status = EUI_PARSER_ERROR;
#endif
                }
                else
                {
                    if( p_link_in->header.data_len )
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
            p_link_in->offset_in    = (uint16_t)byte_in;
            p_link_in->parser.state = exp_offset_b2;
        break;

        case exp_offset_b2:
            p_link_in->offset_in     |= (uint16_t)((uint16_t)byte_in << 8u);
            p_link_in->parser.state  = exp_data;
        break;
#endif
        
        case exp_data:
            // Payload length is known from the header, parse until we've eaten those bytes
            p_link_in->data_in[p_link_in->parser.data_bytes_in] = byte_in;
            p_link_in->parser.data_bytes_in++;

            if( (  (uint16_t)p_link_in->parser.data_bytes_in >= (uint16_t)p_link_in->header.data_len )
                || ( (uint16_t)p_link_in->parser.data_bytes_in >= PAYLOAD_SIZE_MAX) )
            {
                p_link_in->parser.state = exp_crc_b1;
            }
        break;
        
        case exp_crc_b1:
        {
            uint8_t crc_low_byte = (p_link_in->crc_in & 0xFFu);
            
            if( byte_in == crc_low_byte )
            {
                p_link_in->parser.state = exp_crc_b2;
            }
            else  // First byte didn't match CRC, fail early
            {
                parse_status = EUI_PARSER_ERROR;
            }
        }
        break;

        case exp_crc_b2:
            if( byte_in == (p_link_in->crc_in >> 8u) )
            {
                parse_status = EUI_PARSER_OK;
            }
            else
            {
                parse_status = EUI_PARSER_ERROR;
            }

        break;

        default:
            // Shouldn't have unexpected parser state
            parse_status = EUI_PARSER_ERROR;
        break;
    }

    return parse_status;
} 