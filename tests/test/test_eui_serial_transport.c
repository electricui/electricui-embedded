#include "../../eui_serial_transport.h"
#include "unity.h"
#include "unity_fixture.h"
#include <stdlib.h>
#include <string.h>

TEST_GROUP( TransportLayer );

#define CRC_DIVISOR 0xFFFF

time_t t;
//mock an outbound putc style per-byte interface
uint8_t serial_buffer[1024] = { 0 };
uint16_t serial_position    = 0;
uint8_t result = 0;

void byte_into_buffer(uint8_t outbound)
{
    if( serial_position < 2048 )
    {
        serial_buffer[ serial_position ] = outbound;
        serial_position++;
    }
    else
    {
        TEST_ASSERT_MESSAGE( 1, "Mocked serial interface reports an issue");
    }
}

TEST_SETUP( TransportLayer )
{
    //run before each test
    srand((unsigned) time(&t)); //seed rand()

    //reset mocked serial port
    memset(serial_buffer, 0, sizeof(serial_buffer));
    serial_position = 0;
    result = 0;
}

TEST_TEAR_DOWN( TransportLayer )
{
    //run after each test

}

TEST( TransportLayer, CRC16_Basic)
{
    //handy reference https://crccalc.com
    //CRC-16/CCITT-FALSE is initalised with 0xFFFF
    uint16_t temp_crc_1 = CRC_DIVISOR;
    uint16_t temp_crc_2 = CRC_DIVISOR;

    crc16( 0x00, &temp_crc_1 );
    TEST_ASSERT_EQUAL_HEX( 0xE1F0, temp_crc_1 );

    temp_crc_1 = CRC_DIVISOR;
    crc16( 0x0A, &temp_crc_1 );
    TEST_ASSERT_EQUAL_HEX( 0x40BA, temp_crc_1 );

    temp_crc_1 = CRC_DIVISOR;
    crc16( 0xFF, &temp_crc_1 );
    TEST_ASSERT_EQUAL_HEX( 0xFF00, temp_crc_1 );

    temp_crc_1 = CRC_DIVISOR;
    crc16( 0xFF, &temp_crc_1 );
    crc16( 0x0A, &temp_crc_1 );
    TEST_ASSERT_EQUAL_HEX( 0xBFBA, temp_crc_1 );

    //run 00 to 0F sequentially 
    temp_crc_1 = CRC_DIVISOR;
    for(uint8_t i = 0x00; i < 16; i++)
    {
        crc16( i, &temp_crc_1 );
    }
    TEST_ASSERT_EQUAL_HEX( 0x3B37, temp_crc_1 );

    //crc its own value
    temp_crc_1 = CRC_DIVISOR;
    temp_crc_2 = CRC_DIVISOR;
    crc16( 0x06, &temp_crc_1 );
    crc16( 0xAC, &temp_crc_1 ); 
    TEST_ASSERT_EQUAL_HEX( 0xC3CF, temp_crc_1 );    //well we already have issues if this isn't right
    temp_crc_2 = temp_crc_1;
    crc16( temp_crc_2, &temp_crc_1 ); 
    TEST_ASSERT_MESSAGE( 0x0000 == temp_crc_1, "Self-zero's when input is same as cache" );
}

