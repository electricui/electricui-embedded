#include "unity.h"
 
// MODULE UNDER TEST
#include "electricui.h"
#include "mock_eui_serial_transport.h"
#include "mock_eui_crc.h"
#include "mock_eui_offset_validation.h"

// DEFINITIONS 
 
// PRIVATE TYPES
 
// PRIVATE DATA
uint8_t small_data[10] = { 0xFF };
uint8_t exact_data[PAYLOAD_SIZE_MAX] = { 0xFF };
uint8_t large_data[4096] = { 0xFF };

eui_message_t short_payload = { .id = "short", .type = TYPE_UINT8, .size = sizeof(small_data), {.data = &small_data} };
eui_message_t exact_payload = { .id = "exact", .type = TYPE_UINT8, .size = sizeof(exact_data), {.data = &exact_data} };
eui_message_t large_payload = { .id = "large", .type = TYPE_UINT8, .size = sizeof(large_data), {.data = &large_data} };

eui_pkt_settings_t default_settings = 
{   
	.internal  = MSG_DEV,
    .response  = MSG_NRESP,
    .type  	   = TYPE_UINT8,
};

// PRIVATE FUNCTIONS

void stub_output_func( uint8_t *data, uint16_t len )
{
	//do nothing with it...

}
 
// SETUP, TEARDOWN

void setUp(void)
{
    //run before each test
}
 
void tearDown(void)
{

}

// TESTS

void test_send_packet_small( void )
{
    encode_packet_simple_ExpectAnyArgsAndReturn(0);
    eui_send(&stub_output_func, &short_payload, &default_settings );
}

void test_send_packet_boundary_length( void )
{
    encode_packet_simple_ExpectAnyArgsAndReturn(0);
    eui_send(&stub_output_func, &exact_payload, &default_settings );
}

void test_send_packet_large( void )
{
    validate_offset_range_ExpectAnyArgs();          //validates the offset range before starting
    encode_packet_ExpectAnyArgsAndReturn(EUI_OUTPUT_OK);   //metadata message describing the range

    //we expect a call for each packet required to send the offset range
    uint8_t num_messages_sent = sizeof(large_data) / PAYLOAD_SIZE_MAX;
    for(uint16_t i = 0; i <= num_messages_sent; i++)
    {
        encode_packet_ExpectAnyArgsAndReturn(EUI_OUTPUT_OK);
    }

    eui_send(&stub_output_func, &large_payload, &default_settings );
}

