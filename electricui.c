#include "electricui.h"

euiMessage_t * 
find_message_object(const char * msg_id, uint8_t is_internal)
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

void
handle_packet(struct eui_interface *valid_packet)
{
  //we know the message is 'valid', use deconstructed header for convenience
  euiHeader_t header = *(euiHeader_t*)&valid_packet->inboundHeader;

  //ensure response outputs use the same bus as the inbound message
  parserOutputFunc = valid_packet->output_char_fnPtr;

  //pointer to the message object we find
  euiMessage_t *msgObjPtr = find_message_object((char*)valid_packet->inboundID, header.internal);  
  
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
        if(header.type >= TYPE_CUSTOM_MARKER)
        {
          //TODO add handling or callbacks for not-default types if needed?
        }

        //copy payload data into the object blindly providing we actually have data
        if(valid_packet->state.data_bytes_in != 0)
        {
          uint8_t bytes_to_write = (valid_packet->state.data_bytes_in <= msgObjPtr->size) ? valid_packet->state.data_bytes_in : msgObjPtr->size;
          memcpy(msgObjPtr->payload, valid_packet->inboundData, bytes_to_write);
        }
      break;
    }

    //form responses if required
    if(header.ack || header.query)
    {
      euiPacketSettings_t res_header =  { .internal = header.internal, 
                                          .ack      = MSG_NACK, 
                                          .query    = MSG_NQUERY, 
                                          .type     = msgObjPtr->type 
                                        };
      //queries send back the current variable contents, otherwise we don't need any data
      uint8_t res_size = (header.query) ? msgObjPtr->size : 0;

      //respond to the ack with internal value of the requested messageID as confirmation
      form_offset_packet_simple(parserOutputFunc, &res_header, msgObjPtr->msgID, 0x00, res_size, msgObjPtr->payload);
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

void
send_tracked(const char * msg_id, uint8_t is_internal)
{
  euiMessage_t *msgObjPtr = find_message_object( msg_id, is_internal );  

  euiPacketSettings_t temp_header =  {.internal = is_internal, 
                                      .ack      = MSG_NACK, 
                                      .query    = MSG_STANDARD_PACKET, 
                                      .type     = msgObjPtr->type 
                                      };

  form_offset_packet_simple(parserOutputFunc, &temp_header, msgObjPtr->msgID, 0x00, msgObjPtr->size, msgObjPtr->payload);
}

void
send_message(const char * msg_id, struct eui_interface *active_interface)
{
  parserOutputFunc = active_interface->output_char_fnPtr;
  send_tracked(msg_id, MSG_DEV);
}

//application layer developer setup helpers
void
setup_dev_msg(euiMessage_t *msgArray, uint8_t numObjects)
{
  devObjectArray = msgArray;
  numDevObjects = numObjects;
}

void
setup_identifier(void)
{
  //hahahaha
  board_identifier = rand();
}

//application layer callbacks
void
announce_board(void)
{
  //repond to search request with board info
  send_tracked("lv", MSG_INTERNAL);
  send_tracked("bi", MSG_INTERNAL);
  send_tracked("si", MSG_INTERNAL);
}

void
announce_dev_msg(void)
{
  const uint8_t numMessages = numDevObjects;

  //create a generic header for these messages
  euiPacketSettings_t dm_header =  { .internal  = MSG_INTERNAL, 
                                     .ack       = MSG_NACK, 
                                     .query     = MSG_STANDARD_PACKET, 
                                     .type      = TYPE_UINT8 
                                    };

  //tell the UI we are starting the index handshake process
  form_offset_packet_simple(parserOutputFunc, &dm_header, "dms", 0x00, sizeof(numMessages), &numMessages);

  //fill a buffer which contains the developer message ID's
  uint8_t msgBuffer[ (MESSAGEID_SIZE+1)*(PAYLOAD_SIZE_MAX / PACKET_BASE_SIZE) ];
  uint8_t msgBufferPos = 0; //position in buffer
  uint8_t msgIDlen = 0;     //length of a single msgID string
  uint8_t msgIDPacked = 0;  //count messages packed into buffer

  dm_header.type = TYPE_CHAR;

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
      form_offset_packet_simple(parserOutputFunc, &dm_header, "dml", 0x00, msgBufferPos, &msgBuffer);

      //cleanup
      memset(msgBuffer, 0, sizeof(msgBuffer));
      msgBufferPos = 0;
      msgIDPacked = 0;
    }
  }
}

void
announce_dev_vars(void)
{
  euiPacketSettings_t dv_header =  { .internal  = MSG_DEV, 
                                      .ack      = MSG_NACK, 
                                      .query    = MSG_STANDARD_PACKET, 
                                      .type     = TYPE_BYTE 
                                    };

  for(int i = 0; i <= numDevObjects; i++)
  {
    //reuse the header, but use the appropriate type for each var
    dv_header.type = devObjectArray[i].type;

    form_offset_packet_simple(parserOutputFunc, 
                              &dv_header, 
                              devObjectArray[i].msgID, 
                              0x00, 
                              devObjectArray[i].size, 
                              devObjectArray[i].payload
                            );
  }
}

void
report_error(uint8_t error)
{
  last_error = error;
  send_tracked("er", MSG_INTERNAL);
}