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
    uint8_t       size;
    void          *payload;
} euiMessage_t;

enum error_codes {
    err_none = 0,
    err_crc,
    err_invalid_internal,
    err_invalid_developer,
    err_missing_callback,
};

euiMessage_t * findMessageObject(const char * msg_id, uint8_t is_internal);
void handlePacket(struct eui_interface_state *valid_packet);
void sendTracked(const char * msg_id, uint8_t is_internal);
void report_error(uint8_t error);

//dev interface
euiMessage_t *devObjectArray;
uint8_t numDevObjects;
CallBackwithUINT8 parserOutputFunc;  //holding ref for output func

void setupDevMsg(euiMessage_t *msgArray, uint8_t numObjects);
void setupIdentifier();
void sendMessage(const char * msg_id, struct eui_interface_state *active_interface);

//internal
const uint8_t library_version[] = { VER_MAJOR, VER_MINOR, VER_PATCH };
uint8_t heartbeat;
uint8_t board_identifier;
uint8_t session_identifier;

uint8_t last_error;

void announceBoard(void);
void announceDevMsg(void);

const euiMessage_t internal_msg_store[] = {
    {.msgID = "lv", .type = TYPE_UINT8, .size = sizeof(library_version),    .payload = &library_version     },
    {.msgID = "bi", .type = TYPE_UINT8, .size = sizeof(board_identifier),   .payload = &board_identifier    },
    {.msgID = "si", .type = TYPE_UINT8, .size = sizeof(session_identifier), .payload = &session_identifier  },
 
    {.msgID = "er", .type = TYPE_UINT8, .size = sizeof(last_error),         .payload = &last_error          },
    {.msgID = "hb", .type = TYPE_UINT8, .size = sizeof(heartbeat),          .payload = &heartbeat           },

    {.msgID = "dm", .type = TYPE_CALLBACK, .size = sizeof(announceDevMsg),  .payload = &announceDevMsg      },
    {.msgID = "as", .type = TYPE_CALLBACK, .size = sizeof(announceBoard),   .payload = &announceBoard       },
};

#endif