/* Copyright (c) 2016-2019 Electric UI
 * MIT Licenced - see LICENCE for details.
 *
 * 
 */

#ifndef EUI_H
#define EUI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "eui_config.h"
#include "utilities/eui_macro.h"
#include "eui_types.h"

// Warnings based on configuration flags (depends on compiler support)
#ifdef EUI_CONF_OFFSETS_DISABLED
    #warning "ElectricUI will not handle data larger than PAYLOAD_SIZE_MAX"
#endif

#define EUI_LIBRARY_VERSION 8u

typedef struct {
    eui_packet_t        packet;
    callback_data_out_t output_cb;
    callback_uint8_t    interface_cb;
} eui_interface_t;

eui_message_t *
find_tracked_object( const char * search_id );

eui_errors_t
eui_parse( uint8_t inbound_byte, eui_interface_t *p_link );

uint8_t
eui_send(   callback_data_out_t output_function,
            eui_message_t       *p_msg_obj,
            eui_pkt_settings_t  *settings );

#ifndef EUI_CONF_OFFSETS_DISABLED
    uint8_t
    eui_send_range( callback_data_out_t output_function,
                    eui_message_t       *p_msg_obj,
                    eui_pkt_settings_t  *settings,
                    uint16_t            base_addr,
                    uint16_t            end_addr );
#endif

void
eui_setup_interface( eui_interface_t *link );

void
eui_setup_interfaces( eui_interface_t *link_array, uint8_t link_count );

void
eui_setup_tracked( eui_message_t *msg_array, eui_variable_count_t num_tracked );

void
eui_setup_identifier( char * uuid, uint8_t bytes );

void
eui_send_tracked( const char * msg_id );

void
eui_send_tracked_on( const char * msg_id, eui_interface_t *interface );

void
eui_send_untracked( eui_message_t *p_msg_obj );

void
eui_send_untracked_on( eui_message_t *p_msg_obj, eui_interface_t *interface );

#ifdef __cplusplus
}
#endif

#endif //end EUI_H