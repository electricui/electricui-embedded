#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP_RUNNER( SerialCRC16 )
{
    RUN_TEST_CASE( SerialCRC16, CRC16_Basic )
    RUN_TEST_CASE( SerialCRC16, CRC16_Reflect )
    RUN_TEST_CASE( SerialCRC16, CRC16_Repeated0x00 )
    RUN_TEST_CASE( SerialCRC16, CRC16_Fuzzed )
}

TEST_GROUP_RUNNER( SerialCOBS )
{
    RUN_TEST_CASE( SerialCOBS, COBS_Basic_1 )
    RUN_TEST_CASE( SerialCOBS, COBS_Basic_2 )
    RUN_TEST_CASE( SerialCOBS, COBS_Basic_3 )
    RUN_TEST_CASE( SerialCOBS, COBS_Basic_4 )
    RUN_TEST_CASE( SerialCOBS, COBS_Basic_5 )
    RUN_TEST_CASE( SerialCOBS, COBS_Basic_6 )
    RUN_TEST_CASE( SerialCOBS, COBS_Basic_7 )
    RUN_TEST_CASE( SerialCOBS, COBS_Basic_8 )
    RUN_TEST_CASE( SerialCOBS, COBS_Basic_9 )
    RUN_TEST_CASE( SerialCOBS, COBS_Basic_10 )

    RUN_TEST_CASE( SerialCOBS, COBS_Long )
    RUN_TEST_CASE( SerialCOBS, COBS_Realworld )
}

TEST_GROUP_RUNNER( SerialEncoder )
{
    RUN_TEST_CASE( SerialEncoder, encode_packet_simple )
    RUN_TEST_CASE( SerialEncoder, encode_packet )
    RUN_TEST_CASE( SerialEncoder, encode_packet_short_id )
    RUN_TEST_CASE( SerialEncoder, encode_packet_long_id )
    RUN_TEST_CASE( SerialEncoder, encode_packet_internal )
    RUN_TEST_CASE( SerialEncoder, encode_packet_response )
    RUN_TEST_CASE( SerialEncoder, encode_packet_acknum )
    RUN_TEST_CASE( SerialEncoder, encode_packet_float )
    RUN_TEST_CASE( SerialEncoder, encode_packet_offset_last )
    RUN_TEST_CASE( SerialEncoder, encode_packet_offset )
    RUN_TEST_CASE( SerialEncoder, encode_packet_large )

}

TEST_GROUP_RUNNER( SerialDecoder )
{
    RUN_TEST_CASE( SerialDecoder, decode_packet )
    RUN_TEST_CASE( SerialDecoder, decode_packet_short_id )
    RUN_TEST_CASE( SerialDecoder, decode_packet_long_id )
    RUN_TEST_CASE( SerialDecoder, decode_packet_internal )
    RUN_TEST_CASE( SerialDecoder, decode_packet_response )
    RUN_TEST_CASE( SerialDecoder, decode_packet_acknum )
    RUN_TEST_CASE( SerialDecoder, decode_packet_float )
    RUN_TEST_CASE( SerialDecoder, decode_packet_invalidCRC )
    RUN_TEST_CASE( SerialDecoder, decode_packet_offset_last )
    RUN_TEST_CASE( SerialDecoder, decode_packet_offset )
    RUN_TEST_CASE( SerialDecoder, decode_packet_large )
    
}

TEST_GROUP_RUNNER( SerialLoopback )
{
    RUN_TEST_CASE( SerialLoopback, encode_decode_simple )
    RUN_TEST_CASE( SerialLoopback, encode_decode_headerbits )
    RUN_TEST_CASE( SerialLoopback, encode_decode_short_id )
    RUN_TEST_CASE( SerialLoopback, encode_decode_long_id )
    RUN_TEST_CASE( SerialLoopback, encode_decode_no_data )
    RUN_TEST_CASE( SerialLoopback, encode_decode_long_data )
    RUN_TEST_CASE( SerialLoopback, encode_decode_many_zeros )
    
}
