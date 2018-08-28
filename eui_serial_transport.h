#ifndef EUI_SERIAL_TRANSPORT_H
#define EUI_SERIAL_TRANSPORT_H

#include <stdint.h>
#include "eui_config.h"

#define MESSAGEID_BITS      4                       //size of the messageIDlen bitfield
#define MESSAGEID_SIZE      ( 1 << MESSAGEID_BITS ) //max allowed bytes in msgID

#define PACKET_BASE_SIZE    ( sizeof(stHeader) + sizeof(euiHeader_t) \
                            + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(enTransmission) )  //maximum overhead in bytes

#define MSG_INTERNAL        1
#define MSG_DEV             0
#define MSG_RESP            1
#define MSG_NRESP           0
#define MSG_OFFSET_PACKET   1
#define MSG_STANDARD_PACKET 0

#define CALLBACK_SIZE 1 //sizeof( function ) is outside C spec though GCC returns 1. We don't use it.

//control characters in packets
static const uint8_t stHeader       = 0x01;
static const uint8_t enTransmission = 0x04;

typedef struct {
  unsigned data_len   : 10;
  unsigned type       : 4;
  unsigned internal   : 1;
  unsigned offset     : 1;
  unsigned id_len     : MESSAGEID_BITS;
  unsigned response   : 1;
  unsigned acknum     : 3;
} euiHeader_t;

typedef struct {
  unsigned internal   : 1;
  unsigned response   : 1;
  unsigned type       : 4;
} euiPacketSettings_t;

typedef enum {
    TYPE_CALLBACK = 0,
    TYPE_CUSTOM,
    TYPE_OFFSET_METADATA,
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
} euiType_t;

#define READ_ONLY_MASK 0x80
#define READ_ONLY_FLAG 0x01
#define WRITABLE_FLAG 0x00

typedef void (*CallBackwithUINT8)(uint8_t); //callback with single char of data

typedef struct {
  unsigned parser_s         : 4;
  unsigned id_bytes_in      : MESSAGEID_BITS;
  unsigned data_bytes_in    : 10;
} eui_interface_state_t;

enum parseStates {
    find_preamble = 0,
    exp_header_b1,
    exp_header_b2,
    exp_header_b3,   
    exp_message_id,
    exp_offset_b1,
    exp_offset_b2,
    exp_data,
    exp_crc_b1,
    exp_crc_b2,
    exp_eot,
    exp_reset,
};

typedef struct {
    //hold the inbound parser state information
    eui_interface_state_t state;

    //buffer incoming data
    euiHeader_t inboundHeader;
    uint8_t inboundID[MESSAGEID_SIZE];
    uint16_t inboundOffset;
    uint8_t inboundData[PAYLOAD_SIZE_MAX];
    uint16_t runningCRC;

    //maintain a pointer to the output function for this interface
    CallBackwithUINT8 output_char_fnPtr;
} eui_interface;

enum packet_signals {
    parser_idle = 0,
    packet_valid,
    packet_error_crc,
    packet_error_generic,
};

void      crc16(uint8_t data, uint16_t *crc);
uint8_t   encode_packet_simple(CallBackwithUINT8 output_function, euiPacketSettings_t *settings, const char * msg_id, uint16_t payload_len, void* payload);
uint8_t   encode_packet(CallBackwithUINT8 output_function, euiHeader_t * header, const char * msg_id, uint16_t offset, void* payload);
uint8_t   decode_packet(uint8_t inbound_byte, eui_interface *active_interface);

#endif