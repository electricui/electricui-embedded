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
  genHeader.reqACK = reqack;
  genHeader.reserved = reservedbit;

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

euiMessage_t * findMessageObject(const char * msg_id, uint8_t isInternal)
{
  euiMessage_t *foundMsgPtr;

  if(isInternal == MSG_INTERNAL)
  {
    //search the internal array for matching messageID
    for(int i = 0; i < ARR_ELEM(internal_msg_store); i++)
    {
      if( strcmp( msg_id, internal_msg_store[i].msgID ) == 0 )
      {
        foundMsgPtr = &internal_msg_store[i];
      }
    }
    //return 0;
  }
  else if (isInternal == MSG_DEV)
  {
    //search developer space array for matching messageID
    for(int i = 0; i < numDevObjects; i++)
    {
      if( strcmp( msg_id, devObjectArray[i].msgID ) == 0 )
      {
        foundMsgPtr = &devObjectArray[i];
      }
    }
    //return 0;
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
    parserOutputFunc(packetBuffer[i]);
  }
}

void parsePacket(uint8_t inboundByte, struct eui_parser_state *commInterface)
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
     
      //decide if this should be an assert on failure? Can't check at compile.
      if( parsedCallbackHandler ) 
      {
        parsedCallbackHandler();
      }
    break;

    default:
      if(header.customType)
      {
        //TODO add custom handling or callbacks for custom types if needed?
      }

      //copy payload data into the object blindly providing we actually have some
      if(validPacket->inboundSize != 0)
      {
        memcpy(msgObjPtr->payload, validPacket->inboundData, validPacket->inboundSize);
      }
    break;
  }

  //a ACK was requested for this message
  if(header.reqACK)
  {
    euiHeader_t query_header =  { .internal = header.internal, 
                                  .customType = (msgObjPtr->type >= TYPE_CUSTOM_MARKER) ? MSG_TYPE_CUSTOM : MSG_TYPE_TYP, 
                                  .reqACK = MSG_ACK_NOTREQ, 
                                  .reserved = MSG_RES_L, 
                                  .type = msgObjPtr->type 
                                };

    //respond to the ack with internal value of the requested messageID as confirmation
    generatePacket(msgObjPtr->msgID, *(uint8_t*)&query_header, msgObjPtr->size, msgObjPtr->payload);
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

void setupIdentifier()
{
  //hahahaha
  boardidentifier = rand();
}

//application layer callbacks
void announceDevMsg()
{
  const uint8_t numMessages = numDevObjects;

  //generate a generic header for these messages
  euiHeader_t dmHeader = { .internal = MSG_INTERNAL, 
                            .customType = MSG_TYPE_TYP, 
                            .reqACK = MSG_ACK_NOTREQ, 
                            .reserved = MSG_RES_L, 
                            .type = TYPE_UINT8 
                          };

  //tell the UI we are starting the index handshake process
  generatePacket("dms", *(uint8_t*)&dmHeader, sizeof(numMessages), &numMessages);

  //fill a buffer which contains the developer message ID's
  uint8_t msgBuffer[ (MESSAGEID_SIZE+1)*MESSAGES_PK_DISCOVERY ];
  uint8_t msgBufferPos = 0; //position in buffer
  uint8_t msgIDlen = 0;     //length of a single msgID string
  uint8_t msgIDPacked = 0;  //count messages packed into buffer

  for(int i = 0; i <= numMessages; i++)
  {
    //copy messageID into the buffer, use null termination characters as delimiter
    msgIDlen = strlen(devObjectArray[i].msgID) + 1; //+1 to account for null character
    memcpy(msgBuffer+msgBufferPos, devObjectArray[i].msgID, msgIDlen);
    msgBufferPos += msgIDlen;
    msgIDPacked++;  

    //send messages and clear buffer to break list into shorter messagaes
    if(msgIDPacked >= MESSAGES_PK_DISCOVERY || i >= numMessages)
    {
      generatePacket("dml", *(uint8_t*)&dmHeader, msgBufferPos, &msgBuffer);
      
      //cleanup
      memset(msgBuffer, 0, sizeof(msgBuffer));
      msgBufferPos = 0;
      msgIDPacked = 0;
    }
  }

  //tell the UI we've finished sending msg id strings
  generatePacket("dme", *(uint8_t*)&dmHeader, sizeof(numMessages), &numMessages);
}

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
  //repond to search request with board info
  //this information helps populate the connection UI page
  euiHeader_t header =  { .internal = MSG_INTERNAL, 
                          .customType = MSG_TYPE_TYP, 
                          .reqACK = MSG_ACK_NOTREQ, 
                          .reserved = MSG_RES_L, 
                          .type = TYPE_UINT8 
                        };
  uint8_t data = 0;

  //todo remove mock start and finish strings
  generatePacket("hi", *(uint8_t*)&header, sizeof(data), &data);
  sendTracked("lv", MSG_INTERNAL);
  sendTracked("pv", MSG_INTERNAL);
  sendTracked("id", MSG_INTERNAL);
  generatePacket("bye", *(uint8_t*)&header, sizeof(data), &data);
}