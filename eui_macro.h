#include "eui_serial_transport.h"

#define ARR_ELEM(a) (sizeof(a) / sizeof(*a))    //number of elements in array

#define EUI_TRACK( INPUT_ARRAY ) ( setup_dev_msg(INPUT_ARRAY, ARR_ELEM(INPUT_ARRAY)) )
#define EUI_LINK( INTERFACE_ARRAY ) ( setup_interface(INTERFACE_ARRAY, ARR_ELEM(INTERFACE_ARRAY)) )

#define EUI_INTERFACE( OUTPUT_PTR ) { .parser = { 0 }, .output_func = OUTPUT_PTR, .interface_cb = 0 }


//Helper macros to simplify eUI object array declaration in user-code
#define EUI_FUNC(   ID, DATA ) { .msgID = ID, .type = TYPE_CALLBACK|READ_ONLY_MASK, .size = CALLBACK_SIZE, .payload = &DATA }
#define EUI_CUSTOM( ID, DATA ) { .msgID = ID, .type = TYPE_CUSTOM, .size = sizeof(DATA), .payload = &DATA }

#define EUI_CHAR(  ID, DATA ) { .msgID = ID, .type = TYPE_CHAR,  .size = sizeof(DATA), .payload = &DATA }
#define EUI_INT8(  ID, DATA ) { .msgID = ID, .type = TYPE_INT8,  .size = sizeof(DATA), .payload = &DATA }
#define EUI_INT16( ID, DATA ) { .msgID = ID, .type = TYPE_INT16, .size = sizeof(DATA), .payload = &DATA }
#define EUI_INT32( ID, DATA ) { .msgID = ID, .type = TYPE_INT32, .size = sizeof(DATA), .payload = &DATA }
#define EUI_UINT8(  ID, DATA ) { .msgID = ID, .type = TYPE_UINT8,  .size = sizeof(DATA), .payload = &DATA }
#define EUI_UINT16( ID, DATA ) { .msgID = ID, .type = TYPE_UINT16, .size = sizeof(DATA), .payload = &DATA }
#define EUI_UINT32( ID, DATA ) { .msgID = ID, .type = TYPE_UINT32, .size = sizeof(DATA), .payload = &DATA }
#define EUI_FLOAT(  ID, DATA ) { .msgID = ID, .type = TYPE_FLOAT,  .size = sizeof(DATA), .payload = &DATA }
#define EUI_DOUBLE( ID, DATA ) { .msgID = ID, .type = TYPE_DOUBLE, .size = sizeof(DATA), .payload = &DATA }

#define EUI_RO_CUSTOM( ID, DATA ) { .msgID = ID, .type = TYPE_CUSTOM|READ_ONLY_MASK, .size = sizeof(DATA), .payload = &DATA }

#define EUI_RO_CHAR(   ID, DATA ) { .msgID = ID, .type = TYPE_CHAR|READ_ONLY_MASK,   .size = sizeof(DATA), .payload = &DATA }
#define EUI_RO_INT8(   ID, DATA ) { .msgID = ID, .type = TYPE_INT8|READ_ONLY_MASK,   .size = sizeof(DATA), .payload = &DATA }
#define EUI_RO_INT16(  ID, DATA ) { .msgID = ID, .type = TYPE_INT16|READ_ONLY_MASK,  .size = sizeof(DATA), .payload = &DATA }
#define EUI_RO_INT32(  ID, DATA ) { .msgID = ID, .type = TYPE_INT32|READ_ONLY_MASK,  .size = sizeof(DATA), .payload = &DATA }
#define EUI_RO_UINT8(  ID, DATA ) { .msgID = ID, .type = TYPE_UINT8|READ_ONLY_MASK,  .size = sizeof(DATA), .payload = &DATA }
#define EUI_RO_UINT16( ID, DATA ) { .msgID = ID, .type = TYPE_UINT16|READ_ONLY_MASK, .size = sizeof(DATA), .payload = &DATA }
#define EUI_RO_UINT32( ID, DATA ) { .msgID = ID, .type = TYPE_UINT32|READ_ONLY_MASK, .size = sizeof(DATA), .payload = &DATA }
#define EUI_RO_FLOAT(  ID, DATA ) { .msgID = ID, .type = TYPE_FLOAT|READ_ONLY_MASK,  .size = sizeof(DATA), .payload = &DATA }
#define EUI_RO_DOUBLE( ID, DATA ) { .msgID = ID, .type = TYPE_DOUBLE|READ_ONLY_MASK, .size = sizeof(DATA), .payload = &DATA }
