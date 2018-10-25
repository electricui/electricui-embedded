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

eui_message_t test_message = { .msgID = "test", .type = TYPE_UINT8, .size = sizeof(test_data), .payload = &test_data };

eui_pkt_settings_t default_settings = 
{   
    .internal  = MSG_DEV,
    .response  = MSG_NRESP,
    .type      = TYPE_UINT8,
};

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
    setup_dev_msg(&test_message, 1);
}
 
void tearDown(void)
{

}

// TESTS

void test_send_message_on( void )
{
    //check the manually defined interface
    encode_packet_simple_ExpectAnyArgsAndReturn(0);
    send_message_on("test", &mock_interface);

    send_message_on( 0, &mock_interface );
    send_message_on("test", 0 );

}

void test_send_message_on_invalid_setup( void )
{
    // Destroy any internal reference to the variable we are looking for and the interface we use
    // we don't expect any calls to encode a message, it should fall through
    setup_dev_msg(0,0);
    send_message_on("test", &mock_interface);

    // this test is a bit off, we break the interfaces that eUI holds, 
    // but as the function is passed a valid one, things are still ok.
    setup_interface(0, 0);
    setup_dev_msg(0, 0);
    send_message_on("test", &mock_interface);
}

void test_send_message_auto( void )
{  
    //check the message on the automatic interface
    encode_packet_simple_ExpectAnyArgsAndReturn(0);
    send_message("test");

    send_message(0);
}

void test_send_message_auto_invalid_setup( void )
{  
    //destroy any internal reference to the variable we are looking for and the interface we use
    setup_interface(&mock_interface, 1);
    setup_dev_msg(0,0);
    send_message("test");

    setup_interface(0, 0);
    setup_dev_msg(&test_message, 1);
    send_message("test");

    setup_interface(0, 0);
    setup_dev_msg(0, 0);
    send_message("test");
}
