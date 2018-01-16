#include "electricui.h"

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

void handlePacket(struct eui_interface_state *validPacket)
{
  //we know the message is valid, use deconstructed header for convenience
  euiHeader_t header = *(euiHeader_t*)&validPacket->inboundHeader;

  //ensure all outut as part of the response goes through the same bus as the inbound message
  parserOutputFunc = validPacket->output_char_fnPtr;

  //create temp pointer to the message object we find
  euiMessage_t *msgObjPtr = findMessageObject( (char*)validPacket->inboundID, header.internal );  
  
  //check for search miss
  if(msgObjPtr == 0)
  {
    //todo handle error state properly
  }

  switch(msgObjPtr->type)
  {
    case TYPE_CALLBACK:
    {
      //create a function to call from the internally stored pointer
      CallBackType parsedCallbackHandler;
      parsedCallbackHandler = msgObjPtr->payload;
     
      //decide if this should be an assert on failure? Can't check at compile.
      if(parsedCallbackHandler) 
      {
        parserOutputFunc = validPacket->output_char_fnPtr;
        parsedCallbackHandler();
      }
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
    generatePacket(msgObjPtr->msgID, *(uint8_t*)&query_header, msgObjPtr->size, msgObjPtr->payload, validPacket->output_char_fnPtr);
  }
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
  generatePacket(msgObjPtr->msgID, *(uint8_t*)&header, msgObjPtr->size, msgObjPtr->payload, parserOutputFunc);
}

//application layer developer setup helpers
void setupDevMsg(euiMessage_t *msgArray, uint8_t numObjects)
{
  devObjectArray = msgArray;
  numDevObjects = numObjects;
}

void setupIdentifier()
{
  //hahahaha
  board_identifier = rand();
}

//application layer callbacks
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
  generatePacket("as", *(uint8_t*)&header, sizeof(data), &data, parserOutputFunc);
  sendTracked("lv", MSG_INTERNAL);
  sendTracked("pv", MSG_INTERNAL);
  sendTracked("bi", MSG_INTERNAL);
  sendTracked("si", MSG_INTERNAL);
  generatePacket("ae", *(uint8_t*)&header, sizeof(data), &data, parserOutputFunc);
}

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
  generatePacket("dms", *(uint8_t*)&dmHeader, sizeof(numMessages), &numMessages, parserOutputFunc);

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
      generatePacket("dml", *(uint8_t*)&dmHeader, msgBufferPos, &msgBuffer, parserOutputFunc);
      
      //cleanup
      memset(msgBuffer, 0, sizeof(msgBuffer));
      msgBufferPos = 0;
      msgIDPacked = 0;
    }
  }

  //tell the UI we've finished sending msg id strings
  generatePacket("dme", *(uint8_t*)&dmHeader, sizeof(numMessages), &numMessages, parserOutputFunc);
}
