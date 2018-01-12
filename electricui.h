#ifndef EUI_H
#define EUI_H

#include <stdint.h>

//eUI defines
#define MESSAGEID_SIZE 3
#define PAYLOAD_SIZE_MAX 120
#define PACKET_BASE_SIZE 10

#define MESSAGES_PK_DISCOVERY 10

#define MSG_INTERNAL 1
#define MSG_DEV 0
#define MSG_TYPE_CUSTOM 1
#define MSG_TYPE_TYP 0
#define MSG_ACK_REQ 1
#define MSG_ACK_NOTREQ 0
#define MSG_RES_H 1
#define MSG_RES_L 0

#define ARR_ELEM(a) (sizeof(a) / sizeof(*a))    //number of elements in array

//control characters in packets
const uint8_t stHeader = 0x01;
const uint8_t stText = 0x02;
const uint8_t enText = 0x03;
const uint8_t enTransmission = 0x04;

typedef void (*CallBackType)();            //callback with no data
typedef void (*CallBackDataType)(uint8_t); //callback with single char of data

typedef struct {
  unsigned internal   : 1;
  unsigned customType : 1;
  unsigned reqACK     : 1;
  unsigned reserved   : 1;
  unsigned type       : 4;
} euiHeader_t;

typedef enum {
    TYPE_BYTE = 0,
    TYPE_CHAR,
    TYPE_INT8,
    TYPE_UINT8,
    TYPE_INT16,
    TYPE_UINT16,
    TYPE_INT32,
    TYPE_UINT32,
    TYPE_INT64,
    TYPE_UINT64,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_CALLBACK,
    TYPE_CUSTOM_MARKER
} euiType_t;

typedef struct {
    const char*   msgID;
    uint8_t       type;
    uint8_t       size;
    void          *payload;
} euiMessage_t;

struct eui_interface_state {
    uint8_t controlState;

    //buffer incoming data
    uint8_t inboundID[MESSAGEID_SIZE+1];
    uint8_t inboundHeader;
    uint8_t inboundSize;
    uint8_t inboundData[PAYLOAD_SIZE_MAX];
    uint8_t inboundCRC;

    //count the bytes ingested
    uint8_t processedID;
    uint8_t processedData;
    uint8_t processedCRC;

    CallBackDataType output_char_fnPtr;
};

enum parseStates {
    find_preamble = 0,
    exp_header,
    exp_msgID,
    exp_stx,
    exp_payloadlen,
    exp_data,
    exp_etx,
    exp_crc,
    exp_eot,
};

uint8_t calcCRC(uint8_t *toSum, uint8_t datagramLen);
uint8_t generateHeader(uint8_t internalmsg, uint8_t reqack, uint8_t reservedbit, uint8_t customtype, uint8_t payloadtype);
euiMessage_t * findMessageObject(const char * msg_id, uint8_t isInternal);
void generatePacket(const char * msg_id, uint8_t header, uint8_t payloadLen, void* payload);
void parsePacket(uint8_t inboundByte, struct eui_interface_state *commInterface);
void handlePacket(struct eui_interface_state *validPacket);

void sendTracked(const char * msg_id, uint8_t isInternal);

//dev interface
euiMessage_t *devObjectArray;
uint8_t numDevObjects;
CallBackDataType parserOutputFunc;  //holding ref for output func

void setupDevMsg(euiMessage_t *msgArray, uint8_t numObjects);
void setupIdentifier();

//internal
const uint8_t libraryVersion = 1;
const uint8_t protocolVersion = 1;
uint8_t heartbeat;
uint8_t boardidentifier;

void announceDevMsg(void);
void announceBoard(void);

const euiMessage_t internal_msg_store[] = {
    {.msgID = "lv", .type = TYPE_UINT8, .size = sizeof(libraryVersion),     .payload = &libraryVersion },
    {.msgID = "pv", .type = TYPE_UINT8, .size = sizeof(protocolVersion),    .payload = &protocolVersion },
    {.msgID = "hb", .type = TYPE_UINT8, .size = sizeof(heartbeat),          .payload = &heartbeat },
    {.msgID = "id", .type = TYPE_UINT8, .size = sizeof(boardidentifier),    .payload = &boardidentifier },

    {.msgID = "dm", .type = TYPE_CALLBACK, .size = sizeof(announceDevMsg),  .payload = &announceDevMsg },
    {.msgID = "hi", .type = TYPE_CALLBACK, .size = sizeof(announceBoard),   .payload = &announceBoard },
};

#endif