#include "eui_serial_transport.h"

#define EUI_CUSTOM( ID, SIZE, DATA ) { .msgID = ID, .type = TYPE_CUSTOM, .size = SIZE, .payload = DATA }

#define EUI_FUNC(   ID, DATA ) { .msgID = ID, .type = TYPE_CALLBACK, .size = 1, .payload = DATA }

#define EUI_CHAR(  ID, DATA ) { .msgID = ID, .type = TYPE_CHAR,  .size = 1, .payload = DATA }
#define EUI_INT8(  ID, DATA ) { .msgID = ID, .type = TYPE_INT8,  .size = 1, .payload = DATA }
#define EUI_INT16( ID, DATA ) { .msgID = ID, .type = TYPE_INT16, .size = 2, .payload = DATA }
#define EUI_INT32( ID, DATA ) { .msgID = ID, .type = TYPE_INT16, .size = 4, .payload = DATA }
#define EUI_UINT8(  ID, DATA ) { .msgID = ID, .type = TYPE_UINT8,  .size = 1, .payload = DATA }
#define EUI_UINT16( ID, DATA ) { .msgID = ID, .type = TYPE_UINT16, .size = 2, .payload = DATA }
#define EUI_UINT32( ID, DATA ) { .msgID = ID, .type = TYPE_UINT16, .size = 4, .payload = DATA }
#define EUI_FLOAT(  ID, DATA ) { .msgID = ID, .type = TYPE_FLOAT,  .size = 4, .payload = DATA }
#define EUI_DOUBLE( ID, DATA ) { .msgID = ID, .type = TYPE_DOUBLE, .size = 8, .payload = DATA }

#define EUI_CHAR_ARRAY(  ID, DATA ) { .msgID = ID, .type = TYPE_CHAR,    .size = sizeof(DATA), .payload = DATA }
#define EUI_INT8_ARRAY(  ID, DATA ) { .msgID = ID, .type = TYPE_INT8,    .size = sizeof(DATA), .payload = DATA }
#define EUI_INT16_ARRAY( ID, DATA ) { .msgID = ID, .type = TYPE_INT16,   .size = sizeof(DATA), .payload = DATA }
#define EUI_INT32_ARRAY( ID, DATA ) { .msgID = ID, .type = TYPE_INT16,   .size = sizeof(DATA), .payload = DATA }
#define EUI_UINT8_ARRAY(  ID, DATA ) { .msgID = ID, .type = TYPE_UINT8,  .size = sizeof(DATA), .payload = DATA }
#define EUI_UINT16_ARRAY( ID, DATA ) { .msgID = ID, .type = TYPE_UINT16, .size = sizeof(DATA), .payload = DATA }
#define EUI_UINT32_ARRAY( ID, DATA ) { .msgID = ID, .type = TYPE_UINT16, .size = sizeof(DATA), .payload = DATA }
#define EUI_FLOAT_ARRAY(  ID, DATA ) { .msgID = ID, .type = TYPE_FLOAT,  .size = sizeof(DATA), .payload = DATA }
#define EUI_DOUBLE_ARRAY( ID, DATA ) { .msgID = ID, .type = TYPE_DOUBLE, .size = sizeof(DATA), .payload = DATA }

