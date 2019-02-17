#include <string.h>
#include "electricui.h"
#include "electricui_private.h"
#include "utilities/eui_crc.h"
#include "utilities/eui_offset_validation.h"

//internal eUI tracked variables
uint8_t library_version[] = { VER_MAJOR, VER_MINOR, VER_PATCH };

eui_message_t internal_msg_store[] = 
{
    EUI_UINT8_RO(   EUI_INTERNAL_LIB_VER,       library_version     ),
    EUI_UINT16_RO(  EUI_INTERNAL_BOARD_ID,      board_identifier    ),
    EUI_UINT8(      EUI_INTERNAL_HEARTBEAT,     heartbeat           ),

    EUI_FUNC(   EUI_INTERNAL_AM_RO, announce_dev_msg_readonly   ),
    EUI_FUNC(   EUI_INTERNAL_AM_RW, announce_dev_msg_writable   ),
    EUI_FUNC(   EUI_INTERNAL_AV_RO, announce_dev_vars_readonly  ),
    EUI_FUNC(   EUI_INTERNAL_AV_RW, announce_dev_vars_writable  ),

    EUI_FUNC(   EUI_INTERNAL_SEARCH, announce_board ),
};

// Developer facing search
eui_message_t *
find_tracked_object( const char * msg_id )
{
    return find_message_object( msg_id, MSG_DEV );
}

// Internal search in either variable array
eui_message_t * 
find_message_object( const char * msg_id, uint8_t is_internal )
{
    eui_message_t *found_obj_ptr = 0;
    
    if ( MSG_DEV == is_internal && msg_id)
    {
        //search developer space array for matching messageID
        for(eui_variable_count_t i = 0; i < numDevObjects; i++)
        {
            if( strcmp( msg_id, devObjectArray[i].msgID ) == 0 )
            {
                found_obj_ptr = &devObjectArray[i];
                i = numDevObjects;
            }
        }
    }
    else if( MSG_INTERNAL == is_internal && msg_id)
    {
        //search the internal array for matching messageID
        for(eui_variable_count_t i = 0; i < ARR_ELEM( internal_msg_store ); i++)
        {
            if( strcmp( msg_id, internal_msg_store[i].msgID ) == 0 )
            {
                found_obj_ptr = &internal_msg_store[i];
                i = ARR_ELEM( internal_msg_store );
            }
        }
    }

    return found_obj_ptr;
}

eui_interface_t *
auto_interface( void )
{
    eui_interface_t *interface_ptr = 0;

    if( interface_count && interface_arr && last_interface )
    {
        interface_ptr = last_interface;
    }

    return interface_ptr;
}

callback_data_out_t
auto_output( void )
{
    eui_interface_t *selected_interface = auto_interface();

    if( selected_interface )
    {
        return selected_interface->output_func;
    }

    return 0;
}

uint8_t
parse_packet( uint8_t inbound_byte, eui_interface_t *p_link )
{
    uint8_t parse_status = eui_decode(inbound_byte, &p_link->packet);

    if( EUI_OK == parse_status )
    {
        last_interface = p_link;
        eui_header_t   header_in    = *(eui_header_t*)&p_link->packet.header;
        eui_message_t *p_msglocal   = find_message_object(  (char*)p_link->packet.msgid_in,
                                                            header_in.internal );

        if( p_msglocal )
        {
            // check the packet's type matches the internal type before running callbacks or writing
            if( header_in.type == TYPE_OFFSET_METADATA
                || (p_msglocal->type & 0x0F) == header_in.type )
            {
                parse_status = handle_packet_action( p_link, &header_in, p_msglocal );
            }
            else
            {
                parse_status = EUI_ERROR_TYPE_MISMATCH;
            }

            // respond to queries or ack as required
            if( header_in.response && (EUI_OK == parse_status) )
            {
                parse_status = handle_packet_response( p_link, &header_in, p_msglocal );
            }

            // notify the developer
            if( p_link->interface_cb )
            {
                p_link->interface_cb( EUI_CB_TRACKED );
            }
        }
        else
        {
            if( p_link->interface_cb )
            {
                p_link->interface_cb( EUI_CB_UNTRACKED );
            }
        }

        memset( &p_link->packet, 0, sizeof(eui_packet_t) );
    }
    else if( EUI_ERROR_PARSER >= parse_status )
    {
        if( p_link->interface_cb )
        {
            p_link->interface_cb( EUI_CB_PARSE_FAIL );
        }

        memset( &p_link->packet, 0, sizeof(eui_packet_t) );
        parse_status = EUI_ERROR_PARSER;
    }

    return parse_status;
}

