#include "unity.h"
 #include <string.h>

// MODULE UNDER TEST
#include "electricui.c"
#include "mock_eui_binary_transport.h"
#include "mock_eui_utilities.h"

// DEFINITIONS 
 
// PRIVATE TYPES
 
// PRIVATE DATA
char      test_char    = 'a';
uint8_t   test_uint    = 21;

callback_data_out_t attempted_interface = 0;
int attempted_output_times = 0; //CMOCK callback invocation count

// PRIVATE FUNCTIONS
void callback_mocked_output_0( uint8_t *data, uint16_t len )
{
    attempted_interface = &callback_mocked_output_0;
    attempted_output_times++;
}

void callback_mocked_output_1( uint8_t *data, uint16_t len )
{
    attempted_interface = &callback_mocked_output_1;
    attempted_output_times++;
}

void callback_mocked_output_2( uint8_t *data, uint16_t len )
{
    attempted_interface = &callback_mocked_output_2;
    attempted_output_times++;
}

//developer-space messages
eui_message_t internal_callback_test_store[] = {
    { .id = "chw",   .type = TYPE_CHAR,    .size = sizeof(test_char),    {.data = &test_char}   },
    { .id = "u8w",   .type = TYPE_INT8,    .size = sizeof(test_uint),    {.data = &test_uint}   },
};

eui_interface_t multi_interfaces[] = {
    { .packet = { 0 }, .output_cb = &callback_mocked_output_0, .interface_cb = 0 },
    { .packet = { 0 }, .output_cb = &callback_mocked_output_1, .interface_cb = 0 },
    { .packet = { 0 }, .output_cb = &callback_mocked_output_2, .interface_cb = 0 },
};

eui_packet_t * test_pk_ptr_0;  
eui_packet_t * test_pk_ptr_1;  
eui_packet_t * test_pk_ptr_2;  

// SETUP, TEARDOWN
 
void setUp(void)
{
    //reset the state of everything
    test_char = 'a';
    test_uint = 21;

    eui_setup_tracked(internal_callback_test_store, EUI_ARR_ELEM(internal_callback_test_store));
    eui_setup_interfaces( multi_interfaces, EUI_ARR_ELEM(multi_interfaces));

    attempted_interface     = 0;
    attempted_output_times  = 0;
    test_pk_ptr_0           = &multi_interfaces[0].packet;
    test_pk_ptr_1           = &multi_interfaces[1].packet;
    test_pk_ptr_2           = &multi_interfaces[2].packet;
}
 
void tearDown(void)
{

}

// TESTS

void test_active_interface_switching_outputs( void )
{
    // Due to the mocking arrangement, actually validating that the 'upstream' data is 
    // sent to the right callback is a bit tricky. We just validate that the internal
    // interface holding object is correctly changed when outputs should switch

    // We shouldn't have had any outputs yet
    TEST_ASSERT_NULL( attempted_interface );
    TEST_ASSERT_EQUAL( 0, attempted_output_times );
    // by default, the 'first' interface is used
    TEST_ASSERT_EQUAL_PTR( &multi_interfaces[0], p_interface_last );

    // Send a query message to the library from the 'head' interface 
    // (we inject it straight into the inbound interface's state)
    test_pk_ptr_0->parser.state           = exp_crc_b2;
    test_pk_ptr_0->parser.id_bytes_in     = 3;
    test_pk_ptr_0->parser.data_bytes_in   = 0;
    test_pk_ptr_0->parser.frame_offset    = 0;

    test_pk_ptr_0->header.data_len    = 0;
    test_pk_ptr_0->header.type        = TYPE_UINT8;
    test_pk_ptr_0->header.internal    = 0;
    test_pk_ptr_0->header.offset      = 0;
    test_pk_ptr_0->header.id_len      = 4;
    test_pk_ptr_0->header.response    = 1;
    test_pk_ptr_0->header.acknum      = 0;

    memcpy(&test_pk_ptr_0->id_in, "u8w", 3);
    test_pk_ptr_0->offset_in          = 0;
    test_pk_ptr_0->data_in[0]         = 0x00;
    test_pk_ptr_0->crc_in             = 0xfefe;   //not checked - we mock == decoder

    decode_packet_ExpectAnyArgsAndReturn( EUI_PARSER_OK);
    encode_packet_simple_ExpectAnyArgsAndReturn( EUI_OUTPUT_OK );
    eui_errors_t status0 = eui_parse( 0x00, &multi_interfaces[0] );
    TEST_ASSERT_EQUAL_UINT8( EUI_QUERY_OK , status0.query );
    
    // The library tracks it as the 'last valid' and would respond on this interface
    TEST_ASSERT_EQUAL_PTR( &multi_interfaces[0], p_interface_last );

    // Send a message to the library from the 'last' interface in the set
    test_pk_ptr_2->parser.state           = exp_crc_b2;
    test_pk_ptr_2->parser.id_bytes_in     = 3;
    test_pk_ptr_2->parser.data_bytes_in   = 0;
    test_pk_ptr_2->parser.frame_offset    = 0;

    test_pk_ptr_2->header.data_len    = 0;
    test_pk_ptr_2->header.type        = TYPE_CHAR;
    test_pk_ptr_2->header.internal    = 0;
    test_pk_ptr_2->header.offset      = 0;
    test_pk_ptr_2->header.id_len      = 4;
    test_pk_ptr_2->header.response    = 1;
    test_pk_ptr_2->header.acknum      = 0;

    memcpy(&test_pk_ptr_2->id_in, "chw", 3);
    test_pk_ptr_2->offset_in          = 0;
    test_pk_ptr_2->data_in[0]         = 0x00;
    test_pk_ptr_2->crc_in             = 0xfefe;   //not checked - we mock == decoder

    decode_packet_ExpectAnyArgsAndReturn( EUI_PARSER_OK);
    encode_packet_simple_ExpectAnyArgsAndReturn( EUI_OUTPUT_OK );
    eui_errors_t status1 = eui_parse( 0x00, &multi_interfaces[2] );
    TEST_ASSERT_EQUAL_UINT8( EUI_QUERY_OK , status1.query );

    // The library now uses this interface as the 'last valid' interface
    // and would send the response on interface2
    TEST_ASSERT_EQUAL_PTR( &multi_interfaces[2], p_interface_last );
}