#include "../../electricui.h"
#include "unity.h"
#include "unity_fixture.h"
#include <string.h>

TEST_GROUP( InternalEUICallbacks );

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

uint8_t 	callback_serial_buffer[1024] 	= { 0 };
uint16_t 	callback_serial_position    	= 0;


void callback_mocked_output(uint8_t outbound)
{
    if( callback_serial_position < 2048 )
    {
        callback_serial_buffer[ callback_serial_position ] = outbound;
        callback_serial_position++;
    }
    else
    {
        TEST_ASSERT_MESSAGE( 1, "Mocked serial interface reports an issue");
    }
}

eui_interface_t mock_interface = { 0 };

TEST_SETUP( InternalEUICallbacks )
{
	//wipe the mocked serial buffer
    memset(callback_serial_buffer, 0, sizeof(callback_serial_buffer));
    callback_serial_position = 0;

    //reset the state of everything else
	test_char = 'a';
	test_uint = 21;
	
	setup_identifier( (char*)"a", 1 );
    setup_dev_msg(internal_callback_test_store, ARR_ELEM(internal_callback_test_store));
    mock_interface.output_func = &callback_mocked_output;
    setup_interface( &mock_interface, 1);
}

TEST_TEAR_DOWN( InternalEUICallbacks )
{
    //run after each test

}

TEST( InternalEUICallbacks, announce_board )
{
	//expect the library version, board ID and session ID (lv, bi, si)
	announce_board();

	//ground-truth response
    uint8_t expected[] = { 
    	//lv 3x uint8
        0x01,               //preamble
        0x03, 0x58, 0x02,   //header
        0x6C, 0x76,		    //msgid
        VER_MAJOR, VER_MINOR, VER_PATCH,   //payload
        0x8C, 0x56,         //crc
        0x04,               //EOT

    	//bi uint16 hash of ID
        0x01,               //preamble
        0x02, 0x60, 0x02,   //header
        0x62, 0x69, 		//msgid
        0x87, 0x7C,         //payload
        0xE2, 0x06,         //crc
        0x04,               //EOT

    	//si uint8
        0x01,               //preamble
        0x01, 0x58, 0x02,   //header
        0x73, 0x69,         //msgid 
        0x00,               //payload
        0x2D, 0x81,         //crc
        0x04,               //EOT
    };

    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE( expected, callback_serial_buffer, sizeof(expected), "Annoucement didn't publish expected messages" );
}

TEST( InternalEUICallbacks, announce_dev_msg_readonly )
{
    TEST_IGNORE_MESSAGE("TODO: Establish byte-stream for readonly");
    announce_dev_msg_readonly();

    //ground-truth response
    uint8_t expected[] = { 
        //dms
        0x01,
        0x01, 0x58, 0x03,
        0x64, 0x6D, 0x73,   
        ARR_ELEM(internal_callback_test_store),
        0xFA, 0xED,
        0x04,

        0x01,
    };

    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE( expected, callback_serial_buffer, sizeof(expected), "Dev Message CB didn't publish expected messages" );

}

TEST( InternalEUICallbacks, announce_dev_msg_writable )
{
    TEST_IGNORE_MESSAGE("TODO: Establish byte-stream for writable");
    announce_dev_msg_writable();

    //ground-truth response
    uint8_t expected[] = { 
        //dms
        0x01,
        0x01, 0x58, 0x03,
        0x64, 0x6D, 0x73,   
        ARR_ELEM(internal_callback_test_store),
        0xFA, 0xED,
        0x04,

        0x01,
    };

    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE( expected, callback_serial_buffer, sizeof(expected), "Dev Message CB didn't publish expected messages" );
}

TEST( InternalEUICallbacks, announce_dev_vars_readonly )
{
    TEST_IGNORE_MESSAGE("TODO: Establish byte-stream for read only");
    announce_dev_vars_readonly();

    //ground-truth response
    uint8_t expected[] = { 
        //
        0x01,
        0x01, 0x58, 0x03,
        0x64, 0x6D, 0x73,   //dms
        ARR_ELEM(internal_callback_test_store),
        0xFA, 0xED,
        0x04,

        0x01,
            //todo add the rest here
    };

    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE( expected, callback_serial_buffer, sizeof(expected), "Dev variable transfer didn't match" );
}

