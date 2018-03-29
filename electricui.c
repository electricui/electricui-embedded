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

void parse_packet(uint8_t inbound_byte, struct eui_interface *active_interface)
{
  uint8_t parsing_progress = decode_packet(inbound_byte, active_interface);

  switch(parsing_progress)
  {
    case packet_valid:
      handle_packet(active_interface);

      //done handling the message, clear out the state info (but leave the output pointer alone)
      parserOutputFunc = active_interface->output_char_fnPtr;
      memset( active_interface, 0, sizeof(active_interface) );
      active_interface->output_char_fnPtr = parserOutputFunc;    
    break;

    case packet_error_generic:
    case packet_error_crc:
      report_error(err_parser_generic);

      parserOutputFunc = active_interface->output_char_fnPtr;
      memset( active_interface, 0, sizeof(active_interface) );
      active_interface->output_char_fnPtr = parserOutputFunc;    
    break;
  }
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
  if(msgObjPtr)
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

      case TYPE_OFFSET_METADATA:
      {
        report_error(err_todo_functionality);
      }
      break;

      default:
        //Ensure some data was recieved from the inbound packet
        if(valid_packet->state.data_bytes_in)
        {
          //work out the correct length of the write (clamp length to internal variable size)
          uint8_t bytes_to_write = (valid_packet->state.data_bytes_in <= msgObjPtr->size) ? valid_packet->state.data_bytes_in : msgObjPtr->size;

          //Ensure the data won't exceed its bounds if invalid offsets are provided
          if(valid_packet->inboundOffset + bytes_to_write <= msgObjPtr->size)
          {
            //copy payload data into (memory + offset from address) 'blindly'
            memcpy(msgObjPtr->payload + valid_packet->inboundOffset, valid_packet->inboundData, bytes_to_write);
          }
          else
          {
            report_error(err_invalid_offset);
          }
        }
      break;
    }

    //respond as requested by the inbound packet
    if(header.query)
    {
      if(!header.acknum)
      {
        //send ack response
        euiHeader_t detail_header;
        detail_header.internal   = header.internal;
        detail_header.query      = MSG_NACK;
        detail_header.type       = msgObjPtr->type;
        detail_header.id_len     = strlen(msgObjPtr->msgID);
        detail_header.acknum     = header.acknum;
        detail_header.offset     = header.offset;
        detail_header.data_len   = 0;

        encode_packet(parserOutputFunc, &detail_header, msgObjPtr->msgID, valid_packet->inboundOffset, msgObjPtr->payload); 
      }
      else
      {
        //respond to query
        euiPacketSettings_t res_header =  { .internal = header.internal, 
                                            .ack      = MSG_NACK, 
                                            .query    = MSG_NQUERY, 
                                            .type     = msgObjPtr->type,
                                          };

        send_tracked(msgObjPtr, &res_header);      
      }
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
send_tracked(euiMessage_t *msgObjPtr, euiPacketSettings_t *settings)
{
  //decide if data will fit in a normal message, or requires multi-packet output
  if(msgObjPtr->size <= PAYLOAD_SIZE_MAX)
  {
    encode_packet_simple(parserOutputFunc, settings, msgObjPtr->msgID, msgObjPtr->size, msgObjPtr->payload);
  }
  else
  {
    send_tracked_range(msgObjPtr, settings, 0, msgObjPtr->size);
  }
}

void send_tracked_range(euiMessage_t *msgObjPtr, euiPacketSettings_t *settings, uint16_t base_addr, uint16_t end_addr)
{
    uint16_t data_range[] = { base_addr, end_addr }; //start, end offsets for data being sent

    euiHeader_t detail_header;

    detail_header.internal   = settings->internal;
    detail_header.query      = settings->query;
    detail_header.id_len     = strlen(msgObjPtr->msgID);
    detail_header.acknum     = 0;
    detail_header.offset     = 0;

    //generate metadata message with address range
    detail_header.data_len = sizeof(data_range);
    detail_header.type     = TYPE_OFFSET_METADATA;
    encode_packet(parserOutputFunc, &detail_header, msgObjPtr->msgID, 0x00, &data_range);

    //send the offset packets
    detail_header.offset = 1;
    detail_header.type   = msgObjPtr->type;

    for(; data_range[1] > data_range[0]; )
    {
      detail_header.data_len = (data_range[1] > PAYLOAD_SIZE_MAX) ? PAYLOAD_SIZE_MAX: data_range[1];
      data_range[1] -= detail_header.data_len;  //the current position through the buffer in bytes is also the end offset

      encode_packet(parserOutputFunc, &detail_header, msgObjPtr->msgID, data_range[1], msgObjPtr->payload);
    }
}

void
send_message(const char * msg_id, struct eui_interface *active_interface)
{
  parserOutputFunc = active_interface->output_char_fnPtr;

  temp_header.internal  = MSG_DEV;
  temp_header.query     = MSG_STANDARD_PACKET;

  send_tracked( find_message_object( msg_id, MSG_DEV ), &temp_header);
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
  temp_header.internal  = MSG_INTERNAL;
  temp_header.query     = MSG_STANDARD_PACKET;

  send_tracked(find_message_object("lv", MSG_INTERNAL), &temp_header);
  send_tracked(find_message_object("bi", MSG_INTERNAL), &temp_header);
  send_tracked(find_message_object("si", MSG_INTERNAL), &temp_header);
}

void
announce_dev_msg(void)
{
  temp_header.internal  = MSG_INTERNAL;
  temp_header.query     = MSG_STANDARD_PACKET;
  temp_header.type      = TYPE_UINT8;

  //tell the UI we are starting the index handshake process
  encode_packet_simple(parserOutputFunc, &temp_header, "dms", sizeof(numDevObjects), &numDevObjects);

  //fill a buffer which contains the developer message ID's
  uint8_t msgBuffer[ (MESSAGEID_SIZE+1)*(PAYLOAD_SIZE_MAX / PACKET_BASE_SIZE) ];
  uint8_t msgBufferPos = 0; //position in buffer
  uint8_t msgIDlen = 0;     //length of a single msgID string
  uint8_t msgIDPacked = 0;  //count messages packed into buffer

  temp_header.type = TYPE_CHAR;

  for(int i = 0; i <= numDevObjects; i++)
  {
    //copy messageID into the buffer, use null termination characters as delimiter
    msgIDlen = strlen(devObjectArray[i].msgID) + 1; //+1 to account for null character
    memcpy(msgBuffer+msgBufferPos, devObjectArray[i].msgID, msgIDlen);
    msgBufferPos += msgIDlen;
    msgIDPacked++;  

    //send messages and clear buffer to break list into shorter messagaes
    if(msgIDPacked >= (PAYLOAD_SIZE_MAX / PACKET_BASE_SIZE) || i >= numDevObjects)
    {
      encode_packet_simple(parserOutputFunc, &temp_header, "dml", msgBufferPos, &msgBuffer);

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
  temp_header.internal  = MSG_DEV;
  temp_header.query     = MSG_STANDARD_PACKET;

  for(int i = 0; i < numDevObjects; i++)
  {
    send_tracked( devObjectArray + i, &temp_header);
  }
}

void
report_error(uint8_t error)
{
  last_error = error;

  temp_header.internal  = MSG_INTERNAL;
  temp_header.query     = MSG_STANDARD_PACKET;

  send_tracked(find_message_object("er", MSG_INTERNAL), &temp_header);

}