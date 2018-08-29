#include <string.h>
#include "electricui.h"

//internal
uint8_t library_version[] = { VER_MAJOR, VER_MINOR, VER_PATCH };

euiMessage_t internal_msg_store[] = {
  EUI_RO_UINT8(EUI_INTERNAL_LIB_VER, library_version),
  EUI_RO_UINT16(EUI_INTERNAL_BOARD_ID, board_identifier),
  EUI_UINT8(EUI_INTERNAL_SESSION_ID, session_identifier),
  EUI_UINT8(EUI_INTERNAL_ERROR_ID, last_error),
  EUI_UINT8(EUI_INTERNAL_HEARTBEAT, heartbeat),

  EUI_FUNC(EUI_INTERNAL_AM_RO, announce_dev_msg_readonly),
  EUI_FUNC(EUI_INTERNAL_AM_RW, announce_dev_msg_writable),
  EUI_FUNC(EUI_INTERNAL_AV_RO, announce_dev_vars_readonly),
  EUI_FUNC(EUI_INTERNAL_AV_RW, announce_dev_vars_writable),

  EUI_FUNC(EUI_INTERNAL_SEARCH, announce_board),
};


euiMessage_t * 
find_message_object(const char * msg_id, uint8_t is_internal)
{
  euiMessage_t *foundMsgPtr = 0;

  if(is_internal == MSG_INTERNAL)
  {
    //search the internal array for matching messageID
    for(eui_var_count_t i = 0; i < ARR_ELEM(internal_msg_store); i++)
    {
      if( strcmp( msg_id, internal_msg_store[i].msgID ) == 0 )
      {
        foundMsgPtr = &internal_msg_store[i];
        i = ARR_ELEM(internal_msg_store);
      }
    }
  }
  else if (is_internal == MSG_DEV)
  {
    //search developer space array for matching messageID
    for(eui_var_count_t i = 0; i < numDevObjects; i++)
    {
      if( strcmp( msg_id, devObjectArray[i].msgID ) == 0 )
      {
        foundMsgPtr = &devObjectArray[i];
        i = numDevObjects;
      }
    }
  }

  return foundMsgPtr;
}

void parse_packet(uint8_t inbound_byte, eui_interface *active_interface)
{
  uint8_t parsing_progress = decode_packet(inbound_byte, active_interface);

  switch(parsing_progress)
  {
    case packet_valid:
      handle_packet(active_interface);

      //done handling the message, clear out the state info (but leave the output pointer alone)
      parserOutputFunc = active_interface->output_char_fnPtr;
      memset( active_interface, 0, sizeof(&active_interface) );
      active_interface->output_char_fnPtr = parserOutputFunc;    
    break;

    case packet_error_generic:
    case packet_error_crc:
      report_error(err_parser_generic);

      parserOutputFunc = active_interface->output_char_fnPtr;
      memset( active_interface, 0, sizeof(&active_interface) );
      active_interface->output_char_fnPtr = parserOutputFunc;    
    break;

    default:

    break;
  }
}

void
handle_packet(eui_interface *valid_packet)
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
    if(valid_packet->state.data_bytes_in)
    {
      //ignore data in callbacks or offset messages
      if(header.type == TYPE_OFFSET_METADATA || msgObjPtr->type >> 7)
      {
        report_error(err_todo_functionality);
      }
      else  //any other type
      {
        //work out the correct length of the write (clamp length to internal variable size)
        uint8_t bytes_to_write = (valid_packet->state.data_bytes_in <= msgObjPtr->size) ? valid_packet->state.data_bytes_in : msgObjPtr->size;

        //Ensure the data won't exceed its bounds if invalid offsets are provided
        if(valid_packet->inboundOffset + bytes_to_write <= msgObjPtr->size)
        {
          //copy payload data into (memory + offset from address) 'blindly'
          memcpy((char *)msgObjPtr->payload + valid_packet->inboundOffset, valid_packet->inboundData, bytes_to_write);
        }
        else
        {
          report_error(err_invalid_offset);
        }
      }
    }
    else  //no payload data
    {
      if((msgObjPtr->type & 0x0F) == TYPE_CALLBACK)
      {
        if( (header.response && header.acknum) || (!header.response && !header.acknum) )
        {
          //create a function to call from the internally stored pointer
          CallBackType parsedCallbackHandler;
          parsedCallbackHandler = msgObjPtr->payload;

          (parsedCallbackHandler) ? parsedCallbackHandler() : report_error(err_missing_callback);
        }
      }

      //todo: handle ack responses here in the future?
    }

    //inbound packet requested a response on ingest of this packet
    if(header.response)
    {
      if(header.acknum)
      {
        //respond with somewhat empty ack response packet
        euiHeader_t detail_header;
        detail_header.internal   = header.internal;
        detail_header.response   = MSG_NRESP;
        detail_header.type       = msgObjPtr->type;
        detail_header.id_len     = strlen(msgObjPtr->msgID);
        detail_header.acknum     = header.acknum;
        detail_header.offset     = header.offset;
        detail_header.data_len   = 0;

        encode_packet(parserOutputFunc, &detail_header, msgObjPtr->msgID, valid_packet->inboundOffset, msgObjPtr->payload); 
      }
      else
      {
        //respond with data to fufil query behaviour
        euiPacketSettings_t res_header =  { .internal = header.internal, 
                                            .response = MSG_NRESP, 
                                            .type     = msgObjPtr->type,
                                          };

        if(header.type != TYPE_OFFSET_METADATA)
        {
          send_tracked(msgObjPtr, &res_header);      
        } 
#ifndef EUI_CONF_OFFSETS_DISABLED
        else
        {
          uint16_t base_address = (uint16_t)valid_packet->inboundData[1] << 8 | valid_packet->inboundData[0];
          uint16_t end_address  = (uint16_t)valid_packet->inboundData[3] << 8 | valid_packet->inboundData[2];

          //try and send the range as requested
          send_tracked_range(msgObjPtr, &res_header, base_address, end_address);
        }
#endif
      }
    }

  }
  else  //search didn't return a pointer to the object
  {
    (header.internal) ? report_error(err_invalid_internal) : report_error(err_invalid_developer);
  }
}

