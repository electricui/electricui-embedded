#include "../../src/electricui.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP( FindMessageObject );

char      testfind_char    = 'a';
int8_t    testfind_int8    = -21;
uint8_t   testfind_uint8   = 21;

//developer-space messages
eui_message_t test_findobject_store[] = {
    //simple objects with mostly garbage data, we only care about the ID's
    { .msgID = "a",    .type = TYPE_CHAR,    .size = sizeof(testfind_char),     .payload = &testfind_char      },
    { .msgID = "si8",   .type = TYPE_INT8,    .size = sizeof(testfind_int8),     .payload = &testfind_int8     },
    
    { .msgID = "abcdefghijklmno", 
                        .type = TYPE_UINT8,   .size = sizeof(testfind_uint8),    .payload = &testfind_uint8     },
    
    { .msgID = "\x01",  .type = TYPE_UINT8,     .size = sizeof(testfind_int8),  .payload = &testfind_int8   },
    { .msgID = "",      .type = TYPE_UINT8,     .size = sizeof(testfind_uint8), .payload = &testfind_uint8   },
};

TEST_SETUP( FindMessageObject )
{
    //run before each test
    setup_dev_msg(test_findobject_store, ARR_ELEM(test_findobject_store));
}

TEST_TEAR_DOWN( FindMessageObject )
{
    //run after each test

}

TEST( FindMessageObject, find_message_object_developer )
{
    eui_message_t *expecting     = &test_findobject_store[1]; //int8 object
    eui_message_t *result_ptr;   //result should be a pointer to our expected element
    const char * test_message   = "si8";
    uint8_t is_internal         = 0;

    result_ptr = find_message_object(test_message, is_internal);

    TEST_ASSERT_TRUE_MESSAGE( result_ptr == expecting, "Didn't return correct pointer")
}

TEST( FindMessageObject, find_message_object_internal )
{
    TEST_IGNORE_MESSAGE("TODO: Need to allow tests to see the internal object array");

    eui_message_t *expecting     = &test_findobject_store[0];    //todo fix this
    eui_message_t *result_ptr;
    const char * test_message   = "lv";
    uint8_t is_internal         = 1;

    result_ptr = find_message_object(test_message, is_internal);

    TEST_ASSERT_TRUE_MESSAGE( result_ptr == expecting, "Didn't return correct pointer")
}

TEST( FindMessageObject, find_message_object_wrong_internal_flag )
{
    eui_message_t *result_ptr;
    const char * test_message   = "si8";
    uint8_t is_internal         = 1;

    result_ptr = find_message_object(test_message, is_internal);

    TEST_ASSERT_NULL_MESSAGE( result_ptr, "Swapped flag shouldn't return a pointer?")    //it shouldn't find anything
}

TEST( FindMessageObject, find_message_object_invalid_internal_flag )
{
    eui_message_t *result_ptr;
    const char * test_message   = "si8";
    uint8_t is_internal         = 3;

    result_ptr = find_message_object(test_message, is_internal);

    TEST_ASSERT_NULL_MESSAGE(result_ptr, "Invalid internal flag shouldn't return a pointer")    //it shouldn't find anything
}

TEST( FindMessageObject, find_message_object_invalid_id )
{
    eui_message_t *result_ptr;
    const char * test_message   = "666";
    uint8_t is_internal         = 0;

    result_ptr = find_message_object(test_message, is_internal);

    //we expect a null result, there is no "666" message object in either array
    TEST_ASSERT_NULL_MESSAGE(result_ptr, "Found non-existant euiObject")
    
    //try the internal array branch
    is_internal = 1;
    result_ptr  = find_message_object(test_message, is_internal);
    TEST_ASSERT_NULL_MESSAGE(result_ptr, "Found non-existant euiObject")
}

TEST( FindMessageObject, find_message_object_nullptr )
{
    TEST_IGNORE_MESSAGE("Its not fair to require invalid pointer safety? (see note)");

    // Its a little tricky for this case. The search is using strcmp, which will cause segfaults or undefined behaviour
    // when passed invalid pointers. We could protect against null-prt in the library, but would take the hit on every use
    // there might be a more elegant solution, but for now this is probably out of scope.

    eui_message_t *result_ptr;
    const char * test_message   = 0;    //void ptr
    uint8_t is_internal         = 0;

    //will likely segfault on x86 (or whatever arch is unit testing)
    result_ptr = find_message_object(test_message, is_internal);

    TEST_ASSERT_NULL_MESSAGE(result_ptr, "Found euiObject matching nullptr!")

    //test the internal branch
    is_internal = 1;
    result_ptr = find_message_object(test_message, is_internal);
    TEST_ASSERT_NULL_MESSAGE(result_ptr, "Found euiObject matching nullptr")
}

TEST( FindMessageObject, find_message_object_single_letter )
{
    eui_message_t *expecting     = &test_findobject_store[0]; //int8 object
    eui_message_t *result_ptr;   //result should be a pointer to our expected element
    const char * test_message   = "a";
    uint8_t is_internal         = 0;

    result_ptr = find_message_object(test_message, is_internal);

    TEST_ASSERT_TRUE_MESSAGE( result_ptr == expecting, "Didn't return correct pointer")
}

TEST( FindMessageObject, find_message_object_max_letters )
{
    eui_message_t *expecting     = &test_findobject_store[2];
    eui_message_t *result_ptr;   //result should be a pointer to our expected element
    const char * test_message   = "abcdefghijklmno";
    uint8_t is_internal         = 0;

    result_ptr = find_message_object(test_message, is_internal);

    TEST_ASSERT_TRUE_MESSAGE( result_ptr == expecting, "Didn't return correct pointer")
}

TEST( FindMessageObject, find_message_object_no_id )
{
    eui_message_t *expecting     = &test_findobject_store[4]; //int8 object
    eui_message_t *result_ptr;   //result should be a pointer to our expected element
    const char * test_message   = "";
    uint8_t is_internal         = 0;

    result_ptr = find_message_object(test_message, is_internal);

    TEST_ASSERT_TRUE_MESSAGE( result_ptr == expecting, "Didn't return correct pointer")
}

TEST( FindMessageObject, find_message_object_non_printable )
{
    eui_message_t *expecting     = &test_findobject_store[3]; //int8 object
    eui_message_t *result_ptr;   //result should be a pointer to our expected element

    char buffer[] = { 0x01, 0x00 };
    uint8_t is_internal = 0;
    result_ptr = find_message_object(buffer, is_internal);

    TEST_ASSERT_TRUE_MESSAGE( result_ptr == expecting, "Didn't return correct pointer")
}

TEST( FindMessageObject, find_message_devArrayNULL )
{
    TEST_IGNORE_MESSAGE("TODO: Test missing developer object array");
}

TEST( FindMessageObject, find_message_internalArrayNULL )
{
    TEST_IGNORE_MESSAGE("TODO: Test missing internal object array");
}
