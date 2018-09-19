#include <string.h>
#include "electricui.h"

//internal
uint8_t library_version[] = { VER_MAJOR, VER_MINOR, VER_PATCH };

euiMessage_t internal_msg_store[] = 
{
    EUI_UINT8_RO(EUI_INTERNAL_LIB_VER, library_version),
    EUI_UINT16_RO(EUI_INTERNAL_BOARD_ID, board_identifier),
    EUI_UINT8(EUI_INTERNAL_SESSION_ID, session_identifier),
    EUI_UINT8(EUI_INTERNAL_HEARTBEAT, heartbeat),
    EUI_UINT8(EUI_DEFAULT_INTERFACE, default_interface),

    EUI_FUNC(EUI_INTERNAL_AM_RO, announce_dev_msg_readonly),
    EUI_FUNC(EUI_INTERNAL_AM_RW, announce_dev_msg_writable),
    EUI_FUNC(EUI_INTERNAL_AV_RO, announce_dev_vars_readonly),
    EUI_FUNC(EUI_INTERNAL_AV_RW, announce_dev_vars_writable),

    EUI_FUNC(EUI_INTERNAL_SEARCH, announce_board),

#ifndef EUI_CONF_ERROR_DISABLE
    EUI_UINT8(EUI_INTERNAL_ERROR_ID, last_error),
#endif
};


