#ifndef EUI_SERIAL_TRANSPORT_H
#define EUI_SERIAL_TRANSPORT_H

#include <stdint.h>

#define MESSAGEID_SIZE      3      //max allowed bytes in msgID
#define PACKET_BASE_SIZE    ( sizeof(stHeader) + sizeof(euiHeader_t) + MESSAGEID_SIZE \
                            + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(stText) \
                            + sizeof(enText) + sizeof(enTransmission) )  //maximum overhead in bytes
#define PAYLOAD_SIZE_MAX    120    //max payload data size

#define MSG_INTERNAL        1
#define MSG_DEV             0
#define MSG_ACK_REQ         1
#define MSG_ACK_NOTREQ      0
#define MSG_OFFSET_PACKET   1
#define MSG_STANDARD_PACKET 0

//control characters in packets
static const uint8_t stHeader       = 0x01;
static const uint8_t stText         = 0x02;
static const uint8_t enText         = 0x03;
static const uint8_t enTransmission = 0x04;
static const uint8_t enID           = 0x00;

typedef struct {
  unsigned internal   : 1;
  unsigned reqACK     : 1;
  unsigned offsetAd   : 1;
  unsigned type       : 5;
} euiHeader_t;

typedef enum {
    TYPE_CALLBACK = 0,
    TYPE_BYTE,
    TYPE_CHAR,
    TYPE_INT8,
    TYPE_UINT8,
    TYPE_INT16,
    TYPE_UINT16,
    TYPE_INT32,
    TYPE_UINT32,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_CUSTOM_MARKER
} euiType_t;

typedef void (*CallBackwithUINT8)(uint8_t); //callback with single char of data

struct eui_interface_state {
    uint8_t controlState;

    //buffer incoming data
    uint8_t inboundID[MESSAGEID_SIZE+1];
    uint8_t inboundHeader;
    uint8_t inboundSize;
    uint16_t inboundOffset;
    uint8_t inboundData[PAYLOAD_SIZE_MAX];
    uint8_t inboundCRC;

    //count the bytes ingested
    uint8_t processedID;
    uint8_t processedData;
    uint8_t processedCRC;

    CallBackwithUINT8 output_char_fnPtr;
};

enum parseStates {
    find_preamble = 0,
    exp_header,
    exp_msgID,
    exp_payload_len,
    exp_offset,
    exp_stx,
    exp_data,
    exp_etx,
    exp_crc,
    exp_eot,
};

uint8_t calc_crc(uint8_t *to_xor, uint8_t datagram_len);
uint8_t generate_header(uint8_t internal, uint8_t ack, uint8_t offset_packet, uint8_t payloadtype);
void    generate_packet(const char * msg_id, uint8_t header, uint8_t payload_len, void* payload, CallBackwithUINT8 output_function);
void    generate_packet_offset(const char * msg_id, uint8_t header, uint8_t payload_len, uint16_t offset, void* payload, CallBackwithUINT8 output_function);
void    parse_packet(uint8_t inbound_byte, struct eui_interface_state *active_interface);

#endif