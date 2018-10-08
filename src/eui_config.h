#ifndef EUI_CONF_H
#define EUI_CONF_H

#include <stdint.h>

// Add the ability to provide a callback pointer to each tracked variable

// #define EUI_CONF_VARIABLE_CALLBACKS


// Increase supported variable count to 65535
// Default 255 message support is probably enough for most people

// #define EUI_CONF_MANY_VARIABLES


// Disable offset message functionality

// #define EUI_CONF_OFFSETS_DISABLED


// The protocol limits ID lengths to a maximum of 15 characters
// Manually define a maximum here to reduce buffer size usage

//#define MESSAGEID_SIZE_MAX 12


// Default payload size for the inbound parsing buffer is 120 bytes.
// Manually reduce or increase the size as shown below.

//#define PAYLOAD_SIZE_MAX 512

// Disable outbound message queuing

// #define EUI_CONF_COOPERATIVE_DISABLE



#ifndef EUI_OVERRIDE_DEFAULT_PROTOCOL
	#include "eui_serial_transport.h"
    #define eui_crc(DATA, CRC)  crc16(DATA, CRC)
    #define eui_encode_simple(OUTFN, SETTINGS, MSGID, LENGTH, PAYLOAD)  encode_packet_simple(OUTFN, SETTINGS, MSGID, LENGTH, PAYLOAD)
    #define eui_encode(OUTFN, HEADER, MSGID, OFFSET, PAYLOAD)  encode_packet(OUTFN, HEADER, MSGID, OFFSET, PAYLOAD)
    #define eui_decode(DATA, INTERFACE)  decode_packet(DATA, INTERFACE)
#endif

#ifdef EUI_CONF_MANY_VARIABLES
    typedef uint16_t euiVariableCount_t;
#else
    typedef uint8_t euiVariableCount_t;
#endif

#define READ_ONLY_MASK 0x80
#define READ_ONLY_FLAG 0x01
#define WRITABLE_FLAG 0x00

#define MSG_INTERNAL        1
#define MSG_DEV             0
#define MSG_RESP            1
#define MSG_NRESP           0
#define MSG_OFFSET_PACKET   1
#define MSG_STANDARD_PACKET 0

// Default supported message types
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

// Configure internal message ID strings
#define EUI_INTERNAL_LIB_VER	"o"
#define EUI_INTERNAL_BOARD_ID	"i"
#define EUI_INTERNAL_SESSION_ID	"j"
#define EUI_INTERNAL_HEARTBEAT	"h"
#define EUI_DEFAULT_INTERFACE	"k"

#define EUI_INTERNAL_SEARCH		"x"	//preliminary handshake 

// Used for variable sync during first connection
#define EUI_INTERNAL_AM_RO		"p"	//announce readonly ID's 
#define EUI_INTERNAL_AV_RO		"r" //send readonly variables
#define EUI_INTERNAL_AM_RO_LIST	"q"	//delimit readonly ID
#define EUI_INTERNAL_AM_RO_END	"s"	//end of readonly ID's
#define EUI_INTERNAL_AM_RW		"t"	//announce writable ID's 
#define EUI_INTERNAL_AM_RW_LIST	"u" //delimit writable ID
#define EUI_INTERNAL_AM_RW_END	"v" //end of writable ID's
#define EUI_INTERNAL_AV_RW		"w"	//send writable variables

#endif //end EUI_CONF