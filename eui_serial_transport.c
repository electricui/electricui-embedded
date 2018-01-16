#include "eui_serial_transport.h"

uint8_t calcCRC(uint8_t *toSum, uint8_t datagramLen) 
{
  uint8_t XOR; 
  uint8_t i;

  for (XOR = 0, i = 0; i < datagramLen; i++) 
  {
    XOR ^= toSum[i];
  }

  return XOR;
}

uint8_t generateHeader(uint8_t internalmsg, uint8_t reqack, uint8_t reservedbit, uint8_t customtype, uint8_t payloadtype)
{
  euiHeader_t genHeader; 

  genHeader.internal  = internalmsg;
  genHeader.reqACK    = reqack;
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

void generatePacket(const char * msg_id, uint8_t header, uint8_t payloadLen, void* payload, struct eui_interface_state *commInterface)
{
  uint8_t packetBuffer[PACKET_BASE_SIZE + payloadLen + 1];  //todo see if +1 can be removed
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
  packetBuffer[p++] = payloadLen;
  memcpy(packetBuffer+p, payload, payloadLen);
  p += payloadLen;

  //end payload control character, checksum
  packetBuffer[p++] = enText;
  uint8_t crc = calcCRC(packetBuffer, p);
  packetBuffer[p++] = crc;

  //end of packet character, and null-terminate the array
  packetBuffer[p++] = enTransmission;
  packetBuffer[p] = '\0';

  //pass the message to the output function
  for (uint8_t i = 0; i < p; i++) 
  {
    if(commInterface->output_char_fnPtr)  //todo ASSERT if not valid?
    {
      commInterface->output_char_fnPtr(packetBuffer[i]);
    }
  }
}

void parsePacket(uint8_t inboundByte, struct eui_interface_state *commInterface)
{
  switch(commInterface->controlState)
  {
    case find_preamble:
      //random data we don't care about prior to preamble
      if(inboundByte == stHeader)
      {
        commInterface->controlState = exp_header;
        commInterface->processedCRC = 0; //clear out the CRC for this new message
      }
    break;

    case exp_header:
      //we've seen the preamble 0x01 control char. Nnext byte should be the header
      commInterface->inboundHeader = inboundByte;
      commInterface->controlState = exp_msgID;
    break;
    
    case exp_msgID:
      //Check if we've seen a shorter than maxLength msgID
      if(commInterface->processedID && inboundByte == stText )  
      {
        commInterface->controlState = exp_payloadlen;
        commInterface->inboundID[commInterface->processedID + 1] = '\0';  //terminate msgID string
      }
      else
      {
        //read the message ID character in
        commInterface->inboundID[commInterface->processedID] = inboundByte;
        commInterface->processedID++;

        //stop parsing and wait for STX, messageID can't be longer than preset maxlength
        if(commInterface->processedID >= MESSAGEID_SIZE)
        {
          commInterface->controlState = exp_stx;
          commInterface->inboundID[commInterface->processedID] = '\0';  //terminate the ID string
        }
      }
    break;
    
    case exp_stx:
      //wait for a STX to know the payloadlength is coming
      if(inboundByte == stText)
      {
        commInterface->controlState = exp_payloadlen;
      }
    break;
    
    case exp_payloadlen:
      //first byte should be payload length
      commInterface->inboundSize = inboundByte;
      commInterface->controlState = (commInterface->inboundSize == 0) ? exp_etx: exp_data;
    break;
    
    case exp_data:
      //we know the length of the payload, parse until we've eaten that many bytes
      commInterface->inboundData[commInterface->processedData] = inboundByte;
      commInterface->processedData++;

      if(commInterface->processedData >= commInterface->inboundSize)
      {
        //stop parsing and wait for STX, messageID can't be longer than that
        commInterface->controlState = exp_etx;
      }
      //we even eat ETX characters, because they are valid bytes in a payload...
    break;
    
    case exp_etx:
      //payload data saved in, the first character into this state really should be ETX
      //wait for a ETX to know checksum is coming
      if(inboundByte == enText)
      {
        commInterface->controlState = exp_crc;
      }
      else
      {
        //data trails payload
      }
    break;
    
    case exp_crc:
      //the ETX character has been seen, the next byte is the CRC
      commInterface->inboundCRC = inboundByte;
      commInterface->controlState = exp_eot;
    break;
    
    case exp_eot:
      //we've seen the checksum byte and are waiting for end of packet indication
      if(inboundByte == enTransmission)
      {
        //if the running xor matched recieved checksum, running would have changed to 0x00
        //since we add the new data to the running CRC outside this switch (afterwards)
        //0x00 gives validated payload.
        //TODO check if this is dangerous (seems like the same thing as a normal equality check, just later?)
        //TODO could be an issue if the CRC negates itself due to no new data or a false reset?
        if(commInterface->processedCRC == 0)
        {
          handlePacket(commInterface);
        }
        else
        {
          //invalid crc (TODO, add error reporting)
        }

        //done handling the message, clear out the state info (but leave the output pointer alone)
        memset( commInterface, 0, sizeof(struct eui_interface_state) - sizeof(CallBackwithUINT8) );
      }
      else
      {
        //unknown data after CRC and before EOT char
      }
    break;  
  }   //end switch

  commInterface->processedCRC ^= inboundByte;  //running crc
}