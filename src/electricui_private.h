#ifndef EUI_PRIVATE_H
#define EUI_PRIVATE_H

#include "eui_types.h"

// Private functions
eui_message_t *
find_message_object( const char * msg_id, uint8_t is_internal );

eui_interface_t *
auto_interface( void );

callback_data_out_t
auto_output( void );

uint8_t
handle_packet_action(   eui_interface_t *valid_packet,
                        eui_header_t    *header,
                        eui_message_t   *msgObjPtr );

uint8_t
handle_packet_response( eui_interface_t *valid_packet,
                        eui_header_t    *header,
                        eui_message_t   *msgObjPtr );

//application layer functionality
void
announce_board( void );

void
announce_dev_msg_readonly( void );

void
announce_dev_msg_writable( void );

void
announce_dev_vars_readonly( void );

void
announce_dev_vars_writable( void );

eui_variable_count_t
send_tracked_message_id_list( uint8_t read_only );

eui_variable_count_t
send_tracked_variables( uint8_t read_only );

//interface management
eui_interface_t     *interface_arr;
uint8_t             interface_count;

//dev interface
eui_message_t           *devObjectArray;
eui_variable_count_t    numDevObjects;

// eUI variables accessible to developer
uint8_t     heartbeat;
uint16_t    board_identifier;
uint8_t     active_interface;

#endif //end EUI_PRIVATE_H