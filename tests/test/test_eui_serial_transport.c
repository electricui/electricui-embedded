#include "../../eui_serial_transport.h"
#include "unity.h"
#include "unity_fixture.h"
#include <stdlib.h>

TEST_GROUP( TransportLayer );

#define CRC_DIVISOR 0xFFFF

time_t t;


TEST_SETUP( TransportLayer )
{
    //run before each test
    srand((unsigned) time(&t)); //seed rand()

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


TEST( TransportLayer, generate_header )
{
    TEST_IGNORE_MESSAGE("TODO: Add tests");
}

TEST( TransportLayer, encode_packet_simple )
{
    TEST_IGNORE_MESSAGE("TODO: Add tests");
}

TEST( TransportLayer, encode_packet )
{
    TEST_IGNORE_MESSAGE("TODO: Add tests");
}

TEST( TransportLayer, decode_packet )
{
    TEST_IGNORE_MESSAGE("TODO: Add tests");
}