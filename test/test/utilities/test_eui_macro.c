#include "unity.h"
 
// MODULE UNDER TEST
#include "eui_macro.h"
#include "mock_electricui.h"

// DEFINITIONS 
 
// PRIVATE TYPES
typedef struct {
  float x;
  float y;
  float z;
} macro_test_struct_t;
 
// PRIVATE FUNCTIONS
void macro_callback(void);
 
void macro_callback(void)
{
    //this is a test function...
}

// PRIVATE DATA
char        macro_char      = 'a';
int8_t      macro_int8      = -10;
uint8_t     macro_uint8     = 10;
int16_t     macro_int16     = -10;
uint16_t    macro_uint16    = 10;
int32_t     macro_int32     = -10;
uint32_t    macro_uint32    = 10;
float       macro_float     = 10.0f;
double      macro_double    = 10.0f;

char        macro_char_arr[4]       = "abc";
int8_t      macro_int8_arr[4]       = { -10 };
uint8_t     macro_uint8_arr[4]      = { 10 };
int16_t     macro_int16_arr[4]      = { -10 };
uint16_t    macro_uint16_arr[4]     = { 10 };
int32_t     macro_int32_arr[4]      = { -10 };
uint32_t    macro_uint32_arr[4]     = { 10 };
float       macro_float_arr[4]      = { 10.0 };
double      macro_double_arr[4]     = { 10.0 };

macro_test_struct_t macro_trifloat = { 10.0f, 100.0f, 1000.0f };

eui_message_t macro_object_array[] = {
    EUI_FUNC(   "func",     macro_callback ),

    EUI_CHAR(   "char",     macro_char ),
    EUI_INT8(   "int8",     macro_int8 ),
    EUI_INT16(  "int16",    macro_int16 ),
    EUI_INT32(  "int32",    macro_int32 ),
    EUI_UINT8(  "uint8",    macro_uint8 ),
    EUI_UINT16( "uint16",   macro_uint16 ),
    EUI_UINT32( "uint32",   macro_uint32 ),
    EUI_FLOAT(  "float",    macro_float ),
    EUI_DOUBLE( "double",   macro_double ),
    EUI_CUSTOM( "custom",   macro_trifloat ),

    EUI_CHAR_ARRAY(     "arr_char",     macro_char_arr ),
    EUI_INT8_ARRAY(     "arr_int8",     macro_int8_arr ),
    EUI_INT16_ARRAY(    "arr_int16",    macro_int16_arr ),
    EUI_INT32_ARRAY(    "arr_int32",    macro_int32_arr ),
    EUI_UINT8_ARRAY(    "arr_uint8",    macro_uint8_arr ),
    EUI_UINT16_ARRAY(   "arr_uint16",   macro_uint16_arr ),
    EUI_UINT32_ARRAY(   "arr_uint32",   macro_uint32_arr ),
    EUI_FLOAT_ARRAY(    "arr_float",    macro_float_arr ),
    EUI_DOUBLE_ARRAY(   "arr_double",   macro_double_arr ),
    EUI_CUSTOM_ARRAY(   "arr_custom",   macro_trifloat ),
};

eui_message_t macro_object_array_read_only[] = {
    EUI_CHAR_RO(    "char",     macro_char ),
    EUI_INT8_RO(    "int8",     macro_int8 ),
    EUI_INT16_RO(   "int16",    macro_int16 ),
    EUI_INT32_RO(   "int32",    macro_int32 ),
    EUI_UINT8_RO(   "uint8",    macro_uint8 ),
    EUI_UINT16_RO(  "uint16",   macro_uint16 ),
    EUI_UINT32_RO(  "uint32",   macro_uint32 ),
    EUI_FLOAT_RO(   "float",    macro_float ),
    EUI_DOUBLE_RO(  "double",   macro_double ),
    EUI_CUSTOM_RO(  "custom",   macro_trifloat ),

    EUI_CHAR_RO_ARRAY(      "arr_char",     macro_char_arr ),
    EUI_INT8_RO_ARRAY(      "arr_int8",     macro_int8_arr ),
    EUI_INT16_RO_ARRAY(     "arr_int16",    macro_int16_arr ),
    EUI_INT32_RO_ARRAY(     "arr_int32",    macro_int32_arr ),
    EUI_UINT8_RO_ARRAY(     "arr_uint8",    macro_uint8_arr ),
    EUI_UINT16_RO_ARRAY(    "arr_uint16",   macro_uint16_arr ),
    EUI_UINT32_RO_ARRAY(    "arr_uint32",   macro_uint32_arr ),
    EUI_FLOAT_RO_ARRAY(     "arr_float",    macro_float_arr ),
    EUI_DOUBLE_RO_ARRAY(    "arr_double",   macro_double_arr ),
    EUI_CUSTOM_RO_ARRAY(    "arr_custom",   macro_trifloat ),
};

