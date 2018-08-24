#include "../../electricui.h"
#include "unity.h"
#include "unity_fixture.h"
#include <string.h>

TEST_GROUP( InternalEUICallbacks );

char      test_char    = 'a';
uint8_t   test_uint    = 21;

//developer-space messages
euiMessage_t internal_callback_test_store[] = {
    { .msgID = "ch",    .type = TYPE_CHAR,    .size = sizeof(test_char),    .payload = &test_char	},
    { .msgID = "si8",   .type = TYPE_INT8,    .size = sizeof(test_uint),	.payload = &test_uint   },
};

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
    parserOutputFunc = &callback_mocked_output;
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
        0x01, 0x03, 0x01,   //payload
        0x68, 0x8E,         //crc
        0x04,               //EOT

    	//bi uint16 hash of ID
        0x01,               //preamble
        0x02, 0x60, 0x02,   //header
        0x62, 0x69, 		//msgid
        0x9D, 0x77,         //payload
        0xDC, 0x12,         //crc
        0x04,               //EOT

    	//si uint8
        0x01,               //preamble
        0x01, 0x14, 0x03,   //header
        0x61, 0x62, 0x63,   //msgid
        0x2A,               //payload
        0x64, 0xBA,         //crc
        0x04,                //EOT
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

TEST( InternalEUICallbacks, setup_identifier )
{
    TEST_IGNORE_MESSAGE("TODO: Test boardID setup");
}
