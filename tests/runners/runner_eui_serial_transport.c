#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP_RUNNER( SerialCRC16 )
{
    RUN_TEST_CASE( SerialCRC16, CRC16_Basic )
    RUN_TEST_CASE( SerialCRC16, CRC16_Reflect )
    RUN_TEST_CASE( SerialCRC16, CRC16_Repeated0x00 )
    RUN_TEST_CASE( SerialCRC16, CRC16_Fuzzed )
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
}
