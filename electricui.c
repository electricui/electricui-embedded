#include "electricui.h"

euiMessage_t * findMessageObject(const char * msg_id, uint8_t is_internal)
{
  euiMessage_t *foundMsgPtr = 0;

  if(is_internal == MSG_INTERNAL)
  {
    //search the internal array for matching messageID
    for(int i = 0; i < ARR_ELEM(internal_msg_store); i++)
    {
      if( strcmp( msg_id, internal_msg_store[i].msgID ) == 0 )
      {
        foundMsgPtr = &internal_msg_store[i];
      }
    }
  }
  else if (is_internal == MSG_DEV)
  {
    //search developer space array for matching messageID
    for(int i = 0; i < numDevObjects; i++)
    {
      if( strcmp( msg_id, devObjectArray[i].msgID ) == 0 )
      {
        foundMsgPtr = &devObjectArray[i];
      }
    }
  }

  return foundMsgPtr;
}

void handlePacket(struct eui_interface_state *valid_packet)
{
  //we know the message is 'valid', use deconstructed header for convenience
  euiHeader_t header = *(euiHeader_t*)&valid_packet->inboundHeader;

  //ensure response outputs use the same bus as the inbound message
  parserOutputFunc = valid_packet->output_char_fnPtr;

  //pointer to the message object we find
  euiMessage_t *msgObjPtr = findMessageObject( (char*)valid_packet->inboundID, header.internal );  
  
  //Check that the searched ID was found
  if(msgObjPtr != 0)
  {
    switch(msgObjPtr->type)
    {
      case TYPE_CALLBACK:
      {
        //create a function to call from the internally stored pointer
        CallBackType parsedCallbackHandler;
        parsedCallbackHandler = msgObjPtr->payload;
       
        if(parsedCallbackHandler) 
        {
          parsedCallbackHandler();
        }
        else
        {
          report_error(err_missing_callback);
        }
      }
      break;

      default:
        if(header.customType)
        {
          //TODO add custom handling or callbacks for custom types if needed?
        }

        //copy payload data into the object blindly providing we actually have data
        if(valid_packet->inboundSize != 0)
        {
          uint8_t bytes_to_write = (valid_packet->inboundSize <= msgObjPtr->size) ? valid_packet->inboundSize : msgObjPtr->size;
          memcpy(msgObjPtr->payload, valid_packet->inboundData, bytes_to_write);
        }
      break;
    }

    //ACK was requested for this message
    if(header.reqACK)
    {
      euiHeader_t query_header =  { .internal = header.internal, 
                                    .customType = (msgObjPtr->type >= TYPE_CUSTOM_MARKER) ? MSG_TYPE_CUSTOM : MSG_TYPE_TYP, 
                                    .reqACK = MSG_ACK_NOTREQ, 
                                    .reserved = MSG_RES_L, 
                                    .type = msgObjPtr->type 
                                  };

      //respond to the ack with internal value of the requested messageID as confirmation
      generatePacket(msgObjPtr->msgID, *(uint8_t*)&query_header, msgObjPtr->size, msgObjPtr->payload, parserOutputFunc);
    }

  }
  else  //search miss
  {
    if(header.internal)
    {
      report_error(err_invalid_internal);
    } 
    else 
    {    
      report_error(err_invalid_developer);
    }
  }
}

void sendTracked(const char * msg_id, uint8_t is_internal)
{
  euiMessage_t *msgObjPtr = findMessageObject( msg_id, is_internal );  

  euiHeader_t header =  { .internal = is_internal, 
                          .customType = MSG_TYPE_TYP, 
                          .reqACK = MSG_ACK_NOTREQ, 
                          .reserved = MSG_RES_L, 
                          .type = msgObjPtr->type 
                        };

  generatePacket(msgObjPtr->msgID, *(uint8_t*)&header, msgObjPtr->size, msgObjPtr->payload, parserOutputFunc);
}

void sendMessage(const char * msg_id, struct eui_interface_state *active_interface)
{
  parserOutputFunc = active_interface->output_char_fnPtr;
  sendTracked(msg_id, MSG_DEV);
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
  sendTracked("lv", MSG_INTERNAL);
  sendTracked("bi", MSG_INTERNAL);
  sendTracked("si", MSG_INTERNAL);
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
  uint8_t msgBuffer[ (MESSAGEID_SIZE+1)*(PAYLOAD_SIZE_MAX / PACKET_BASE_SIZE) ];
  uint8_t msgBufferPos = 0; //position in buffer
  uint8_t msgIDlen = 0;     //length of a single msgID string
  uint8_t msgIDPacked = 0;  //count messages packed into buffer

  dmHeader.type = TYPE_CHAR;

  for(int i = 0; i <= numMessages; i++)
  {
    //copy messageID into the buffer, use null termination characters as delimiter
    msgIDlen = strlen(devObjectArray[i].msgID) + 1; //+1 to account for null character
    memcpy(msgBuffer+msgBufferPos, devObjectArray[i].msgID, msgIDlen);
    msgBufferPos += msgIDlen;
    msgIDPacked++;  

    //send messages and clear buffer to break list into shorter messagaes
    if(msgIDPacked >= (PAYLOAD_SIZE_MAX / PACKET_BASE_SIZE) || i >= numMessages)
    {
      generatePacket("dml", *(uint8_t*)&dmHeader, msgBufferPos, &msgBuffer, parserOutputFunc);
      
      //cleanup
      memset(msgBuffer, 0, sizeof(msgBuffer));
      msgBufferPos = 0;
      msgIDPacked = 0;
    }
  }
}

void announce_dev_vars(void)
{
  euiHeader_t dvHeader = { .internal = MSG_DEV, 
                            .customType = MSG_TYPE_TYP, 
                            .reqACK = MSG_ACK_NOTREQ, 
                            .reserved = MSG_RES_L, 
                            .type = TYPE_BYTE 
                          };

  for(int i = 0; i <= numDevObjects; i++)
  {
    dvHeader.type = devObjectArray[i].type;

    generatePacket( devObjectArray[i].msgID, 
                    *(uint8_t*)&dvHeader, 
                    devObjectArray[i].size, 
                    devObjectArray[i].payload, 
                    parserOutputFunc
                  );
  }
}

void report_error(uint8_t error)
{
  last_error = error;
  sendTracked("er", MSG_INTERNAL);
}