#include "unity.h"
#include <string.h>

// MODULE UNDER TEST
#include "eui_binary_transport.h"
#include "eui_utilities.h"

// DEFINITIONS 
 
// PRIVATE TYPES
 
// PRIVATE DATA

// PRIVATE FUNCTIONS
 
// SETUP, TEARDOWN
 
void setUp(void)
{

}
 
void tearDown(void)
{

}
 
// TESTS
 
void test_header_serialise_empty(void)
{
    //bare minimum valid packet
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 0;
    test_header.acknum     = 0;
    test_header.offset     = 0;
    test_header.id_len     = 0;
    test_header.data_len   = 0;

    uint8_t buffer[5] = { 0xFF };
    uint8_t expected[] = { 0x00, 0x00, 0x00 };

    //get back 3 bytes
    uint8_t result = encode_header( &test_header, &buffer[0] );

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 3, result, "Unexpected return count" );
}

void test_header_serialise_bare(void)
{
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 0;
    test_header.acknum     = 0;
    test_header.offset     = 0;
    test_header.id_len     = 1;
    test_header.data_len   = 0;

    uint8_t buffer[5] = { 0xFF };
    uint8_t expected[] = { 0x00, 0x00, 0x01 };
    uint8_t result = encode_header( &test_header, &buffer[0] );

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 3, result, "Unexpected return count" );
}

void test_header_serialise_typical(void)
{
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 5;
    test_header.acknum     = 0;
    test_header.offset     = 0;
    test_header.id_len     = 3;
    test_header.data_len   = 4;

    uint8_t buffer[5] = { 0xFF };
    uint8_t expected[] = { 0x04, 0x14, 0x03 };
    uint8_t result = encode_header( &test_header, &buffer[0] );

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 3, result, "Unexpected return count" );
}

void test_header_serialise_long_payload(void)
{
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 0;
    test_header.acknum     = 0;
    test_header.offset     = 0;
    test_header.id_len     = 1;
    test_header.data_len   = 666;

    uint8_t buffer[5] = { 0xFF };
    uint8_t expected[] = { 0x9A, 0x02, 0x01 };
    uint8_t result = encode_header( &test_header, &buffer[0] );

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 3, result, "Unexpected return count" );
}

void test_header_serialise_overflow_payload(void)
{
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 0;
    test_header.acknum     = 0;
    test_header.offset     = 0;
    test_header.id_len     = 1;
    test_header.data_len   = 1023;  
    test_header.data_len  += 2;     //overflows to 0x01
    uint8_t buffer[5] = { 0xFF };
    uint8_t expected[] = { 0x01, 0x00, 0x01 };
    uint8_t result = encode_header( &test_header, &buffer[0] );

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 3, result, "Unexpected return count" );
}

void test_header_serialise_overflow_messageid(void)
{
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 0;
    test_header.acknum     = 0;
    test_header.offset     = 0;
    test_header.id_len     = 15;
    test_header.id_len    += 1; //overflows to 0x01
    test_header.data_len   = 1;

    uint8_t buffer[5] = { 0xFF };
    uint8_t expected[] = { 0x01, 0x00, 0x00 };
    uint8_t result = encode_header( &test_header, &buffer[0] );

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 3, result, "Unexpected return count" );
}

void test_header_invalid_both(void)
{
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 5;
    test_header.acknum     = 0;
    test_header.offset     = 0;
    test_header.id_len     = 3;
    test_header.data_len   = 4;

    uint8_t buffer[5];
    memset(buffer, 0xFF, sizeof(buffer));
    uint8_t expected[] = { 0xFF, 0xFF, 0xFF };

    uint8_t result = encode_header( 0, 0 );

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, result, "Unexpected return count" );
}

void test_header_invalid_header(void)
{
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 5;
    test_header.acknum     = 0;
    test_header.offset     = 0;
    test_header.id_len     = 3;
    test_header.data_len   = 4;

    uint8_t buffer[5];
    memset(buffer, 0xFF, sizeof(buffer));
    uint8_t expected[] = { 0xFF, 0xFF, 0xFF };

    uint8_t result = encode_header( 0, &buffer[0] );

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, result, "Unexpected return count" );
}

void test_header_invalid_buffer(void)
{
    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 5;
    test_header.acknum     = 0;
    test_header.offset     = 0;
    test_header.id_len     = 3;
    test_header.data_len   = 4;

    uint8_t buffer[5];
    memset(buffer, 0xFF, sizeof(buffer));
    uint8_t expected[] = { 0xFF, 0xFF, 0xFF };

    uint8_t result = encode_header( &test_header, 0 );

    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, buffer, sizeof(expected) );
    TEST_ASSERT_EQUAL_UINT8_MESSAGE( 0, result, "Unexpected return count" );
}

/*
    for(uint8_t i = 0; i < sizeof(expected); i++)
    {
        printf("0x%02X ", buffer[i]);
    }
*/