eui_message_t expected_object_array[] = {
    { .id = "func",   .type = TYPE_CALLBACK|READ_ONLY_MASK,  .size = 1,      {.callback = &macro_callback}  },

    { .id = "char",   .type = TYPE_CHAR,     .size = 1,  {.payload = &macro_char}     },
    { .id = "int8",   .type = TYPE_INT8,     .size = 1,  {.payload = &macro_int8}     },
    { .id = "int16",  .type = TYPE_INT16,    .size = 2,  {.payload = &macro_int16}    },
    { .id = "int32",  .type = TYPE_INT32,    .size = 4,  {.payload = &macro_int32}    },
    { .id = "uint8",  .type = TYPE_UINT8,    .size = 1,  {.payload = &macro_uint8}    },
    { .id = "uint16", .type = TYPE_UINT16,   .size = 2,  {.payload = &macro_uint16}   },
    { .id = "uint32", .type = TYPE_UINT32,   .size = 4,  {.payload = &macro_uint32}   },
    { .id = "float",  .type = TYPE_FLOAT,    .size = 4,  {.payload = &macro_float}    },
    { .id = "double", .type = TYPE_DOUBLE,   .size = 8,  {.payload = &macro_double}   },
    { .id = "custom", .type = TYPE_CUSTOM,   .size = 12, {.payload = &macro_trifloat} },

    { .id = "arr_char",   .type = TYPE_CHAR,     .size = sizeof(macro_char_arr),     {.payload = &macro_char_arr}     },
    { .id = "arr_int8",   .type = TYPE_INT8,     .size = sizeof(macro_int8_arr),     {.payload = &macro_int8_arr}     },
    { .id = "arr_int16",  .type = TYPE_INT16,    .size = sizeof(macro_int16_arr),    {.payload = &macro_int16_arr}    },
    { .id = "arr_int32",  .type = TYPE_INT32,    .size = sizeof(macro_int32_arr),    {.payload = &macro_int32_arr}    },
    { .id = "arr_uint8",  .type = TYPE_UINT8,    .size = sizeof(macro_uint8_arr),    {.payload = &macro_uint8_arr}    },
    { .id = "arr_uint16", .type = TYPE_UINT16,   .size = sizeof(macro_uint16_arr),   {.payload = &macro_uint16_arr}   },
    { .id = "arr_uint32", .type = TYPE_UINT32,   .size = sizeof(macro_uint32_arr),   {.payload = &macro_uint32_arr}   },
    { .id = "arr_float",  .type = TYPE_FLOAT,    .size = sizeof(macro_float_arr),    {.payload = &macro_float_arr}    },
    { .id = "arr_double", .type = TYPE_DOUBLE,   .size = sizeof(macro_double_arr),   {.payload = &macro_double_arr}   },
    { .id = "arr_custom", .type = TYPE_CUSTOM,   .size = sizeof(macro_trifloat),     {.payload = &macro_trifloat}     },
};

