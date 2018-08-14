#include "../../eui_serial_transport.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP( SerialDecoder );

uint8_t decode_result = 0;


TEST_SETUP( SerialDecoder )
{
    //run before each test
    decode_result = 0;
}

TEST_TEAR_DOWN( SerialDecoder )
{
    //run after each test

}

TEST( SerialDecoder, decode_packet )
{
    eui_interface test_interface = {0};
    uint8_t inbound_bytes[] = { 
        0x01,               //preamble
        0x01, 0x14, 0x03,   //header
        0x61, 0x62, 0x63,   //msgid
        //0x03,             //offset 
        0x2A,               //payload
        0x64, 0xBA,         //crc
        0x04                //EOT
    };

    uint8_t expected_payload[] = { 
        0x2A,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 1, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.inboundHeader.data_len,    "Unexpected data_length" );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 5, test_interface.inboundHeader.type,        "Unexpected type"   );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.internal,    "Expected dev msg"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.offset,      "Unexpected offset bit"         );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 3, test_interface.inboundHeader.id_len,      "Msg length err"    );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.response,    "Didn't expect a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.acknum,      "Unexpected ack number"         );

    TEST_ASSERT_EQUAL_STRING( "abc", test_interface.inboundID);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.inboundOffset, "Wasn't expecting offset packet" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.inboundData, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0xBA64, test_interface.runningCRC );
}

TEST( SerialDecoder, decode_packet_short_id )
{
    eui_interface test_interface = {0};
    uint8_t inbound_bytes[] = { 
        0x01,               //preamble
        0x01, 0x14, 0x01,   //header
        0x61,               //msgid
        0x2A,               //payload
        0x08, 0xE0,         //crc
        0x04                //EOT
    };

    uint8_t expected_payload[] = { 
        0x2A,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 1, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.inboundHeader.data_len,    "Unexpected data_length"        );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 5, test_interface.inboundHeader.type,        "Unexpected type"               );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.internal,    "Expected dev msg"              );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.offset,      "Unexpected offset bit"         );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.inboundHeader.id_len,      "Msg length err"                );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.response,    "Didn't expect a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.acknum,      "Unexpected ack number"         );

    TEST_ASSERT_EQUAL_STRING( "a", test_interface.inboundID);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.inboundOffset, "Wasn't expecting offset packet" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.inboundData, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0xE008, test_interface.runningCRC );
}

TEST( SerialDecoder, decode_packet_long_id )
{
    eui_interface test_interface = {0};
    uint8_t inbound_bytes[] = { 
        0x01,               //preamble
        0x01, 0x14, 0x0f,   //header
        0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, //msgid
        0x2A,               //payload
        0x05, 0x8B,         //crc
        0x04                //EOT
    };

    uint8_t expected_payload[] = { 
        0x2A,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 1, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.inboundHeader.data_len,    "Unexpected data_length"        );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 5, test_interface.inboundHeader.type,        "Unexpected type"               );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.internal,    "Expected dev msg"              );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.offset,      "Unexpected offset bit"         );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 15, test_interface.inboundHeader.id_len,      "Msg length err"               );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.response,    "Didn't expect a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.acknum,      "Unexpected ack number"         );

    TEST_ASSERT_EQUAL_STRING( "abcdefghijklmno", test_interface.inboundID);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.inboundOffset, "Wasn't expecting offset packet" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.inboundData, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0x8B05, test_interface.runningCRC );
}

TEST( SerialDecoder, decode_packet_internal )
{
    eui_interface test_interface = {0};
    uint8_t inbound_bytes[] = { 
        0x01,               //preamble
        0x01, 0x54, 0x03,   //header
        0x61, 0x62, 0x63,   //msgid
        0x2A,               //payload
        0x74, 0xD0,         //crc
        0x04                //EOT
    };

    uint8_t expected_payload[] = { 
        0x2A,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 1, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.inboundHeader.data_len,    "Unexpected data_length" );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 5, test_interface.inboundHeader.type,        "Unexpected type"   );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.inboundHeader.internal,    "Expected internal msg"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.offset,      "Unexpected offset bit"         );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 3, test_interface.inboundHeader.id_len,      "Msg length err"    );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.response,    "Didn't expect a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.acknum,      "Unexpected ack number"         );

    TEST_ASSERT_EQUAL_STRING( "abc", test_interface.inboundID);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.inboundOffset, "Wasn't expecting offset packet" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.inboundData, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0xD074, test_interface.runningCRC );
}

