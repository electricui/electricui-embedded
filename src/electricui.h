#ifndef EUI_H
#define EUI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "eui_config.h"
#include "utilities/eui_macro.h"

// Warnings based on configuration flags (depends on compiler support)
#ifdef EUI_CONF_QUEUE_DISABLE
    #warning "ElectricUI may have issues with outbound buffer overruns or pre-emptive tasking"
#endif

#ifdef EUI_CONF_OFFSETS_DISABLED
    #warning "ElectricUI will not handle data larger than PAYLOAD_SIZE_MAX"
#endif

#define VER_MAJOR 0     //library versions follow semvar2 style
#define VER_MINOR 6
#define VER_PATCH 0

typedef void (*eui_cb_t)(void);            //callback with no data

typedef struct {
    const char*   msgID;
    uint8_t       type;
    uint16_t      size;
    void          *payload;
#ifdef EUI_CONF_VARIABLE_CALLBACKS
    eui_cb_t callback;
#endif
} eui_message_t;

typedef struct {
    eui_packet_t        packet;
    callback_uint8_t    output_func;
    callback_uint8_t    interface_cb;
} eui_interface_t;

enum status_codes {
    status_ok = 0,
    status_crc_err,
    status_parser_generic,
    status_unknown_id,
    status_missing_callback,
    status_offset_er,
    status_todo,
};

enum callback_codes {
    cb_generic = 0,
    cb_untracked,
    cb_todo,
};

eui_message_t *
find_message_object( const char * msg_id, uint8_t is_internal );

uint8_t
parse_packet( uint8_t inbound_byte, eui_interface_t *p_link );

void
send_tracked( callback_uint8_t output_function, eui_message_t *msgObjPtr, eui_pkt_settings_t *settings );

#ifndef EUI_CONF_OFFSETS_DISABLED
    void
    send_tracked_range( callback_uint8_t output_function, eui_message_t *msgObjPtr, eui_pkt_settings_t *settings, uint16_t base_addr, uint16_t end_addr );
#endif

void
setup_interface( eui_interface_t *link_array, uint8_t link_count );

void
setup_dev_msg( eui_message_t *msgArray, euiVariableCount_t numObjects );

void
setup_identifier( char * uuid, uint8_t bytes );

void
send_message( const char * msg_id );

void
send_message_on( const char * msg_id, eui_interface_t *active_interface );

#ifdef __cplusplus
}
#endif

#endif //end EUI_H