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

//developer-space messages
eui_message_t internal_callback_test_store[] = {
    { .msgID = "chw",   .type = TYPE_CHAR,    .size = sizeof(test_char),    .payload = &test_char   },
    { .msgID = "u8w",   .type = TYPE_INT8,    .size = sizeof(test_uint),    .payload = &test_uint   },
    
    { .msgID = "chr",   .type = TYPE_CHAR|READ_ONLY_MASK,    .size = sizeof(test_char),    .payload = &test_char   },
    { .msgID = "u8r",   .type = TYPE_INT8|READ_ONLY_MASK,    .size = sizeof(test_uint),    .payload = &test_uint   },
};

const uint16_t number_ro_expected = 2;
const uint16_t number_rw_expected = 2;

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
    
    setup_dev_msg(internal_callback_test_store, ARR_ELEM(internal_callback_test_store));
    mock_interface.output_func = &callback_mocked_output;
    setup_interface( &mock_interface );
}
 
void tearDown(void)
{

}

// TESTS

void test_announce_board( void )
{
    //expect the library version, board ID and session ID (lv, bi, si)
    encode_packet_simple_ExpectAnyArgsAndReturn(0);
    encode_packet_simple_ExpectAnyArgsAndReturn(0);
    encode_packet_simple_ExpectAnyArgsAndReturn(0);

    announce_board();
}

void test_announce_dev_msg_readonly( void )
{
    // expect message(s): payload is the messageIDs of ro vars  
    encode_packet_simple_ExpectAnyArgsAndReturn(0);

    // expect a message describing the number of ro vars
    encode_packet_simple_ExpectAnyArgsAndReturn(0);

    announce_dev_msg_readonly();
}

void test_announce_dev_msg_writable( void )
{
    // expect message(s): payload is the messageIDs of rw vars  
    encode_packet_simple_ExpectAnyArgsAndReturn(0);

    // expect a message describing the number of rw vars
    encode_packet_simple_ExpectAnyArgsAndReturn(0);

    announce_dev_msg_writable();
}

void test_announce_dev_vars_readonly( void )
{
    for(uint16_t i = 0; i < number_ro_expected; i++)
    {
        encode_packet_simple_ExpectAnyArgsAndReturn(0);
    }

    announce_dev_vars_readonly();
}

void test_announce_dev_vars_writable( void )
{
    for(uint16_t i = 0; i < number_rw_expected; i++)
    {
        encode_packet_simple_ExpectAnyArgsAndReturn(0);
    }

    announce_dev_vars_writable();
}

