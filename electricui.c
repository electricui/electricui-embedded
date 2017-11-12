#include "electricui.h"

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

  genHeader.internal = internalmsg;
  genHeader.customType = customtype;
  genHeader.reqACK = reqack;
  genHeader.reserved = reservedbit;
  genHeader.type = payloadtype;

  return *(uint8_t*)&genHeader;
}

euiMessage_t * findMessageObject(const char * msg_id, uint8_t isInternal)
{
  static euiMessage_t *foundMsgPtr;

  if(isInternal)
  {
    //search the internal array for matching messageID
    for(int i = 0; i < ARR_ELEM(int_msg_store); i++)
    {
      if( strcmp( msg_id, int_msg_store[i].msgID ) == 0 )
      {
        foundMsgPtr = &int_msg_store[i];
      }
    }
    return 0;
  }
  else
  {
    //search developer space array for matching messageID
    for(int i = 0; i < numDevObjects; i++)
    {
      if( strcmp( msg_id, devObjectArray[i].msgID ) == 0 )
      {
        foundMsgPtr = &devObjectArray[i];
      }
    }
    return 0;
  }

  return foundMsgPtr;
}

void generatePacket(const char * msg_id, uint8_t header, uint8_t payloadLen, void* payload)
{
  uint8_t packetBuffer[PACKET_BASE_SIZE + PAYLOAD_SIZE_MAX];
  uint8_t p = 0;

  //preamble and header
  packetBuffer[p++] = stHeader;
  packetBuffer[p++] = header;

  //copy the message ID in
  memcpy(packetBuffer+p, msg_id, MESSAGEID_SIZE);
  p += MESSAGEID_SIZE;

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
    parserOutputFunc(packetBuffer[i]);
  }
}

void parsePacket(uint8_t inboundByte, struct eui_parser_state *commInterface)
{
  switch(commInterface->controlState)
  {
    case s_invalidData:
      //random data we don't care about prior to preamble
      if(inboundByte == stHeader)
      {
        commInterface->controlState = s_preamble;
        commInterface->processedCRC = 0; //clear out the CRC for this new message
      }
    break;

    case s_preamble:
      //we've seen the preamble 0x01 control char. Nnext byte should be the header
      commInterface->inboundHeader = inboundByte;
      commInterface->controlState = s_header;
    break;
    
    case s_header:
      //read the message ID in
      commInterface->inboundID[commInterface->processedID] = inboundByte;
      commInterface->processedID++;

      if(commInterface->processedID >= MESSAGEID_SIZE)
      {
        //stop parsing and wait for STX, messageID can't be longer than that
        commInterface->controlState = s_msg_id;
        commInterface->inboundID[commInterface->processedID] = '\0';  //terminate the ID string
      }

      if(inboundByte == stText && commInterface->processedID)  //need at least one byte before a STX
      {
        commInterface->controlState = s_datastart;
        commInterface->inboundID[commInterface->processedID] = '\0';  //terminate the ID string
      }
    break;
    
    case s_msg_id:
      //wait for a STX to know the payload is coming (this state shouldn't trigger)
      if(inboundByte == stText)
      {
        commInterface->controlState = s_datastart;
      }
      else
      {
        //data after msgID we aren't expecting.
      }
    break;
    
    case s_datastart:
      //first byte should be payload length
      commInterface->inboundSize = inboundByte;
      //commInterface->controlState = s_datalen;
      commInterface->controlState = (commInterface->inboundSize == 0) ? s_payload: s_datalen;
    break;
    
    case s_datalen:
      //we know the length of the payload, parse until we've eaten that many bytes
      commInterface->inboundData[commInterface->processedData] = inboundByte;
      commInterface->processedData++;

      if(commInterface->processedData >= commInterface->inboundSize)
      {
        //stop parsing and wait for STX, messageID can't be longer than that
        commInterface->controlState = s_payload;
      }
      //we even eat ETX characters, because they are valid bytes in a payload...
    break;
    
    case s_payload:
      //payload data saved in, the first character into this state really should be ETX
      //wait for a ETX to know checksum is coming
      if(inboundByte == enText)
      {
        commInterface->controlState = s_dataend;
      }
      else
      {
        //data trails payload
      }
    break;
    
    case s_dataend:
      //the ETX character has been seen, the next byte is the CRC
      commInterface->inboundCRC = inboundByte;
      commInterface->controlState = s_checksum;
    break;
    
    case s_checksum:
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

        memset( commInterface, 0, sizeof(struct eui_parser_state) );
      }
      else
      {
        //unknown data after CRC and before EOT char
      }
    break;  
  }   //end switch

  commInterface->processedCRC ^= inboundByte;  //running crc
}

void handlePacket(struct eui_parser_state *validPacket)
{
  //we know the message is valid, use deconstructed header for convenience
  euiHeader_t header = *(euiHeader_t*)&validPacket->inboundHeader;

  //create temp pointer to the message object we find
  euiMessage_t *msgObjPtr = findMessageObject( (char*)validPacket->inboundID, header.internal );  
  
  //check for search miss
  if(msgObjPtr == 0)
  {
    //todo handle error state properly
  }

  //TODO decide if we trust the packet or internal store that matches the ID?
  switch(header.type)
  {
    case TYPE_CALLBACK:
    //create a function to call from the internally stored pointer
    parsedCallbackHandler = msgObjPtr->payload;
   
    if(parsedCallbackHandler) //decide if this should be an assert? Can't check at compile.
    {
      parsedCallbackHandler();
    }

    break;

    case TYPE_QUERY:
    //parrot back the variable contents in the payload

    //we basically just ignore the payload, and the ack at the end of ingest should sort this out
    break;

    default:
    if(header.customType)
    {
      //TODO add support for custom types...
    }

    //any other normal message type, just copy payload data into the object blindly
    //todo add some checks here?
    memcpy(msgObjPtr->payload, validPacket->inboundData, validPacket->inboundSize);
    break;
  }

  //a ACK was requested for this message
  if(header.reqACK)
  {
    //send "ack" with the message ID as contents? 
    //or send the message ID with an ACK type? 
    //or send messageID with ack payload?
  }
}


//application layer developer setup helpers
void setupParser(CallBackDataType parserFuncPtr)
{
  parserOutputFunc = parserFuncPtr;
}

void setupDevMsg(euiMessage_t *msgArray, uint8_t numObjects)
{
  devObjectArray = msgArray;
  numDevObjects = numObjects;
}

//application layer callbacks
void announceDevMsg()
{
  //announce start
  //iterate through the dev message list
  //announce end
void sendTracked(const char * msg_id, uint8_t isInternal)
{
  //find the tracked variable in the array
  euiMessage_t *msgObjPtr = findMessageObject( msg_id, isInternal );  

  euiHeader_t header =  { .internal = isInternal, 
                          .customType = MSG_TYPE_TYP, 
                          .reqACK = MSG_ACK_NOTREQ, 
                          .reserved = MSG_RES_L, 
                          .type = msgObjPtr->type 
                        };

  //generate the message
  generatePacket(msgObjPtr->msgID, *(uint8_t*)&header, msgObjPtr->size, msgObjPtr->payload);
}

void announceBoard()
{
  //tell the other side who/what we are as part of discovery
}