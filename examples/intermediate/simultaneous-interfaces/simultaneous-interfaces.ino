#include "electricui.h"

//example interactive data
uint8_t led_brightness  = 2;
uint8_t btn1_state      = 0;
uint8_t btn2_state      = 0;
uint16_t delta_time     = 0;
uint16_t loop_time      = 0;

//example function called by UI
void toggleLed()
{
  digitalWrite(4, !digitalRead(4));
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
const euiMessage_t dev_msg_store[] = {

    EUI_UINT8( "led", led_brightness ),
    EUI_FUNC(  "tgl", toggleLed ),
    EUI_UINT8( "btA", btn1_state ),
    EUI_UINT8( "btB", btn2_state ),
    EUI_UINT16("lop", delta_time ),

    //type examples
    EUI_UINT8(  "ui8", example_uint8 ),
    EUI_UINT16( "i16", example_uint16 ),
    EUI_UINT32( "i32", example_uint32 ),
    EUI_FLOAT(  "fPI", example_float ),
    EUI_UINT8(  "ua8", example_uint8_arr ),
    EUI_UINT16( "ia6", example_uint16_arr ),
    EUI_UINT32( "ia2", example_uint32_arr ),
    EUI_FLOAT(  "fpA", example_float_arr),
    EUI_CHAR(   "dst", demo_string ),

    EUI_UINT32( "lat", large_int_array ),

    //custom type examples
    EUI_CUSTOM( "rgb", example_rgb ),
    EUI_CUSTOM( "imu", example_imu ),
};

euiInterface_t transport_methods[] = {
    EUI_INTERFACE( &cdc_tx_putc ),
    EUI_INTERFACE( &uart_tx_putc ),
};

void setup() 
{
  Serial.begin(115200);   //USB CDC instance
  Serial1.begin(115200);  //hardware uart instance
  pinMode(4, OUTPUT);     //set led to output
  randomSeed( analogRead(A4) );

  //eUI setup
  EUI_LINK(transport_methods);
  EUI_TRACK(dev_msg_store);
  setup_identifier("hello", 5);

  loop_time = micros(); //loop counter in microseconds
}

uint8_t led_status_counter = 0;

void loop() 
{
  uart_rx_handler();  //check serial rx fifo

  //Interact with the real world
  analogWrite(9, led_brightness);   //draw to led
  btn1_state = digitalRead(5);      //buttonA on helloboard
  btn2_state = digitalRead(8);      //buttonB

  if(led_status_counter++ >= 200)
  {
    digitalWrite(13, !digitalRead(13));
    led_status_counter = 0;
  }

  delta_time = micros() - loop_time;  //counter diff between last loop, and now
  loop_time = micros();

  delay(rand()%(10-1) + 1); //randomly select loop delay for 1-10ms to simulate varying cpu load
}

void uart_rx_handler()
{
  //USB CDC VCP
  while(Serial.available() > 0)  //rx has data
  {  
    parse_packet(Serial.read(), &usb_comms);  //eat a byte
  }

  //Hardware Serial Port
  while(Serial1.available() > 0)
  {  
    parse_packet(Serial1.read(), &uart_comms);
  }
}

//helps us pretend what most other microcontrollers use as an output function
void cdc_tx_putc(uint8_t data)
{
  Serial.write(data);  //output on usb cdc virtual com
}

void uart_tx_putc(uint8_t data)
{
  Serial1.write(data); //write to hardware uart
}