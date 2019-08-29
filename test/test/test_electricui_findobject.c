#include "unity.h"
#include <string.h>

// MODULE UNDER TEST
#include "electricui.h"
#include "electricui_private.h"
#include "mock_eui_serial_transport.h"
#include "mock_eui_crc.h"
#include "mock_eui_offset_validation.h"

// DEFINITIONS 
 
// PRIVATE TYPES
 
// PRIVATE DATA
char      testfind_char    = 'a';
int8_t    testfind_int8    = -21;
uint8_t   testfind_uint8   = 21;

eui_message_t *expecting = 0;
uint8_t is_internal = 0;

//developer-space messages
eui_message_t test_findobject_store[] = {
    //simple objects with mostly garbage data, we only care about the ID's
    { .id = "a",    .type = TYPE_CHAR,    .size = sizeof(testfind_char),     {.data = &testfind_char}    },
    { .id = "si8",  .type = TYPE_INT8,    .size = sizeof(testfind_int8),     {.data = &testfind_int8}    },
    
    { .id = "abcdefghijklmno", 
                    .type = TYPE_UINT8,   .size = sizeof(testfind_uint8),    {.data = &testfind_uint8}   },
    
    { .id = "\x01",  .type = TYPE_UINT8,     .size = sizeof(testfind_int8),  {.data = &testfind_int8}    },
    { .id = "",      .type = TYPE_UINT8,     .size = sizeof(testfind_uint8), {.data = &testfind_uint8}   },
};

// PRIVATE FUNCTIONS
 
// SETUP, TEARDOWN

void setUp(void)
{
    //run before each test
    eui_setup_tracked( test_findobject_store, EUI_ARR_ELEM(test_findobject_store) );
    expecting = 0;
    is_internal = 0;
}
 
void tearDown(void)
{

}

// TESTS
void test_find_tracked_object( void )
{
    expecting = &test_findobject_store[1];
    const char * test_message = "si8";

    TEST_ASSERT_EQUAL_PTR(expecting, find_tracked_object(test_message));
}

void test_find_message_object_developer( void )
{
    expecting = &test_findobject_store[1];
    const char * test_message = "si8";

    TEST_ASSERT_EQUAL_PTR(expecting, find_message_object(test_message, is_internal));
}

void test_find_message_object_internal( void )
{
    const char * test_message = EUI_INTERNAL_LIB_VER;
    is_internal = 1;

    TEST_ASSERT_NOT_NULL_MESSAGE( find_message_object(test_message, is_internal), "Didn't return correct pointer")
}

void test_find_message_object_wrong_internal_flag( void )
{
    const char * test_message = "si8";
    is_internal = 1;

    TEST_ASSERT_EQUAL_PTR(expecting, find_message_object(test_message, is_internal));
}

void test_find_message_object_invalid_internal_flag( void )
{
    const char * test_message = "si8";
    is_internal = 3;

    TEST_ASSERT_EQUAL_PTR(expecting, find_message_object(test_message, is_internal));
}

void test_find_message_object_invalid_id( void )
{
    const char * test_message = "666";

    //we expect a null result, there is no "666" message object in either array
    TEST_ASSERT_NULL_MESSAGE(find_message_object(test_message, is_internal), "Found non-existant euiObject")
    
    //try the internal array branch
    is_internal = 1;
    TEST_ASSERT_NULL_MESSAGE(find_message_object(test_message, is_internal), "Found non-existant euiObject")
}

void test_find_message_object_nullptr( void )
{
    // The search is using strcmp, which will cause segfaults or undefined behaviour when passed invalid pointers.
    const char * test_message = 0;

    //will likely segfault if no null check on id
    is_internal = 0;
    TEST_ASSERT_EQUAL_PTR(expecting, find_message_object(test_message, is_internal));

    //test the internal branch
    is_internal = 1;
    TEST_ASSERT_EQUAL_PTR(expecting, find_message_object(test_message, is_internal));
}

void test_find_message_object_single_letter( void )
{
    expecting = &test_findobject_store[0];
    const char * test_message = "a";

    TEST_ASSERT_EQUAL_PTR(expecting, find_message_object(test_message, is_internal));
}

void test_find_message_object_max_letters( void )
{
    expecting = &test_findobject_store[2];
    const char * test_message = "abcdefghijklmno";

    TEST_ASSERT_EQUAL_PTR(expecting, find_message_object(test_message, is_internal));
}

void test_find_message_object_no_id( void )
{
    expecting = &test_findobject_store[4];
    const char * test_message = "";

    TEST_ASSERT_EQUAL_PTR(expecting, find_message_object(test_message, is_internal));
}

void test_find_message_object_non_printable( void )
{
    expecting = &test_findobject_store[3];
    char buffer[] = { 0x01, 0x00 }; 

    TEST_ASSERT_EQUAL_PTR(expecting, find_message_object(buffer, is_internal));
}

void test_find_message_devArrayNULL( void )
{
    eui_setup_tracked( 0, 0);

    const char * test_message = "si8";

    TEST_ASSERT_EQUAL_PTR(expecting, find_tracked_object(test_message));
}