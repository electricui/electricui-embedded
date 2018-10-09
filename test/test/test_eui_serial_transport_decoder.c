#include "unity.h"
#include <stdlib.h>
 
// MODULE UNDER TEST
#include "eui_serial_transport.h"
 
// DEFINITIONS 
 
// PRIVATE TYPES
 
// PRIVATE DATA
uint8_t decode_result = 0;

// PRIVATE FUNCTIONS
 
// SETUP, TEARDOWN
 
void setUp(void)
{
    decode_result = 0;
}
 
void tearDown(void)
{

}

// TESTS

void test_decode_packet( void )
{
    eui_packet_t test_interface = {0};
    uint8_t inbound_bytes[] = { 
        0x00,
        0x0A,
        0x01, 0x14, 0x03,   //header
        0x61, 0x62, 0x63,   //msgid
        0x2A,               //payload
        0x64, 0xBA,         //crc
    };

    uint8_t expected_payload[] = { 
        0x2A,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( parser_complete, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.header.data_len,    "Unexpected data_length" );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 5, test_interface.header.type,        "Unexpected type"   );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.internal,    "Expected dev msg"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.offset,      "Unexpected offset bit"         );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 3, test_interface.header.id_len,      "Msg length err"    );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.response,    "Didn't expect a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.acknum,      "Unexpected ack number"         );

    TEST_ASSERT_EQUAL_STRING( "abc", test_interface.msgid_in);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Wasn't expecting offset packet" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.data_in, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0xBA64, test_interface.crc_in );
}

void test_decode_packet_short_id( void )
{
    eui_packet_t test_interface = {0};
    uint8_t inbound_bytes[] = {
        0x00,
        0x08,
        0x01, 0x14, 0x01,   //header
        0x61,               //msgid
        0x2A,               //payload
        0x08, 0xE0,         //crc
    };

    uint8_t expected_payload[] = { 
        0x2A,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( parser_complete, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.header.data_len,    "Unexpected data_length"        );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 5, test_interface.header.type,        "Unexpected type"               );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.internal,    "Expected dev msg"              );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.offset,      "Unexpected offset bit"         );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.header.id_len,      "Msg length err"                );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.response,    "Didn't expect a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.acknum,      "Unexpected ack number"         );

    TEST_ASSERT_EQUAL_STRING( "a", test_interface.msgid_in);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Wasn't expecting offset packet" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.data_in, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0xE008, test_interface.crc_in );
}

void test_decode_packet_long_id( void )
{
    eui_packet_t test_interface = {0};
    uint8_t inbound_bytes[] = { 
        0x00,
        0x16,
        0x01, 0x14, 0x0f,   //header
        0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, //msgid
        0x2A,               //payload
        0x05, 0x8B,         //crc
    };

    uint8_t expected_payload[] = { 
        0x2A,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( parser_complete, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.header.data_len,    "Unexpected data_length"        );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 5, test_interface.header.type,        "Unexpected type"               );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.internal,    "Expected dev msg"              );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.offset,      "Unexpected offset bit"         );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 15, test_interface.header.id_len,     "Msg length err"                );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.response,    "Didn't expect a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.acknum,      "Unexpected ack number"         );

    TEST_ASSERT_EQUAL_STRING( "abcdefghijklmno", test_interface.msgid_in);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Wasn't expecting offset packet" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.data_in, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0x8B05, test_interface.crc_in );
}

void test_decode_packet_internal( void )
{
    eui_packet_t test_interface = {0};
    uint8_t inbound_bytes[] = { 
        0x00,
        0x0A,
        0x01, 0x54, 0x03,   //header
        0x61, 0x62, 0x63,   //msgid
        0x2A,               //payload
        0x74, 0xD0,         //crc
    };

    uint8_t expected_payload[] = { 
        0x2A,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( parser_complete, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.header.data_len,    "Unexpected data_length" );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 5, test_interface.header.type,        "Unexpected type"   );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.header.internal,    "Expected internal msg"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.offset,      "Unexpected offset bit"         );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 3, test_interface.header.id_len,      "Msg length err"    );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.response,    "Didn't expect a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.acknum,      "Unexpected ack number"         );

    TEST_ASSERT_EQUAL_STRING( "abc", test_interface.msgid_in);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Wasn't expecting offset packet" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.data_in, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0xD074, test_interface.crc_in );
}

