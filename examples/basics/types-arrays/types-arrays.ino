#include "electricui.h"

// Arrays of our standard types
int8_t   example_int8_arr[]  = { -21, 20, -19 };
uint8_t  example_uint8_arr[] = { 21, 20, 19 };

int16_t  example_int16_arr[]  = { -321, -320, 319 };
uint16_t example_uint16_arr[] = { 321, 320, 319 };

int32_t  example_int32_arr[]  = { 654321, -654320, -654319 };
uint32_t example_uint32_arr[] = { 654321, 654320, 654319 };

float    example_float_arr[]  = { 1.60217, 9.1093, -1.0015 };
double   example_double_arr[] = { 1.60217, 9.1093, 6.0015 };

//a 100-element array of 32-bit ints is 400B of data
uint32_t  large_int_array[] = 
{ 
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
  30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
  40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
  50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
  60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
  70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
  80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
  90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
};

// An array of characters is a string
char demo_string[] = "HelloElectric";

// Arrays are tracked the same way as a normal variable
euiMessage_t dev_msg_store[] = 
{
    EUI_INT8(   "si8a", example_int8_arr  ),
    EUI_UINT8(  "ui8a", example_uint8_arr ),

    EUI_INT16(  "si16a", example_int16_arr  ),
    EUI_UINT16( "ui16a", example_uint16_arr ),
    
    EUI_INT32(  "si32a", example_int32_arr  ),
    EUI_UINT32( "ui32a", example_uint32_arr ),

    EUI_FLOAT(  "fspa", example_float_arr  ),
    EUI_DOUBLE( "fdpa", example_double_arr ),

    EUI_CHAR( "hi", demo_string),
};

eui_interface serial_comms;

void setup() 
{
    Serial.begin(115200);

    //eUI setup
    serial_comms.output_char_fnPtr = &tx_putc;
    setup_dev_msg(dev_msg_store, ARR_ELEM(dev_msg_store));
    setup_identifier("arrays", 6);
}

void loop() 
{
    uart_rx_handler();

    delay(1);
}

void uart_rx_handler()
{
  while(Serial.available() > 0)
  {  
      parse_packet(Serial.read(), &serial_comms);
  }
}

void tx_putc(uint8_t data)
{
    Serial.write(data);
}
