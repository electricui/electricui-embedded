#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP_RUNNER(SessionLayer)
{
    RUN_TEST_CASE(SessionLayer, find_message_object )
    RUN_TEST_CASE(SessionLayer, parse_packet )
    RUN_TEST_CASE(SessionLayer, handle_packet )
    RUN_TEST_CASE(SessionLayer, send_tracked )
    RUN_TEST_CASE(SessionLayer, send_tracked_range )
    RUN_TEST_CASE(SessionLayer, send_message )
    RUN_TEST_CASE(SessionLayer, setup_dev_msg )
    RUN_TEST_CASE(SessionLayer, setup_identifier )
    RUN_TEST_CASE(SessionLayer, announce_board )
    RUN_TEST_CASE(SessionLayer, announce_dev_msg )
    RUN_TEST_CASE(SessionLayer, announce_dev_vars )
    RUN_TEST_CASE(SessionLayer, report_error )
}