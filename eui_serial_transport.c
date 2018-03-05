#include "eui_serial_transport.h"

uint16_t
crc16(uint8_t *data, uint8_t len)
{
    uint16_t crc = 0xffff;

    while (len-- > 0) 
    {
      crc  = (uint8_t)(crc >> 8) | (crc << 8);
      crc ^= *data++;
      crc ^= (uint8_t)(crc & 0xff) >> 4;
      crc ^= (crc << 8) << 4;
      crc ^= ((crc & 0xff) << 4) << 1;
    }

    return(crc);
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
form_offset_packet_simple(CallBackwithUINT8 output_function, euiPacketSettings_t *settings, const char * msg_id, uint16_t offset_addr, uint8_t payload_len, void* payload)
{
  euiHeader_t temp_header;

  temp_header.internal   = settings->internal;
  temp_header.ack        = settings->ack;
  temp_header.query      = settings->query;
  temp_header.offset     = (offset_addr) ? MSG_OFFSET_PACKET : MSG_STANDARD_PACKET;
  temp_header.type       = settings->type;
  temp_header.id_len     = strlen(msg_id);
  temp_header.data_len   = payload_len;
  temp_header.seq        = 0;  //todo implement this properly

  write_packet(output_function, &temp_header, msg_id, offset_addr, payload);
}

void
write_packet(CallBackwithUINT8 output_function, euiHeader_t * header, const char * msg_id, uint16_t offset, void* payload)
{
  uint8_t packetBuffer[PACKET_BASE_SIZE + header->id_len + header->data_len];  //holding array large enough to fit the entire packet
  uint8_t p = 0;

  //preamble
  packetBuffer[p++] = stHeader;

  //header
  packetBuffer[p] = 0;
  packetBuffer[p] |= header->internal << 0;
  packetBuffer[p] |= header->ack      << 1;
  packetBuffer[p] |= header->query    << 2;
  packetBuffer[p] |= header->offset   << 3;
  packetBuffer[p] |= header->type     << 4;
  p++;

  uint16_t header_tail;
  header_tail |= header->seq << 14;
  header_tail |= header->id_len  << 10;
  header_tail |= (header->data_len);

  packetBuffer[p++] = header_tail & 0xFF;
  packetBuffer[p++] = header_tail >> 8;

  //copy the message ID in
  memcpy(packetBuffer+p, msg_id, header->id_len);
  p += header->id_len;

  //data offset if used
  if(header->offset)
  {
    memcpy(packetBuffer+p, offset, sizeof(offset));
    p += sizeof(offset);
  }
  
  //payload
  memcpy(packetBuffer+p, payload, header->data_len);
  p += header->data_len;

  //checksum between the preamble and CRC
  uint16_t crc = crc16(&packetBuffer[sizeof(stHeader)], p - sizeof(stHeader) );
  packetBuffer[p++] = crc & 0xFF;
  packetBuffer[p++] = crc >> 8;

  //end of packet character, and null-terminate the array
  packetBuffer[p++] = enTransmission;
  packetBuffer[p] = '\0';

  //pass the message to the output function
  for (uint8_t i = 0; i < p; i++) 
  {
    if(output_function)  //todo ASSERT if not valid?
    {
      output_function(packetBuffer[i]);
    }
  }
}

void
parse_packet(uint8_t inbound_byte, struct eui_interface *active_interface)
{
  switch(active_interface->state.parser_s)
  {
    case find_preamble:
      //Ignore random bytes prior to preamble
      if(inbound_byte == stHeader)
      {
        active_interface->state.parser_s = exp_header;
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
        active_interface->inboundHeader.internal  = (active_interface->header_data[0] >> 7) & 1;
        active_interface->inboundHeader.ack       = (active_interface->header_data[0] >> 6) & 1;
        active_interface->inboundHeader.query     = (active_interface->header_data[0] >> 5) & 1;
        active_interface->inboundHeader.offset    = (active_interface->header_data[0] >> 4) & 1;
        active_interface->inboundHeader.type      = active_interface->header_data[0] >> 3;

        uint16_t h_bytes_temp = ((uint16_t)active_interface->header_data[2] << 8) | active_interface->header_data[1];

        active_interface->inboundHeader.id_len    = (h_bytes_temp >> 12) + 1;
        active_interface->inboundHeader.data_len  = (h_bytes_temp & 0x0ffc) >> 2;
        active_interface->inboundHeader.seq       = h_bytes_temp & 0x03;

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
          active_interface->state.parser_s = exp_data;
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
      if(active_interface->state.data_bytes_in >= active_interface->inboundHeader.id_len)
      {
        active_interface->state.parser_s = exp_crc;
      }
    break;
    
    case exp_crc:
      //ingest the two bytes for the CRC
      if(!active_interface->state.crc_bytes_in)
      {
          //ingest first byte into LSB
          active_interface->inboundCRC = inbound_byte;
          active_interface->inboundCRC << 8;
          active_interface->state.crc_bytes_in++;
      }
      else
      {
        //ingest second byte as the MSB
        active_interface->inboundCRC |= inbound_byte;
        active_interface->state.parser_s = exp_eot;
      }
    break;
    
    case exp_eot:
      //we've seen the checksum byte and are waiting for end of packet indication
      if(inbound_byte == enTransmission)
      {
        //crc the data that falls between the SOH and CRC data (header, msgID, offset, payload)
        uint8_t length_to_crc = sizeof(euiHeader_t) + active_interface->inboundHeader.id_len + sizeof(active_interface->inboundOffset) + active_interface->inboundHeader.data_len;
        uint16_t calculated_crc = crc16(&active_interface->inboundHeader, length_to_crc );

        //when parsed and calculated checksum match, its a valid packet
        if(calculated_crc == active_interface->inboundCRC)
        {
          handle_packet(active_interface);
        }
        else
        {
          //invalid crc (TODO, add error reporting)
        }

        //done handling the message, clear out the state info (but leave the output pointer alone)
        memset( active_interface, 0, sizeof(struct eui_interface) - sizeof(CallBackwithUINT8) );
      }
      else
      {
        //unknown data after CRC and before EOT char
      }
    break;  
  }
}