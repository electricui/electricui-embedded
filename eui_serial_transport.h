#ifndef EUI_SERIAL_TRANSPORT_H
#define EUI_SERIAL_TRANSPORT_H

#include <stdint.h>

#define MESSAGEID_BITS      4                       //size of the messageIDlen bitfield
#define MESSAGEID_SIZE      ( 2^MESSAGEID_BITS )    //max allowed bytes in msgID

#define PACKET_BASE_SIZE    ( sizeof(stHeader) + sizeof(euiHeader_t) \
                            + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(enTransmission) )  //maximum overhead in bytes
#define PAYLOAD_SIZE_MAX    120    //max payload data size

#define MSG_INTERNAL        1
#define MSG_DEV             0
#define MSG_ACK             1
#define MSG_NACK            0
#define MSG_QUERY           1
#define MSG_NQUERY          0
#define MSG_OFFSET_PACKET   1
#define MSG_STANDARD_PACKET 0

//control characters in packets
static const uint8_t stHeader       = 0x01;
static const uint8_t enTransmission = 0x04;

typedef struct {
  unsigned internal   : 1;
  unsigned ack        : 1;
  unsigned query      : 1;
  unsigned offset     : 1;
  unsigned type       : 4;
  unsigned id_len     : MESSAGEID_BITS;
  unsigned data_len   : 10;
  unsigned seq        : 2;
} euiHeader_t;

typedef struct {
  unsigned internal   : 1;
  unsigned ack        : 1;
  unsigned query      : 1;
  unsigned type       : 4;
} euiPacketSettings_t;

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

typedef struct {
  unsigned parser_s         : 3;
  unsigned header_bytes_in  : 2;
  unsigned offset_bytes_in  : 1;
  unsigned crc_bytes_in     : 1;
  unsigned data_bytes_in    : 10;
  unsigned id_bytes_in      : MESSAGEID_BITS;
} eui_interface_state_t;

enum parseStates {
    find_preamble = 0,
    exp_header,
    exp_message_id,
    exp_offset,
    exp_data,
    exp_crc,
    exp_eot,
};

struct eui_interface {
    //hold the inbound parser state information
    eui_interface_state_t state;

    //buffer incoming data
    euiHeader_t inboundHeader;
    uint8_t inboundID[MESSAGEID_SIZE+1];
    uint16_t inboundOffset;
    uint8_t inboundData[PAYLOAD_SIZE_MAX];
    uint16_t inboundCRC;

    //maintain a pointer to the output function for this interface
    CallBackwithUINT8 output_char_fnPtr;
};

uint16_t        crc16(uint8_t *data, uint8_t len);
euiHeader_t *   generate_header(uint8_t internal, uint8_t ack, uint8_t query, uint8_t offset_packet, uint8_t data_type, uint8_t msgID_len, uint8_t data_length, uint8_t sequence_num);
void            form_offset_packet_simple(CallBackwithUINT8 output_function, euiPacketSettings_t *settings, const char * msg_id, uint16_t offset_addr, uint8_t payload_len, void* payload);
void            write_packet(CallBackwithUINT8 output_function, euiHeader_t * header, const char * msg_id, uint16_t offset, void* payload);
void            parse_packet(uint8_t inbound_byte, struct eui_interface *active_interface);

#endif