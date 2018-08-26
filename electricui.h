#ifndef EUI_H
#define EUI_H

#include <stdint.h>
#include "eui_serial_transport.h"
#include "eui_macro.h"

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
void send_tracked_range(euiMessage_t *msgObjPtr, euiPacketSettings_t *settings, uint16_t base_addr, uint16_t end_addr);
void report_error(uint8_t error);

//dev interface
euiMessage_t *devObjectArray;
uint8_t numDevObjects;
CallBackwithUINT8 parserOutputFunc;  //holding ref for output func

void setup_dev_msg(euiMessage_t *msgArray, uint8_t numObjects);
void setup_identifier(char * uuid, uint8_t bytes);
void send_message(const char * msg_id, eui_interface *active_interface);

void announce_board(void);
uint8_t send_tracked_message_id_list(uint8_t read_only);
uint8_t send_tracked_variables(uint8_t read_only);

void announce_dev_msg_readonly(void);
void announce_dev_msg_writable(void);
void announce_dev_vars_readonly(void);
void announce_dev_vars_writable(void);

uint8_t heartbeat;
uint16_t board_identifier;
uint8_t session_identifier;

uint8_t last_error;

euiPacketSettings_t temp_header;

#endif