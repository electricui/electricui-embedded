#include "unity.h"
#include <string.h>
 
// MODULE UNDER TEST
#include "eui_binary_transport.h"
#include "eui_utilities.h"

// DEFINITIONS 
 
// PRIVATE TYPES
 
// PRIVATE DATA
//mock an outbound putc style per-byte interface
uint8_t loopback_buffer[1024]   = { 0xFF };
uint16_t lb_buf_pos             = 0;

// PRIVATE FUNCTIONS
void loopback_interface( uint8_t *data, uint16_t len )
{
    for( uint16_t i = 0; i < len; i++ )
    {
        if( lb_buf_pos < 1024 )
        {
            loopback_buffer[ lb_buf_pos ] = data[i];
            lb_buf_pos++;
        }
        else
        {
            TEST_ASSERT_MESSAGE( 1, "Mocked serial interface reports an issue");
        }
    }
}
 
// SETUP, TEARDOWN
 
void setUp(void)
{
    memset(loopback_buffer, 0xFF, sizeof(loopback_buffer));
    lb_buf_pos = 0;
}
 
void tearDown(void)
{

}

// TESTS

// Single byte payload
void test_encode_decode_simple( void )
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
    TEST_ASSERT_EQUAL_STRING( test_id, test_interface.id_in                     );
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Offset buffer garbage appeared" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( test_payload, test_interface.data_in, sizeof(test_payload)       );
}