uint8_t
handle_packet_action(   eui_interface_t *valid_packet,
                        eui_header_t    *header,
                        eui_message_t   *msgObjPtr )
{
    uint8_t status = EUI_OK;

    uint8_t is_writable = !(msgObjPtr->type >> 7);
    uint8_t is_callback = (msgObjPtr->type & 0x0F) == TYPE_CALLBACK;

    if( is_callback )
    {
        if( (header->response && header->acknum) || (!header->response && !header->acknum) )
        {
            // Create a function to call from the internal stored pointer
            eui_cb_t cb_packet_h = msgObjPtr->payload;

            if( cb_packet_h )
            {
                cb_packet_h();
            }
            else
            {
                status = EUI_ERROR_CALLBACK;
            }
        }
    }
    else if( TYPE_OFFSET_METADATA != header->type
             && is_writable
             && valid_packet->packet.parser.data_bytes_in )
    {
        // Ensure data won't exceed bounds with invalid offsets
        if( valid_packet->packet.offset_in + header->data_len <= msgObjPtr->size )
        {
            memcpy( (uint8_t *)msgObjPtr->payload + valid_packet->packet.offset_in,
                    valid_packet->packet.data_in,
                    header->data_len );
        }
        else
        {
            status = EUI_ERROR_OFFSET;
        }
    }

    return status;
}

// Check the inbound packet's response requirements and output as required
uint8_t
handle_packet_response( eui_interface_t *valid_packet,
                        eui_header_t    *header,
                        eui_message_t   *msgObjPtr )
{
    uint8_t status = EUI_OK;

    if( header->acknum )
    {
        eui_header_t tmp_header = { .internal   = header->internal,
                                    .response   = MSG_NRESP,
                                    .type       = msgObjPtr->type,
                                    .id_len     = strlen(msgObjPtr->msgID),
                                    .acknum     = header->acknum,
                                    .offset     = header->offset,
                                    .data_len   = 0  };

        status = eui_encode(    valid_packet->output_func,
                                &tmp_header,
                                msgObjPtr->msgID,
                                valid_packet->packet.offset_in,
                                msgObjPtr->payload );
    }
    else
    {
        //respond with data to fufil query behaviour
        eui_pkt_settings_t res_header = { .internal = header->internal,
                                          .response = MSG_NRESP, 
                                          .type     = msgObjPtr->type };

        // inverted logic used to keep ifdef disable clean
        if( TYPE_OFFSET_METADATA != header->type )
        {
            status = send_packet(valid_packet->output_func, msgObjPtr, &res_header);
        }
#ifndef EUI_CONF_OFFSETS_DISABLED
        else
        {
            uint16_t base_address = 0;
            uint16_t end_address  = 0;
            
            base_address  = (uint16_t)valid_packet->packet.data_in[1] << 8;
            base_address |= valid_packet->packet.data_in[0];

            end_address  = (uint16_t)valid_packet->packet.data_in[3] << 8;
            end_address |= valid_packet->packet.data_in[2];

            status = send_packet_range( valid_packet->output_func,
                                        msgObjPtr,
                                        &res_header,
                                        base_address,
                                        end_address );
        }
#endif
    }

    return status;
}

uint8_t
send_packet(    callback_data_out_t output_function,
                eui_message_t       *msgObjPtr,
                eui_pkt_settings_t  *settings )
{
    uint8_t status = EUI_ERROR_SEND;

    if( output_function && msgObjPtr )
    {
        settings->type = msgObjPtr->type;
 
        //decide if data will fit in a normal message, or requires multi-packet output
        if( msgObjPtr->size <= PAYLOAD_SIZE_MAX )
        {
            status = eui_encode_simple( output_function,
                                        settings,
                                        msgObjPtr->msgID,
                                        msgObjPtr->size,
                                        msgObjPtr->payload );
        }
#ifndef EUI_CONF_OFFSETS_DISABLED
        else
        {
            status = send_packet_range( output_function,
                                        msgObjPtr,
                                        settings,
                                        0,
                                        msgObjPtr->size );
        }
#endif
    }

    return status;
}

