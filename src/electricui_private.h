#ifndef EUI_PRIVATE_H
#define EUI_PRIVATE_H

#include "eui_types.h"

// Private functions
eui_message_t *
find_message_object( const char * search_id, uint8_t is_internal );

eui_interface_t *
auto_interface( void );

callback_data_out_t
auto_output( void );

uint8_t
handle_packet_action(   eui_interface_t *valid_packet,
                        eui_header_t    *header,
                        eui_message_t   *p_msg_obj );

uint8_t
handle_packet_ack( 	eui_interface_t *valid_packet,
                    eui_header_t    *header,
                    eui_message_t   *p_msg_obj );

uint8_t
handle_packet_query( 	eui_interface_t *valid_packet,
                        eui_header_t    *header,
                        eui_message_t   *p_msg_obj );

//application layer functionality
void
announce_dev_msg( void );

void
announce_dev_vars( void );

eui_variable_count_t
send_tracked_message_id_list( void );

void
send_tracked_variables( void );

//interface management
eui_interface_t     *p_interface_arr;
uint8_t             interface_num;
eui_interface_t 	*p_interface_last;

//dev interface
eui_message_t           *p_dev_tracked;
eui_variable_count_t    dev_tracked_num;

// eUI variables accessible to developer
uint8_t     heartbeat;
uint16_t    board_identifier;

#endif //end EUI_PRIVATE_H