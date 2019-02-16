#include "unity.h"
 
// MODULE UNDER TEST
#include "electricui.h"
#include "electricui_private.h"
#include "mock_eui_serial_transport.h"
#include "mock_eui_crc.h"
#include "mock_eui_offset_validation.h"

// DEFINITIONS 
 
void send_packet_Callback(  callback_data_out_t output_function,
                            eui_message_t       *msgObjPtr,
                            eui_pkt_settings_t  *settings,
                            int num_calls );

// PRIVATE TYPES
 
// PRIVATE DATA
char      test_char    = 'a';
uint8_t   test_uint    = 21;

callback_data_out_t attempted_interface = 0;
int attempted_output_times = 0; //CMOCK callback invocation count

// PRIVATE FUNCTIONS
void callback_mocked_output_0( uint8_t *data, uint16_t len )
{

}

void callback_mocked_output_1( uint8_t *data, uint16_t len )
{

}

void callback_mocked_output_2( uint8_t *data, uint16_t len )
{

}

//developer-space messages
eui_message_t internal_callback_test_store[] = {
    { .msgID = "chw",   .type = TYPE_CHAR,    .size = sizeof(test_char),    .payload = &test_char   },
    { .msgID = "u8w",   .type = TYPE_INT8,    .size = sizeof(test_uint),    .payload = &test_uint   },
};

eui_interface_t multi_interfaces[] = {
    { .packet = { 0 }, .output_func = &callback_mocked_output_0, .interface_cb = 0 },
    { .packet = { 0 }, .output_func = &callback_mocked_output_1, .interface_cb = 0 },
    { .packet = { 0 }, .output_func = &callback_mocked_output_2, .interface_cb = 0 },
};


// CMOCK callback

void encode_packet_simple_Callback( callback_data_out_t output_function,
                                    eui_pkt_settings_t  *settings,
                                    const char          *msg_id,
                                    uint16_t            payload_len,
                                    void*               payload,
                                    int                 NumCalls )
{
    attempted_interface = output_function;
    attempted_output_times++;
}

// SETUP, TEARDOWN
 
void setUp(void)
{
    //reset the state of everything
    test_char = 'a';
    test_uint = 21;

    setup_dev_msg(internal_callback_test_store, ARR_ELEM(internal_callback_test_store));
    setup_interfaces( multi_interfaces, ARR_ELEM(multi_interfaces));

    active_interface = 0;
    attempted_interface = 0;
    attempted_output_times = 0;
}
 
void tearDown(void)
{

}

// TESTS

//gives us a pointer to an interface object
void test_active_interface_get( void )
{    
    //The default interface for a 1-interface set should be 0
    setup_interface( &multi_interfaces[1]);
    TEST_ASSERT_EQUAL( 0, get_default_interface() );

    //The default interface for a n-interface should also be 0
    setup_interfaces( multi_interfaces, ARR_ELEM(multi_interfaces) );
    TEST_ASSERT_EQUAL( 0, get_default_interface() );

    //manually mutate the active interface variable in the 'private scope' and check we fetch that
    active_interface = 14;
    TEST_ASSERT_EQUAL( 14, get_default_interface() );

}

void test_active_interface_set( void )
{
    //check the default value is 0 (so we know it does change)
    setup_interfaces( multi_interfaces, ARR_ELEM(multi_interfaces) );
    TEST_ASSERT_EQUAL(0, active_interface);

    //set it to a non-zero interface (legal situation, interfaces on [0], [1], [2])
    set_default_interface(1);
    //grab the actual active interface variable from the 'private scope'
    TEST_ASSERT_EQUAL(1, active_interface);

    //set it to a (very) invalid interface index, shouldn't change
    set_default_interface(6);
    TEST_ASSERT_EQUAL(1, active_interface);

    //test off-by one from the last interface
    set_default_interface(2);
    set_default_interface(3);
    TEST_ASSERT_EQUAL(2, active_interface);

    //test negative interface index (underflow)
    set_default_interface(-1);
    TEST_ASSERT_EQUAL(2, active_interface);

}

