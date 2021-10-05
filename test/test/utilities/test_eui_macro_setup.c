#include "unity.h"
 
// MODULE UNDER TEST
#include "eui_macro.h"
#include "mock_electricui.h"

// DEFINITIONS 
 
// PRIVATE TYPES

 
// PRIVATE FUNCTIONS
void output_callback(uint8_t *data, uint16_t length);
void interface_callback(uint8_t flag);

void output_callback(uint8_t *data, uint16_t length)
{
    //this is a test function...
}

void interface_callback(uint8_t flag)
{
    //this is a test function...
}

// PRIVATE DATA
char char_test = 'a';
uint8_t u8_test = 10;

eui_message_t basic_tracked_vars[] = {

    EUI_CHAR(  "char", char_test ),
    EUI_UINT8( "u8", u8_test ),
};

eui_interface_t init_macro_arr[] = { EUI_INTERFACE( &output_callback ), EUI_INTERFACE( &output_callback )};

// SETUP, TEARDOWN
 
void setUp(void)
{

}
 
void tearDown(void)
{

}

// TESTS
void test_link_setup_macro( void )
{
    // Using the helper macro to setup the interface
    eui_setup_interfaces_Expect( init_macro_arr, 2 );
    EUI_LINK( init_macro_arr );
}

void test_tracked_setup_macro( void )
{
    // Using the helper macro to setup the tracked variables
    eui_setup_tracked_Expect( basic_tracked_vars, 2 );
    EUI_TRACK( basic_tracked_vars );
}