/* Copyright (c) 2016-2020 Electric UI
 * MIT Licenced - see LICENCE for details.
 *
 * 
 */

#ifndef EUI_MACRO_H
#define EUI_MACRO_H

/**
 * @file eui_macro.h
 * @brief Macros to simplify variable and interface instantiation
 *
 */

#define EUI_ARR_ELEM(a) (sizeof(a) / sizeof(*a))    //number of elements in array

#define EUI_TRACK( INPUT_ARRAY )    ( eui_setup_tracked(INPUT_ARRAY, EUI_ARR_ELEM(INPUT_ARRAY)) )
#define EUI_LINK( INTERFACE_ARRAY ) ( eui_setup_interfaces(INTERFACE_ARRAY, EUI_ARR_ELEM(INTERFACE_ARRAY)) )

// Helper macro to statically init a eui_packet_t, used in eui_interface_t static init
// as recursive sub-struct {0} isn't cross-compiler compatible
#ifndef EUI_CONF_OFFSETS_DISABLED
#define EUI_PACKET_EMPTY { {0}, {0}, {0}, 0, 0, {0} }
#else
#define EUI_PACKET_EMPTY { {0}, {0}, {0}, 0 , {0} }
#endif

#define EUI_INTERFACE( OUTPUT_PTR ) {               .packet = EUI_PACKET_EMPTY, .output_cb = OUTPUT_PTR, .interface_cb = 0       }
#define EUI_INTERFACE_CB( OUTPUT_PTR, DEV_CB ) {    .packet = EUI_PACKET_EMPTY, .output_cb = OUTPUT_PTR, .interface_cb = DEV_CB  }

//Helper macros to simplify eUI object array declaration in user-code
#define EUI_FUNC(   ID, CB ) { .id = ID, .type = TYPE_CALLBACK|READ_ONLY_MASK, .size = 0, {.callback = &CB} }

// Typed RW Variables
#define EUI_CUSTOM( ID, DATA ) { .id = ID, .type = TYPE_CUSTOM, .size = sizeof(DATA),    {.data = &DATA} }
#define EUI_CHAR(   ID, DATA ) { .id = ID, .type = TYPE_CHAR,   .size = 1,               {.data = &DATA} }
#define EUI_INT8(   ID, DATA ) { .id = ID, .type = TYPE_INT8,   .size = 1,               {.data = &DATA} }
#define EUI_INT16(  ID, DATA ) { .id = ID, .type = TYPE_INT16,  .size = 2,               {.data = &DATA} }
#define EUI_INT32(  ID, DATA ) { .id = ID, .type = TYPE_INT32,  .size = 4,               {.data = &DATA} }
#define EUI_UINT8(  ID, DATA ) { .id = ID, .type = TYPE_UINT8,  .size = 1,               {.data = &DATA} }
#define EUI_UINT16( ID, DATA ) { .id = ID, .type = TYPE_UINT16, .size = 2,               {.data = &DATA} }
#define EUI_UINT32( ID, DATA ) { .id = ID, .type = TYPE_UINT32, .size = 4,               {.data = &DATA} }
#define EUI_FLOAT(  ID, DATA ) { .id = ID, .type = TYPE_FLOAT,  .size = 4,               {.data = &DATA} }
#define EUI_DOUBLE( ID, DATA ) { .id = ID, .type = TYPE_DOUBLE, .size = 8,               {.data = &DATA} }

// Read Only Variables
#define EUI_CUSTOM_RO( ID, DATA ) { .id = ID, .type = TYPE_CUSTOM|READ_ONLY_MASK, .size = sizeof(DATA), {.data = &DATA} }
#define EUI_CHAR_RO(   ID, DATA ) { .id = ID, .type = TYPE_CHAR|READ_ONLY_MASK,   .size = 1, {.data = &DATA} }
#define EUI_INT8_RO(   ID, DATA ) { .id = ID, .type = TYPE_INT8|READ_ONLY_MASK,   .size = 1, {.data = &DATA} }
#define EUI_INT16_RO(  ID, DATA ) { .id = ID, .type = TYPE_INT16|READ_ONLY_MASK,  .size = 2, {.data = &DATA} }
#define EUI_INT32_RO(  ID, DATA ) { .id = ID, .type = TYPE_INT32|READ_ONLY_MASK,  .size = 4, {.data = &DATA} }
#define EUI_UINT8_RO(  ID, DATA ) { .id = ID, .type = TYPE_UINT8|READ_ONLY_MASK,  .size = 1, {.data = &DATA} }
#define EUI_UINT16_RO( ID, DATA ) { .id = ID, .type = TYPE_UINT16|READ_ONLY_MASK, .size = 2, {.data = &DATA} }
#define EUI_UINT32_RO( ID, DATA ) { .id = ID, .type = TYPE_UINT32|READ_ONLY_MASK, .size = 4, {.data = &DATA} }
#define EUI_FLOAT_RO(  ID, DATA ) { .id = ID, .type = TYPE_FLOAT|READ_ONLY_MASK,  .size = 4, {.data = &DATA} }
#define EUI_DOUBLE_RO( ID, DATA ) { .id = ID, .type = TYPE_DOUBLE|READ_ONLY_MASK, .size = 8, {.data = &DATA} }

