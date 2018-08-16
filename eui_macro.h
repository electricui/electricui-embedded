#include "eui_serial_transport.h"

#define EUI_FUNC(   ID, DATA ) { .msgID = ID, .type = TYPE_CALLBACK, .size = 1, .payload = &DATA }
#define EUI_CUSTOM( ID, DATA ) { .msgID = ID, .type = TYPE_CUSTOM, .size = sizeof(DATA), .payload = &DATA }

#define EUI_CHAR(  ID, DATA ) { .msgID = ID, .type = TYPE_CHAR,  .size = sizeof(DATA), .payload = &DATA }
#define EUI_INT8(  ID, DATA ) { .msgID = ID, .type = TYPE_INT8,  .size = sizeof(DATA), .payload = &DATA }
#define EUI_INT16( ID, DATA ) { .msgID = ID, .type = TYPE_INT16, .size = sizeof(DATA), .payload = &DATA }
#define EUI_INT32( ID, DATA ) { .msgID = ID, .type = TYPE_INT16, .size = sizeof(DATA), .payload = &DATA }
#define EUI_UINT8(  ID, DATA ) { .msgID = ID, .type = TYPE_UINT8,  .size = sizeof(DATA), .payload = &DATA }
#define EUI_UINT16( ID, DATA ) { .msgID = ID, .type = TYPE_UINT16, .size = sizeof(DATA), .payload = &DATA }
#define EUI_UINT32( ID, DATA ) { .msgID = ID, .type = TYPE_UINT16, .size = sizeof(DATA), .payload = &DATA }
#define EUI_FLOAT(  ID, DATA ) { .msgID = ID, .type = TYPE_FLOAT,  .size = sizeof(DATA), .payload = &DATA }
#define EUI_DOUBLE( ID, DATA ) { .msgID = ID, .type = TYPE_DOUBLE, .size = sizeof(DATA), .payload = &DATA }