uint8_t
send_packet_range(  callback_data_out_t output_function, 
                    eui_message_t       *msgObjPtr, 
                    eui_pkt_settings_t  *settings, 
                    uint16_t            base_addr, 
                    uint16_t            end_addr ) 
{
    uint8_t status = EUI_ERROR_SEND_OFFSET;

    uint16_t data_range[2]  = { 0 };
    validate_offset_range(  base_addr,
                            end_addr,
                            (msgObjPtr->type & 0x0F),
                            msgObjPtr->size,
                            &data_range[0],
                            &data_range[1]);

    eui_header_t tmp_header = { 0 };
    tmp_header.internal   = settings->internal;
    tmp_header.response   = settings->response;
    tmp_header.id_len     = strlen(msgObjPtr->msgID);

    //generate metadata message with address range
    tmp_header.data_len     = sizeof(base_addr) * 2; //base and end are sent
    tmp_header.type         = TYPE_OFFSET_METADATA;

    status = eui_encode( output_function, &tmp_header, msgObjPtr->msgID, 0x00, &data_range);

    //send the offset packets
    tmp_header.offset = 1;
    tmp_header.type   = msgObjPtr->type;

    while( end_addr > base_addr && ( EUI_OK == status) )
    {
        uint16_t bytes_remaining = end_addr - base_addr;

        if( bytes_remaining > PAYLOAD_SIZE_MAX )
        {
            tmp_header.data_len = PAYLOAD_SIZE_MAX;
        }
        else
        {
            tmp_header.data_len = bytes_remaining;
        }
        
        //the current position through the buffer in bytes is also the end offset
        end_addr -= tmp_header.data_len;  

        status = eui_encode(    output_function,
                                &tmp_header,
                                msgObjPtr->msgID,
                                end_addr,
                                msgObjPtr->payload );
    }

    return status;
}

void
send_tracked( const char * msg_id )
{
    if( msg_id )
    {
        eui_pkt_settings_t temp_header = { 0 };
        temp_header.internal  = MSG_DEV;
        temp_header.response  = MSG_NRESP;

        send_packet(    auto_output(),
                        find_message_object( msg_id, MSG_DEV ),
                        &temp_header );
    }
}

void
send_tracked_on(const char * msg_id, eui_interface_t *interface)
{
    if( msg_id && interface )
    {
        eui_pkt_settings_t      temp_header = { 0 };
        temp_header.internal  = MSG_DEV;
        temp_header.response  = MSG_NRESP;

        send_packet(    interface->output_func,
                        find_message_object( msg_id, MSG_DEV ),
                        &temp_header );
    }
}

void
send_untracked( eui_message_t *msg_obj_ptr )
{
    if( msg_obj_ptr )
    {
        eui_pkt_settings_t      temp_header = { 0 };
        temp_header.internal  = MSG_DEV;
        temp_header.response  = MSG_NRESP;

        send_packet(    auto_output(),
                        msg_obj_ptr,
                        &temp_header );
    }
}

void
send_untracked_on( eui_message_t *msg_obj_ptr, eui_interface_t *interface )
{
    if( msg_obj_ptr && interface )
    {
        eui_pkt_settings_t      temp_header = { 0 };
        temp_header.internal  = MSG_DEV;
        temp_header.response  = MSG_NRESP;

        send_packet(    interface->output_func,
                        msg_obj_ptr,
                        &temp_header );
    }
}

//application layer developer setup helpers
void
setup_interface( eui_interface_t *link )
{
    setup_interfaces( link, 1 );
}

void
setup_interfaces( eui_interface_t *link_array, uint8_t link_count )
{
    if( link_array && link_count )
    {
        interface_arr   = link_array;
        interface_count = link_count;

        // bootstrap the auto_interface with the 0th interface from the array
        last_interface = link_array;
    }
    else
    {
        interface_arr   = 0;
        interface_count = 0;
        last_interface = 0;
    }

}

