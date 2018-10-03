#include "../../eui_serial_transport.h"
#include "unity.h"
#include "unity_fixture.h"
#include <string.h>

TEST_GROUP( SerialLoopback );

//mock an outbound putc style per-byte interface
uint8_t loopback_buffer[1024]   = { 0 };
uint16_t lb_buf_pos             = 0;

void loopback_interface(uint8_t outbound)
{
    if( lb_buf_pos < 1024 )
    {
        loopback_buffer[ lb_buf_pos ] = outbound;
        lb_buf_pos++;
    }
    else
    {
        TEST_ASSERT_MESSAGE( 1, "Mocked serial interface reports an issue");
    }
}

TEST_SETUP( SerialLoopback )
{
    memset(loopback_buffer, 0, sizeof(loopback_buffer));
    lb_buf_pos = 0;
}

TEST_TEAR_DOWN( SerialLoopback )
{

}

// Single byte payload
TEST( SerialLoopback, encode_decode_simple )
{
    //pass data to encoder
    //attach encoder to decoder
    //check parsed result matches inputs

    //decoder data structure
    eui_packet_t test_interface = {0};

    //encoder inputs
    const char * test_id = "abc";
    uint8_t test_payload[] = { 42 };

    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 5;
    test_header.acknum     = 0;
    test_header.offset     = 0;
    test_header.id_len     = strlen(test_id);
    test_header.data_len   = sizeof(test_payload);

    //test it against our mocked buffer
    encode_packet(&loopback_interface, &test_header, test_id, 0, &test_payload);

    for( uint16_t rxByte = 0; rxByte < lb_buf_pos; rxByte++ )
    {
        decode_packet( loopback_buffer[rxByte], &test_interface );
    }

    //Test the decoded results against the inputs provided to the encoder
    TEST_ASSERT_EQUAL_INT( sizeof(test_payload), test_interface.header.data_len );
    TEST_ASSERT_EQUAL_INT( test_header.type, test_interface.header.type         );
    TEST_ASSERT_EQUAL_INT( test_header.internal, test_interface.header.internal );
    TEST_ASSERT_EQUAL_INT( test_header.offset, test_interface.header.offset     );
    TEST_ASSERT_EQUAL_INT( strlen(test_id), test_interface.header.id_len        );
    TEST_ASSERT_EQUAL_INT( test_header.response, test_interface.header.response );
    TEST_ASSERT_EQUAL_INT( test_header.acknum, test_interface.header.acknum     );
    TEST_ASSERT_EQUAL_STRING( test_id, test_interface.msgid_in                  );
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Offset buffer garbage appeared" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( test_payload, test_interface.data_in, sizeof(test_payload)       );
}

// All header bits are non-zero
TEST( SerialLoopback, encode_decode_headerbits )
{
    //decoder data structure
    eui_packet_t test_interface = {0};

    //encoder inputs
    const char * test_id = "abc";
    uint8_t test_payload[] = { 42 };

    eui_header_t test_header;
    test_header.internal   = 1;
    test_header.response   = 1;
    test_header.type       = 5;
    test_header.acknum     = 1;
    test_header.offset     = 1;
    test_header.id_len     = strlen(test_id);
    test_header.data_len   = sizeof(test_payload);

    uint16_t offset_address = 0x0F;

    //test it against our mocked buffer
    encode_packet(&loopback_interface, &test_header, test_id, offset_address, &test_payload);

    for( uint16_t rxByte = 0; rxByte < lb_buf_pos; rxByte++ )
    {
        decode_packet( loopback_buffer[rxByte], &test_interface );
    }

    // for( uint16_t i = 0; i < lb_buf_pos; i++)
    // {     
    //     printf("[%d]: %X\n", i, loopback_buffer[i]);
    // }

    //Test the decoded results against the inputs provided to the encoder
    TEST_ASSERT_EQUAL_INT( sizeof(test_payload), test_interface.header.data_len );
    TEST_ASSERT_EQUAL_INT( test_header.type, test_interface.header.type         );
    TEST_ASSERT_EQUAL_INT( test_header.internal, test_interface.header.internal );
    TEST_ASSERT_EQUAL_INT( test_header.offset, test_interface.header.offset     );
    TEST_ASSERT_EQUAL_INT( strlen(test_id), test_interface.header.id_len        );
    TEST_ASSERT_EQUAL_INT( test_header.response, test_interface.header.response );
    TEST_ASSERT_EQUAL_INT( test_header.acknum, test_interface.header.acknum     );
    TEST_ASSERT_EQUAL_STRING( test_id, test_interface.msgid_in                  );
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( offset_address, test_interface.offset_in, "Offset buffer garbage appeared" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( test_payload, test_interface.data_in, sizeof(test_payload)       );

}

TEST( SerialLoopback, encode_decode_short_id )
{
    TEST_IGNORE();

}

TEST( SerialLoopback, encode_decode_long_id )
{
    TEST_IGNORE();

}

TEST( SerialLoopback, encode_decode_no_data )
{
    TEST_IGNORE();

}

TEST( SerialLoopback, encode_decode_long_data )
{
    TEST_IGNORE();

}

TEST( SerialLoopback, encode_decode_many_zeros )
{
    TEST_IGNORE();

}