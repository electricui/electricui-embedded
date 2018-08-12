#include "../../eui_serial_transport.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(TransportLayer);


TEST_SETUP(TransportLayer)
{
    //run before each test

}

TEST_TEAR_DOWN(TransportLayer)
{
    //run after each test

}

TEST(TransportLayer, CRC16_Basic)
{
    //handy reference https://crccalc.com
    //CRC-16/CCITT-FALSE is initalised with 0xFFFF
    uint16_t temp_crc_1 = 0xFFFF;

    crc16( 0x00, &temp_crc_1 );
    TEST_ASSERT_EQUAL_HEX( 0xE1F0, temp_crc_1 );

    temp_crc_1 = 0xFFFF;
    crc16( 0x0A, &temp_crc_1 );
    TEST_ASSERT_EQUAL_HEX( 0x40BA, temp_crc_1 );

    temp_crc_1 = 0xFFFF;
    crc16( 0xFF, &temp_crc_1 );
    TEST_ASSERT_EQUAL_HEX( 0xFF00, temp_crc_1 );

    //repeated null bytes should give unique crc 
    temp_crc_1 = 0xFFFF;
    uint16_t temp_crc_2 = 0xFFFF;

    crc16( 0x00, &temp_crc_1 );
    crc16( 0x00, &temp_crc_1 );
    crc16( 0x00, &temp_crc_2 );
    TEST_ASSERT_MESSAGE( temp_crc_1 != temp_crc_2, "Consecutive 0x00 aren't unique" )
    
}

TEST(TransportLayer, generate_header )
{
    TEST_IGNORE_MESSAGE("TODO: Add tests");
}

TEST(TransportLayer, ncode_packet_simple )
{
    TEST_IGNORE_MESSAGE("TODO: Add tests");
}

TEST(TransportLayer, encode_packet )
{
    TEST_IGNORE_MESSAGE("TODO: Add tests");
}

TEST(TransportLayer, generate_headerdecode_packet )
{
    TEST_IGNORE_MESSAGE("TODO: Add tests");
}