void
send_tracked(euiMessage_t *msgObjPtr, euiPacketSettings_t *settings)
{
  //write the correct type into the settings based on the objects internal type, not whatever has been provided by the dev...
  settings->type = msgObjPtr->type;

  //decide if data will fit in a normal message, or requires multi-packet output
  if(msgObjPtr->size <= PAYLOAD_SIZE_MAX)
  {
    encode_packet_simple(parserOutputFunc, settings, msgObjPtr->msgID, msgObjPtr->size, msgObjPtr->payload);
  }
#ifndef EUI_CONF_OFFSETS_DISABLED
  else
  {
    send_tracked_range(msgObjPtr, settings, 0, msgObjPtr->size);
  }
#endif
}

void send_tracked_range(euiMessage_t *msgObjPtr, euiPacketSettings_t *settings, uint16_t base_addr, uint16_t end_addr)
{
#ifndef EUI_CONF_OFFSETS_DISABLED
    uint8_t type_size = 0;

    switch(msgObjPtr->type & 0x0F)
    {
      case TYPE_INT16:
      case TYPE_UINT16:
        type_size = 2;
      break;

      case TYPE_INT32:
      case TYPE_UINT32:
      case TYPE_FLOAT:
        type_size = 4;
      break;

      case TYPE_DOUBLE:
        type_size = 8;
      break;

      default:  
        //single byte types and customs
        type_size = 1;
      break;
    }

    //shift the offset up to align with the type size
    base_addr = ((base_addr + (type_size-1)) / type_size) * type_size;
    end_addr  = ((end_addr  + (type_size-1)) / type_size) * type_size;

    //requested start and end ranges are within the bounds of the managed variable
    if( end_addr > msgObjPtr->size || !end_addr)
    {
      end_addr = msgObjPtr->size;
    }

    if( base_addr >= end_addr)
    {
      base_addr = end_addr - type_size;
    }

    euiHeader_t detail_header;
    detail_header.internal   = settings->internal;
    detail_header.response   = settings->response;
    detail_header.id_len     = strlen(msgObjPtr->msgID);
    detail_header.acknum     = 0;
    detail_header.offset     = 0;

    //generate metadata message with address range
    detail_header.data_len = sizeof(base_addr) * 2; //base and end are sent
    detail_header.type     = TYPE_OFFSET_METADATA;
    uint16_t data_range[] = { base_addr, end_addr }; //start, end offsets for data being sent
    encode_packet(parserOutputFunc, &detail_header, msgObjPtr->msgID, 0x00, &data_range);

    //send the offset packets
    detail_header.offset = 1;
    detail_header.type   = msgObjPtr->type;

    while( end_addr > base_addr )
    {
      uint16_t bytes_remaining = end_addr - base_addr;
      detail_header.data_len = ( bytes_remaining > PAYLOAD_SIZE_MAX ) ? PAYLOAD_SIZE_MAX : bytes_remaining;
      
      end_addr -= detail_header.data_len;  //the current position through the buffer in bytes is also the end offset

      encode_packet(parserOutputFunc, &detail_header, msgObjPtr->msgID, end_addr, msgObjPtr->payload);
    }
#endif
}

void
send_message(const char * msg_id, eui_interface *active_interface)
{
  parserOutputFunc = active_interface->output_char_fnPtr;

  temp_header.internal  = MSG_DEV;
  temp_header.response  = MSG_NRESP;

  send_tracked( find_message_object( msg_id, MSG_DEV ), &temp_header);
}

