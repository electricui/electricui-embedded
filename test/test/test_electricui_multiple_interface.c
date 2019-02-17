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

    attempted_interface = 0;
    attempted_output_times = 0;
}
 
void tearDown(void)
{

}

// TESTS

void test_active_interface_switching_outputs( void )
{
    TEST_IGNORE_MESSAGE("Test micro handshake responses going over correct links");

}
