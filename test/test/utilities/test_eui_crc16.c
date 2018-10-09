#include "unity.h"
#include <stdlib.h>
 
// MODULE UNDER TEST
#include "eui_crc.h"
 
// DEFINITIONS 
#define CRC_DIVISOR 0xFFFF
 
// PRIVATE TYPES
 
// PRIVATE DATA

time_t t;

uint16_t temp_crc_1 = 0;
uint16_t temp_crc_2 = 0;

// PRIVATE FUNCTIONS
 
// SETUP, TEARDOWN
 
void setUp(void)
{
    srand((unsigned) time(&t)); //seed rand()

    //handy reference https://crccalc.com
    //CRC-16/CCITT-FALSE is initalised with 0xFFFF
    temp_crc_1 = CRC_DIVISOR;
    temp_crc_2 = CRC_DIVISOR;
}
 
void tearDown(void)
{

}
 
// TESTS
 
void test_basic(void)
{
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
}

void test_reflection(void)
{
    //warm it up
    crc16( 0x06, &temp_crc_1 );
    crc16( 0xAC, &temp_crc_1 ); 
    TEST_ASSERT_EQUAL_HEX( 0xC3CF, temp_crc_1 );    //well we already have issues if this isn't right

    TEST_IGNORE_MESSAGE("CRC reflection test skipped");

    //crc its own value
    temp_crc_2 = temp_crc_1;
    crc16( temp_crc_2, &temp_crc_1 ); 
    TEST_ASSERT_MESSAGE( 0x0000 == temp_crc_1, "Self-zero's when input is same as cache" );

    for(uint8_t i = 0x00; i < 32; i++)  //32 random bytes
    {
        crc16( rand() % 256, &temp_crc_1 );
    }
    temp_crc_2 = temp_crc_1;    //buffer resultant crc
    crc16( temp_crc_2, &temp_crc_1 ); //perform 'reflection attack' by hashing itself
    TEST_ASSERT_MESSAGE( 0x0000 == temp_crc_1, "Reflection of CRC check (fuzzed inputs)" )
}

void test_repeated_zeros(void)
{
    //repeated null bytes should give unique crc 
    for(uint8_t i = 0x00; i < 8; i++)
    {
        crc16( 0x00, &temp_crc_1 );
    }
    crc16( 0x00, &temp_crc_2 ); //baseline should be different
    TEST_ASSERT_MESSAGE( temp_crc_1 != temp_crc_2, "Consecutive 0x00 bytes aren't unique" )

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

    TEST_IGNORE_MESSAGE("Seed attack + consecutive 0x00 test skipped");

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

void test_fuzz(void)
{
    temp_crc_1 = CRC_DIVISOR;
    temp_crc_2 = 0x0000;    //will act as the 'previous step' for this test, don't make them the same to start with
    for(uint8_t i = 0x00; i < 64; i++)  //64 random bytes
    {
        crc16( rand() % 256, &temp_crc_1 );
        TEST_ASSERT_NOT_EQUAL_MESSAGE( temp_crc_1, temp_crc_2, "Fuzzed CRC input has same CRC as previous result" )
        temp_crc_2 = temp_crc_1;
    }
}
