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

//developer-space messages
eui_message_t internal_callback_test_store[] = {
    { .id = "chw",   .type = TYPE_CHAR,    .size = sizeof(test_char),    {.data = &test_char}   },
    { .id = "u8w",   .type = TYPE_INT8,    .size = sizeof(test_uint),    {.data = &test_uint}   },
    
    { .id = "chr",   .type = TYPE_CHAR|READ_ONLY_MASK,    .size = sizeof(test_char),    {.data = &test_char}   },
    { .id = "u8r",   .type = TYPE_INT8|READ_ONLY_MASK,    .size = sizeof(test_uint),    {.data = &test_uint}   },
};

const uint16_t number_ro_expected = 2;
const uint16_t number_rw_expected = 2;
const uint16_t number_expected    = number_ro_expected + number_rw_expected;

eui_interface_t mock_interface = { 0 };

// PRIVATE FUNCTIONS
void callback_mocked_output(uint8_t *data, uint16_t len)
{

}

// SETUP, TEARDOWN
 
void setUp(void)
{
    //reset the state of everything
    test_char = 'a';
    test_uint = 21;
    
    // setup_identifier( (char*)"a", 1 );
    
    eui_setup_tracked(internal_callback_test_store, EUI_ARR_ELEM(internal_callback_test_store));
    mock_interface.output_cb = &callback_mocked_output;
    eui_setup_interface( &mock_interface );
}
 
void tearDown(void)
{

}

// TESTS

void test_announce_dev_msg( void )
{
    // expect message(s): payload is the messageIDs of vars  
    encode_packet_simple_ExpectAnyArgsAndReturn(0);

    // expect a message describing the number of vars
    encode_packet_simple_ExpectAnyArgsAndReturn(0);

    announce_dev_msg();
}

void test_announce_dev_vars( void )
{
    for(uint16_t i = 0; i < number_expected; i++)
    {
        encode_packet_simple_ExpectAnyArgsAndReturn(0);
    }

    send_tracked_variables();
}

// tests that the function correctly counts the number of messageID's sent
void test_send_id_list_callback( void )
{
    uint16_t ro_expected = 0;
    uint16_t rw_expected = 0;
    uint16_t total_expected = 0;

    //test reasonable number of both kinds
    eui_message_t ro_rw_testset[] = {
        EUI_CHAR( "cw",     test_char ),
        EUI_INT8( "iw",     test_uint ),
        EUI_CHAR_RO( "cr",  test_char ),
        EUI_INT8_RO( "ir",  test_uint ),
    };

    ro_expected = 2;
    rw_expected = 2;
    total_expected = ro_expected + rw_expected;
    eui_setup_tracked( ro_rw_testset, EUI_ARR_ELEM(ro_rw_testset) );

    encode_packet_simple_IgnoreAndReturn(0);

    TEST_ASSERT_EQUAL_INT_MESSAGE(  total_expected, 
                                    send_tracked_message_id_list(), 
                                    "Base - Tracked id count incorrect" );


    //test an array with nothing
    eui_message_t empty_testset[] = { };

    ro_expected = 0;
    rw_expected = 0;
    total_expected = ro_expected + rw_expected;
    eui_setup_tracked( empty_testset, EUI_ARR_ELEM(empty_testset) );

    TEST_ASSERT_EQUAL_INT_MESSAGE(  total_expected, 
                                    send_tracked_message_id_list(), 
                                    "Empty - Tracked id count incorrect" );

    //test mixed order of vars
    eui_message_t mixed_testset[] = {
        EUI_CHAR(       "cw0", test_char ),
        EUI_INT8(       "iw0", test_uint ),
        EUI_CHAR_RO(    "cr0", test_char ),
        EUI_INT8_RO(    "ir0", test_uint ),

        EUI_CHAR(       "cw1", test_char ),
        EUI_INT8_RO(    "ir1", test_uint ),
        EUI_INT8(       "iw1", test_uint ),
        EUI_CHAR_RO(    "cr1", test_char ),

        EUI_CHAR(       "cw2", test_char ),
        EUI_INT8(       "iw2", test_uint ),
    };

    ro_expected = 4;
    rw_expected = 6;
    total_expected = ro_expected + rw_expected;

    eui_setup_tracked( mixed_testset, EUI_ARR_ELEM(mixed_testset) );

    TEST_ASSERT_EQUAL_INT_MESSAGE(  total_expected, 
                                    send_tracked_message_id_list(), 
                                    "Mixed set - Tracked id count incorrect" );

    //test many vars
    eui_message_t large_testset[60] = { 0 };

    for( uint8_t i = 0; i < 60; i++)
    {
        if( i % 2)
        {
            large_testset[i].id = "cw";
            large_testset[i].type = TYPE_CHAR;
            large_testset[i].size = 1;
            large_testset[i].ptr.data = &test_char;
        }
        else
        {
            large_testset[i].id = "cr";
            large_testset[i].type = TYPE_CHAR|READ_ONLY_MASK;
            large_testset[i].size = 1;
            large_testset[i].ptr.data = &test_char;
        }
    }

    ro_expected = 30;
    rw_expected = 30;
    total_expected = ro_expected + rw_expected;
    eui_setup_tracked( large_testset, EUI_ARR_ELEM(large_testset) );

    TEST_ASSERT_EQUAL_INT_MESSAGE(  total_expected, 
                                    send_tracked_message_id_list(), 
                                    "Large set - Tracked id count incorrect" );

}

void test_send_variable_callback( void )
{
    uint16_t number_sent = 0;

    //test variable output count
    for(uint16_t i = 0; i < number_expected; i++)
    {
        encode_packet_simple_ExpectAnyArgsAndReturn(0);
    }

    send_tracked_variables();
}

//as the crc function is mocked, it doesn't write the crc result to the board_identifier value.
//we don't care, because just calling it tests _this_ function
void test_setup_identifier( void )
{
    char * uuid = "unique_text";
    for(uint16_t i = 0; i < sizeof(uuid); i++)
    {
        crc16_ExpectAnyArgs();
    }

    eui_setup_identifier(uuid, sizeof(uuid));
}

void test_setup_identifier_invalids( void )
{
    //with a non-valid input param, expect no output
    eui_setup_identifier(0,0);
    eui_setup_identifier("unique", 0);
    eui_setup_identifier(0, 3);
}