void
setup_dev_msg( eui_message_t *msgArray, eui_variable_count_t numObjects )
{
    if( msgArray && numObjects )
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
setup_identifier( char * uuid, uint8_t bytes )
{
    if( uuid && bytes )
    {
        //generate a 'hashed' int16 of their UUID
        for(uint8_t i = 0; i < bytes; i++)
        {
            eui_crc(uuid[i], &board_identifier);
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
announce_board( void )
{
    //repond to search request with board info
    eui_pkt_settings_t  temp_header;
    temp_header.internal  = MSG_INTERNAL;
    temp_header.response  = MSG_NRESP;

    send_packet(    auto_output(), 
                    find_message_object(EUI_INTERNAL_LIB_VER, MSG_INTERNAL), 
                    &temp_header);

    send_packet(    auto_output(),
                    find_message_object(EUI_INTERNAL_BOARD_ID, MSG_INTERNAL),
                    &temp_header);

    send_packet(    auto_output(),
                    find_message_object(EUI_INTERNAL_SESSION_ID, MSG_INTERNAL),
                    &temp_header);

    // Developer callback allows them to publish connection hints etc
    eui_interface_t *selected_interface = auto_interface();

    if( selected_interface && selected_interface->interface_cb )
    {
        selected_interface->interface_cb( EUI_CB_ANNOUNCE );
    }
}

void
announce_dev_msg_readonly( void )
{
    eui_variable_count_t num_read_only  = 0;
    num_read_only = send_tracked_message_id_list( READ_ONLY_FLAG );

    eui_pkt_settings_t temp_header = { 0 };
    temp_header.internal  = MSG_INTERNAL;
    temp_header.response  = MSG_NRESP;
    temp_header.type      = TYPE_MANY_VARIABLES_SIZED;
    eui_encode_simple(  auto_output(),
                        &temp_header,
                        EUI_INTERNAL_AM_RO_END,
                        sizeof(num_read_only),
                        &num_read_only);
}

void
announce_dev_msg_writable( void )
{
    eui_variable_count_t num_writable  = 0;
    num_writable = send_tracked_message_id_list( WRITABLE_FLAG );

    eui_pkt_settings_t temp_header = { 0 };
    temp_header.internal  = MSG_INTERNAL;
    temp_header.response  = MSG_NRESP;
    temp_header.type      = TYPE_MANY_VARIABLES_SIZED;
    eui_encode_simple(  auto_output(),
                        &temp_header, 
                        EUI_INTERNAL_AM_RW_END,
                        sizeof(num_writable),
                        &num_writable);
}

void
announce_dev_vars_readonly( void )
{
    send_tracked_variables( READ_ONLY_FLAG );
}

void
announce_dev_vars_writable( void )
{
    send_tracked_variables( WRITABLE_FLAG );
}

eui_variable_count_t
send_tracked_message_id_list( uint8_t read_only )
{
    eui_variable_count_t variables_sent = 0;

    eui_pkt_settings_t temp_header = { 0 };
    temp_header.internal  = MSG_INTERNAL;
    temp_header.response  = MSG_NRESP;
    temp_header.type      = TYPE_CUSTOM;

    uint8_t msgBuffer[ (16)*4 ];
    uint8_t msgBufferPos  = 0;  //position in buffer
    uint8_t msgIDlen      = 0;  //length of a single msgID string
    uint8_t msgIDPacked   = 0;  //count messages packed into buffer

    for( eui_variable_count_t i = 0; i < numDevObjects; i++ )
    {
        // filter based on writable flag
        if( read_only == devObjectArray[i].type >> 7 )
        {
            //copy messageID into the buffer, account for null termination characters as delimiter
            msgIDlen = strlen(devObjectArray[i].msgID) + 1;
            memcpy(msgBuffer+msgBufferPos, devObjectArray[i].msgID, msgIDlen);
            msgBufferPos += msgIDlen;
            msgIDPacked++;

            variables_sent++;
        }
    
        //send messages and clear buffer
        if( ((sizeof(msgBuffer) - 16/2) <= msgBufferPos) || (numDevObjects - 1 <= i ) )
        {
            const char * headerID = (read_only) ? EUI_INTERNAL_AM_RO_LIST : EUI_INTERNAL_AM_RW_LIST;
            eui_encode_simple(  auto_output(),
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

eui_variable_count_t
send_tracked_variables( uint8_t read_or_writable )
{
    eui_variable_count_t    sent_variables = 0;
    eui_pkt_settings_t      temp_header = { 0 };

    temp_header.internal    = MSG_DEV;
    temp_header.response    = MSG_NRESP;

    for(eui_variable_count_t i = 0; i < numDevObjects; i++)
    {
        //only send messages which have the specified read-only bit state
        if( read_or_writable == devObjectArray[i].type >> 7 )
        {
            send_packet( auto_output(), devObjectArray + i, &temp_header );
            sent_variables++;
        }
    }
    return sent_variables;
}

// END electricui.c