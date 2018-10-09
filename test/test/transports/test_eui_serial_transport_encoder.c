#include "unity.h"
#include <string.h>
 
// MODULE UNDER TEST
#include "eui_serial_transport.h"
#include "eui_crc.h"

// DEFINITIONS 
 
// PRIVATE TYPES
 
// PRIVATE DATA
//mock an outbound putc style per-byte interface
uint8_t serial_buffer[1024] = { 0xFF };
uint16_t serial_position    = 0;
uint8_t encode_result       = 0;

// PRIVATE FUNCTIONS
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
 
// SETUP, TEARDOWN
 
void setUp(void)
{
    memset(serial_buffer, 0xFF, sizeof(serial_buffer));
    serial_position = 0;
    encode_result = 0;
}
 
void tearDown(void)
{

}

// TESTS

void test_encode_packet_simple( void )
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


void test_encode_packet( void )
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

void test_encode_packet_short_id( void )
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

void test_encode_packet_long_id( void )
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

void test_encode_packet_internal( void )
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

void test_encode_packet_response( void )
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

void test_encode_packet_acknum( void ){
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

void test_encode_packet_float( void )
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

void test_encode_packet_offset_last( void )
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
void test_encode_packet_offset( void )
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
    test_header.type       = 5; //int8 (4byte)
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

//use offsets to encode 2 bytes from a 4 byte array
void test_encode_packet_large( void )
{
    //input parameters
    const char * test_message = "abc";
    uint8_t test_payload[] = { 
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    };

    uint16_t offset = 0;
    
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 5;
    test_header.acknum     = 0;
    test_header.offset     = 0; 
    test_header.id_len     = strlen(test_message);
    test_header.data_len   = 112;   //70

    //test it against our mocked buffer
    encode_result = encode_packet( &byte_into_buffer, &test_header, test_message, offset, &test_payload );

    //ground-truth
    uint8_t expected[] = { 
        0x00,
        0x07,
        0x70, 0x14, 0x03,       //header
        0x61, 0x62, 0x63,       //msgid
        0x72, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
        0xBE, 0xF4,             //crc
    };

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, serial_buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, encode_result, "Encoder didn't return expected status code" );

}

void test_encode_packet_null_output_pointer( void )
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
    test_header.id_len     = 0;
    test_header.data_len   = sizeof(test_payload);

    //test it against our mocked buffer
    encode_result = encode_packet( 0, &test_header, 0, offset, &test_payload );

    TEST_ASSERT_NOT_EQUAL( 0, encode_result );
}

void test_encode_packet_null_header( void )
{
    //input parameters
    const char * test_message = "abc";
    uint8_t test_payload[] = { 
        42, 
    };
    uint16_t offset = 0;
    
    // eui_header_t test_header;

    //test it against our mocked buffer
    encode_result = encode_packet( &byte_into_buffer, 0, test_message, offset, &test_payload );

    TEST_ASSERT_NOT_EQUAL( 0, encode_result );
}

void test_encode_packet_null_id( void )
{
    //input parameters
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
    test_header.id_len     = 0;
    test_header.data_len   = sizeof(test_payload);

    //test it against our mocked buffer
    encode_result = encode_packet( &byte_into_buffer, &test_header, 0, offset, &test_payload );

    TEST_ASSERT_NOT_EQUAL( 0, encode_result );
}

void test_encode_packet_null_data_pointer( void )
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
    encode_result = encode_packet( &byte_into_buffer, &test_header, test_message, offset, 0 );

    TEST_ASSERT_NOT_EQUAL( 0, encode_result );
}

void test_encode_packet_null_everything( void )
{
    encode_result = encode_packet( 0, 0, 0, 0, 0 );

    TEST_ASSERT_NOT_EQUAL( 0, encode_result );
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
