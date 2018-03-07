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
generate_header(uint8_t internal, uint8_t ack, uint8_t query, uint8_t offset_packet, uint8_t data_type, uint8_t msgID_len, uint8_t data_length, uint8_t sequence_num)
{
  euiHeader_t temp_header; 

  temp_header.internal   = internal;
  temp_header.ack        = ack;
  temp_header.query      = query;
  temp_header.offset     = offset_packet;
  temp_header.type       = data_type;
  temp_header.id_len     = msgID_len;
  temp_header.data_len   = data_length;
  temp_header.seq        = sequence_num;

  return &temp_header;
}

void
form_packet_simple(CallBackwithUINT8 output_function, euiPacketSettings_t *settings, const char * msg_id, uint8_t payload_len, void* payload)
{
  //just call the full one with default seq# and offset values
  form_packet_full(output_function, settings, 0, msg_id, 0x00, payload_len, payload);
}

void
form_packet_full(CallBackwithUINT8 output_function, euiPacketSettings_t *settings, uint8_t sequence_num, const char * msg_id, uint16_t offset_addr, uint8_t payload_len, void* payload)
{
  euiHeader_t temp_header;

  temp_header.internal   = settings->internal;
  temp_header.ack        = settings->ack;
  temp_header.query      = settings->query;
  temp_header.offset     = (offset_addr) ? MSG_OFFSET_PACKET : MSG_STANDARD_PACKET;
  temp_header.type       = settings->type;
  temp_header.id_len     = strlen(msg_id);
  temp_header.data_len   = payload_len;
  temp_header.seq        = sequence_num;  //todo implement this properly

  write_packet(output_function, &temp_header, msg_id, offset_addr, payload);
}

void
write_packet(CallBackwithUINT8 output_function, euiHeader_t * header, const char * msg_id, uint16_t offset, void* payload)
{
  if(output_function)  //todo ASSERT if not valid?
  {  
    //preamble
    output_function( stHeader );
    
    uint16_t outbound_crc = 0xffff;

    //header
    uint16_t header_buffer = 0;
    header_buffer |= header->internal << 0;
    header_buffer |= header->ack      << 1;
    header_buffer |= header->query    << 2;
    header_buffer |= header->offset   << 3;
    header_buffer |= header->type     << 4;
    output_function( header_buffer );
    crc16(header_buffer, &outbound_crc); 

    header_buffer = 0;  //reuse the buffer for the remaining 2-bytes
    header_buffer |= header->seq << 14;
    header_buffer |= header->id_len  << 10;
    header_buffer |= (header->data_len);
    output_function( header_buffer & 0xFF );
    crc16(header_buffer & 0xFF , &outbound_crc); 
    output_function( header_buffer >> 8 );
    crc16(header_buffer >> 8 , &outbound_crc); 

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
      output_function( *((uint8_t *)payload + i) );
      crc16(*((uint8_t *)payload + i), &outbound_crc); 
    }

    //checksum between the preamble and CRC
    output_function( outbound_crc & 0xFF );
    output_function( outbound_crc >> 8 );

    //packet terminator
    output_function( enTransmission );
  }
}

