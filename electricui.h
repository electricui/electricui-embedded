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
    typedef uint16_t eui_var_count_t;
#else
    typedef uint8_t eui_var_count_t;
#endif

//eUI defines
#define VER_MAJOR 1     //library versions follow semvar2 style (implementation limit of 255 per step)
#define VER_MINOR 3
#define VER_PATCH 1

typedef void (*CallBackType)(void);            //callback with no data

typedef struct {
    const char*   msgID;
    uint8_t       type;
    uint16_t      size;
    void          *payload;
#ifdef EUI_CONF_VARIABLE_CALLBACKS
    // void          *callback; //todo add functionality
#endif
} euiMessage_t;

enum error_codes {
    err_none = 0,
    err_crc,
    err_parser_generic,
    err_invalid_internal,
    err_invalid_developer,
    err_missing_callback,
    err_invalid_offset,
    err_todo_functionality,
};

euiMessage_t * find_message_object(const char * msg_id, uint8_t is_internal);
void parse_packet(uint8_t inbound_byte, eui_interface *active_interface);
void handle_packet(eui_interface *valid_packet);
void send_tracked(euiMessage_t *msgObjPtr, euiPacketSettings_t *settings);
#ifndef EUI_CONF_OFFSETS_DISABLED
    void send_tracked_range(euiMessage_t *msgObjPtr, euiPacketSettings_t *settings, uint16_t base_addr, uint16_t end_addr);
#endif
void report_error(uint8_t error);

//dev interface
euiMessage_t *devObjectArray;
eui_var_count_t numDevObjects;
CallBackwithUINT8 parserOutputFunc;  //holding ref for output func

void setup_dev_msg(euiMessage_t *msgArray, eui_var_count_t numObjects);
void setup_identifier(char * uuid, uint8_t bytes);
void send_message(const char * msg_id, eui_interface *active_interface);

void announce_board(void);
eui_var_count_t send_tracked_message_id_list(uint8_t read_only);
eui_var_count_t send_tracked_variables(uint8_t read_only);

void announce_dev_msg_readonly(void);
void announce_dev_msg_writable(void);
void announce_dev_vars_readonly(void);
void announce_dev_vars_writable(void);

uint8_t heartbeat;
uint16_t board_identifier;
uint8_t session_identifier;

#ifndef EUI_CONF_ERROR_DISABLE
    uint8_t last_error;
#endif

euiPacketSettings_t temp_header;

#endif

#ifdef __cplusplus
}
#endif