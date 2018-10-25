#include "unity.h"
 
// MODULE UNDER TEST
#include "electricui.h"
#include "mock_eui_serial_transport.h"
#include "mock_eui_crc.h"
#include "mock_eui_offset_validation.h"

// DEFINITIONS 
 
// PRIVATE TYPES
 
// PRIVATE DATA

// PRIVATE FUNCTIONS
 
// SETUP, TEARDOWN

void setUp(void)
{
    //run before each test
}
 
void tearDown(void)
{

}

// TESTS

// byte input which doesn't complete a valid packet
void test_ingest_unfinished( void )
{
    TEST_IGNORE();

}

// packet with a malformed crc, etc
void test_ingest_error( void )
{
    TEST_IGNORE();

}

// packet with a msgID not in our tracked list
void test_ingest_unknown_id( void )
{
    TEST_IGNORE();

}

// simple packet with some data to write
void test_ingest_data_packet( void )
{
    TEST_IGNORE();

}

// inbound payload is larger than the internal variable
void test_ingest_data_packet_exceeds_size( void )
{
    TEST_IGNORE();

}

// offset packet with target range outside variable bounds
void test_ingest_data_packet_exceeds_range( void )
{
    TEST_IGNORE();

}

// packet is querying a variable 
void test_ingest_response_packet_query( void )
{
    TEST_IGNORE();

}

// packet is querying a range of data 
void test_ingest_response_packet_query_offset( void )
{
    TEST_IGNORE();

}

// packet wants ack response
void test_ingest_response_packet_ack( void )
{
    TEST_IGNORE();

}

// packet is just a callback, no data, no response
void test_ingest_callback_packet( void )
{
    TEST_IGNORE();

}