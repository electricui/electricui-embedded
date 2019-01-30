#ifndef EUI_SERIAL_TRANSPORT_H
#define EUI_SERIAL_TRANSPORT_H

#include <stdint.h>
#include "../eui_types.h"
#include "eui_transport_types.h"

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

uint8_t
encode_header( eui_header_t *header, uint8_t *buffer );

uint8_t
encode_framing( uint8_t *buffer, uint16_t size );

uint8_t
encode_packet_simple(   callback_data_out_t output_function,
                        eui_pkt_settings_t  *settings,
                        const char          *msg_id,
                        uint16_t            payload_len,
                        void*               payload);

uint8_t
encode_packet(  callback_data_out_t out_char,
                eui_header_t        *header,
                const char          *msg_id,
                uint16_t            offset,
                void*               payload );

uint8_t
decode_packet(uint8_t inbound_byte, eui_packet_t *p_link_in);

uint8_t
parse_decoded_packet(uint8_t byte_in, eui_packet_t *p_link_in);

#endif //end EUI_SERIAL_TRANSPORT_H