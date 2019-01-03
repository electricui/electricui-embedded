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

    validate_offset_range(  0,
                            6,
                            TYPE_UINT16,
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

void test_offset_validation_int32( void )
{
    uint16_t output_start  = 0;
    uint16_t output_end    = 0;

    validate_offset_range(  0,
                            12,
                            TYPE_INT32,
                            40,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 0, output_start );
    TEST_ASSERT_EQUAL( 12, output_end );

    validate_offset_range(  0,
                            12,
                            TYPE_UINT32,
                            40,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 0, output_start );
    TEST_ASSERT_EQUAL( 12, output_end );

    validate_offset_range(  6,
                            14,
                            TYPE_INT32,
                            40,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 8, output_start );
    TEST_ASSERT_EQUAL( 16, output_end );
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

    TEST_ASSERT_EQUAL( 4, output_start );
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

    //should clamp upper bound to max
    validate_offset_range(  16,
                            22,
                            TYPE_FLOAT,
                            20,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 16,  output_start );
    TEST_ASSERT_EQUAL( 20, output_end );

    // should clamp lower edge to natural edge, downwards
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

void test_offset_validation_invalid_inputs( void )
{
	//invalid
    uint16_t output_start  = 0;
    uint16_t output_end    = 0;

    //no data range requested, should spit back 'tail edge'
    validate_offset_range(  0,
                            0,
                            TYPE_FLOAT,
                            20,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 0, output_start );
    TEST_ASSERT_EQUAL( 20, output_end );

    //invalid existing types should be per-byte
    validate_offset_range(  0,
                            8,
                            TYPE_CALLBACK,
                            6,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 0,  output_start );
    TEST_ASSERT_EQUAL( 6,  output_end );

    //both values are out of bounds, clamp
    validate_offset_range(  16,
                            24,
                            TYPE_FLOAT,
                            10,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 6,  output_start );
    TEST_ASSERT_EQUAL( 10,  output_end );

    //unknown types should fallback to 1-byte mode
    validate_offset_range(  0,
                            6,
                            150,
                            10,
                            &output_start,
                            &output_end );

    TEST_ASSERT_EQUAL( 0,  output_start );
    TEST_ASSERT_EQUAL( 6,  output_end );
}