//application layer developer setup helpers
void
setup_dev_msg(euiMessage_t *msgArray, eui_var_count_t numObjects)
{
  devObjectArray = msgArray;
  numDevObjects = numObjects;
}

void
setup_identifier(char * uuid, uint8_t bytes)
{
	if(uuid && bytes)
	{
		//generate a 'hashed' int16 of their UUID
		for(uint8_t i = 0; i < bytes; i++)
		{
		  crc16(uuid[i], &board_identifier);
		}
	}
	else
	{
		//a null identifier demonstrates an issue
		board_identifier = 0;
	}
}

//application layer callbacks
void
announce_board(void)
{
  //repond to search request with board info
  temp_header.internal  = MSG_INTERNAL;
  temp_header.response  = MSG_NRESP;

  send_tracked(find_message_object(EUI_INTERNAL_LIB_VER, MSG_INTERNAL), &temp_header);
  send_tracked(find_message_object(EUI_INTERNAL_BOARD_ID, MSG_INTERNAL), &temp_header);
  send_tracked(find_message_object(EUI_INTERNAL_SESSION_ID, MSG_INTERNAL), &temp_header);
}

void
announce_dev_msg_readonly(void)
{
  eui_var_count_t num_read_only  = 0;
  num_read_only = send_tracked_message_id_list(READ_ONLY_FLAG);

  temp_header.internal  = MSG_INTERNAL;
  temp_header.response  = MSG_NRESP;
  temp_header.type      = TYPE_UINT8;
  encode_packet_simple(parserOutputFunc, &temp_header, EUI_INTERNAL_AM_RO_END, sizeof(num_read_only), &num_read_only);
}

void
announce_dev_msg_writable(void)
{
  eui_var_count_t num_writable  = 0;
  num_writable = send_tracked_message_id_list(WRITABLE_FLAG);

  temp_header.internal  = MSG_INTERNAL;
  temp_header.response  = MSG_NRESP;
  temp_header.type      = TYPE_UINT8;
  encode_packet_simple(parserOutputFunc, &temp_header, EUI_INTERNAL_AM_RW_END, sizeof(num_writable), &num_writable);
}

void
announce_dev_vars_readonly(void)
{
  send_tracked_variables(READ_ONLY_FLAG);
}

void
announce_dev_vars_writable(void)
{
  send_tracked_variables(WRITABLE_FLAG);
}

eui_var_count_t
send_tracked_message_id_list(uint8_t read_only)
{
  eui_var_count_t variables_sent = 0;

  temp_header.internal  = MSG_INTERNAL;
  temp_header.response  = MSG_NRESP;
  temp_header.type      = TYPE_CUSTOM;

  uint8_t msgBuffer[ (MESSAGEID_SIZE+1)*4 ];
  uint8_t msgBufferPos  = 0;  //position in buffer
  uint8_t msgIDlen      = 0;  //length of a single msgID string
  uint8_t msgIDPacked   = 0;  //count messages packed into buffer

  for(eui_var_count_t i = 0; i < numDevObjects; i++)
  {
    // filter based on writable flag
    if( devObjectArray[i].type >> 7 == read_only )
    {
      //copy messageID into the buffer, use null termination characters as delimiter
      msgIDlen = strlen(devObjectArray[i].msgID) + 1; //+1 to account for null character
      memcpy(msgBuffer+msgBufferPos, devObjectArray[i].msgID, msgIDlen);
      msgBufferPos += msgIDlen;
      msgIDPacked++;

      variables_sent++;
    }
  
    //send messages and clear buffer
    if(msgBufferPos >= (sizeof(msgBuffer) - MESSAGEID_SIZE/2 ) || i >= numDevObjects-1)
    {
      const char * headerID = (read_only) ? EUI_INTERNAL_AM_RO_LIST : EUI_INTERNAL_AM_RW_LIST;
      encode_packet_simple(parserOutputFunc, &temp_header, headerID, msgBufferPos, &msgBuffer);

      //cleanup
      memset(msgBuffer, 0, sizeof(msgBuffer));
      msgBufferPos = 0;
      msgIDPacked = 0;
    }
  }

  return variables_sent;
}

eui_var_count_t
send_tracked_variables(uint8_t read_only)
{
  eui_var_count_t sent_variables = 0;
  temp_header.internal  = MSG_DEV;
  temp_header.response  = MSG_NRESP;

  for(eui_var_count_t i = 0; i < numDevObjects; i++)
  {
    //only send messages which have the specified read-only bit state
    if( devObjectArray[i].type >> 7 == read_only )
    {
      send_tracked( devObjectArray + i, &temp_header);
      sent_variables++;
    }
  }
  return sent_variables;
}

void
report_error(uint8_t error)
{
  last_error = error;

  temp_header.internal  = MSG_INTERNAL;
  temp_header.response  = MSG_NRESP;

  send_tracked(find_message_object(EUI_INTERNAL_ERROR_ID, MSG_INTERNAL), &temp_header);

}