TEST( TransportLayer, CRC16_Advanced )
{
    uint16_t temp_crc_1 = CRC_DIVISOR;
    uint16_t temp_crc_2 = CRC_DIVISOR;

    //repeated null bytes should give unique crc 
    temp_crc_1 = CRC_DIVISOR;
    temp_crc_2 = CRC_DIVISOR;
    for(uint8_t i = 0x00; i < 8; i++)
    {
        crc16( 0x00, &temp_crc_1 );
    }
    crc16( 0x00, &temp_crc_2 ); //baseline should be different
    TEST_ASSERT_MESSAGE( temp_crc_1 != temp_crc_2, "Consecutive 0x00 bytes aren't unique" )

    //test a 'already warm' crc with repeated 0x00 afterwards
    temp_crc_1 = CRC_DIVISOR;
    temp_crc_2 = CRC_DIVISOR;
    for(uint8_t i = 0x00; i < 16; i++)
    {
        crc16( i, &temp_crc_1 );
    }
    temp_crc_2 = temp_crc_1;    //buffer this value
    for(uint8_t i = 0x00; i < 8; i++)
    {
        crc16( 0x00, &temp_crc_1 );
    }
    TEST_ASSERT_MESSAGE( temp_crc_1 != temp_crc_2, "Consecutive 0x00 aren't handled after 00-0F ingested" )

    //repeated null bytes should give unique crc after other non-zero bytes
    temp_crc_1 = CRC_DIVISOR;
    temp_crc_2 = CRC_DIVISOR;
    crc16( 0x01, &temp_crc_1 );
    crc16( 0xAF, &temp_crc_1 );
    temp_crc_2 = temp_crc_1;
    TEST_ASSERT_EQUAL_HEX( 0x6A3B, temp_crc_1 );
    for(uint8_t i = 0x00; i < 8; i++)
    {
        crc16( 0x00, &temp_crc_1 );
    }
    TEST_ASSERT_MESSAGE( temp_crc_1 != temp_crc_2, "Consecutive 0x00 aren't handled after 0x01Af" )

    //attack against the seed values with repeated bytes
    temp_crc_1 = CRC_DIVISOR;
    temp_crc_2 = CRC_DIVISOR;
    crc16( 0xFF, &temp_crc_1 );
    crc16( 0xFF, &temp_crc_1 );
    temp_crc_2 = temp_crc_1;
    for(uint8_t i = 0x00; i < 8; i++)
    {
        crc16( 0x00, &temp_crc_1 );
    }
    TEST_ASSERT_MESSAGE( temp_crc_1 != temp_crc_2, "Consecutive 0x00 aren't handled after 0xFFFF (attack with seed)" )
}

TEST( TransportLayer, CRC16_Fuzzed )
{
    uint16_t temp_crc_1 = CRC_DIVISOR;
    uint16_t temp_crc_2 = CRC_DIVISOR;

    temp_crc_1 = CRC_DIVISOR;
    temp_crc_2 = CRC_DIVISOR;
    for(uint8_t i = 0x00; i < 32; i++)  //32 random bytes
    {
        crc16( rand() % 256, &temp_crc_1 );
    }
    temp_crc_2 = temp_crc_1;    //buffer resultant crc
    crc16( temp_crc_2, &temp_crc_1 ); //perform 'reflection attack' by hashing itself
    TEST_ASSERT_MESSAGE( 0x0000 == temp_crc_1, "Reflection of CRC check (fuzzed inputs)" )

    temp_crc_1 = CRC_DIVISOR;
    temp_crc_2 = 0x0000;    //will act as the 'previous step' for this test, don't make them the same to start with
    for(uint8_t i = 0x00; i < 64; i++)  //64 random bytes
    {
        crc16( rand() % 256, &temp_crc_1 );
        TEST_ASSERT_MESSAGE( temp_crc_1 == temp_crc_2, "Fuzzed CRC input has same CRC as previous result" )
        temp_crc_2 = temp_crc_1;
    }
}


TEST( TransportLayer, encode_packet_simple )
{
    //input parameters
    const char * test_message = "abc";
    uint8_t test_payload[] = { 
        42, 
    };
    
    euiPacketSettings_t test_simple_header;
    test_simple_header.internal  = 0;
    test_simple_header.response  = 0;
    test_simple_header.type      = 5;

    //test it against our mocked buffer
    result = encode_packet_simple(&byte_into_buffer, &test_simple_header, test_message, sizeof(test_payload), &test_payload);

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
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, result, "Encoder didn't return expected status code" );

}

TEST( TransportLayer, encode_packet )
{
    //input parameters
    const char * test_message = "abc";
    uint8_t test_payload[] = { 
        42, 
    };
    uint16_t offset = 0;
    
    euiHeader_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 5; //int8
    test_header.acknum     = 0;
    test_header.offset     = (offset) ? 1 : 0;
    test_header.id_len     = strlen(test_message);
    test_header.data_len   = sizeof(test_payload);

    //test it against our mocked buffer
    result = encode_packet( &byte_into_buffer, &test_header, test_message, offset, &test_payload );

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
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, result, "Encoder didn't return expected status code" );

}

TEST( TransportLayer, decode_packet )
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
        result = decode_packet( inbound_bytes[rxByte], &test_interface );
    }
    
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 1, result, "Decoder didn't finish with a valid packet" );

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
