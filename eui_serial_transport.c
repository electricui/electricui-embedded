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

euiHeader_t *
generate_header(uint8_t internal, uint8_t query, uint8_t offset_packet, uint8_t data_type, uint8_t msgID_len, uint16_t data_length, uint8_t ack_num)
{
  euiHeader_t gen_header; 

  gen_header.internal   = internal;
  gen_header.query      = query;
  gen_header.offset     = offset_packet;
  gen_header.type       = data_type;
  gen_header.id_len     = msgID_len;
  gen_header.data_len   = data_length;
  gen_header.acknum     = ack_num;

  return &gen_header;
}

uint8_t
encode_packet_simple(CallBackwithUINT8 output_function, euiPacketSettings_t *settings, const char * msg_id, uint16_t payload_len, void* payload)
{
  //just call the full one with default ack# and offset values
  euiHeader_t expanded_header;

  expanded_header.internal   = settings->internal;
  expanded_header.query      = settings->query;
  expanded_header.type       = settings->type;
  expanded_header.acknum     = 0;
  expanded_header.offset     = 0;
  expanded_header.id_len     = strlen(msg_id);
  expanded_header.data_len   = payload_len;

  return encode_packet(output_function, &expanded_header, msg_id, 0x00, payload);
}

uint8_t
encode_packet(CallBackwithUINT8 output_function, euiHeader_t * header, const char * msg_id, uint16_t offset, void* payload)
{
  if(output_function)  //todo ASSERT if not valid?
  {  
    //preamble
    output_function( stHeader );
    
    uint16_t outbound_crc = 0xffff;

    //header
    uint16_t header_buffer = 0;

    header_buffer |= (header->data_len);
    header_buffer |= header->type     << 10;
    header_buffer |= header->internal << 14;
    header_buffer |= header->offset   << 15;

    //write first byte
    output_function( header_buffer & 0xFF );
    crc16(header_buffer & 0xFF, &outbound_crc); 

    //write second byte
    output_function( header_buffer >> 8 );
    crc16(header_buffer >> 8, &outbound_crc); 

    header_buffer = 0;  //reuse the buffer for the remaining byte

    header_buffer |= header->id_len;
    header_buffer |= header->query  << 4;
    header_buffer |= header->acknum << 5;
    
    //write third byte
    output_function( header_buffer );
    crc16(header_buffer, &outbound_crc); 

    //message identifier
    for(int i = 0; i < header->id_len; i++)
    {
      output_function( msg_id[i] );
      crc16(msg_id[i], &outbound_crc); 
    }

    //data offset if used
    if(header->offset)
    {
      output_function( offset & 0xFF );
      crc16(offset & 0xFF, &outbound_crc); 

      output_function( offset >> 8 );
      crc16(offset >> 8, &outbound_crc); 
    }
    
    //payload data
    for(int i = 0; i < header->data_len; i++)
    {
      output_function( *((uint8_t *)payload + i + offset) );
      crc16(*((uint8_t *)payload + i + offset), &outbound_crc); 
    }

    //checksum between the preamble and CRC
    output_function( outbound_crc & 0xFF );
    output_function( outbound_crc >> 8 );

    //packet terminator
    output_function( enTransmission );
  }

  return 0;
}