// tests that the function correctly counts the number of messageID's sent
void test_send_msgID_list_callback( void )
{
    uint16_t ro_expected = 0;
    uint16_t rw_expected = 0;

    //test reasonable number of both kinds
    eui_message_t ro_rw_testset[] = {
        EUI_CHAR( "cw",     test_char ),
        EUI_INT8( "iw",     test_uint ),
        EUI_CHAR_RO( "cr",  test_char ),
        EUI_INT8_RO( "ir",  test_uint ),
    };

    ro_expected = 2;
    rw_expected = 2;
    setup_dev_msg( ro_rw_testset, ARR_ELEM(ro_rw_testset) );

    encode_packet_simple_IgnoreAndReturn(0);

    TEST_ASSERT_EQUAL_INT_MESSAGE(  rw_expected, 
                                    send_tracked_message_id_list( 0 ), 
                                    "Base - Writable msgID count incorrect" );

    TEST_ASSERT_EQUAL_INT_MESSAGE(  ro_expected, 
                                    send_tracked_message_id_list( 1 ), 
                                    "Base - Read-Only msgID count incorrect" );

    //test an array with nothing
    eui_message_t empty_testset[] = { };

    ro_expected = 0;
    rw_expected = 0;
    setup_dev_msg( empty_testset, ARR_ELEM(empty_testset) );

    TEST_ASSERT_EQUAL_INT_MESSAGE(  rw_expected, 
                                    send_tracked_message_id_list( 0 ), 
                                    "Empty - Writable msgID count incorrect" );

    TEST_ASSERT_EQUAL_INT_MESSAGE(  ro_expected, 
                                    send_tracked_message_id_list( 1 ), 
                                    "Empty - Read-Only msgID count incorrect" );

    //test writable only
    eui_message_t rw_testset[] = {
        EUI_CHAR( "cw",     test_char ),
        EUI_INT8( "iw",     test_uint ),
    };

    ro_expected = 0;
    rw_expected = 2;
    setup_dev_msg( rw_testset, ARR_ELEM(rw_testset) );

    TEST_ASSERT_EQUAL_INT_MESSAGE(  rw_expected, 
                                    send_tracked_message_id_list( 0 ), 
                                    "Read Only - Writable msgID count incorrect" );

    TEST_ASSERT_EQUAL_INT_MESSAGE(  ro_expected, 
                                    send_tracked_message_id_list( 1 ), 
                                    "Read Only - Read-Only msgID count incorrect" );

    //test writable only
    eui_message_t ro_testset[] = {
        EUI_CHAR_RO( "cr",  test_char ),
        EUI_INT8_RO( "ir",  test_uint ),
    };

    ro_expected = 2;
    rw_expected = 0;
    setup_dev_msg( ro_testset, ARR_ELEM(ro_testset) );

    TEST_ASSERT_EQUAL_INT_MESSAGE(  rw_expected, 
                                    send_tracked_message_id_list( 0 ), 
                                    "Writable Only - Writable msgID count incorrect" );

    TEST_ASSERT_EQUAL_INT_MESSAGE(  ro_expected, 
                                    send_tracked_message_id_list( 1 ), 
                                    "Writable Only - Read-Only msgID count incorrect" );

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
    setup_dev_msg( mixed_testset, ARR_ELEM(mixed_testset) );

    TEST_ASSERT_EQUAL_INT_MESSAGE(  rw_expected, 
                                    send_tracked_message_id_list( 0 ), 
                                    "Mixed set - Writable msgID count incorrect" );

    TEST_ASSERT_EQUAL_INT_MESSAGE(  ro_expected, 
                                    send_tracked_message_id_list( 1 ), 
                                    "Mixed set - Read-Only msgID count incorrect" );

    //test many vars
    eui_message_t large_testset[60] = { 0 };

    for( uint8_t i = 0; i < 60; i++)
    {
        if( i % 2)
        {
            large_testset[i].msgID = "cw";
            large_testset[i].type = TYPE_CHAR;
            large_testset[i].size = 1;
            large_testset[i].payload = &test_char;
        }
        else
        {
            large_testset[i].msgID = "cr";
            large_testset[i].type = TYPE_CHAR|READ_ONLY_MASK;
            large_testset[i].size = 1;
            large_testset[i].payload = &test_char;
        }
    }

    ro_expected = 30;
    rw_expected = 30;
    setup_dev_msg( large_testset, ARR_ELEM(large_testset) );

    TEST_ASSERT_EQUAL_INT_MESSAGE(  rw_expected, 
                                    send_tracked_message_id_list( 0 ), 
                                    "Large set - Writable msgID count incorrect" );

    TEST_ASSERT_EQUAL_INT_MESSAGE(  ro_expected, 
                                    send_tracked_message_id_list( 1 ), 
                                    "Large set - Read-Only msgID count incorrect" );
}

void test_send_variable_callback( void )
{
    uint16_t number_sent = 0;

    //test readonly
    for(uint16_t i = 0; i < number_ro_expected; i++)
    {
        encode_packet_simple_ExpectAnyArgsAndReturn(0);
    }

    number_sent = send_tracked_variables( 0 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_rw_expected, number_sent, "Writable variable count incorrect" );

    //test writable
    for(uint16_t j = 0; j < number_rw_expected; j++)
    {
        encode_packet_simple_ExpectAnyArgsAndReturn(0);
    }

    number_sent = send_tracked_variables( 1 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_ro_expected, number_sent, "Read-Only variable count incorrect" );
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

    setup_identifier(uuid, sizeof(uuid));
}

void test_setup_identifier_invalids( void )
{
    //with a non-valid input param, expect no output
    setup_identifier(0,0);
    setup_identifier("unique", 0);
    setup_identifier(0, 3);
}
