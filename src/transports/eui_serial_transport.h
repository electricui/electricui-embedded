#ifndef EUI_SERIAL_TRANSPORT_H
#define EUI_SERIAL_TRANSPORT_H

#include "../eui_config.h"

#define PACKET_BASE_SIZE    ( sizeof(eui_header_t) \
                            + sizeof(uint16_t) \
                            + sizeof(uint16_t) )

#define MESSAGEID_BITS      4

#ifndef MESSAGEID_SIZE_MAX
    #define MESSAGEID_SIZE    ( 1 << MESSAGEID_BITS )
#endif

#ifndef PAYLOAD_SIZE_MAX
    #define PAYLOAD_SIZE_MAX  120 //default inbound buffer size
#endif

typedef struct {
    unsigned data_len   : 10;
    unsigned type       : 4;
    unsigned internal   : 1;
    unsigned offset     : 1;
    unsigned id_len     : MESSAGEID_BITS;
    unsigned response   : 1;
    unsigned acknum     : 3;
} eui_header_t;

typedef struct {
    unsigned internal   : 1;
    unsigned response   : 1;
    unsigned type       : 4;
} eui_pkt_settings_t;

typedef void (*callback_uint8_t)(uint8_t);

typedef struct {
    unsigned state          : 4;
    unsigned id_bytes_in    : MESSAGEID_BITS;
    unsigned data_bytes_in  : 10;
    uint8_t  frame_offset;
} eui_parser_state_t;

enum parseStates {
        exp_frame_offset = 0,
        exp_header_b1,
        exp_header_b2,
        exp_header_b3,   
        exp_message_id,
        exp_offset_b1,
        exp_offset_b2,
        exp_data,
        exp_crc_b1,
        exp_crc_b2,
};

//hold the inbound packet information
typedef struct {
        eui_parser_state_t parser;

        //buffer incoming data
        eui_header_t header;
        uint8_t     msgid_in[MESSAGEID_SIZE];
#ifndef EUI_CONF_OFFSETS_DISABLED
        uint16_t    offset_in;
#endif
        uint8_t     data_in[PAYLOAD_SIZE_MAX];
        uint16_t    crc_in;

} eui_packet_t;

enum packet_signals {
    parser_idle = 0,
    parser_complete,
    parser_error,
};

uint8_t
encode_header( eui_header_t *header, uint8_t *buffer );

uint8_t
encode_framing( uint8_t *buffer, uint16_t size );

uint8_t
encode_packet_simple(callback_uint8_t output_function, eui_pkt_settings_t *settings, const char * msg_id, uint16_t payload_len, void* payload);

uint8_t
encode_packet(callback_uint8_t out_char, eui_header_t * header, const char * msg_id, uint16_t offset, void* payload);

uint8_t
decode_packet(uint8_t inbound_byte, eui_packet_t *p_link_in);

uint8_t
parse_decoded_packet(uint8_t byte_in, eui_packet_t *p_link_in);

#endif