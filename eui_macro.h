#include "eui_serial_transport.h"

#define ARR_ELEM(a) (sizeof(a) / sizeof(*a))    //number of elements in array

#define EUI_TRACK( INPUT_ARRAY ) ( setup_dev_msg(INPUT_ARRAY, ARR_ELEM(INPUT_ARRAY)) )
#define EUI_LINK( INTERFACE_ARRAY ) ( setup_interface(INTERFACE_ARRAY, ARR_ELEM(INTERFACE_ARRAY)) )

#define EUI_INTERFACE( OUTPUT_PTR ) { .packet = { 0 }, .output_func = OUTPUT_PTR, .interface_cb = 0 }
#define EUI_INTERFACE_CB( OUTPUT_PTR, DEV_CB ) { .packet = { 0 }, .output_func = OUTPUT_PTR, .interface_cb = DEV_CB }


//Helper macros to simplify eUI object array declaration in user-code
#define EUI_FUNC(   ID, DATA ) { .msgID = ID, .type = TYPE_CALLBACK|READ_ONLY_MASK, .size = CALLBACK_SIZE, .payload = &DATA }

#define EUI_CUSTOM( ID, DATA ) { .msgID = ID, .type = TYPE_CUSTOM, .size = sizeof(DATA), .payload = &DATA,  .callback = 0 }
#define EUI_CHAR(   ID, DATA ) { .msgID = ID, .type = TYPE_CHAR,   .size = sizeof(DATA), .payload = &DATA,  .callback = 0 }
#define EUI_INT8(   ID, DATA ) { .msgID = ID, .type = TYPE_INT8,   .size = sizeof(DATA), .payload = &DATA,  .callback = 0 }
#define EUI_INT16(  ID, DATA ) { .msgID = ID, .type = TYPE_INT16,  .size = sizeof(DATA), .payload = &DATA,  .callback = 0 }
#define EUI_INT32(  ID, DATA ) { .msgID = ID, .type = TYPE_INT32,  .size = sizeof(DATA), .payload = &DATA,  .callback = 0 }
#define EUI_UINT8(  ID, DATA ) { .msgID = ID, .type = TYPE_UINT8,  .size = sizeof(DATA), .payload = &DATA,  .callback = 0 }
#define EUI_UINT16( ID, DATA ) { .msgID = ID, .type = TYPE_UINT16, .size = sizeof(DATA), .payload = &DATA,  .callback = 0 }
#define EUI_UINT32( ID, DATA ) { .msgID = ID, .type = TYPE_UINT32, .size = sizeof(DATA), .payload = &DATA,  .callback = 0 }
#define EUI_FLOAT(  ID, DATA ) { .msgID = ID, .type = TYPE_FLOAT,  .size = sizeof(DATA), .payload = &DATA,  .callback = 0 }
#define EUI_DOUBLE( ID, DATA ) { .msgID = ID, .type = TYPE_DOUBLE, .size = sizeof(DATA), .payload = &DATA,  .callback = 0 }

// Read Only Variables
#define EUI_CUSTOM_RO( ID, DATA ) { .msgID = ID, .type = TYPE_CUSTOM|READ_ONLY_MASK, .size = sizeof(DATA), .payload = &DATA,    .callback = 0 }
#define EUI_CHAR_RO(   ID, DATA ) { .msgID = ID, .type = TYPE_CHAR|READ_ONLY_MASK,   .size = sizeof(DATA), .payload = &DATA,    .callback = 0 }
#define EUI_INT8_RO(   ID, DATA ) { .msgID = ID, .type = TYPE_INT8|READ_ONLY_MASK,   .size = sizeof(DATA), .payload = &DATA,    .callback = 0 }
#define EUI_INT16_RO(  ID, DATA ) { .msgID = ID, .type = TYPE_INT16|READ_ONLY_MASK,  .size = sizeof(DATA), .payload = &DATA,    .callback = 0 }
#define EUI_INT32_RO(  ID, DATA ) { .msgID = ID, .type = TYPE_INT32|READ_ONLY_MASK,  .size = sizeof(DATA), .payload = &DATA,    .callback = 0 }
#define EUI_UINT8_RO(  ID, DATA ) { .msgID = ID, .type = TYPE_UINT8|READ_ONLY_MASK,  .size = sizeof(DATA), .payload = &DATA,    .callback = 0 }
#define EUI_UINT16_RO( ID, DATA ) { .msgID = ID, .type = TYPE_UINT16|READ_ONLY_MASK, .size = sizeof(DATA), .payload = &DATA,    .callback = 0 }
#define EUI_UINT32_RO( ID, DATA ) { .msgID = ID, .type = TYPE_UINT32|READ_ONLY_MASK, .size = sizeof(DATA), .payload = &DATA,    .callback = 0 }
#define EUI_FLOAT_RO(  ID, DATA ) { .msgID = ID, .type = TYPE_FLOAT|READ_ONLY_MASK,  .size = sizeof(DATA), .payload = &DATA,    .callback = 0 }
#define EUI_DOUBLE_RO( ID, DATA ) { .msgID = ID, .type = TYPE_DOUBLE|READ_ONLY_MASK, .size = sizeof(DATA), .payload = &DATA,    .callback = 0 }

// Variables with additional callback
#define EUI_CUSTOM_CB( ID, DATA, DEV_CB ) { .msgID = ID, .type = TYPE_CUSTOM, .size = sizeof(DATA), .payload = &DATA,   .callback = DEV_CB }
#define EUI_CHAR_CB(   ID, DATA, DEV_CB ) { .msgID = ID, .type = TYPE_CHAR,   .size = sizeof(DATA), .payload = &DATA,   .callback = DEV_CB }
#define EUI_INT8_CB(   ID, DATA, DEV_CB ) { .msgID = ID, .type = TYPE_INT8,   .size = sizeof(DATA), .payload = &DATA,   .callback = DEV_CB }
#define EUI_INT16_CB(  ID, DATA, DEV_CB ) { .msgID = ID, .type = TYPE_INT16,  .size = sizeof(DATA), .payload = &DATA,   .callback = DEV_CB }
#define EUI_INT32_CB(  ID, DATA, DEV_CB ) { .msgID = ID, .type = TYPE_INT32,  .size = sizeof(DATA), .payload = &DATA,   .callback = DEV_CB }
#define EUI_UINT8_CB(  ID, DATA, DEV_CB ) { .msgID = ID, .type = TYPE_UINT8,  .size = sizeof(DATA), .payload = &DATA,   .callback = DEV_CB }
#define EUI_UINT16_CB( ID, DATA, DEV_CB ) { .msgID = ID, .type = TYPE_UINT16, .size = sizeof(DATA), .payload = &DATA,   .callback = DEV_CB }
#define EUI_UINT32_CB( ID, DATA, DEV_CB ) { .msgID = ID, .type = TYPE_UINT32, .size = sizeof(DATA), .payload = &DATA,   .callback = DEV_CB }
#define EUI_FLOAT_CB(  ID, DATA, DEV_CB ) { .msgID = ID, .type = TYPE_FLOAT,  .size = sizeof(DATA), .payload = &DATA,   .callback = DEV_CB }
#define EUI_DOUBLE_CB( ID, DATA, DEV_CB ) { .msgID = ID, .type = TYPE_DOUBLE, .size = sizeof(DATA), .payload = &DATA,   .callback = DEV_CB }