TEST( InternalEUICallbacks, announce_dev_vars_writable )
{
    TEST_IGNORE_MESSAGE("TODO: Establish byte-stream for writable");
    announce_dev_vars_writable();

    //ground-truth response
    uint8_t expected[] = { 
        //
        0x01,
        0x01, 0x58, 0x03,
        0x64, 0x6D, 0x73,   //dms
        ARR_ELEM(internal_callback_test_store),
        0xFA, 0xED,
        0x04,

        0x01,
            //todo add the rest here
    };

    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE( expected, callback_serial_buffer, sizeof(expected), "Dev variable transfer didn't match" );
}

TEST( InternalEUICallbacks, send_msgID_list_callback )
{
    //only tests that the function counts the number of messageID's sent, we use the higher level callback test for byte-level tests
    uint16_t msgID_count = 0;
    uint16_t number_ro_expected = 0;
    uint16_t number_rw_expected = 0;

    //test reasonable number of both kinds
    eui_message_t ro_rw_testset[] = {
        EUI_CHAR( "cw",     test_char ),
        EUI_INT8( "iw",     test_uint ),
        EUI_CHAR_RO( "cr",  test_char ),
        EUI_INT8_RO( "ir",  test_uint ),
    };

    number_ro_expected = 2;
    number_rw_expected = 2;
    setup_dev_msg( ro_rw_testset, ARR_ELEM(ro_rw_testset) );

    msgID_count = send_tracked_message_id_list( 0 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_rw_expected, msgID_count, "Base - Writable msgID count incorrect" );

    msgID_count = send_tracked_message_id_list( 1 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_ro_expected, msgID_count, "Base - Read-Only msgID count incorrect" );

    //test an array with nothing
    eui_message_t empty_testset[] = { };

    number_ro_expected = 0;
    number_rw_expected = 0;
    setup_dev_msg( empty_testset, ARR_ELEM(empty_testset) );

    msgID_count = send_tracked_message_id_list( 0 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_rw_expected, msgID_count, "Empty - Writable msgID count incorrect" );

    msgID_count = send_tracked_message_id_list( 1 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_ro_expected, msgID_count, "Empty - Read-Only msgID count incorrect" );

    //test writable only
    eui_message_t rw_testset[] = {
        EUI_CHAR( "cw",     test_char ),
        EUI_INT8( "iw",     test_uint ),
    };

    number_ro_expected = 0;
    number_rw_expected = 2;
    setup_dev_msg( rw_testset, ARR_ELEM(rw_testset) );

    msgID_count = send_tracked_message_id_list( 0 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_rw_expected, msgID_count, "Read Only - Writable msgID count incorrect" );

    msgID_count = send_tracked_message_id_list( 1 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_ro_expected, msgID_count, "Read Only - Read-Only msgID count incorrect" );

    //test writable only
    eui_message_t ro_testset[] = {
        EUI_CHAR_RO( "cr",  test_char ),
        EUI_INT8_RO( "ir",  test_uint ),
    };

    number_ro_expected = 2;
    number_rw_expected = 0;
    setup_dev_msg( ro_testset, ARR_ELEM(ro_testset) );

    msgID_count = send_tracked_message_id_list( 0 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_rw_expected, msgID_count, "Writable Only - Writable msgID count incorrect" );

    msgID_count = send_tracked_message_id_list( 1 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_ro_expected, msgID_count, "Writable Only - Read-Only msgID count incorrect" );

    //test mixed order of vars
    eui_message_t mixed_testset[] = {
        EUI_CHAR( "cw0",     test_char ),
        EUI_INT8( "iw0",     test_uint ),
        EUI_CHAR_RO( "cr0",  test_char ),
        EUI_INT8_RO( "ir0",  test_uint ),

        EUI_CHAR( "cw1",     test_char ),
        EUI_INT8_RO( "ir1",  test_uint ),
        EUI_INT8( "iw1",     test_uint ),
        EUI_CHAR_RO( "cr1",  test_char ),

        EUI_CHAR( "cw2",     test_char ),
        EUI_INT8( "iw2",     test_uint ),
    };

    number_ro_expected = 4;
    number_rw_expected = 6;
    setup_dev_msg( mixed_testset, ARR_ELEM(mixed_testset) );

    msgID_count = send_tracked_message_id_list( 0 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_rw_expected, msgID_count, "Mixed set - Writable msgID count incorrect" );

    msgID_count = send_tracked_message_id_list( 1 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_ro_expected, msgID_count, "Mixed set - Read-Only msgID count incorrect" );

    //test many vars (should force a few messages to be sent)
    eui_message_t large_testset[] = {
        EUI_CHAR( "cw0",     test_char ),
        EUI_INT8( "iw0",     test_uint ),
        EUI_CHAR_RO( "cr0",  test_char ),
        EUI_INT8_RO( "ir0",  test_uint ),
        EUI_CHAR( "cw1",     test_char ),
        EUI_INT8( "iw1",     test_uint ),
        EUI_CHAR_RO( "cr1",  test_char ),
        EUI_INT8_RO( "ir1",  test_uint ),
        EUI_CHAR( "cw2",     test_char ),
        EUI_INT8( "iw2",     test_uint ),
        EUI_CHAR_RO( "cr2",  test_char ),
        EUI_INT8_RO( "ir2",  test_uint ),
        EUI_CHAR( "cw3",     test_char ),
        EUI_INT8( "iw3",     test_uint ),
        EUI_CHAR_RO( "cr3",  test_char ),
        EUI_INT8_RO( "ir3",  test_uint ),
        EUI_CHAR( "cw4",     test_char ),
        EUI_INT8( "iw4",     test_uint ),
        EUI_CHAR_RO( "cr4",  test_char ),
        EUI_INT8_RO( "ir4",  test_uint ),
        EUI_CHAR( "cw5",     test_char ),
        EUI_INT8( "iw5",     test_uint ),
        EUI_CHAR_RO( "cr5",  test_char ),
        EUI_INT8_RO( "ir5",  test_uint ),
        EUI_CHAR( "cw6",     test_char ),
        EUI_INT8( "iw6",     test_uint ),
        EUI_CHAR_RO( "cr6",  test_char ),
        EUI_INT8_RO( "ir6",  test_uint ),
        EUI_CHAR( "cw7",     test_char ),
        EUI_INT8( "iw7",     test_uint ),
        EUI_CHAR_RO( "cr7",  test_char ),
        EUI_INT8_RO( "ir7",  test_uint ),
        EUI_CHAR( "cw8",     test_char ),
        EUI_INT8( "iw8",     test_uint ),
        EUI_CHAR_RO( "cr8",  test_char ),
        EUI_INT8_RO( "ir8",  test_uint ),
        EUI_CHAR( "cw10",     test_char ),
        EUI_INT8( "iw10",     test_uint ),
        EUI_CHAR_RO( "cr10",  test_char ),
        EUI_INT8_RO( "ir10",  test_uint ),
        EUI_CHAR( "cw11",     test_char ),
        EUI_INT8( "iw11",     test_uint ),
        EUI_CHAR_RO( "cr11",  test_char ),
        EUI_INT8_RO( "ir11",  test_uint ),
        EUI_CHAR( "cw12",     test_char ),
        EUI_INT8( "iw12",     test_uint ),
        EUI_CHAR_RO( "cr12",  test_char ),
        EUI_INT8_RO( "ir12",  test_uint ),
        EUI_CHAR( "cw13",     test_char ),
        EUI_INT8( "iw13",     test_uint ),
        EUI_CHAR_RO( "cr13",  test_char ),
        EUI_INT8_RO( "ir13",  test_uint ),
        EUI_CHAR( "cw14",     test_char ),
        EUI_INT8( "iw14",     test_uint ),
        EUI_CHAR_RO( "cr14",  test_char ),
        EUI_INT8_RO( "ir14",  test_uint ),
        EUI_CHAR( "cw15",     test_char ),
        EUI_INT8( "iw15",     test_uint ),
        EUI_CHAR_RO( "cr15",  test_char ),
        EUI_INT8_RO( "ir15",  test_uint ),
    };

    number_ro_expected = 30;
    number_rw_expected = 30;
    setup_dev_msg( large_testset, ARR_ELEM(large_testset) );

    msgID_count = send_tracked_message_id_list( 0 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_rw_expected, msgID_count, "Large set - Writable msgID count incorrect" );

    msgID_count = send_tracked_message_id_list( 1 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_ro_expected, msgID_count, "Large set - Read-Only msgID count incorrect" );
}

TEST( InternalEUICallbacks, send_variable_callback )
{
    uint16_t number_sent = 0;

    number_sent = send_tracked_variables( 0 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_rw_expected, number_sent, "Writable variable count incorrect" );

    number_sent = send_tracked_variables( 1 );
    TEST_ASSERT_EQUAL_INT_MESSAGE( number_ro_expected, number_sent, "Read-Only variable count incorrect" );

}

TEST( InternalEUICallbacks, setup_identifier )
{
    TEST_IGNORE_MESSAGE("TODO: Test boardID setup");
}
