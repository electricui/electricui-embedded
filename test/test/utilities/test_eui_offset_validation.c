#include "unity.h"
 
// MODULE UNDER TEST
#include "eui_offset_validation.h"
#include "eui_config.h"
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

void test_offset_validation_int8( void )
{
    uint16_t output_start  = 0;
    uint16_t output_end    = 0;

    validate_offset_range(  0,
                            4,
                            TYPE_INT8,
                            20,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 0, output_start );
    TEST_ASSERT_EQUAL( 4, output_end );

    validate_offset_range(  4,
                            10,
                            TYPE_INT8,
                            20,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 4, output_start );
    TEST_ASSERT_EQUAL( 10, output_end );
}

void test_offset_validation_int16( void )
{
    uint16_t output_start  = 0;
    uint16_t output_end    = 0;

    validate_offset_range(  0,
                            6,
                            TYPE_INT16,
                            40,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 0, output_start );
    TEST_ASSERT_EQUAL( 6, output_end );

    validate_offset_range(  6,
                            14,
                            TYPE_INT16,
                            40,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 6, output_start );
    TEST_ASSERT_EQUAL( 14, output_end );
}

void test_offset_validation_float( void )
{
    uint16_t output_start  = 0;
    uint16_t output_end    = 0;

    validate_offset_range(  0,
                            12,
                            TYPE_FLOAT,
                            40,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 0,  output_start );
    TEST_ASSERT_EQUAL( 12, output_end );

    validate_offset_range(  4,
                            16,
                            TYPE_FLOAT,
                            40,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 4,  output_start );
    TEST_ASSERT_EQUAL( 16, output_end );
}

void test_offset_validation_double( void )
{
    uint16_t output_start  = 0;
    uint16_t output_end    = 0;

    validate_offset_range(  0,
                            16,
                            TYPE_DOUBLE,
                            40,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 0,  output_start );
    TEST_ASSERT_EQUAL( 16, output_end );

    validate_offset_range(  8,
                            24,
                            TYPE_DOUBLE,
                            40,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 8,  output_start );
    TEST_ASSERT_EQUAL( 24, output_end );
}

void test_offset_validation_custom( void )
{
    uint16_t output_start  = 0;
    uint16_t output_end    = 0;

    validate_offset_range(  0,
                            9,
                            TYPE_CUSTOM,
                            10,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 0,  output_start );
    TEST_ASSERT_EQUAL( 9, output_end );
}

void test_offset_validation_swap_ends( void )
{
    uint16_t output_start  = 0;
    uint16_t output_end    = 0;

    validate_offset_range(  4,
                            0,
                            TYPE_INT8,
                            20,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 19, output_start );
    TEST_ASSERT_EQUAL( 20, output_end );

    validate_offset_range(  10,
                            4,
                            TYPE_INT8,
                            20,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 3, output_start );
    TEST_ASSERT_EQUAL( 4, output_end );
}

void test_offset_validation_overrun_input( void )
{
    uint16_t output_start  = 0;
    uint16_t output_end    = 0;

    validate_offset_range(  4,
                            40,
                            TYPE_INT8,
                            20,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 4, output_start );
    TEST_ASSERT_EQUAL( 20, output_end );

    validate_offset_range(  10,
                            40,
                            TYPE_INT8,
                            20,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 10, output_start );
    TEST_ASSERT_EQUAL( 20, output_end );
}

void test_offset_validation_align_edge( void )
{
    uint16_t output_start  = 0;
    uint16_t output_end    = 0;

    validate_offset_range(  0,
                            11,
                            TYPE_FLOAT,
                            20,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 0,  output_start );
    TEST_ASSERT_EQUAL( 12, output_end );

    validate_offset_range(  3,
                            16,
                            TYPE_FLOAT,
                            20,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 4,  output_start );
    TEST_ASSERT_EQUAL( 16, output_end );

    validate_offset_range(  3,
                            14,
                            TYPE_FLOAT,
                            20,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 4,  output_start );
    TEST_ASSERT_EQUAL( 16, output_end );
}

void test_offset_validation_align_edge_into_overrun( void )
{
    uint16_t output_start  = 0;
    uint16_t output_end    = 0;

    validate_offset_range(  16,
                            22,
                            TYPE_FLOAT,
                            20,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 16,  output_start );
    TEST_ASSERT_EQUAL( 20, output_end );

    validate_offset_range(  17,
                            20,
                            TYPE_FLOAT,
                            20,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 16,  output_start );
    TEST_ASSERT_EQUAL( 20, output_end );

    validate_offset_range(  19,
                            23,
                            TYPE_FLOAT,
                            20,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 16,  output_start );
    TEST_ASSERT_EQUAL( 20, output_end );
}
