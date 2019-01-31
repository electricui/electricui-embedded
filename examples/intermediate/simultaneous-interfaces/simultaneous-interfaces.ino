#include "electricui.h"

#if defined(ESP32) || defined(ESP8266)

#else
  #ifndef HAVE_HWSERIAL1
    #include <SoftwareSerial.h>

    SoftwareSerial Serial1(10, 11);
  #endif
#endif

//example interactive data
// Simple variables to modify the LED behaviour
uint8_t   blink_enable = 0; //if the blinker should be running
uint8_t   led_state  = 0;   //track if the LED is illuminated
uint16_t  glow_time  = 200; //in milliseconds
uint16_t  dark_time  = 200; //in milliseconds

// Keep track of how much time we've spent on or off
uint16_t time_on = 0;
uint16_t time_off = 0;

uint16_t delta_time     = 0;
uint16_t loop_time      = 0;

//example function called by UI
void toggleLed()
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

//example variable types
uint8_t   example_uint8   = 21;
uint16_t  example_uint16  = 321;
uint32_t  example_uint32  = 654321;
float     example_float   = 3.14159;

uint8_t   example_uint8_arr[]  = { 21, 20, 19 };
uint16_t  example_uint16_arr[] = { 321, 320, 319 };
uint32_t  example_uint32_arr[] = { 654321, 654320, 654319 };
float     example_float_arr[]  = { 1.60217, 9.1093, -1.0015 };

//a 100-element array of 32-bit ints is 400B of data
uint32_t  large_int_array[] = { 
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

char demo_string[] = "HelloElectric";

//example custom type
typedef struct {
  int red;
  int green;
  int blue;
} rgb_t;

typedef struct {
  float x;
  float y;
  float z;
} imu_t;

rgb_t example_rgb = { 182, 236, 20 };
imu_t example_imu = { 0.002, 0.003, -9.782 };

//internal index of developer-space message metadata
eui_message_t dev_msg_store[] = {

    EUI_UINT8( "led_blink",  blink_enable ),
    EUI_UINT8( "led_state",  led_state ),
    EUI_UINT16("lit_time",   glow_time ),
    EUI_UINT16("unlit_time", dark_time ),
    EUI_UINT16("lop", delta_time ),

    //type examples
    EUI_UINT8(  "ui8", example_uint8 ),
    EUI_UINT16( "i16", example_uint16 ),
    EUI_UINT32( "i32", example_uint32 ),
    EUI_FLOAT(  "fPI", example_float ),
    EUI_UINT8_ARRAY(  "ua8", example_uint8_arr ),
    EUI_UINT16_ARRAY( "ia6", example_uint16_arr ),
    EUI_UINT32_ARRAY( "ia2", example_uint32_arr ),
    EUI_FLOAT_ARRAY(  "fpA", example_float_arr),
    EUI_CHAR_ARRAY(   "dst", demo_string ),

    EUI_UINT32_ARRAY( "lat", large_int_array ),

    //custom type examples
    EUI_CUSTOM( "rgb", example_rgb ),
    EUI_CUSTOM( "imu", example_imu ),
};

eui_interface_t transport_methods[] = {
    EUI_INTERFACE( &serial0_tx ),
    EUI_INTERFACE( &serial1_tx ),
};

void setup() 
{
  Serial.begin(115200);   // USB Connector

  //most arduino targets will use the next hardware serial (or softserial from top of file)
  Serial1.begin(115200);  

  pinMode(LED_BUILTIN, OUTPUT);

  //eUI setup
  EUI_LINK(transport_methods);
  EUI_TRACK(dev_msg_store);
  setup_identifier("hello", 5);

  //led timers
  time_on = millis();
  time_off = millis();

  loop_time = micros(); //loop counter in microseconds
}

uint8_t led_status_counter = 0;

void loop() 
{
  uart_rx_handler();  //check serial rx fifo

  //Interact with the real world
  if( led_state == LOW ) //LED is off
  {
      if( millis() - time_off > dark_time )
      {
          led_state = 1;
          time_on = millis();
      }
  }
  else  //led is on
  {
      if( millis() - time_on > glow_time )
      {
          led_state = 0;
          time_off = millis();
      }
  }

  if(blink_enable == 0)
  {
      digitalWrite( LED_BUILTIN, LOW );
  }
  else
  {
      digitalWrite( LED_BUILTIN, led_state ); //allow the led to blink
  }


  delta_time = micros() - loop_time;  //counter diff between last loop, and now
  loop_time = micros();
}

void uart_rx_handler()
{
  //USB CDC VCP
  while(Serial.available() > 0)  //rx has data
  {  
    parse_packet(Serial.read(), &transport_methods[0]);  //eat a byte
  }

  //Hardware Serial Port
  while(Serial1.available() > 0)
  {  
    parse_packet(Serial1.read(), &transport_methods[1]);
  }
}

void serial0_tx( uint8_t *data, uint16_t len )
{
  //output over usb connector
  for( uint16_t i = 0; i < len; i++ )
  {
      Serial.write( data[i] );
  }
}

void serial1_tx( uint8_t *data, uint16_t len )
{
  //write to second serial port (or software serial on 10/11 if none exists)
  for( uint16_t i = 0; i < len; i++ )
  {
      Serial1.write( data[i] );
  }
}