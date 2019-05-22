#ifndef EUI_TRANSPORT_TYPES_H
#define EUI_TRANSPORT_TYPES_H

#include <stdint.h>
#include "../eui_types.h"

#define PACKET_BASE_SIZE    ( sizeof(eui_header_t) \
                            + sizeof(uint16_t) \
                            + sizeof(uint16_t) )

#ifndef PAYLOAD_SIZE_MAX
    #define PAYLOAD_SIZE_MAX  120   //default inbound buffer size
#endif

#define MSGID_SIZE 16

typedef struct {
    unsigned state          : 4;
    unsigned id_bytes_in    : 4;
    unsigned data_bytes_in  : 10;
    uint8_t  frame_offset;
} eui_parser_state_t;

//inbound packet buffer and state information
typedef struct {
    eui_parser_state_t parser;
    eui_header_t header;
    uint8_t     id_in[15];

#ifndef EUI_CONF_OFFSETS_DISABLED
    uint16_t    offset_in;
#endif
    
    uint8_t     data_in[PAYLOAD_SIZE_MAX];
    uint16_t    crc_in;
} eui_packet_t;

#endif //end EUI_TRANSPORT_TYPES_H
