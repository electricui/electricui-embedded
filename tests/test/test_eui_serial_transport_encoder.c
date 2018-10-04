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
        0x00,
        0x0A,
        0x01, 0x14, 0x03,   //header
        0x61, 0x62, 0x63,   //msgid
        0x2A,               //payload
        0x64, 0xBA,         //crc
        0x00
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
        0x00,
        0x0A,
        0x01, 0x14, 0x03,   //header
        0x61, 0x62, 0x63,   //msgid
        //0x03,             //offset 
        0x2A,               //payload
        0x64, 0xBA,         //crc
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
        0x00,
        0x08,
        0x01, 0x14, 0x01,   //header
        0x61,               //msgid
        0x2A,               //payload
        0x08, 0xE0,         //crc
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
        0x00,
        0x16,
        0x01, 0x14, 0x0f,   //header
        0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, //msgid
        0x2A,               //payload
        0x05, 0x8B,         //crc
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
        0x00,
        0x0A,
        0x01, 0x54, 0x03,   //header
        0x61, 0x62, 0x63,   //msgid
        0x2A,               //payload
        0x74, 0xD0,         //crc
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
        0x00,
        0x0A,
        0x01, 0x14, 0x13,   //header
        0x61, 0x62, 0x63,   //msgid
        0x2A,               //payload
        0x3E, 0xBE,         //crc
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
        0x00,
        0x0A,
        0x01, 0x14, 0x63,   //header
        0x61, 0x62, 0x63,   //msgid
        0x2A,               //payload
        0xB8, 0xA3,         //crc
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
        0x00,
        0x0D,
        0x04, 0x2c, 0x03,   //header
        0x61, 0x62, 0x63,   //msgid
        0x14, 0xAE, 0x29, 0x42, //payload
        0x8B, 0x1D,         //crc
    };

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, serial_buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, encode_result, "Encoder didn't return expected status code" );
}

TEST( SerialEncoder, encode_packet_offset_last )
{
    //input parameters
    const char * test_message = "abc";
    uint8_t test_payload[] = { 
        0x14, 0xAE, 0x29, 0x42 
    };
    uint16_t offset = 0;    //we force it, but use an offset of 0x0000 (base address)
    
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 11; //float (4byte)
    test_header.acknum     = 0;
    test_header.offset     = 1; //force an offset
    test_header.id_len     = strlen(test_message);
    test_header.data_len   = sizeof(test_payload);

    //test it against our mocked buffer
    encode_result = encode_packet( &byte_into_buffer, &test_header, test_message, offset, &test_payload );

    //ground-truth
    uint8_t expected[] = { 
        0x00,
        0x07,
        0x04, 0xAC, 0x03,       //header
        0x61, 0x62, 0x63,       //msgid
        0x01, 0x07,             //offset is 00, 00 but we have COBS on-top
        0x14, 0xAE, 0x29, 0x42, //payload
        0x48, 0x31,             //crc
    };

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, serial_buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, encode_result, "Encoder didn't return expected status code" );
}

//use offsets to encode 2 bytes from a 4 byte array
TEST( SerialEncoder, encode_packet_offset )
{
    //input parameters
    const char * test_message = "abc";
    uint8_t test_payload[] = { 
        0x14, 0xAE, 0x29, 0x42 
    };
    uint16_t offset = 2;    //2 byte offset
    
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 5; //float (4byte)
    test_header.acknum     = 0;
    test_header.offset     = 1; //force an offset
    test_header.id_len     = strlen(test_message);
    test_header.data_len   = 2;

    //test it against our mocked buffer
    encode_result = encode_packet( &byte_into_buffer, &test_header, test_message, offset, &test_payload );

    //ground-truth
    uint8_t expected[] = { 
        0x00,
        0x08,
        0x02, 0x94, 0x03,       //header
        0x61, 0x62, 0x63,       //msgid
        0x02, 0x05,             //offset is 02, 00 but we have COBS on-top
        0x29, 0x42,             //payload
        0xD8, 0x96,             //crc
    };

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, serial_buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, encode_result, "Encoder didn't return expected status code" );
}

/*
    for( uint16_t i = 0; i < sizeof(expected); i++)
    {
        if(serial_buffer[i] == expected[i])
        {
            printf("[%d]: %X\n", i, serial_buffer[i]);
        }
        else
        {
            printf("[%d]: %X expected %X\n", i, serial_buffer[i], expected[i]);
        }
    }
*/

//test null pointer
//test null msgid
//test null payloads