void
parse_packet(uint8_t inbound_byte, struct eui_interface *active_interface)
{
  if(active_interface->state.parser_s < exp_crc)    //only CRC the data between preamble and the CRC (exclusive)
  {
    crc16(inbound_byte, &(active_interface->runningCRC)); 
  }

  switch(active_interface->state.parser_s)
  {
    case find_preamble:
      //Ignore random bytes prior to preamble
      if(inbound_byte == stHeader)
      {
        active_interface->state.parser_s = exp_header;
        active_interface->runningCRC = 0xffff;
      }
    break;

    case exp_header:
      //Next bytes are the header
      active_interface->header_data[active_interface->state.header_bytes_in] = inbound_byte;
      active_interface->state.header_bytes_in++;

      //finished reading in header data
      if(active_interface->state.header_bytes_in >= sizeof(euiHeader_t))
      {
        //populate the header from recieved data
        active_interface->inboundHeader.internal  = (active_interface->header_data[0] >> 0) & 1;
        active_interface->inboundHeader.ack       = (active_interface->header_data[0] >> 1) & 1;
        active_interface->inboundHeader.query     = (active_interface->header_data[0] >> 2) & 1;
        active_interface->inboundHeader.offset    = (active_interface->header_data[0] >> 3) & 1;
        active_interface->inboundHeader.type      = active_interface->header_data[0] >> 4;

        uint16_t h_bytes_temp = ((uint16_t)active_interface->header_data[2] << 8) | active_interface->header_data[1];

        active_interface->inboundHeader.seq       = (h_bytes_temp >> 14);         //shift 14 bits
        active_interface->inboundHeader.id_len    = (h_bytes_temp >> 10) & 0x0F;  //shift 10-bits, and mask lowest 4
        active_interface->inboundHeader.data_len  = h_bytes_temp & 0x03FF;        //mask for lowest 10 bits

        active_interface->state.parser_s = exp_message_id;
      }
    break;
    
    case exp_message_id:
      //Bytes are messageID until we hit the length specified in the header
      active_interface->inboundID[active_interface->state.id_bytes_in] = inbound_byte;
      active_interface->state.id_bytes_in++;

      //we've read the number of bytes specified by the header count OR
      //we've ingested the maximum allowable length of the message ID
      if(active_interface->state.id_bytes_in >= active_interface->inboundHeader.id_len || active_interface->state.id_bytes_in >= MESSAGEID_SIZE)
      {
        //terminate msgID string if shorter than max size
        if(active_interface->state.id_bytes_in < MESSAGEID_SIZE)
        {
          active_interface->inboundID[active_interface->state.id_bytes_in + 1] = '\0';
        }

        //start reading in the offset or data based on header guide
        if(active_interface->inboundHeader.offset)
        {
          active_interface->state.parser_s = exp_offset;
        }
        else
        {
          if(active_interface->inboundHeader.data_len)
          {
            active_interface->state.parser_s = exp_data;            
          }
          else
          {
            active_interface->state.parser_s = exp_crc;            
          }
        }
      }
    break;

    case exp_offset:
      //first byte value determines if the offset functionality is being used
      //TODO rewrite this to be less type/size specific and generally more consistent counting behaviour
      if(!active_interface->state.offset_bytes_in)
      {
          //ingest first byte of offset into LSB
          active_interface->inboundOffset = inbound_byte;
          active_interface->inboundOffset << 8;
          active_interface->state.offset_bytes_in++;
      }
      else
      {
        //ingest second offset byte as the MSB
        active_interface->inboundOffset |= inbound_byte;
        active_interface->state.parser_s = exp_data;
      }
    break;
    
    case exp_data:
      //we know the payload length, parse until we've eaten those bytes
      active_interface->inboundData[active_interface->state.data_bytes_in] = inbound_byte;
      active_interface->state.data_bytes_in++;

      //prepare for the crc data
      if(active_interface->state.data_bytes_in >= active_interface->inboundHeader.data_len)
      {
        active_interface->state.parser_s = exp_crc;
      }
    break;
    
    case exp_crc:
      //ingest the two bytes for the CRC
      if(active_interface->state.crc == crc_no_data)
      {
        //ingest first byte into a buffer
        active_interface->crc_buffer = inbound_byte;
        active_interface->state.crc = crc_half_ingested;          
      }
      else
      {
        //ingest second byte as the MSB, and compare against the running CRC
        if(active_interface->runningCRC == ( ((uint16_t)inbound_byte << 8) | active_interface->crc_buffer )) 
        {
          active_interface->state.crc = crc_validated;
          active_interface->state.parser_s = exp_eot;
        }
      }
    break;
    
    case exp_eot:
      //we've seen the checksum byte and are waiting for end of packet indication
      if(inbound_byte == enTransmission)
      {
        //when parsed and calculated checksum match, its a valid packet
        if(active_interface->state.crc == crc_validated)
        {
          handle_packet(active_interface);
        }
        else
        {
          //invalid crc (TODO, add error reporting)
        }

        //done handling the message, clear out the state info (but leave the output pointer alone)
        //todo clean this up
        memset( active_interface, 0, sizeof(struct eui_interface) - sizeof(CallBackwithUINT8) );
      }
      else
      {
        //unknown data after CRC and before EOT char
      }
    break;  
  }
}