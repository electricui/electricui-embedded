#include "../../eui_serial_transport.h"
#include "unity.h"
#include "unity_fixture.h"
#include <string.h>

TEST_GROUP( SerialEncoder );

//mock an outbound putc style per-byte interface
uint8_t serial_buffer[1024] = { 0 };
uint16_t serial_position    = 0;
uint8_t encode_result       = 0;

void byte_into_buffer(uint8_t outbound)
{
    if( serial_position < 1024 )
    {
        serial_buffer[ serial_position ] = outbound;
        serial_position++;
    }
    else
    {
        TEST_ASSERT_MESSAGE( 1, "Mocked serial interface reports an issue");
    }
}

TEST_SETUP( SerialEncoder )
{
    //run before each test
    memset(serial_buffer, 0, sizeof(serial_buffer));
    serial_position = 0;
    encode_result = 0;
}

TEST_TEAR_DOWN( SerialEncoder )
{
    //run after each test

}

TEST( SerialEncoder, encode_packet_simple )
{
    //input parameters
    const char * test_message = "abc";
    uint8_t test_payload[] = { 
        42, 
    };
    
    eui_pkt_settings_t test_simple_header;
    test_simple_header.internal  = 0;
    test_simple_header.response  = 0;
    test_simple_header.type      = 5;

    //test it against our mocked buffer
    encode_result = encode_packet_simple(&byte_into_buffer, &test_simple_header, test_message, sizeof(test_payload), &test_payload);

    //ground-truth
    uint8_t expected[] = { 
        0x01,               //preamble
        0x01, 0x14, 0x03,   //header
        0x61, 0x62, 0x63,   //msgid
        //0x03,             //offset 
        0x2A,               //payload
        0x64, 0xBA,         //crc
        0x04                //EOT
    };

    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE( expected, serial_buffer, sizeof(expected), "Payload not valid" );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, encode_result, "Encoder didn't return expected status code" );
}
//test a few manually formed packets
//test failure cases
//test when args are null


TEST( SerialEncoder, encode_packet )
{
    //input parameters
    const char * test_message = "abc";
    uint8_t test_payload[] = { 
        42, 
    };
    uint16_t offset = 0;
    
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 5; //int8
    test_header.acknum     = 0;
    test_header.offset     = (offset) ? 1 : 0;
    test_header.id_len     = strlen(test_message);
    test_header.data_len   = sizeof(test_payload);

    //test it against our mocked buffer
    encode_result = encode_packet( &byte_into_buffer, &test_header, test_message, offset, &test_payload );

    //ground-truth
    uint8_t expected[] = { 
        0x01,               //preamble
        0x01, 0x14, 0x03,   //header
        0x61, 0x62, 0x63,   //msgid
        //0x03,             //offset 
        0x2A,               //payload
        0x64, 0xBA,         //crc
        0x04                //EOT
    };

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, serial_buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, encode_result, "Encoder didn't return expected status code" );
}

TEST( SerialEncoder, encode_packet_short_id )
{
    //input parameters
    const char * test_message = "a";
    uint8_t test_payload[] = { 
        42, 
    };
    uint16_t offset = 0;
    
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 5; //int8
    test_header.acknum     = 0;
    test_header.offset     = (offset) ? 1 : 0;
    test_header.id_len     = strlen(test_message);
    test_header.data_len   = sizeof(test_payload);

    //test it against our mocked buffer
    encode_result = encode_packet( &byte_into_buffer, &test_header, test_message, offset, &test_payload );

    //ground-truth
    uint8_t expected[] = { 
        0x01,               //preamble
        0x01, 0x14, 0x01,   //header
        0x61,               //msgid
        0x2A,               //payload
        0x08, 0xE0,         //crc
        0x04                //EOT
    };

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, serial_buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, encode_result, "Encoder didn't return expected status code" );
}

TEST( SerialEncoder, encode_packet_long_id )
{
    //input parameters
    const char * test_message = "abcdefghijklmno";
    uint8_t test_payload[] = { 
        42, 
    };
    uint16_t offset = 0;
    
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 5; //int8
    test_header.acknum     = 0;
    test_header.offset     = (offset) ? 1 : 0;
    test_header.id_len     = strlen(test_message);
    test_header.data_len   = sizeof(test_payload);

    //test it against our mocked buffer
    encode_result = encode_packet( &byte_into_buffer, &test_header, test_message, offset, &test_payload );

    //ground-truth
    uint8_t expected[] = { 
        0x01,               //preamble
        0x01, 0x14, 0x0f,   //header
        0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, //msgid
        0x2A,               //payload
        0x05, 0x8B,         //crc
        0x04                //EOT
    };

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, serial_buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, encode_result, "Encoder didn't return expected status code" );
}

