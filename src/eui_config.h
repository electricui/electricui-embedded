/* Copyright (c) 2016-2019 Electric UI
 * MIT Licenced - see LICENCE for details.
 *
 * 
 */

#ifndef EUI_CONF_H
#define EUI_CONF_H

#include <stdint.h>

// Increase supported variable count to 65535
// Default 255 message support is probably enough for most people

// #define EUI_CONF_MANY_VARIABLES


// Disable offset message functionality

// #define EUI_CONF_OFFSETS_DISABLED

// Default payload size for the inbound parsing buffer is 120 bytes.
// Manually reduce or increase the size as shown below.

//#define PAYLOAD_SIZE_MAX 512

#ifndef EUI_OVERRIDE_DEFAULT_PROTOCOL
    #include "transports/eui_serial_transport.h"
    #define eui_crc(DATA, CRC)  crc16(DATA, CRC)
    #define eui_encode_simple(OUTFN, SETTINGS, MSGID, LENGTH, PAYLOAD)  encode_packet_simple(OUTFN, SETTINGS, MSGID, LENGTH, PAYLOAD)
    #define eui_encode(OUTFN, HEADER, MSGID, OFFSET, PAYLOAD)  encode_packet(OUTFN, HEADER, MSGID, OFFSET, PAYLOAD)
    #define eui_decode(DATA, INTERFACE)  decode_packet(DATA, INTERFACE)
#endif


#define READ_ONLY_MASK 0x80u
#define READ_ONLY_FLAG 0x01u
#define WRITABLE_FLAG 0x00u

#define MSG_INTERNAL        1u
#define MSG_DEV             0u
#define MSG_RESP            1u
#define MSG_NRESP           0u
#define MSG_OFFSET_PACKET   1u
#define MSG_STANDARD_PACKET 0u

// Configure internal message ID strings
#define EUI_INTERNAL_LIB_VER    "o"
#define EUI_INTERNAL_BOARD_ID   "i"
#define EUI_INTERNAL_HEARTBEAT  "h"

// Used for variable sync during first connection
#define EUI_INTERNAL_AM      "t" //announce writable ID's 
#define EUI_INTERNAL_AM_LIST "u" //delimit writable ID
#define EUI_INTERNAL_AM_END  "v" //end of writable ID's
#define EUI_INTERNAL_AV      "w" //send writable variables

#endif //end EUI_CONF