// Array of typed variables
#define EUI_CUSTOM_ARRAY( ID, DATA ) { .id = ID, .type = TYPE_CUSTOM, .size = sizeof(DATA),  {.data = &DATA} }
#define EUI_CHAR_ARRAY(   ID, DATA ) { .id = ID, .type = TYPE_CHAR,   .size = sizeof(DATA),  {.data = &DATA} }
#define EUI_INT8_ARRAY(   ID, DATA ) { .id = ID, .type = TYPE_INT8,   .size = sizeof(DATA),  {.data = &DATA} }
#define EUI_INT16_ARRAY(  ID, DATA ) { .id = ID, .type = TYPE_INT16,  .size = sizeof(DATA),  {.data = &DATA} }
#define EUI_INT32_ARRAY(  ID, DATA ) { .id = ID, .type = TYPE_INT32,  .size = sizeof(DATA),  {.data = &DATA} }
#define EUI_UINT8_ARRAY(  ID, DATA ) { .id = ID, .type = TYPE_UINT8,  .size = sizeof(DATA),  {.data = &DATA} }
#define EUI_UINT16_ARRAY( ID, DATA ) { .id = ID, .type = TYPE_UINT16, .size = sizeof(DATA),  {.data = &DATA} }
#define EUI_UINT32_ARRAY( ID, DATA ) { .id = ID, .type = TYPE_UINT32, .size = sizeof(DATA),  {.data = &DATA} }
#define EUI_FLOAT_ARRAY(  ID, DATA ) { .id = ID, .type = TYPE_FLOAT,  .size = sizeof(DATA),  {.data = &DATA} }
#define EUI_DOUBLE_ARRAY( ID, DATA ) { .id = ID, .type = TYPE_DOUBLE, .size = sizeof(DATA),  {.data = &DATA} }

// Array of read-only typed variables
#define EUI_CUSTOM_ARRAY_RO( ID, DATA ) { .id = ID, .type = TYPE_CUSTOM|READ_ONLY_MASK, .size = sizeof(DATA), {.data = &DATA} }
#define EUI_CHAR_ARRAY_RO(   ID, DATA ) { .id = ID, .type = TYPE_CHAR|READ_ONLY_MASK,   .size = sizeof(DATA), {.data = &DATA} }
#define EUI_INT8_ARRAY_RO(   ID, DATA ) { .id = ID, .type = TYPE_INT8|READ_ONLY_MASK,   .size = sizeof(DATA), {.data = &DATA} }
#define EUI_INT16_ARRAY_RO(  ID, DATA ) { .id = ID, .type = TYPE_INT16|READ_ONLY_MASK,  .size = sizeof(DATA), {.data = &DATA} }
#define EUI_INT32_ARRAY_RO(  ID, DATA ) { .id = ID, .type = TYPE_INT32|READ_ONLY_MASK,  .size = sizeof(DATA), {.data = &DATA} }
#define EUI_UINT8_ARRAY_RO(  ID, DATA ) { .id = ID, .type = TYPE_UINT8|READ_ONLY_MASK,  .size = sizeof(DATA), {.data = &DATA} }
#define EUI_UINT16_ARRAY_RO( ID, DATA ) { .id = ID, .type = TYPE_UINT16|READ_ONLY_MASK, .size = sizeof(DATA), {.data = &DATA} }
#define EUI_UINT32_ARRAY_RO( ID, DATA ) { .id = ID, .type = TYPE_UINT32|READ_ONLY_MASK, .size = sizeof(DATA), {.data = &DATA} }
#define EUI_FLOAT_ARRAY_RO(  ID, DATA ) { .id = ID, .type = TYPE_FLOAT|READ_ONLY_MASK,  .size = sizeof(DATA), {.data = &DATA} }
#define EUI_DOUBLE_ARRAY_RO( ID, DATA ) { .id = ID, .type = TYPE_DOUBLE|READ_ONLY_MASK, .size = sizeof(DATA), {.data = &DATA} }

#endif //end EUI_MACRO_H