TEST( SerialEncoder, encode_packet_internal )
{
    //input parameters
    const char * test_message = "abc";
    uint8_t test_payload[] = { 
        42, 
    };
    uint16_t offset = 0;
    
    eui_header_t test_header;
    test_header.internal   = 1;
    test_header.response   = 0;
    test_header.type       = 5; //int8
    test_header.acknum     = 0;
    test_header.offset     = (offset) ? 1 : 0;
    test_header.id_len     = strlen(test_message);
    test_header.data_len   = sizeof(test_payload);

    //test it against our mocked buffer
    encode_result = encode_packet( &byte_into_buffer, &test_header, test_message, offset, &test_payload );

    //ground-truth
    uint8_t expected[] = { 
        0x01,               //preamble
        0x01, 0x54, 0x03,   //header
        0x61, 0x62, 0x63,   //msgid
        0x2A,               //payload
        0x74, 0xD0,         //crc
        0x04                //EOT
    };

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, serial_buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, encode_result, "Encoder didn't return expected status code" );
}

TEST( SerialEncoder, encode_packet_response )
{
    //input parameters
    const char * test_message = "abc";
    uint8_t test_payload[] = { 
        42, 
    };
    uint16_t offset = 0;
    
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 1;
    test_header.type       = 5; //int8
    test_header.acknum     = 0;
    test_header.offset     = (offset) ? 1 : 0;
    test_header.id_len     = strlen(test_message);
    test_header.data_len   = sizeof(test_payload);

    //test it against our mocked buffer
    encode_result = encode_packet( &byte_into_buffer, &test_header, test_message, offset, &test_payload );

    //ground-truth
    uint8_t expected[] = { 
        0x01,               //preamble
        0x01, 0x14, 0x13,   //header
        0x61, 0x62, 0x63,   //msgid
        0x2A,               //payload
        0x3E, 0xBE,         //crc
        0x04                //EOT
    };

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, serial_buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, encode_result, "Encoder didn't return expected status code" );
}

TEST( SerialEncoder, encode_packet_acknum)
{
    //input parameters
    const char * test_message = "abc";
    uint8_t test_payload[] = { 
        42, 
    };
    uint16_t offset = 0;
    
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 5; //int8
    test_header.acknum     = 3;
    test_header.offset     = (offset) ? 1 : 0;
    test_header.id_len     = strlen(test_message);
    test_header.data_len   = sizeof(test_payload);

    //test it against our mocked buffer
    encode_result = encode_packet( &byte_into_buffer, &test_header, test_message, offset, &test_payload );

    //ground-truth
    uint8_t expected[] = { 
        0x01,               //preamble
        0x01, 0x14, 0x63,   //header
        0x61, 0x62, 0x63,   //msgid
        0x2A,               //payload
        0xB8, 0xA3,         //crc
        0x04                //EOT
    };

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, serial_buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, encode_result, "Encoder didn't return expected status code" );
}

TEST( SerialEncoder, encode_packet_float )
{
    //input parameters
    const char * test_message = "abc";
    uint8_t test_payload[] = { 
        0x14, 0xAE, 0x29, 0x42 
    };
    uint16_t offset = 0;
    
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 11; //float (4byte)
    test_header.acknum     = 0;
    test_header.offset     = (offset) ? 1 : 0;
    test_header.id_len     = strlen(test_message);
    test_header.data_len   = sizeof(test_payload);

    //test it against our mocked buffer
    encode_result = encode_packet( &byte_into_buffer, &test_header, test_message, offset, &test_payload );

    //ground-truth
    uint8_t expected[] = { 
        0x01,               //preamble
        0x04, 0x2c, 0x03,   //header
        0x61, 0x62, 0x63,   //msgid
        0x14, 0xAE, 0x29, 0x42, //payload
        0x8B, 0x1D,         //crc
        0x04                //EOT
    };

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, serial_buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, encode_result, "Encoder didn't return expected status code" );
}

//test null pointer
//test null msgid
//test null payloads
//test offset functionality
//test ack functionality