// All header bits are non-zero
void test_encode_decode_headerbits( void )
{
    //decoder data structure
    eui_packet_t test_interface = {0};

    //encoder inputs
    const char * test_id = "abc";
    uint8_t test_payload[] = { 0x42, 0x41, 0x40, 0x39, 0x38, 0x37, 0x36, 0x35 };

    eui_header_t test_header;
    test_header.internal   = 1;
    test_header.response   = 1;
    test_header.type       = 5;
    test_header.acknum     = 1;
    test_header.offset     = 1;
    test_header.id_len     = strlen(test_id);
    test_header.data_len   = 3;

    uint16_t offset_address = 0x04;
    uint8_t offset_payload_expected[] = { /*0x42, 0x41, 0x40, 0x39,*/ 0x38, 0x37, 0x36, /* 0x35 */ };

    //test it against our mocked buffer
    encode_packet(&loopback_interface, &test_header, test_id, offset_address, &test_payload);

    for( uint16_t rxByte = 0; rxByte < lb_buf_pos; rxByte++ )
    {
        decode_packet( loopback_buffer[rxByte], &test_interface );
    }

    //Test the decoded results against the inputs provided to the encoder
    TEST_ASSERT_EQUAL_INT( sizeof(offset_payload_expected), test_interface.header.data_len );
    TEST_ASSERT_EQUAL_INT( test_header.type, test_interface.header.type         );
    TEST_ASSERT_EQUAL_INT( test_header.internal, test_interface.header.internal );
    TEST_ASSERT_EQUAL_INT( test_header.offset, test_interface.header.offset     );
    TEST_ASSERT_EQUAL_INT( strlen(test_id), test_interface.header.id_len        );
    TEST_ASSERT_EQUAL_INT( test_header.response, test_interface.header.response );
    TEST_ASSERT_EQUAL_INT( test_header.acknum, test_interface.header.acknum     );
    TEST_ASSERT_EQUAL_STRING( test_id, test_interface.id_in                     );
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( offset_address, test_interface.offset_in, "Offset buffer garbage appeared" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( offset_payload_expected, test_interface.data_in, sizeof(offset_payload_expected) );

}

void test_encode_decode_short_id( void )
{
    eui_packet_t test_interface = {0};

    //encoder inputs
    const char * test_id = "t";
    uint8_t test_payload[] = { 42 };

    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 5;
    test_header.acknum     = 0;
    test_header.offset     = 0;
    test_header.id_len     = strlen(test_id);
    test_header.data_len   = sizeof(test_payload);

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
    TEST_ASSERT_EQUAL_STRING( test_id, test_interface.id_in                     );
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Offset buffer garbage appeared" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( test_payload, test_interface.data_in, sizeof(test_payload)       );
}

void test_encode_decode_long_id( void )
{
    eui_packet_t test_interface = {0};

    //encoder inputs
    const char * test_id = "abcdefghijklmno";
    uint8_t test_payload[] = { 42 };

    eui_pkt_settings_t test_header;
    test_header.internal  = 0;
    test_header.response  = 0;
    test_header.type      = 5;

    //test it against our mocked buffer
    encode_packet_simple(&loopback_interface, &test_header, test_id, sizeof(test_payload), &test_payload);

    for( uint16_t rxByte = 0; rxByte < lb_buf_pos; rxByte++ )
    {
        decode_packet( loopback_buffer[rxByte], &test_interface );
    }

    //Test the decoded results against the inputs provided to the encoder
    TEST_ASSERT_EQUAL_INT( sizeof(test_payload), test_interface.header.data_len );
    TEST_ASSERT_EQUAL_INT( test_header.type, test_interface.header.type         );
    TEST_ASSERT_EQUAL_INT( test_header.internal, test_interface.header.internal );
    TEST_ASSERT_EQUAL_INT( 0, test_interface.header.offset                      );
    TEST_ASSERT_EQUAL_INT( strlen(test_id), test_interface.header.id_len        );
    TEST_ASSERT_EQUAL_INT( 0, test_interface.header.response                    );
    TEST_ASSERT_EQUAL_INT( 0, test_interface.header.acknum                      );
    TEST_ASSERT_EQUAL_STRING( test_id, test_interface.id_in                     );
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Offset buffer garbage appeared" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( test_payload, test_interface.data_in, sizeof(test_payload)       );
}

void test_encode_decode_no_data( void )
{
    eui_packet_t test_interface = {0};

    //encoder inputs
    const char * test_id = "abc";
    uint8_t test_payload[] = { };

    eui_pkt_settings_t test_header;
    test_header.internal  = 0;
    test_header.response  = 0;
    test_header.type      = 5;

    //test it against our mocked buffer
    encode_packet_simple(&loopback_interface, &test_header, test_id, sizeof(test_payload), &test_payload);

    for( uint16_t rxByte = 0; rxByte < lb_buf_pos; rxByte++ )
    {
        decode_packet( loopback_buffer[rxByte], &test_interface );
    }

    //Test the decoded results against the inputs provided to the encoder
    TEST_ASSERT_EQUAL_INT( 0, test_interface.header.data_len                    );
    TEST_ASSERT_EQUAL_INT( test_header.type, test_interface.header.type         );
    TEST_ASSERT_EQUAL_INT( test_header.internal, test_interface.header.internal );
    TEST_ASSERT_EQUAL_INT( 0, test_interface.header.offset                      );
    TEST_ASSERT_EQUAL_INT( strlen(test_id), test_interface.header.id_len        );
    TEST_ASSERT_EQUAL_INT( 0, test_interface.header.response                    );
    TEST_ASSERT_EQUAL_INT( 0, test_interface.header.acknum                      );
    TEST_ASSERT_EQUAL_STRING( test_id, test_interface.id_in                     );
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Offset buffer garbage appeared" );
}

void test_encode_decode_long_data( void )
{
    eui_packet_t test_interface = {0};

    //encoder inputs
    const char * test_id = "abcdefghijklmno";
    uint8_t test_payload[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F
    };

    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 5;
    test_header.acknum     = 0;
    test_header.offset     = 0;
    test_header.id_len     = strlen(test_id);
    test_header.data_len   = sizeof(test_payload);

    //test it against our mocked buffer
    // encode_packet_simple(&loopback_interface, &test_header, test_id, sizeof(test_payload), &test_payload);
    encode_packet(&loopback_interface, &test_header, test_id, 0, &test_payload);

    for( uint16_t rxByte = 0; rxByte < lb_buf_pos; rxByte++ )
    {
        decode_packet( loopback_buffer[rxByte], &test_interface );
    }
    
    //Test the decoded results against the inputs provided to the encoder
    TEST_ASSERT_EQUAL_INT( sizeof(test_payload), test_interface.header.data_len );
    TEST_ASSERT_EQUAL_INT( test_header.type, test_interface.header.type         );
    TEST_ASSERT_EQUAL_INT( test_header.internal, test_interface.header.internal );
    TEST_ASSERT_EQUAL_INT( 0, test_interface.header.offset                      );
    TEST_ASSERT_EQUAL_INT( strlen(test_id), test_interface.header.id_len        );
    TEST_ASSERT_EQUAL_INT( 0, test_interface.header.response                    );
    TEST_ASSERT_EQUAL_INT( 0, test_interface.header.acknum                      );
    TEST_ASSERT_EQUAL_STRING( test_id, test_interface.id_in                     );
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Offset buffer garbage appeared" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( test_payload, test_interface.data_in, sizeof(test_payload)       );

}

void test_encode_decode_many_zeros( void )
{
    eui_packet_t test_interface = {0};

    //encoder inputs
    const char * test_id = "abc";
    uint8_t test_payload[] = {
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00
    };

    eui_header_t test_header;
    test_header.internal   = 0;
    test_header.response   = 0;
    test_header.type       = 5;
    test_header.acknum     = 0;
    test_header.offset     = 0;
    test_header.id_len     = strlen(test_id);
    test_header.data_len   = sizeof(test_payload);

    //test it against our mocked buffer
    // encode_packet_simple(&loopback_interface, &test_header, test_id, sizeof(test_payload), &test_payload);
    encode_packet(&loopback_interface, &test_header, test_id, 0, &test_payload);

    for( uint16_t rxByte = 0; rxByte < lb_buf_pos; rxByte++ )
    {
        decode_packet( loopback_buffer[rxByte], &test_interface );
    }
    
    //Test the decoded results against the inputs provided to the encoder
    TEST_ASSERT_EQUAL_INT( sizeof(test_payload), test_interface.header.data_len );
    TEST_ASSERT_EQUAL_INT( test_header.type, test_interface.header.type         );
    TEST_ASSERT_EQUAL_INT( test_header.internal, test_interface.header.internal );
    TEST_ASSERT_EQUAL_INT( 0, test_interface.header.offset                      );
    TEST_ASSERT_EQUAL_INT( strlen(test_id), test_interface.header.id_len        );
    TEST_ASSERT_EQUAL_INT( 0, test_interface.header.response                    );
    TEST_ASSERT_EQUAL_INT( 0, test_interface.header.acknum                      );
    TEST_ASSERT_EQUAL_STRING( test_id, test_interface.id_in                     );
    TEST_ASSERT_EQUAL_UINT16_MESSAGE( 0, test_interface.offset_in, "Offset buffer garbage appeared" );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( test_payload, test_interface.data_in, sizeof(test_payload)       );

}