#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP_RUNNER( SessionLayer )
{

    RUN_TEST_CASE( SessionLayer, parse_packet )
    RUN_TEST_CASE( SessionLayer, handle_packet )
    RUN_TEST_CASE( SessionLayer, send_tracked )
    RUN_TEST_CASE( SessionLayer, send_tracked_range )
    RUN_TEST_CASE( SessionLayer, send_message )
    RUN_TEST_CASE( SessionLayer, setup_dev_msg )
    RUN_TEST_CASE( SessionLayer, setup_identifier )
    RUN_TEST_CASE( SessionLayer, announce_board )
    RUN_TEST_CASE( SessionLayer, announce_dev_msg )
    RUN_TEST_CASE( SessionLayer, announce_dev_vars )
    RUN_TEST_CASE( SessionLayer, report_error )
}

TEST_GROUP_RUNNER( FindMessageObject )
{
    RUN_TEST_CASE( FindMessageObject, find_message_object_developer )
    RUN_TEST_CASE( FindMessageObject, find_message_object_internal )
    RUN_TEST_CASE( FindMessageObject, find_message_object_wrong_internal_flag )
    RUN_TEST_CASE( FindMessageObject, find_message_object_invalid_internal_flag )
    RUN_TEST_CASE( FindMessageObject, find_message_object_invalid_id )
    RUN_TEST_CASE( FindMessageObject, find_message_object_nullptr )
    RUN_TEST_CASE( FindMessageObject, find_message_object_single_letter )
    RUN_TEST_CASE( FindMessageObject, find_message_object_max_letters )
    RUN_TEST_CASE( FindMessageObject, find_message_object_no_id )
    RUN_TEST_CASE( FindMessageObject, find_message_object_non_printable )
    RUN_TEST_CASE( FindMessageObject, find_message_devArrayNULL )
    RUN_TEST_CASE( FindMessageObject, find_message_internalArrayNULL )
}