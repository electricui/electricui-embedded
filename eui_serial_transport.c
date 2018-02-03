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

uint8_t generate_header(uint8_t internal, uint8_t ack, uint8_t reservedbit, uint8_t customtype, uint8_t payloadtype)
{
  euiHeader_t genHeader; 

  genHeader.internal  = internal;
  genHeader.reqACK    = ack;
  genHeader.reserved  = reservedbit;

  if(payloadtype >= TYPE_CUSTOM_MARKER || customtype)
  {
    genHeader.customType = MSG_TYPE_CUSTOM; //they've passed a type larger than we expect, must be custom
    genHeader.type = payloadtype - MSG_TYPE_CUSTOM;
  }
  else
  {
    genHeader.customType = customtype;
    genHeader.type = payloadtype;
  }

  return *(uint8_t*)&genHeader;
}

void generate_packet(const char * msg_id, uint8_t header, uint8_t payload_len, void* payload, CallBackwithUINT8 output_function)
{
  uint8_t packetBuffer[PACKET_BASE_SIZE + payload_len + 1];  //todo see if +1 can be removed
  uint8_t p = 0;

  //preamble and header
  packetBuffer[p++] = stHeader;
  packetBuffer[p++] = header;

  //copy the message ID in
  strcpy(packetBuffer+p, msg_id);
  p += strlen(msg_id);

  //start of payload control character
  packetBuffer[p++] = stText;

  //payload length and payload data
  packetBuffer[p++] = payload_len;
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
      if(active_interface->processedID && inbound_byte == stText )  
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
          active_interface->controlState = exp_stx;
          active_interface->inboundID[active_interface->processedID] = '\0';  //terminate the ID string
        }
      }
    break;
    
    case exp_stx:
      //wait for a STX to know the payloadlength is coming
      if(inbound_byte == stText)
      {
        active_interface->controlState = exp_payload_len;
      }
    break;
    
    case exp_payload_len:
      //first byte should be payload length
      active_interface->inboundSize = inbound_byte;
      active_interface->controlState = (active_interface->inboundSize == 0) ? exp_etx: exp_data;
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
      //we even eat ETX characters, because they are valid bytes in a payload...
    break;
    
    case exp_etx:
      //payload data saved in, the first character into this state really should be ETX
      //wait for a ETX to know checksum is coming
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
      //the ETX character has been seen, the next byte is the CRC
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
        //TODO check if this is dangerous (seems like the same thing as a normal equality check, just later?)
        //TODO could be an issue if the CRC negates itself due to no new data or a false reset?
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