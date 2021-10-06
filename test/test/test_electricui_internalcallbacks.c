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

    // test mixed order of vars
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

    // test many vars
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

// MAX Number of message ID's to attempt to track/handshake with
#define STRESS_ID_COUNT 30

void test_send_id_list_callback_all( void )
{
    // Temp buffer for message ID strings
    char msgID_strbuf[STRESS_ID_COUNT][EUI_MAX_MSGID_SIZE+1] = { 0 };

    // The 'tracked' variables being managed by eUI
    eui_message_t track_stressed[STRESS_ID_COUNT] = { 0 };

    // bytes that can fit in a single packet's payload 
    const uint16_t buffer_size = EUI_MAX_MSGID_SIZE*4;

    // For a msgid length of 1 to max length
    // for a count of msgids from 1 to STRESS_ID_COUNT
    // generate a array of tracked variables, and send the 'list function'
    //      ensure the count of variables sent is correct
    //      ensure that the messages are broken into new packets as the buffer overflows

    for( uint8_t id_length = 1; id_length < EUI_MAX_MSGID_SIZE; id_length++ )
    {
        // Generate a string with n bytes corresponding to this pass of the test
        // Will be used as the 'template' msgID string in the child loop
        char msg_id[EUI_MAX_MSGID_SIZE] = { 0 };
        memset( &msg_id, 'a', id_length);
        msg_id[id_length + 1] = '\0';

        for( uint8_t id_num = 1; id_num < STRESS_ID_COUNT; id_num++ )
        {
            // Clear out our array etc
            memset( track_stressed, 0, sizeof(track_stressed));
            memset( msgID_strbuf, 0, sizeof(msgID_strbuf));

            // Walk through the ID strings and see if they fit in the buffer
            // When they don't, increment the packet count and continue through
            uint32_t buffer_bytes_used = 0;
            uint32_t packets_required = 1;

            // Setup a 'tracked variable set' of many variables with the specified dyn length
            for( uint16_t i = 0; i < id_num; i++)
            {
                // eUI only needs pointers to strings, so generate a string in a buffer
                // and then pass the pointer for that entry
                strcpy(msgID_strbuf[id_num], msg_id);

                track_stressed[i].id = &msgID_strbuf[id_num][0];
                track_stressed[i].type = TYPE_CHAR;
                track_stressed[i].size = 1;
                track_stressed[i].ptr.data = &test_char;


                // Calculate if this message would require an additional packet to fit
                uint32_t msg_bytes_req = strlen(track_stressed[i].id) + 1;

                if( (buffer_bytes_used + msg_bytes_req) > buffer_size )
                {
                    packets_required++;
                    buffer_bytes_used = 0;
                }

                // Keep track of bytes required for packet counting check
                buffer_bytes_used += msg_bytes_req;
            }

            // Configure eUI with the generated array of variables to track
            eui_setup_tracked( track_stressed, id_num );

            // Expect a packet to be sent for each set
            for( uint16_t i = 1; i <= packets_required; i++)
            {            
                encode_packet_simple_ExpectAnyArgsAndReturn(0);
            }

            TEST_ASSERT_EQUAL_INT_MESSAGE(  id_num, 
                                            send_tracked_message_id_list(), 
                                            "Bruteforced send-tracked-id returned an incorrect count" );

        } // end 'number of message' iterations

    }   // end id_length iterations


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
