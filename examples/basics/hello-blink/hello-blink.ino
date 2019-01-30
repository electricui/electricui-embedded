//Import the ElectricUI Library
#include "electricui.h"

#define LED_PIN 13

// Simple variables to modify the LED behaviour
uint8_t   blink_enable = 1; //if the blinker should be running
uint8_t   led_state  = 0;   //track if the LED is illuminated
uint16_t  glow_time  = 200; //in milliseconds
uint16_t  dark_time  = 200; //in milliseconds

// Keep track of when the light turns on or off
uint32_t time_on = 0;
uint32_t time_off = 0;

// Electric UI manages variables referenced in this array
eui_message_t dev_msg_store[] = 
{
  EUI_UINT8(  "led_blink",  blink_enable ),
  EUI_UINT8(  "led_state",  led_state ),
  EUI_UINT16( "lit_time",   glow_time ),
  EUI_UINT16( "unlit_time", dark_time ),
};

// Instantiate the communication interface's management object
eui_interface_t serial_comms; 

void setup() 
{
  // Setup the serial port and status LED
  Serial.begin( 115200 );
  pinMode( LED_PIN, OUTPUT );

  // Each communications interface uses a settable output function
  serial_comms.output_func = &tx_putc;

  // Provide the library with the interface we just setup
  setup_interface( &serial_comms );

  // Provide the tracked variables to the library
  EUI_TRACK( dev_msg_store );

  // Provide a identifier to make this board easy to find in the UI
  setup_identifier( "hello", 5 );


  time_on = millis();
  time_off = millis();
}

void loop() 
{
  uart_rx_handler();  //check for new inbound data

  if( led_state == LOW ) //LED is off
  {
    // Check if the LED has been off for the configured duration
    if( millis() - time_off >= dark_time )
    {
      led_state = HIGH;
      time_on = millis();
    }
  }
  else  //LED is on
  {
    if( millis() - time_on >= glow_time )
    {
      led_state = LOW;
      time_off = millis();
    }
  }

  if( blink_enable )
  {
    digitalWrite( LED_PIN, led_state ); //allow the led to blink
  }
  else
  {
    digitalWrite( LED_PIN, led_state );
  }

}

void uart_rx_handler()
{
  // While we have data, we will pass those bytes to the ElectricUI parser
  while( Serial.available() > 0 )  
  {  
    parse_packet( Serial.read(), &serial_comms );  // Ingest a byte
  }
}
  
void tx_putc( uint8_t *data, uint16_t len )
{
  //output on the main serial port
  for( uint16_t i = 0; i <= len; i++ )
  {
      Serial.write( data[i] );
  }
}
