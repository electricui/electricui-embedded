#include "unity.h"
 
// MODULE UNDER TEST
#include "electricui.h"
#include "mock_eui_serial_transport.h"
#include "mock_eui_utilities.h"

// DEFINITIONS 
void stub_output_func( uint8_t *data, uint16_t len );
 
// PRIVATE TYPES
 
// PRIVATE DATA
uint8_t test_data[10] = { 0xFF };

eui_message_t test_message = { .id = "test", .type = TYPE_UINT8, .size = sizeof(test_data), {.data = &test_data} };

eui_interface_t mock_interface = { 0 };

// PRIVATE FUNCTIONS

void stub_output_func( uint8_t *data, uint16_t len )
{
	//do nothing with it...

}
 
// SETUP, TEARDOWN

void setUp(void)
{
    //run before each test
    mock_interface.output_cb = &stub_output_func;
    eui_setup_interfaces(&mock_interface, 1);
    eui_setup_tracked(&test_message, 1);
}
 
void tearDown(void)
{

}

// TESTS

void test_send_tracked_on( void )
{
    //check the manually defined interface
    encode_packet_simple_ExpectAnyArgsAndReturn(0);
    eui_send_tracked_on("test", &mock_interface);

    eui_send_tracked_on( 0, &mock_interface );
    eui_send_tracked_on("test", 0 );
}

void test_send_tracked_on_invalid_setup( void )
{
    // Destroy any internal reference to the variable we are looking for and the interface we use
    // we don't expect any calls to encode a message, it should fall through
    eui_setup_tracked(0,0);
    eui_send_tracked_on("test", &mock_interface);

    // this test is a bit off, we break the interfaces that eUI holds, 
    // but as the function is passed a valid one, things are still ok.
    eui_setup_interface(0);
    eui_setup_tracked(0, 0);
    eui_send_tracked_on("test", &mock_interface);
}

void test_send_tracked_auto( void )
{  
    //check the message on the automatic interface
    encode_packet_simple_ExpectAnyArgsAndReturn(0);
    eui_send_tracked("test");

    eui_send_tracked(0);
}

void test_send_tracked_auto_invalid_setup( void )
{  
    //destroy any internal reference to the variable we are looking for and the interface we use
    eui_setup_interface(&mock_interface);
    eui_setup_tracked(0,0);
    eui_send_tracked("test");

    eui_setup_interfaces(0, 0);
    eui_setup_tracked(&test_message, 1);
    eui_send_tracked("test");

    eui_setup_interface(0);
    eui_setup_tracked(0, 0);
    eui_send_tracked("test");
}
