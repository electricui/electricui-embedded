#ifdef __cplusplus
extern "C" {
#endif

#ifndef EUI_H
#define EUI_H

#include <stdint.h>
#include "eui_serial_transport.h"
#include "eui_macro.h"
#include "eui_config.h"

// Warnings based on configuration flags (depends on compiler support)
#ifdef EUI_CONF_QUEUE_DISABLE
    #warning "ElectricUI may have issues with outbound buffer overruns or pre-emptive tasking"
#endif

#ifdef EUI_CONF_OFFSETS_DISABLED
    #warning "ElectricUI will not handle data larger than PAYLOAD_SIZE_MAX"
#endif

#ifdef EUI_CONF_MANY_VARIABLES
    typedef uint16_t euiVariableCount_t;
#else
    typedef uint8_t euiVariableCount_t;
#endif

//eUI defines
#define VER_MAJOR 0     //library versions follow semvar2 style (implementation limit of 255 per step)
#define VER_MINOR 6
#define VER_PATCH 0

typedef void (*eui_cb_t)(void);            //callback with no data

typedef struct {
    const char*   msgID;
    uint8_t       type;
    uint16_t      size;
    void          *payload;
// #ifdef EUI_CONF_VARIABLE_CALLBACKS
    eui_cb_t callback;
// #endif
} euiMessage_t;

typedef struct {
    eui_packet_t        packet;
    callback_uint8_t    output_func;
    eui_cb_t            interface_cb;
} euiInterface_t;

enum error_codes {
    err_none = 0,
    err_crc,
    err_parser_generic,
    err_unknown_id,
    err_missing_callback,
    err_invalid_offset,
    err_todo_functionality,
};

//interface management
euiInterface_t *interfaceArray;
uint8_t         numInterfaces;

//dev interface
euiMessage_t        *devObjectArray;
euiVariableCount_t  numDevObjects;

euiMessage_t * find_message_object(const char * msg_id, uint8_t is_internal);
callback_uint8_t * auto_output(void);
void parse_packet(uint8_t inbound_byte, euiInterface_t *p_link);
void handle_packet_data( euiInterface_t *valid_packet, euiHeader_t *header, euiMessage_t *msgObjPtr );
void handle_packet_empty( euiHeader_t *header, euiMessage_t *msgObjPtr );
void handle_packet_callback( euiMessage_t *msgObjPtr );
void handle_packet_response( euiInterface_t *valid_packet, euiHeader_t *header, euiMessage_t *msgObjPtr );

void send_tracked(callback_uint8_t output_function, euiMessage_t *msgObjPtr, euiPacketSettings_t *settings);
void cb_dev_interface_complete( euiInterface_t *p_link );

void validate_offset_range( uint16_t base, uint16_t offset, uint16_t type_bytes, uint16_t size, uint16_t *start, uint16_t *end);

#ifndef EUI_CONF_OFFSETS_DISABLED
    void send_tracked_range(callback_uint8_t output_function, euiMessage_t *msgObjPtr, euiPacketSettings_t *settings, uint16_t base_addr, uint16_t end_addr);
#endif
void report_error(uint8_t error);

void setup_interface(euiInterface_t *link_array, uint8_t link_count);
void setup_dev_msg(euiMessage_t *msgArray, euiVariableCount_t numObjects);
void setup_identifier(char * uuid, uint8_t bytes);
void setup_handshake_cb(eui_cb_t *dev_cb);
void send_message(const char * msg_id);
void send_message_on(const char * msg_id, euiInterface_t *active_interface);

void announce_board(void);
void announce_dev_msg_readonly(void);
void announce_dev_msg_writable(void);
void announce_dev_vars_readonly(void);
void announce_dev_vars_writable(void);
euiVariableCount_t send_tracked_message_id_list(uint8_t read_only);
euiVariableCount_t send_tracked_variables(uint8_t read_only);

eui_cb_t developer_handshake_cb;

uint8_t     heartbeat;
uint16_t    board_identifier;
uint8_t     session_identifier;
uint8_t     default_interface;

#ifndef EUI_CONF_ERROR_DISABLE
    uint8_t last_error;
#endif

euiPacketSettings_t temp_header;

#endif

#ifdef __cplusplus
}
#endif