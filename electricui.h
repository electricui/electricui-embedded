#ifndef EUI_H
#define EUI_H

#include <stdint.h>

//eUI defines
#define MESSAGEID_SIZE 3
#define PAYLOAD_SIZE_MAX 120
#define PACKET_BASE_SIZE 10

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
    TYPE_QUERY
} euiType_t;

typedef struct {
    const char*   msgID;
    uint8_t       type;
    void          *payload;
} euiMessage_t;

struct eui_parser_state {
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
};

enum parseStates {
    s_invalidData = 0,
    s_preamble,
    s_header,
    s_msg_id,
    s_datastart,
    s_datalen,
    s_payload,
    s_dataend,
    s_checksum,
};

typedef void (*CallBackType)();
CallBackType parsedCallbackHandler;


uint8_t calcCRC(uint8_t *toSum, uint8_t datagramLen);
uint8_t generateHeader(uint8_t internalmsg, uint8_t reqack, uint8_t reservedbit, uint8_t customtype, uint8_t payloadtype);
void generatePacket(const char * msg_id, uint8_t header, uint8_t payloadLen, void* payload);
void parsePacket(uint8_t inboundByte, struct eui_parser_state *commInterface);
void handlePacket(struct eui_parser_state *validPacket);



//application layer declarations

//dev interface
euiMessage_t *devObjectArray;
uint8_t numDevObjects;

//callback with single char of data
typedef void (*CallBackDataType)(uint8_t);
CallBackDataType parserOutputFunc;

void setupParser(CallBackDataType parserFuncPtr);
void setupDevMsg(euiMessage_t *msgArray, uint8_t numObjects);

//internal
const uint8_t libraryVersion = 1;
const uint8_t protocolVersion = 1;
void announceDevMsg(void);
void announceBoard(void);

const euiMessage_t int_msg_store[] = {
    {.msgID = "LiV", .type = TYPE_UINT8, .payload = &libraryVersion },
    {.msgID = "PrV", .type = TYPE_UINT8, .payload = &protocolVersion },
    {.msgID = "aDM", .type = TYPE_CALLBACK, .payload = &announceDevMsg },
    {.msgID = "eHi", .type = TYPE_CALLBACK, .payload = &announceBoard },
};

#endif