#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP_RUNNER( TransportLayer )
{
    RUN_TEST_CASE( TransportLayer, CRC16_Basic )
    RUN_TEST_CASE( TransportLayer, CRC16_Advanced )
    RUN_TEST_CASE( TransportLayer, CRC16_Fuzzed )

    RUN_TEST_CASE( TransportLayer, encode_packet_simple )
    RUN_TEST_CASE( TransportLayer, encode_packet )
    RUN_TEST_CASE( TransportLayer, decode_packet )
}