#ifndef EUI_H
#define EUI_H

#include <stdint.h>
#include <eui_serial_transport.h>

//eUI defines
#define VER_MAJOR 1     //library versioning follows semvar2 style (implementation limit of 255 per step)
#define VER_MINOR 3
#define VER_PATCH 1

#define ARR_ELEM(a) (sizeof(a) / sizeof(*a))    //number of elements in array

typedef void (*CallBackType)();            //callback with no data

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
};

euiMessage_t * find_message_object(const char * msg_id, uint8_t is_internal);
void parse_packet(uint8_t inbound_byte, struct eui_interface *active_interface);
void handle_packet(struct eui_interface *valid_packet);
void send_tracked(euiMessage_t *msgObjPtr, euiPacketSettings_t *settings);
void report_error(uint8_t error);

//dev interface
euiMessage_t *devObjectArray;
uint8_t numDevObjects;
CallBackwithUINT8 parserOutputFunc;  //holding ref for output func

void setup_dev_msg(euiMessage_t *msgArray, uint8_t numObjects);
void setup_identifier(void);
void send_message(const char * msg_id, struct eui_interface *active_interface);

//internal
const uint8_t library_version[] = { VER_MAJOR, VER_MINOR, VER_PATCH };
uint8_t heartbeat;
uint8_t board_identifier;
uint8_t session_identifier;

uint8_t last_error;

void announce_board(void);
void announce_dev_msg(void);
void announce_dev_vars(void);

const euiMessage_t internal_msg_store[] = {
    {.msgID = "lv", .type = TYPE_UINT8, .size = sizeof(library_version),    .payload = &library_version     },
    {.msgID = "bi", .type = TYPE_UINT8, .size = sizeof(board_identifier),   .payload = &board_identifier    },
    {.msgID = "si", .type = TYPE_UINT8, .size = sizeof(session_identifier), .payload = &session_identifier  },
 
    {.msgID = "er", .type = TYPE_UINT8, .size = sizeof(last_error),         .payload = &last_error          },
    {.msgID = "hb", .type = TYPE_UINT8, .size = sizeof(heartbeat),          .payload = &heartbeat           },

    {.msgID = "dm", .type = TYPE_CALLBACK, .size = sizeof(announce_dev_msg),  .payload = &announce_dev_msg  },
    {.msgID = "dv", .type = TYPE_CALLBACK, .size = sizeof(announce_dev_vars), .payload = &announce_dev_vars },
    {.msgID = "as", .type = TYPE_CALLBACK, .size = sizeof(announce_board),    .payload = &announce_board    },
};

#endif