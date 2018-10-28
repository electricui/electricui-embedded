#include "unity.h"
 
// MODULE UNDER TEST
#include "electricui.h"
#include "mock_eui_serial_transport.h"
#include "mock_eui_crc.h"
#include "mock_eui_offset_validation.h"

// DEFINITIONS 
void stub_output_func( uint8_t byte );
 
// PRIVATE TYPES
 
// PRIVATE DATA
uint8_t test_data[10] = { 0xFF };

eui_interface_t mock_interface = { 0 };

// PRIVATE FUNCTIONS

void stub_output_func( uint8_t byte )
{
	//do nothing with it...

}
 
// SETUP, TEARDOWN

void setUp(void)
{
    //run before each test
    mock_interface.output_func = &stub_output_func;
    setup_interface(&mock_interface, 1);
}
 
void tearDown(void)
{

}

// TESTS

void test_send_untracked_on( void )
{
    //check the manually defined interface
    eui_message_t new_message = { .msgID = "new", .type = TYPE_UINT8, .size = sizeof(test_data), .payload = &test_data };

    encode_packet_simple_ExpectAnyArgsAndReturn(0);
    send_untracked_on(&new_message, &mock_interface);

    send_untracked_on( 0, &mock_interface );
    send_untracked_on(&new_message, 0 );
}

void test_send_untracked_on_invalid_setup( void )
{
    eui_message_t new_message = { .msgID = "new", .type = TYPE_UINT8, .size = sizeof(test_data), .payload = &test_data };

    // this test is a bit off, we break the interfaces that eUI holds, 
    // but as the function is passed a valid one, things are still ok.
    setup_interface(0, 0);
    encode_packet_simple_ExpectAnyArgsAndReturn(0);
    send_untracked_on(&new_message, &mock_interface);
}

void test_send_untracked_auto( void )
{  
    eui_message_t new_message = { .msgID = "new", .type = TYPE_UINT8, .size = sizeof(test_data), .payload = &test_data };

    //check the message on the automatic interface
    encode_packet_simple_ExpectAnyArgsAndReturn(0);
    send_untracked(&new_message);

    send_untracked(0);
}

void test_send_untracked_auto_invalid_setup( void )
{  
    eui_message_t new_message = { .msgID = "new", .type = TYPE_UINT8, .size = sizeof(test_data), .payload = &test_data };

    setup_interface(0, 0);
    send_untracked(&new_message);
}