eui_message_t expected_object_array_read_only[] = {
    { .id = "char",   .type = TYPE_CHAR|READ_ONLY_MASK,      .size = 1,  {.payload = &macro_char}     },
    { .id = "int8",   .type = TYPE_INT8|READ_ONLY_MASK,      .size = 1,  {.payload = &macro_int8}     },
    { .id = "int16",  .type = TYPE_INT16|READ_ONLY_MASK,     .size = 2,  {.payload = &macro_int16}    },
    { .id = "int32",  .type = TYPE_INT32|READ_ONLY_MASK,     .size = 4,  {.payload = &macro_int32}    },
    { .id = "uint8",  .type = TYPE_UINT8|READ_ONLY_MASK,     .size = 1,  {.payload = &macro_uint8}    },
    { .id = "uint16", .type = TYPE_UINT16|READ_ONLY_MASK,    .size = 2,  {.payload = &macro_uint16}   },
    { .id = "uint32", .type = TYPE_UINT32|READ_ONLY_MASK,    .size = 4,  {.payload = &macro_uint32}   },
    { .id = "float",  .type = TYPE_FLOAT|READ_ONLY_MASK,     .size = 4,  {.payload = &macro_float}    },
    { .id = "double", .type = TYPE_DOUBLE|READ_ONLY_MASK,    .size = 8,  {.payload = &macro_double}   },
    { .id = "custom", .type = TYPE_CUSTOM|READ_ONLY_MASK,    .size = 12, {.payload = &macro_trifloat} },

    { .id = "arr_char",   .type = TYPE_CHAR|READ_ONLY_MASK,  .size = sizeof(macro_char_arr),     {.payload = &macro_char_arr}     },
    { .id = "arr_int8",   .type = TYPE_INT8|READ_ONLY_MASK,  .size = sizeof(macro_int8_arr),     {.payload = &macro_int8_arr}     },
    { .id = "arr_int16",  .type = TYPE_INT16|READ_ONLY_MASK, .size = sizeof(macro_int16_arr),    {.payload = &macro_int16_arr}    },
    { .id = "arr_int32",  .type = TYPE_INT32|READ_ONLY_MASK, .size = sizeof(macro_int32_arr),    {.payload = &macro_int32_arr}    },
    { .id = "arr_uint8",  .type = TYPE_UINT8|READ_ONLY_MASK, .size = sizeof(macro_uint8_arr),    {.payload = &macro_uint8_arr}    },
    { .id = "arr_uint16", .type = TYPE_UINT16|READ_ONLY_MASK,.size = sizeof(macro_uint16_arr),   {.payload = &macro_uint16_arr}   },
    { .id = "arr_uint32", .type = TYPE_UINT32|READ_ONLY_MASK,.size = sizeof(macro_uint32_arr),   {.payload = &macro_uint32_arr}   },
    { .id = "arr_float",  .type = TYPE_FLOAT|READ_ONLY_MASK, .size = sizeof(macro_float_arr),    {.payload = &macro_float_arr}    },
    { .id = "arr_double", .type = TYPE_DOUBLE|READ_ONLY_MASK,.size = sizeof(macro_double_arr),   {.payload = &macro_double_arr}   },
    { .id = "arr_custom", .type = TYPE_CUSTOM|READ_ONLY_MASK,.size = sizeof(macro_trifloat),     {.payload = &macro_trifloat}     },
};

//test arrays for the array sizing macro
uint8_t no_array[0]         =   {   };
uint8_t small_array[1]      =   { 0 };
uint8_t standard_array[40]  =   { 0 };
uint8_t large_array[350]    =   { 0 };

// SETUP, TEARDOWN
 
void setUp(void)
{

}
 
void tearDown(void)
{

}

// TESTS

void test_array_element_count( void )
{
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0,   ARR_ELEM(no_array),         "Null array elements incorrectly counted." );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1,   ARR_ELEM(small_array),      "Small array elements incorrectly counted." );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 40,  ARR_ELEM(standard_array),   "Standard array elements incorrectly counted." );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 350, ARR_ELEM(large_array),      "Large array elements incorrectly counted." );

    TEST_ASSERT_EQUAL_INT_MESSAGE( 21, ARR_ELEM(macro_object_array),    "Macro array elements incorrectly counted." );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 21, ARR_ELEM(expected_object_array), "Ground-truth array elements incorrectly counted." );
}

void test_readwrite( void )
{
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(   expected_object_array, 
                                            macro_object_array, 
                                            sizeof(expected_object_array), 
                                            "Macro populated array != ground truth array"); 
}

void test_readonly( void )
{
    TEST_ASSERT_EQUAL_MEMORY_ARRAY_MESSAGE( expected_object_array_read_only, 
                                            macro_object_array_read_only, 
                                            sizeof(eui_message_t), 
                                            ARR_ELEM(expected_object_array_read_only), 
                                            "Read only macros != ground truth array"); 
}
