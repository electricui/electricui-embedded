/*
   Covers the basics of using different types for tracked variables
   Teaches read only, arrays, manual variable definition
*/

#include "electricui.h"

// Define a few variables to play with
int8_t   example_int8   = -5;
uint8_t  example_uint8  = 21;

int16_t  example_int16  = -13397;
uint16_t example_uint16 = 23525;

int32_t  example_int32  = -1659853797;
uint32_t example_uint32 = 1651006437;

float    example_float  = 3.141592;
double   example_double = 2.71828182845904;

typedef struct
{
    float x;
    float y;
    float z;
} Imu_t;

Imu_t accelerometer = { 0.0202, -9.8303, 1.3022 };

Imu_t triple_sensor[] = {
  {  0.0202, -9.8303,  1.3022 }, // Accelerometer
  {  0.6324,  0.5476, -0.3324 }, // Gyroscope
  { -0.9764,  0.3247, -0.2366 }  // Magnetometer
};

uint8_t  example_uint8_arr[] = { 21, 20, 19 };

char demo_string[] = "Hello Electric";

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

eui_message_t dev_msg_store[] = 
{
    // Track a variable - Provide an ID string (up to 16 characters) which is also used in the UI
    EUI_UINT8(    "u8", example_uint8 ),

    // Read Only - When variables shouldn't be modified by the UI, like sensor readings
    EUI_UINT8_RO( "u8", example_uint8 ),

    // If you want to manually provide the structure, this is what the EUI_UINT8 macro expands into 
    { .id = "manual_u8", .type = TYPE_INT8, .size = sizeof(example_uint8), .payload = &example_uint8 },

    // There are macros for most types available
    EUI_INT8( "s8", example_int8 ),
    EUI_UINT8( "u8", example_uint8 ),
    EUI_INT16( "s16", example_int16 ),
    EUI_UINT16( "u16", example_uint16 ),
    EUI_INT32( "s32", example_int32 ),
    EUI_UINT32( "u32", example_uint32 ),
    EUI_FLOAT( "spf", example_float ),
    EUI_DOUBLE( "dpf", example_double ),

    // User-defined structures are also supported
    EUI_CUSTOM( "tri-float", accelerometer ),

    // Array of structures behave like a larger structure
    EUI_CUSTOM( "tri-float-arr", triple_sensor ),

    // Arrays of default types indicated like so
    EUI_UINT8_ARRAY( "u8-arr", example_uint8_arr ),
    EUI_UINT32_ARRAY( "big-arr", large_int_array ),

    EUI_CHAR_ARRAY( "hi", demo_string),
};

eui_interface_t serial_comms;

void setup() 
{
    Serial.begin(115200);

    //eUI setup
    serial_comms.output_cb = &tx_putc;
    setup_interface(&serial_comms);

    EUI_TRACK(dev_msg_store);
    setup_identifier("types", 6);
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

void tx_putc( uint8_t *data, uint16_t len )
{
  Serial.write( data, len );
}
