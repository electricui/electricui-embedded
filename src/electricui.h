#ifndef EUI_H
#define EUI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "eui_config.h"
#include "utilities/eui_macro.h"
#include "eui_types.h"

// Warnings based on configuration flags (depends on compiler support)
#ifdef EUI_CONF_QUEUE_DISABLE
    #warning "ElectricUI may have issues with outbound buffer overruns or pre-emptive tasking"
#endif

#ifdef EUI_CONF_OFFSETS_DISABLED
    #warning "ElectricUI will not handle data larger than PAYLOAD_SIZE_MAX"
#endif

//library versions follow semvar2 style
#define VER_MAJOR 0
#define VER_MINOR 7
#define VER_PATCH 0

typedef struct {
    eui_packet_t        packet;
    callback_data_out_t output_func;
    callback_uint8_t    interface_cb;
} eui_interface_t;

eui_message_t *
find_tracked_object( const char * msg_id );

uint8_t
parse_packet( uint8_t inbound_byte, eui_interface_t *p_link );

uint8_t
send_packet(    callback_data_out_t output_function,
                eui_message_t       *msgObjPtr,
                eui_pkt_settings_t  *settings );

#ifndef EUI_CONF_OFFSETS_DISABLED
    uint8_t
    send_packet_range(  callback_data_out_t output_function,
                        eui_message_t       *msgObjPtr,
                        eui_pkt_settings_t  *settings,
                        uint16_t            base_addr,
                        uint16_t            end_addr );
#endif

void
setup_interface( eui_interface_t *link );

void
setup_interfaces( eui_interface_t *link_array, uint8_t link_count );

void
setup_dev_msg( eui_message_t *msgArray, eui_variable_count_t numObjects );

void
setup_identifier( char * uuid, uint8_t bytes );

void
send_tracked( const char * msg_id );

void
send_tracked_on( const char * msg_id, eui_interface_t *interface );

void
send_untracked( eui_message_t *msg_obj_ptr );

void
send_untracked_on( eui_message_t *msg_obj_ptr, eui_interface_t *interface );

#ifdef __cplusplus
}
#endif

#endif //end EUI_H