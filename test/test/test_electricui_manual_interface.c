#include "unity.h"
 
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

uint8_t interface_expecting = 0;

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

    interface_expecting = 0;
    active_interface = 0;
}
 
void tearDown(void)
{

}

// TESTS

//gives us a pointer to an interface object
void test_active_interface_get( void )
{
    interface_expecting = 0;
    
    //The default interface for a 1-interface set should be 0
    setup_interface( &multi_interfaces[1]);
    TEST_ASSERT_EQUAL( interface_expecting, get_default_interface() );

    //The default interface for a n-interface should also be 0
    setup_interfaces( multi_interfaces, ARR_ELEM(multi_interfaces) );
    TEST_ASSERT_EQUAL( interface_expecting, get_default_interface() );

    //manually mutate the active interface variable in the 'private scope' and check we fetch that
    active_interface = 14;
    interface_expecting = 14;
    TEST_ASSERT_EQUAL( interface_expecting, get_default_interface() );

}

void test_active_interface_set( void )
{
    interface_expecting = 0;

    //check the default value is 0 (so we know it does change)
    setup_interfaces( multi_interfaces, ARR_ELEM(multi_interfaces) );
    TEST_ASSERT_EQUAL(interface_expecting, active_interface);

    //set it to a non-zero interface (legal situation, interfaces on [0], [1], [2])
    interface_expecting = 1;
    set_default_interface(1);
    //grab the actual active interface variable from the 'private scope'
    TEST_ASSERT_EQUAL(interface_expecting, active_interface);

    //set it to a (very) invalid interface index, shouldn't change
    interface_expecting = 1;
    set_default_interface(6);
    TEST_ASSERT_EQUAL(interface_expecting, active_interface);

    //test off-by one from the last interface
    interface_expecting = 2;
    set_default_interface(2);
    set_default_interface(3);
    TEST_ASSERT_EQUAL(interface_expecting, active_interface);

    //test negative interface index (underflow)
    set_default_interface(-1);
    TEST_ASSERT_EQUAL(interface_expecting, active_interface);

}

void test_active_if_init_safety( void )
{
    setup_interfaces( multi_interfaces, ARR_ELEM(multi_interfaces) );
    interface_expecting = 2;
    set_default_interface(2);
    TEST_ASSERT_EQUAL( interface_expecting, get_default_interface() );

    //see if reconfiguring the interface(s) changes the active interface
    interface_expecting = 0;
    setup_interface( &multi_interfaces[0] );
    TEST_ASSERT_EQUAL(interface_expecting, active_interface);

    //set it to a non-zero interface (illegal situation)
    interface_expecting = 0;
    set_default_interface(1);
    TEST_ASSERT_EQUAL(interface_expecting, active_interface);

    set_default_interface(2);
    TEST_ASSERT_EQUAL(interface_expecting, active_interface);
}

void test_active_interface_changed_by_ui( void )
{
    TEST_IGNORE_MESSAGE("Add tests where UI modifies default interface");

    // ui sets it to a legal interface number
    // ui sets it to a illegal (too high) interface number

}

void test_interface_outputs( void )
{
    TEST_IGNORE_MESSAGE("Add tests where UI modifies default interface");

    // ui sets it to a legal interface number
    // ui sets it to a illegal (too high) interface number

}