void test_decode_packet_response( void )
{
    eui_packet_t test_interface = {0};
    uint8_t inbound_bytes[] = { 
        0x00,
        0x0A,
        0x01, 0x14, 0x13,   //header
        0x61, 0x62, 0x63,   //msgid
        //0x03,             //offset 
        0x2A,               //payload
        0x3E, 0xBE,         //crc
    };

    uint8_t expected_payload[] = { 
        0x2A,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( parser_complete, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.header.data_len,    "Unexpected data_length" );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 5, test_interface.header.type,        "Unexpected type"   );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.internal,    "Expected dev msg"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.offset,      "Unexpected offset bit"    );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 3, test_interface.header.id_len,      "Msg length err"    );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.header.response,    "Expected a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.acknum,      "Unexpected ack number"    );

    TEST_ASSERT_EQUAL_STRING( "abc", test_interface.msgid_in);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Wasn't expecting offset packet" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.data_in, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0xBE3E, test_interface.crc_in );
}

void test_decode_packet_acknum( void ){
    eui_packet_t test_interface = {0};
    uint8_t inbound_bytes[] = { 
        0x00,
        0x0A,
        0x01, 0x14, 0x63,   //header
        0x61, 0x62, 0x63,   //msgid
        0x2A,               //payload
        0xB8, 0xA3,         //crc
    };

    uint8_t expected_payload[] = { 
        0x2A,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( parser_complete, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed decode_results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.header.data_len,    "Unexpected data_length" );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 5, test_interface.header.type,       "Unexpected type"   );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.internal,    "Expected dev msg"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.offset,      "Unexpected offset bit"         );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 3, test_interface.header.id_len,      "Msg length err"    );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.response,    "Didn't expect a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 3, test_interface.header.acknum,      "Incorrect ack number"         );

    TEST_ASSERT_EQUAL_STRING( "abc", test_interface.msgid_in);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Wasn't expecting offset packet" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.data_in, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0xA3B8, test_interface.crc_in );
}

void test_decode_packet_float( void )
{
    eui_packet_t test_interface = {0};
    uint8_t inbound_bytes[] = { 
        0x00,
        0x0D,               //frame offset
        0x04, 0x2c, 0x03,   //header
        0x61, 0x62, 0x63,   //msgid
        0x14, 0xAE, 0x29, 0x42, //payload
        0x8B, 0x1D,         //crc
    };

    uint8_t expected_payload[] = { 
        0x14, 0xAE, 0x29, 0x42,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( parser_complete, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 4, test_interface.header.data_len,    "Unexpected data_length" );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 11, test_interface.header.type,       "Unexpected type"   );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.internal,    "Expected dev msg"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.offset,      "Unexpected offset bit"         );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 3, test_interface.header.id_len,      "Msg length err"    );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.response,    "Didn't expect a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.acknum,      "Unexpected ack number"         );

    TEST_ASSERT_EQUAL_STRING( "abc", test_interface.msgid_in);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Wasn't expecting offset packet" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.data_in, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0x1D8B, test_interface.crc_in );
}

