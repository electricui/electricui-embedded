#include "eui_serial_transport.h"

uint8_t calc_crc(uint8_t *to_xor, uint8_t datagram_len) 
{
  uint8_t XOR; 
  uint8_t i;

  for (XOR = 0, i = 0; i < datagram_len; i++) 
  {
    XOR ^= to_xor[i];
  }

  return XOR;
}

uint8_t generate_header(uint8_t internal, uint8_t ack, uint8_t offset_packet, uint8_t payloadtype)
{
  euiHeader_t genHeader; 

  genHeader.internal  = internal;
  genHeader.reqACK    = ack;
  genHeader.offsetAd  = offset_packet;
  genHeader.type      = payloadtype;

  return *(uint8_t*)&genHeader;
}

void generate_packet(const char * msg_id, uint8_t header, uint8_t payload_len, void* payload, CallBackwithUINT8 output_function)
{
  generate_packet_offset(msg_id, header, payload_len, MSG_STANDARD_PACKET, payload, output_function);
}

void generate_packet_offset(const char * msg_id, uint8_t header, uint8_t payload_len, uint16_t offset, void* payload, CallBackwithUINT8 output_function)
{
  uint8_t packetBuffer[PACKET_BASE_SIZE + payload_len];  //holding array large enough to fit the entire packet
  uint8_t p = 0;

  //preamble and header
  packetBuffer[p++] = stHeader;
  packetBuffer[p++] = *(uint8_t*)header;

  //copy the message ID in
  strcpy(packetBuffer+p, msg_id);
  p += strlen(msg_id);

  //payload length
  memcpy(packetBuffer+p, payload_len, sizeof(payload_len));
  p += sizeof(payload_len);

  //payload offset if used
  if((*(euiHeader_t*)header).offsetAd)
  {
    memcpy(packetBuffer+p, offset, sizeof(offset));
    p += sizeof(offset);
  }

  //start of payload control character
  packetBuffer[p++] = stText;

  //payload
  memcpy(packetBuffer+p, payload, payload_len);
  p += payload_len;

  //end payload control character, checksum
  packetBuffer[p++] = enText;
  uint8_t crc = calc_crc(packetBuffer, p);
  packetBuffer[p++] = crc;

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

void parse_packet(uint8_t inbound_byte, struct eui_interface_state *active_interface)
{
  switch(active_interface->controlState)
  {
    case find_preamble:
      //random data we don't care about prior to preamble
      if(inbound_byte == stHeader)
      {
        active_interface->controlState = exp_header;
        active_interface->processedCRC = 0; //clear out the CRC for this new message
      }
    break;

    case exp_header:
      //we've seen the preamble 0x01 control char. Nnext byte should be the header
      active_interface->inboundHeader = inbound_byte;
      active_interface->controlState = exp_msgID;
    break;
    
    case exp_msgID:
      //Check if we've seen a shorter than maxLength msgID
      if(active_interface->processedID && inbound_byte == enID )  
      {
        active_interface->controlState = exp_payload_len;
        active_interface->inboundID[active_interface->processedID + 1] = '\0';  //terminate msgID string
      }
      else
      {
        //read the message ID character in
        active_interface->inboundID[active_interface->processedID] = inbound_byte;
        active_interface->processedID++;

        //stop parsing and wait for STX, messageID can't be longer than preset maxlength
        if(active_interface->processedID >= MESSAGEID_SIZE)
        {
          active_interface->controlState = exp_payload_len;
          active_interface->inboundID[active_interface->processedID] = '\0';  //terminate the ID string
        }
      }
    break;
    
    case exp_payload_len:
      active_interface->inboundSize = inbound_byte;
      active_interface->controlState = exp_offset;
    break;

    case exp_offset:
      //first byte value determines if the offset functionality is being used
      if(!active_interface->inboundOffset)
      {
        if(inbound_byte == stText) 
        {
          //there is no offset value
          active_interface->inboundOffset = 0;  //todo remove redundant set?
          active_interface->controlState = exp_data;
        }
        else
        {
          //ingest first byte of offset into LSB
          active_interface->inboundOffset = inbound_byte;
          active_interface->inboundOffset << 8;
        }
      }
      else
      {
        //ingest second offset byte as the MSB
        active_interface->inboundOffset |= inbound_byte;
        active_interface->controlState = exp_stx;
      }
    break;
    
    case exp_stx:
      //wait for a STX to know the payload is coming
      if(inbound_byte == stText)
      {
        active_interface->controlState = exp_data;
      }
    break;
    
    case exp_data:
      //we know the length of the payload, parse until we've eaten that many bytes
      active_interface->inboundData[active_interface->processedData] = inbound_byte;
      active_interface->processedData++;

      if(active_interface->processedData >= active_interface->inboundSize)
      {
        //stop parsing and wait for STX, messageID can't be longer than that
        active_interface->controlState = exp_etx;
      }
      //we even eat ETX characters, because they are valid bytes in a payload... todo add improvements
    break;
    
    case exp_etx:
      //payload data saved in, the first character into this state really should be ETX
      if(inbound_byte == enText)
      {
        active_interface->controlState = exp_crc;
      }
      else
      {
        //data trails payload
      }
    break;
    
    case exp_crc:
      //the ETX character has been seen, the newest byte is the CRC
      active_interface->inboundCRC = inbound_byte;
      active_interface->controlState = exp_eot;
    break;
    
    case exp_eot:
      //we've seen the checksum byte and are waiting for end of packet indication
      if(inbound_byte == enTransmission)
      {
        //if the running xor matched recieved checksum, running would have changed to 0x00
        //since we add the new data to the running CRC outside this switch (afterwards)
        //0x00 gives validated payload.
        if(active_interface->processedCRC == 0)
        {
          handle_packet(active_interface);
        }
        else
        {
          //invalid crc (TODO, add error reporting)
        }

        //done handling the message, clear out the state info (but leave the output pointer alone)
        memset( active_interface, 0, sizeof(struct eui_interface_state) - sizeof(CallBackwithUINT8) );
      }
      else
      {
        //unknown data after CRC and before EOT char
      }
    break;  
  }   //end switch

  active_interface->processedCRC ^= inbound_byte;  //running crc
}