euiMessage_t * 
find_message_object(const char * msg_id, uint8_t is_internal)
{
    euiMessage_t *foundMsgPtr = 0;

    if(is_internal == MSG_INTERNAL)
    {
        //search the internal array for matching messageID
        for(euiVariableCount_t i = 0; i < ARR_ELEM(internal_msg_store); i++)
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
        for(euiVariableCount_t i = 0; i < numDevObjects; i++)
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

euiCallbackUint8_t *
auto_output(void)
{
    //work out which interface to output data on, and pass callback to the relevant function

    //todo intelligently select an interface

    return &interfaceArray[default_interface].output_func;
}

void
parse_packet( uint8_t inbound_byte, euiInterface_t *p_link )
{
    uint8_t parse_status = decode_packet(inbound_byte, &p_link->parser);

    if( parse_status == packet_valid )
    {
        //Use deconstructed header for convenience, find pointer to stored object
        euiHeader_t  header     = *(euiHeader_t*)&p_link->parser.header;
        euiMessage_t *p_msglocal = find_message_object( (char*)p_link->parser.inboundID, 
                                                        header.internal );  
        
        if( p_msglocal )
        {
            if( p_link->parser.state.data_bytes_in )
            {
                handle_packet_data( p_link, &header, p_msglocal );
            }
            else
            {
                handle_packet_empty( &header, p_msglocal );
            }

            handle_packet_response( p_link, &header, p_msglocal );
            handle_packet_callback( p_msglocal );
        }
        else  //search didn't return a pointer to the object
        {
            report_error( err_unknown_id );
        }

        cb_dev_interface_complete( p_link );

        memset( &p_link->parser, 0, sizeof(eui_parser_t) );        
    }
    else
    {
        report_error(err_parser_generic);
        memset( &p_link->parser, 0, sizeof(eui_parser_t) );        
    }
}

void
handle_packet_data( euiInterface_t  *valid_packet,
                    euiHeader_t     *header,
                    euiMessage_t    *msgObjPtr )
{
    //ignore data in callbacks or offset messages, ignore read only data
    if( header->type == TYPE_OFFSET_METADATA 
        || header->type == TYPE_CALLBACK 
        || msgObjPtr->type >> 7 )
    {
        report_error( err_todo_functionality );
    }
    else
    {
        //work out the correct length of the write (clamp length to actual size)
        // TODO work out if we want larger per-packet writes?
        uint8_t bytes_to_write = 0;
        
        if( valid_packet->parser.state.data_bytes_in <= msgObjPtr->size )
        {
            bytes_to_write = valid_packet->parser.state.data_bytes_in;
        }
        else
        {
            bytes_to_write = msgObjPtr->size;
        }

        //Ensure data won't exceed bounds with invalid offsets
        if( valid_packet->parser.inboundOffset + bytes_to_write <= msgObjPtr->size )
        {
            memcpy( (char *)msgObjPtr->payload + valid_packet->parser.inboundOffset,
                    valid_packet->parser.data_in,
                    bytes_to_write );
        }
        else
        {
            report_error( err_invalid_offset );
        }
    }
}

void
handle_packet_empty( euiHeader_t     *header,
                     euiMessage_t    *msgObjPtr )
{
    if((msgObjPtr->type & 0x0F) == TYPE_CALLBACK)
    {
        if( (header->response && header->acknum) 
            || (!header->response && !header->acknum) )
        {
            //create a function to call from the internal stored pointer
            euiCallback_t cb_packet_h;
            cb_packet_h = msgObjPtr->payload;

            (cb_packet_h) ? cb_packet_h() : report_error(err_missing_callback);
        }
    }
    //todo: handle ack responses here in the future?
}

// Check the inbound packet's response requirements and output as required
void
handle_packet_response( euiInterface_t *packet_in,
                        euiHeader_t *header,
                        euiMessage_t *msgObjPtr )
{
    if(header->response)
    {
        if(header->acknum)
        {
            euiHeader_t tmp_header;
            tmp_header.internal   = header->internal;
            tmp_header.response   = MSG_NRESP;
            tmp_header.type       = msgObjPtr->type;
            tmp_header.id_len     = strlen(msgObjPtr->msgID);
            tmp_header.acknum     = header->acknum;
            tmp_header.offset     = header->offset;
            tmp_header.data_len   = 0;

            encode_packet(  packet_in->output_func, 
                            &tmp_header, 
                            msgObjPtr->msgID, 
                            packet_in->parser.inboundOffset, 
                            msgObjPtr->payload ); 
        }
        else
        {
            //respond with data to fufil query behaviour
            euiPacketSettings_t res_header =  { .internal = header->internal, 
                                                .response = MSG_NRESP, 
                                                .type     = msgObjPtr->type };

            if(header->type == TYPE_OFFSET_METADATA)
            {
#ifndef EUI_CONF_OFFSETS_DISABLED
                uint16_t base_address = 0;
                uint16_t end_address  = 0; 
                
                base_address = (uint16_t)packet_in->parser.data_in[1] << 8 | packet_in->parser.data_in[0];
                end_address  = (uint16_t)packet_in->parser.data_in[3] << 8 | packet_in->parser.data_in[2];

                send_tracked_range( packet_in->output_func,
                                    msgObjPtr,
                                    &res_header,
                                    base_address,
                                    end_address );
#endif
            } 
            else
            {
                send_tracked(packet_in->output_func, msgObjPtr, &res_header);      
            }
        }
    }
}

void
handle_packet_callback( euiMessage_t *msgObjPtr )
{
    // Call the callback assigned to this message ID
    euiCallback_t dev_var_cb;
    dev_var_cb = msgObjPtr->callback;
    if(dev_var_cb)
    {
        dev_var_cb();
    }
}

// Optional developer callback when a message has been parsed.
// Allows for developer-side direct access to the packet data before erase.
void
cb_dev_interface_complete( euiInterface_t *p_link )
{
    euiCallback_t dev_cb;
    dev_cb = p_link->interface_cb;

    if(dev_cb)
    {
        dev_cb();
    }
}

void
send_tracked(   euiCallbackUint8_t  output_function, 
                euiMessage_t        *msgObjPtr, 
                euiPacketSettings_t *settings )
{
    settings->type = msgObjPtr->type;

    //decide if data will fit in a normal message, or requires multi-packet output
    if(msgObjPtr->size <= PAYLOAD_SIZE_MAX)
    {
        encode_packet_simple(   output_function, 
                                settings, 
                                msgObjPtr->msgID, 
                                msgObjPtr->size, 
                                msgObjPtr->payload );
    }
#ifndef EUI_CONF_OFFSETS_DISABLED
    else
    {
        send_tracked_range( output_function,
                            msgObjPtr,
                            settings,
                            0,
                            msgObjPtr->size );
    }
#endif
}


void
send_tracked_range( euiCallbackUint8_t  output_function, 
                    euiMessage_t        *msgObjPtr, 
                    euiPacketSettings_t *settings, 
                    uint16_t            base_addr, 
                    uint16_t            end_addr ) 
{

    uint16_t data_range[]   = { base_addr, end_addr };
    validate_offset_range(  base_addr,
                            end_addr,
                            (msgObjPtr->type & 0x0F),
                            msgObjPtr->size,
                            &data_range[0],
                            &data_range[1]);

    euiHeader_t tmp_header;
    tmp_header.internal   = settings->internal;
    tmp_header.response   = settings->response;
    tmp_header.id_len     = strlen(msgObjPtr->msgID);
    tmp_header.acknum     = 0;
    tmp_header.offset     = 0;

    //generate metadata message with address range
    tmp_header.data_len     = sizeof(base_addr) * 2; //base and end are sent
    tmp_header.type         = TYPE_OFFSET_METADATA;

    encode_packet(  output_function,
                    &tmp_header,
                    msgObjPtr->msgID,
                    0x00,
                    &data_range);

    //send the offset packets
    tmp_header.offset = 1;
    tmp_header.type   = msgObjPtr->type;

    while( end_addr > base_addr )
    {
        uint16_t bytes_remaining = end_addr - base_addr;
        tmp_header.data_len = ( bytes_remaining > PAYLOAD_SIZE_MAX ) ? PAYLOAD_SIZE_MAX : bytes_remaining;
        
        //the current position through the buffer in bytes is also the end offset
        end_addr -= tmp_header.data_len;  

        encode_packet(  output_function,
                        &tmp_header,
                        msgObjPtr->msgID,
                        end_addr,
                        msgObjPtr->payload );
    }
}

void
validate_offset_range(  uint16_t base,
                        uint16_t offset,
                        uint16_t  type_bytes,
                        uint16_t size,
                        uint16_t *start,
                        uint16_t *end )
{
    uint8_t type_size = 0;

    switch( type_bytes )
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
    base    = ((base    + (type_size-1)) / type_size) * type_size;
    offset  = ((offset  + (type_size-1)) / type_size) * type_size;

    if( offset > size || !offset)
    {
        *end = size;
    }

    if( base >= offset)
    {
        *start = offset - type_size;
    }
}

void
send_message(const char * msg_id)
{
    temp_header.internal  = MSG_DEV;
    temp_header.response  = MSG_NRESP;

    send_tracked(   *auto_output(),
                    find_message_object( msg_id, MSG_DEV ),
                    &temp_header);
}

void
send_message_on(const char * msg_id, euiInterface_t *active_interface)
{
    temp_header.internal  = MSG_DEV;
    temp_header.response  = MSG_NRESP;

    send_tracked(   active_interface->output_func,
                    find_message_object( msg_id, MSG_DEV ),
                    &temp_header);
}

//application layer developer setup helpers
void
setup_interface(euiInterface_t *link_array, uint8_t link_count)
{
    if(link_array && link_count)
    {
        interfaceArray  = link_array;
        numInterfaces   = link_count;
    }
    else
    {
        interfaceArray  = 0;
        numInterfaces   = 0;
    }
}

void
setup_dev_msg(euiMessage_t *msgArray, euiVariableCount_t numObjects)
{
    if(msgArray && numObjects)
    {
        devObjectArray  = msgArray;
        numDevObjects   = numObjects;
    }
    else
    {
        devObjectArray  = 0;
        numDevObjects   = 0;
    }
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

void
setup_handshake_cb(euiCallback_t *dev_cb)
{
    //store the function pointer to the developer side
    if(dev_cb)
    {
        developer_handshake_cb = *dev_cb;
    }
}


//application layer callbacks
void
announce_board(void)
{
    //repond to search request with board info
    temp_header.internal  = MSG_INTERNAL;
    temp_header.response  = MSG_NRESP;

    send_tracked(   *auto_output(), 
                    find_message_object(EUI_INTERNAL_LIB_VER, MSG_INTERNAL), 
                    &temp_header);

    send_tracked(   *auto_output(),
                    find_message_object(EUI_INTERNAL_BOARD_ID, MSG_INTERNAL),
                    &temp_header);

    send_tracked(   *auto_output(),
                    find_message_object(EUI_INTERNAL_SESSION_ID, MSG_INTERNAL),
                    &temp_header);

    if(developer_handshake_cb)
    {
        developer_handshake_cb();
    }
}

void
announce_dev_msg_readonly(void)
{
    euiVariableCount_t num_read_only  = 0;
    num_read_only = send_tracked_message_id_list(READ_ONLY_FLAG);

    temp_header.internal  = MSG_INTERNAL;
    temp_header.response  = MSG_NRESP;
    temp_header.type      = TYPE_UINT8;
    encode_packet_simple(   *auto_output(),
                            &temp_header,
                            EUI_INTERNAL_AM_RO_END,
                            sizeof(num_read_only),
                            &num_read_only);
}

void
announce_dev_msg_writable(void)
{
    euiVariableCount_t num_writable  = 0;
    num_writable = send_tracked_message_id_list(WRITABLE_FLAG);

    temp_header.internal  = MSG_INTERNAL;
    temp_header.response  = MSG_NRESP;
    temp_header.type      = TYPE_UINT8;
    encode_packet_simple(   *auto_output(),
                            &temp_header, 
                            EUI_INTERNAL_AM_RW_END,
                            sizeof(num_writable),
                            &num_writable);
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

euiVariableCount_t
send_tracked_message_id_list(uint8_t read_only)
{
    euiVariableCount_t variables_sent = 0;

    temp_header.internal  = MSG_INTERNAL;
    temp_header.response  = MSG_NRESP;
    temp_header.type      = TYPE_CUSTOM;

    uint8_t msgBuffer[ (MESSAGEID_SIZE+1)*4 ];
    uint8_t msgBufferPos  = 0;  //position in buffer
    uint8_t msgIDlen      = 0;  //length of a single msgID string
    uint8_t msgIDPacked   = 0;  //count messages packed into buffer

    for(euiVariableCount_t i = 0; i < numDevObjects; i++)
    {
        // filter based on writable flag
        if( devObjectArray[i].type >> 7 == read_only )
        {
            //copy messageID into the buffer, account for null termination characters as delimiter
            msgIDlen = strlen(devObjectArray[i].msgID) + 1;
            memcpy(msgBuffer+msgBufferPos, devObjectArray[i].msgID, msgIDlen);
            msgBufferPos += msgIDlen;
            msgIDPacked++;

            variables_sent++;
        }
    
        //send messages and clear buffer
        if( (msgBufferPos >= (sizeof(msgBuffer) - MESSAGEID_SIZE/2)) || (i >= numDevObjects - 1) )
        {
            const char * headerID = (read_only) ? EUI_INTERNAL_AM_RO_LIST : EUI_INTERNAL_AM_RW_LIST;
            encode_packet_simple(   *auto_output(),
                                    &temp_header,
                                    headerID,
                                    msgBufferPos,
                                    &msgBuffer );

            //cleanup
            memset(msgBuffer, 0, sizeof(msgBuffer));
            msgBufferPos = 0;
            msgIDPacked = 0;
        }
    }

    return variables_sent;
}

euiVariableCount_t
send_tracked_variables(uint8_t read_only)
{
    euiVariableCount_t sent_variables = 0;
    temp_header.internal  = MSG_DEV;
    temp_header.response  = MSG_NRESP;

    for(euiVariableCount_t i = 0; i < numDevObjects; i++)
    {
        //only send messages which have the specified read-only bit state
        if( devObjectArray[i].type >> 7 == read_only )
        {
            send_tracked( *auto_output(), devObjectArray + i, &temp_header);
            sent_variables++;
        }
    }
    return sent_variables;
}

void
report_error(uint8_t error)
{
#ifndef EUI_CONF_ERROR_DISABLE
    last_error = error;

    temp_header.internal  = MSG_INTERNAL;
    temp_header.response  = MSG_NRESP;

    send_tracked(   *auto_output(), 
                    find_message_object( EUI_INTERNAL_ERROR_ID, MSG_INTERNAL ), 
                    &temp_header);
#endif
}

// END electricui.c