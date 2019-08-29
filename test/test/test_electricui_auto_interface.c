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
char      test_char    = 'a';
uint8_t   test_uint    = 21;

// PRIVATE FUNCTIONS
void callback_mocked_output_1( uint8_t *data, uint16_t len )
{

}

void callback_mocked_output_2( uint8_t *data, uint16_t len )
{

}

void callback_mocked_output_3( uint8_t *data, uint16_t len )
{

}

//developer-space messages
eui_message_t internal_callback_test_store[] = {
    { .id = "chw",   .type = TYPE_CHAR,    .size = sizeof(test_char),    {.data = &test_char}   },
    { .id = "u8w",   .type = TYPE_INT8,    .size = sizeof(test_uint),    {.data = &test_uint}   },
};

eui_interface_t multi_interfaces[] = {
    { .packet = { 0 }, .output_cb = &callback_mocked_output_1, .interface_cb = 0 },
    { .packet = { 0 }, .output_cb = &callback_mocked_output_2, .interface_cb = 0 },
    { .packet = { 0 }, .output_cb = &callback_mocked_output_3, .interface_cb = 0 },
};

// SETUP, TEARDOWN
 
void setUp(void)
{
    //reset the state of everything
    test_char = 'a';
    test_uint = 21;

    eui_setup_tracked(internal_callback_test_store, EUI_ARR_ELEM(internal_callback_test_store));
    eui_setup_interfaces( multi_interfaces, EUI_ARR_ELEM(multi_interfaces));
}
 
void tearDown(void)
{

}

// TESTS

//gives us a pointer to an interface object
void test_auto_interface_single( void )
{
    eui_setup_interface( &multi_interfaces[1] );

    //pretend we've had a message come in and have set last_interface
    p_interface_last = &multi_interfaces[1];

    TEST_ASSERT_EQUAL_PTR(&multi_interfaces[1], auto_interface());
}

void test_auto_interface_many( void )
{
    // When multiple come in, only the 0th index will be used
    TEST_ASSERT_EQUAL_PTR(&multi_interfaces[0], auto_interface());

    //test return of changing interface
    for(uint8_t i = 0; i < EUI_ARR_ELEM(multi_interfaces); i++)
    {
        p_interface_last = &multi_interfaces[i];
        TEST_ASSERT_EQUAL_PTR( &multi_interfaces[i], auto_interface() );
    }
}

//test with invalid developer provided information
void test_auto_interface_invalid( void )
{
    //test a 'not setup yet' interface
    eui_setup_interfaces( 0, 0);
    for(uint8_t i = 0; i < EUI_ARR_ELEM(multi_interfaces); i++)
    {
        p_interface_last = &multi_interfaces[i];
        TEST_ASSERT_NULL( auto_interface() );
    }

    //test a malformed interface setup by manually overriding the 'internal' interface vars
    p_interface_arr = &multi_interfaces[1];
    interface_num   = 0;

    for(uint8_t i = 0; i < EUI_ARR_ELEM(multi_interfaces); i++)
    {
        p_interface_last = &multi_interfaces[i];
        TEST_ASSERT_NULL( auto_interface() );
    }

    p_interface_arr = 0;
    interface_num   = 3;

    for(uint8_t i = 0; i < EUI_ARR_ELEM(multi_interfaces); i++)
    {
        p_interface_last = &multi_interfaces[i];
        TEST_ASSERT_NULL( auto_interface() );
    }
}