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

// SETUP, TEARDOWN
 
void setUp(void)
{

}
 
void tearDown(void)
{

}

// TESTS

void test_packet_instantiation_macro( void )
{
    eui_packet_t empty_manual = {   .parser = {0}, 
                                    .header = {0}, 
                                    .id_in="", 
                                    .offset_in=0, 
                                    .crc_in=0, 
                                    .data_in={0}
                                };

    eui_packet_t empty_macro = EUI_PACKET_EMPTY;

    TEST_ASSERT_EQUAL_MEMORY_ARRAY_MESSAGE( &empty_manual, 
                                            &empty_macro, 
                                            sizeof(eui_packet_t), 
                                            1, 
                                            "Empty packet macro not identical to manual init"); 
}


void test_interface_instantiation_macro( void )
{
    // Just the output function callback
    eui_interface_t init_manual = { .packet = {0}, 
                                    .output_cb = &output_callback, 
                                    .interface_cb = 0 
                                };

    eui_interface_t init_macro = EUI_INTERFACE( output_callback );


    TEST_ASSERT_EQUAL_MEMORY_ARRAY_MESSAGE( &init_manual, 
                                            &init_macro, 
                                            sizeof(eui_interface_t), 
                                            1, 
                                            "Init interface macro not identical to manual init"); 

    // With the interface callback as well
    eui_interface_t empty_manual_ifcb = {   .packet = {0}, 
                                            .output_cb = &output_callback,
                                            .interface_cb = &interface_callback 
                                        };

    eui_interface_t init_macro_ifcb = EUI_INTERFACE_CB( output_callback, interface_callback );

    TEST_ASSERT_EQUAL_MEMORY_ARRAY_MESSAGE( &empty_manual_ifcb, 
                                            &init_macro_ifcb, 
                                            sizeof(eui_interface_t), 
                                            1, 
                                            "Init interface+ifcb macro not identical to manual init"); 
}