void test_active_interface_init_safety( void )
{
    setup_interfaces( multi_interfaces, ARR_ELEM(multi_interfaces) );
    set_default_interface(2);
    TEST_ASSERT_EQUAL( 2, get_default_interface() );

    //see if reconfiguring the interface(s) changes the active interface
    setup_interface( &multi_interfaces[0] );
    TEST_ASSERT_EQUAL(0, active_interface);

    //set it to a non-zero interface (illegal situation)
    set_default_interface(1);
    TEST_ASSERT_EQUAL(0, active_interface);

    set_default_interface(2);
    TEST_ASSERT_EQUAL(0, active_interface);
}

void test_active_interface_switching_outputs( void )
{
    encode_packet_simple_ExpectAnyArgsAndReturn(0);

    // use CMOCK's callback to call us here with the arguments of the encode_packet_simple( ... )
    // the callback instead captures the function pointer intended as the developer's output
    encode_packet_simple_StubWithCallback( encode_packet_simple_Callback );

    set_default_interface(0);
    send_tracked("u8w");
    TEST_ASSERT_EQUAL_PTR_MESSAGE( multi_interfaces[0].output_func, attempted_interface, "Wrong output interface was used" );

    set_default_interface(1);
    send_tracked("chw");
    TEST_ASSERT_EQUAL_PTR_MESSAGE( multi_interfaces[1].output_func, attempted_interface, "Wrong output interface was used" );

    set_default_interface(2);
    send_tracked("u8w");
    TEST_ASSERT_EQUAL_PTR_MESSAGE( multi_interfaces[2].output_func, attempted_interface, "Wrong output interface was used" );


    //developer attempts an invalid interface selection and it shouldn't change of the previous setting
    set_default_interface(7);
    send_tracked("u8w");
    TEST_ASSERT_EQUAL_PTR_MESSAGE( multi_interfaces[2].output_func, attempted_interface, "Wrong output interface was used" );

}


void test_active_interface_switch_by_ui( void )
{
    encode_packet_simple_ExpectAnyArgsAndReturn(0);

    // use CMOCK's callback to call us here with the arguments of the encode_packet_simple( ... )
    // the callback instead captures the function pointer intended as the developer's output
    encode_packet_simple_StubWithCallback( encode_packet_simple_Callback );

    send_tracked("u8w");
    TEST_ASSERT_EQUAL_MESSAGE( 1, attempted_output_times, "Expected a output attempt" );
    TEST_ASSERT_EQUAL_PTR_MESSAGE( multi_interfaces[0].output_func, attempted_interface, "Wrong output interface was used" );
    
    //reset the interface callback and counter
    attempted_interface = 0;
    attempted_output_times = 0;

    //emulate a UI interface change
    active_interface = 1;
    send_tracked("u8w");
    TEST_ASSERT_EQUAL_MESSAGE( 1, attempted_output_times, "Expected a output attempt" );
    TEST_ASSERT_EQUAL_PTR_MESSAGE( multi_interfaces[1].output_func, attempted_interface, "Wrong output interface was used" );

}

void test_active_interface_invalid_switch_by_ui( void )
{
    // We should not see the library calling for output if the UI sets the active interface to an invalid number
    //encode_packet_simple_ExpectAnyArgsAndReturn(0);

    // use CMOCK's callback to call us here with the arguments of the encode_packet_simple( ... )
    // the callback instead captures the function pointer intended as the developer's output
    encode_packet_simple_StubWithCallback( encode_packet_simple_Callback );

    //UI sets an invalid interface target
    active_interface = 14;
    send_tracked("u8w");

    TEST_ASSERT_EQUAL_MESSAGE( 0, attempted_output_times, "Shouldn't have tried to send" );
}