void test_decode_packet_invalidCRC( void )
{
    //we expect the decoder to error on either CRC byte mismatch, it will reset when we send in the EOT or second byte.
    //therefore send only enough bytes to get it up to the point where it reports the failure

    //test the second byte being incorrect
    eui_packet_t test_interface_2 = {0};
    uint8_t invalid_second_byte[] = { 
        0x00,
        0x0A,
        0x01, 0x14, 0x03,
        0x61, 0x62, 0x63,
        0x2A,
        0x64, 0xFF,         //crc should be 0x64BA
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(invalid_second_byte); rxByte++ )
    {
        decode_result = decode_packet( invalid_second_byte[rxByte], &test_interface_2 );
    }

    //peek into the CRC, we expect it still to be correct
    TEST_ASSERT_EQUAL_UINT16( 0xBA64, test_interface_2.crc_in );

    //the decoder should return the error flag    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( parser_error, decode_result, "Decoder didn't error on invalid CRC byte2" );

    //test the first byte being incorrect
    eui_packet_t test_interface_1 = {0};
    uint8_t invalid_first_byte[] = { 
        0x00,
        0x0A,
        0x01, 0x14, 0x03,
        0x61, 0x62, 0x63,
        0x2A,
        0xFF, //skip second byte       //crc should be 0x64BA
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(invalid_first_byte); rxByte++ )
    {
        decode_result = decode_packet( invalid_first_byte[rxByte], &test_interface_1 );
    }

    //peek into the CRC, we expect it still to be correct
    TEST_ASSERT_EQUAL_UINT16( 0xBA64, test_interface_1.crc_in );

    //the decoder should return the error flag    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( parser_error, decode_result, "Decoder didn't error on invalid CRC byte1" );
}

void test_decode_packet_offset_last( void )
{
    eui_packet_t test_interface = {0};
    uint8_t inbound_bytes[] = { 
        0x00,
        0x07,
        0x04, 0xAC, 0x03,       //header
        0x61, 0x62, 0x63,       //msgid
        0x01, 0x07,             //offset is 00, 00 but we have COBS on-top
        0x14, 0xAE, 0x29, 0x42, //payload
        0x48, 0x31,             //crc
    };

    uint8_t expected_payload[] = { 
        0x14, 0xAE, 0x29, 0x42,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( parser_complete, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 4, test_interface.header.data_len,    "Unexpected data_length" );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 11, test_interface.header.type,       "Unexpected type"   );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.internal,    "Expected dev msg"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.header.offset,      "Unexpected offset bit"         );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 3, test_interface.header.id_len,      "Msg length err"    );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.response,    "Didn't expect a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.acknum,      "Unexpected ack number"         );

    TEST_ASSERT_EQUAL_STRING( "abc", test_interface.msgid_in);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Offset should be zero" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.data_in, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0x3148, test_interface.crc_in );

}


void test_decode_packet_offset( void )
{
    eui_packet_t test_interface = {0};
    uint8_t inbound_bytes[] = { 
        0x00,
        0x08,
        0x02, 0x94, 0x03,       //header
        0x61, 0x62, 0x63,       //msgid
        0x02, 0x05,             //offset is 02, 00 but we have COBS on-top
        0x29, 0x42,             //payload
        0xD8, 0x96,             //crc
    };

    uint8_t expected_payload[] = { 
        0x29, 0x42,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( parser_complete, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 2, test_interface.header.data_len,    "Unexpected data_length" );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 5, test_interface.header.type,        "Unexpected type"   );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.internal,    "Expected dev msg"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.header.offset,      "Unexpected offset bit"         );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 3, test_interface.header.id_len,      "Msg length err"    );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.response,    "Didn't expect a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.acknum,      "Unexpected ack number"         );

    TEST_ASSERT_EQUAL_STRING( "abc", test_interface.msgid_in);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 2, test_interface.offset_in, "Offset should be two bytes" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.data_in, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0x96D8, test_interface.crc_in );

}

void test_decode_packet_large( void )
{
    eui_packet_t test_interface = {0};
    uint8_t inbound_bytes[] = { 
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

    uint8_t expected_payload[] = { 
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( parser_complete, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( sizeof(expected_payload), test_interface.header.data_len,    "Unexpected data_length" );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 5, test_interface.header.type,        "Unexpected type"   );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.internal,    "Expected dev msg"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.offset,      "Unexpected offset bit"         );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 3, test_interface.header.id_len,      "Msg length err"    );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.response,    "Didn't expect a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.header.acknum,      "Unexpected ack number"         );

    TEST_ASSERT_EQUAL_STRING( "abc", test_interface.msgid_in);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Wasn't expecting offset packet" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.data_in, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0xF4BE, test_interface.crc_in );
}