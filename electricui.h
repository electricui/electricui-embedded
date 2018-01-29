#ifndef EUI_H
#define EUI_H

#include <stdint.h>
#include <eui_serial_transport.h>

//eUI defines

#define ARR_ELEM(a) (sizeof(a) / sizeof(*a))    //number of elements in array

typedef void (*CallBackType)();            //callback with no data

typedef struct {
    const char*   msgID;
    uint8_t       type;
    uint8_t       size;
    void          *payload;
} euiMessage_t;

euiMessage_t * findMessageObject(const char * msg_id, uint8_t isInternal);
void handlePacket(struct eui_interface_state *validPacket);
void sendTracked(const char * msg_id, uint8_t isInternal);

//dev interface
euiMessage_t *devObjectArray;
uint8_t numDevObjects;
CallBackwithUINT8 parserOutputFunc;  //holding ref for output func

void setupDevMsg(euiMessage_t *msgArray, uint8_t numObjects);
void setupIdentifier();

//internal
const uint8_t libraryVersion    = 1;
const uint8_t protocolVersion   = 1;

uint8_t heartbeat;
uint8_t board_identifier;
uint8_t session_identifier;

void announceBoard(void);
void announceDevMsg(void);

const euiMessage_t internal_msg_store[] = {
    {.msgID = "lv", .type = TYPE_UINT8, .size = sizeof(libraryVersion),     .payload = &libraryVersion      },
    {.msgID = "pv", .type = TYPE_UINT8, .size = sizeof(protocolVersion),    .payload = &protocolVersion     },

    {.msgID = "hb", .type = TYPE_UINT8, .size = sizeof(heartbeat),          .payload = &heartbeat           },
    {.msgID = "bi", .type = TYPE_UINT8, .size = sizeof(board_identifier),   .payload = &board_identifier    },
    {.msgID = "si", .type = TYPE_UINT8, .size = sizeof(session_identifier), .payload = &session_identifier  },

    {.msgID = "dm", .type = TYPE_CALLBACK, .size = sizeof(announceDevMsg),  .payload = &announceDevMsg      },
    {.msgID = "as", .type = TYPE_CALLBACK, .size = sizeof(announceBoard),   .payload = &announceBoard       },
};

#endif