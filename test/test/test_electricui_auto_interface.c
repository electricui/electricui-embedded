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

eui_interface_t *interface_result = 0;
eui_interface_t *interface_expecting = 0;

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
    { .msgID = "chw",   .type = TYPE_CHAR,    .size = sizeof(test_char),    .payload = &test_char   },
    { .msgID = "u8w",   .type = TYPE_INT8,    .size = sizeof(test_uint),    .payload = &test_uint   },
};

eui_interface_t multi_interfaces[] = {
    { .packet = { 0 }, .output_func = &callback_mocked_output_1, .interface_cb = 0 },
    { .packet = { 0 }, .output_func = &callback_mocked_output_2, .interface_cb = 0 },
    { .packet = { 0 }, .output_func = &callback_mocked_output_3, .interface_cb = 0 },
};

// SETUP, TEARDOWN
 
void setUp(void)
{
    //reset the state of everything
    test_char = 'a';
    test_uint = 21;

    setup_dev_msg(internal_callback_test_store, ARR_ELEM(internal_callback_test_store));
    setup_interfaces( multi_interfaces, ARR_ELEM(multi_interfaces));
}
 
void tearDown(void)
{

}

// TESTS

//gives us a pointer to an interface object
void test_auto_interface_single( void )
{
    setup_interface( &multi_interfaces[1] );

    //ask the auto-interface for an interface pointer
    interface_expecting = &multi_interfaces[1];

    TEST_ASSERT_EQUAL_PTR(interface_expecting, auto_interface());
}

void test_auto_interface_many( void )
{
    //test return of each interface
    for(uint8_t i = 0; i < ARR_ELEM(multi_interfaces); i++)
    {
        active_interface = i;
        TEST_ASSERT_EQUAL_PTR( &multi_interfaces[i], auto_interface() );
    }

    //an invalid active interface value should return an error (null ptr)
    active_interface = 5;
    TEST_ASSERT_NULL( auto_interface() );

    //test off-by-one case
    //the ARR_ELEM macro gives logical count, will be +1 over index notation
    active_interface = ARR_ELEM(multi_interfaces);  
    TEST_ASSERT_NULL( auto_interface() );
}

//test with invalid developer provided information
void test_auto_interface_invalid( void )
{
    //test a 'not setup yet' interface
    setup_interfaces( 0, 0);
    for(uint8_t i = 0; i < ARR_ELEM(multi_interfaces); i++)
    {
        active_interface = i;
        TEST_ASSERT_NULL( auto_interface() );
    }

    //test a malformed interface setup by manually overriding the 'internal' interface vars
    interface_arr = &multi_interfaces[1];
    interface_count = 0;

    for(uint8_t i = 0; i < ARR_ELEM(multi_interfaces); i++)
    {
        active_interface = i;
        TEST_ASSERT_NULL( auto_interface() );
    }

    interface_arr = 0;
    interface_count = 3;

    for(uint8_t i = 0; i < ARR_ELEM(multi_interfaces); i++)
    {
        active_interface = i;
        TEST_ASSERT_NULL( auto_interface() );
    }
}

// gives us a output function
void test_auto_output( void )
{
    //ask the auto-interface for an interface pointer
    callback_data_out_t cb_result = 0;
    callback_data_out_t cb_expecting = &callback_mocked_output_2;
    active_interface = 1;

    cb_result = auto_output();
    TEST_ASSERT_EQUAL_PTR(cb_expecting, cb_result);
}