uint8_t
decode_packet(uint8_t inbound_byte, struct eui_interface *active_interface)
{
  if(active_interface->state.parser_s < exp_crc_b1)    //only CRC the data between preamble and the CRC (exclusive)
  {
    crc16(inbound_byte, &(active_interface->runningCRC)); 
  }

  switch(active_interface->state.parser_s)
  {
    case find_preamble:
    case exp_reset:
      //Ignore random bytes prior to preamble
      if(inbound_byte == stHeader)
      {
        active_interface->runningCRC = 0xFFFF;
        active_interface->state.parser_s = exp_header_b1;
      }
      else if(exp_reset)
      {
        //wipe out the array
      }
    break;

    case exp_header_b1:
      //populate the header bitfield from recieved byte
      active_interface->inboundHeader.internal  = (inbound_byte >> 0) & 1;
      // active_interface->inboundHeader.ack       = (inbound_byte >> 1) & 1;
      active_interface->inboundHeader.query     = (inbound_byte >> 2) & 1;
      active_interface->inboundHeader.offset    = (inbound_byte >> 3) & 1;
      active_interface->inboundHeader.type      = inbound_byte >> 4;

      active_interface->state.parser_s = exp_header_b2;
    break;

   case exp_header_b2:
      active_interface->inboundHeader.data_len = inbound_byte;
      
      active_interface->state.parser_s = exp_header_b3;
    break;

    case exp_header_b3:
      active_interface->inboundHeader.acknum    = (inbound_byte >> 6);         //read last two bits
      active_interface->inboundHeader.id_len    = (inbound_byte >> 2) & 0x0F;  //shift 2-bits, mask lowest 4
      active_interface->inboundHeader.data_len |= ((uint16_t)inbound_byte << 8) & 0x0300; //the 'last' two length bits = first 2b of this byte
      
      active_interface->state.parser_s = exp_message_id;
    break;   
    
    case exp_message_id:
      //Bytes are messageID until we hit the length specified in the header
      active_interface->inboundID[active_interface->state.id_bytes_in] = inbound_byte;
      active_interface->state.id_bytes_in++;

      //we've read the number of message ID bytes specified by the header
      if(active_interface->state.id_bytes_in >= active_interface->inboundHeader.id_len)
      {
        //terminate msgID string if shorter than max size
        if(active_interface->state.id_bytes_in < MESSAGEID_SIZE)
        {
          active_interface->inboundID[active_interface->state.id_bytes_in] = '\0';
        }

        //start reading in the offset or data based on header guide
        if(active_interface->inboundHeader.offset)
        {
          active_interface->state.parser_s = exp_offset_b1;
        }
        else
        {
          if(active_interface->inboundHeader.data_len)
          {
            active_interface->state.parser_s = exp_data;            
          }
          else
          {
            active_interface->state.parser_s = exp_crc_b1;            
          }
        }
      }
    break;

    case exp_offset_b1:
      active_interface->inboundOffset = inbound_byte;
      active_interface->inboundOffset << 8;

      active_interface->state.parser_s = exp_offset_b2;
    break;

    case exp_offset_b2:
      //ingest second offset byte as the MSB
      active_interface->inboundOffset |= inbound_byte;
      active_interface->state.parser_s = exp_data;
    break;
    
    case exp_data:
      //we know the payload length, parse until we've eaten those bytes
      active_interface->inboundData[active_interface->state.data_bytes_in] = inbound_byte;
      active_interface->state.data_bytes_in++;

      //prepare for the crc data
      if(active_interface->state.data_bytes_in >= active_interface->inboundHeader.data_len)
      {
        active_interface->state.parser_s = exp_crc_b1;
      }
    break;
    
    case exp_crc_b1:
      //check the inbound byte against the corresponding CRC byte
      if(inbound_byte == (active_interface->runningCRC & 0xFF) )
      {
        active_interface->state.parser_s = exp_crc_b2;        
      }
      else  //first byte didn't match CRC, fail early
      {
        active_interface->state.parser_s = exp_reset;
        return packet_error_crc;
      }
    break;

    case exp_crc_b2:
      if(inbound_byte == (active_interface->runningCRC >> 8) )  //CRC is correct 
      {
        active_interface->state.parser_s = exp_eot;  
      }
      else  //second byte didn't match CRC
      {
        active_interface->state.parser_s = exp_reset;
        return packet_error_crc;
      }
    break;

    case exp_eot:
      active_interface->state.parser_s = exp_reset;

      //we've recieved the end of packet indication
      if(inbound_byte == enTransmission)
      {
        return packet_valid; //signal to the application layer that a valid packet is waiting
      }
    break;  
  }
  
  return parser_idle;
}