TEST( SerialDecoder, decode_packet_response )
{
    eui_interface test_interface = {0};
    uint8_t inbound_bytes[] = { 
        0x01,               //preamble
        0x01, 0x14, 0x13,   //header
        0x61, 0x62, 0x63,   //msgid
        //0x03,             //offset 
        0x2A,               //payload
        0x3E, 0xBE,         //crc
        0x04                //EOT
    };

    uint8_t expected_payload[] = { 
        0x2A,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 1, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.inboundHeader.data_len,    "Unexpected data_length" );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 5, test_interface.inboundHeader.type,        "Unexpected type"   );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.internal,    "Expected dev msg"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.offset,      "Unexpected offset bit"    );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 3, test_interface.inboundHeader.id_len,      "Msg length err"    );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.inboundHeader.response,    "Expected a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.acknum,      "Unexpected ack number"    );

    TEST_ASSERT_EQUAL_STRING( "abc", test_interface.inboundID);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.inboundOffset, "Wasn't expecting offset packet" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.inboundData, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0xBE3E, test_interface.runningCRC );
}

TEST( SerialDecoder, decode_packet_acknum)
{
    eui_interface test_interface = {0};
    uint8_t inbound_bytes[] = { 
        0x01,               //preamble
        0x01, 0x14, 0x63,   //header
        0x61, 0x62, 0x63,   //msgid
        0x2A,               //payload
        0xB8, 0xA3,         //crc
        0x04                //EOT
    };

    uint8_t expected_payload[] = { 
        0x2A,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 1, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed decode_results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 1, test_interface.inboundHeader.data_len,    "Unexpected data_length" );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 5, test_interface.inboundHeader.type,       "Unexpected type"   );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.internal,    "Expected dev msg"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.offset,      "Unexpected offset bit"         );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 3, test_interface.inboundHeader.id_len,      "Msg length err"    );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.response,    "Didn't expect a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 3, test_interface.inboundHeader.acknum,      "Incorrect ack number"         );

    TEST_ASSERT_EQUAL_STRING( "abc", test_interface.inboundID);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.inboundOffset, "Wasn't expecting offset packet" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.inboundData, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0xA3B8, test_interface.runningCRC );
}

TEST( SerialDecoder, decode_packet_float )
{
    eui_interface test_interface = {0};
    uint8_t inbound_bytes[] = { 
        0x01,               //preamble
        0x04, 0x2c, 0x03,   //header
        0x61, 0x62, 0x63,   //msgid
        0x14, 0xAE, 0x29, 0x42, //payload
        0x8B, 0x1D,         //crc
        0x04                //EOT
    };

    uint8_t expected_payload[] = { 
        0x14, 0xAE, 0x29, 0x42,
    };

    for( uint16_t rxByte = 0; rxByte < sizeof(inbound_bytes); rxByte++ )
    {
        decode_result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 1, decode_result, "Decoder didn't finish with a valid packet" );

    //check parsed results from data structure directly
    TEST_ASSERT_EQUAL_INT_MESSAGE( 4, test_interface.inboundHeader.data_len,    "Unexpected data_length" );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 11, test_interface.inboundHeader.type,       "Unexpected type"   );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.internal,    "Expected dev msg"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.offset,      "Unexpected offset bit"         );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 3, test_interface.inboundHeader.id_len,      "Msg length err"    );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.response,    "Didn't expect a response bit"  );
    TEST_ASSERT_EQUAL_INT_MESSAGE( 0, test_interface.inboundHeader.acknum,      "Unexpected ack number"         );

    TEST_ASSERT_EQUAL_STRING( "abc", test_interface.inboundID);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.inboundOffset, "Wasn't expecting offset packet" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected_payload, test_interface.inboundData, sizeof(expected_payload) );
    TEST_ASSERT_EQUAL_UINT16( 0x1D8B, test_interface.runningCRC );
}


//offset messages
//offset ranges valid
//offset ranges invalid
//invalid CRCs
//missing preamble
//missing closing byte
//message in payload of another message
