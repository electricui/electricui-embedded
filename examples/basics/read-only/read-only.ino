#include "electricui.h"

#define LED_PIN 13

// Example follows on from the hello-blink example, but shows how
// write protected variables are used to allow read-only access to
// immutable data like internal state or sensor readings.

// Simple variables to modify the LED behaviour
uint8_t   blink_enable = 1; // if the blinker should be running
uint8_t   led_state  = 0;   // track if the LED is illuminated
uint16_t  glow_time  = 200; // in milliseconds
uint16_t  dark_time  = 200; // in milliseconds
uint32_t time_on     = 0;
uint32_t time_off    = 0;

// Analog sensor value read and stored in this variable
uint16_t adc_reading = 0;

// Electric UI manages variables referenced in this array
eui_message_t dev_msg_store[] = 
{
  EUI_UINT8(    "led_blink",  blink_enable ),
  EUI_UINT8_RO( "led_state",  led_state ),
  EUI_UINT16(   "lit_time",   glow_time ),
  EUI_UINT16(   "unlit_time", dark_time ),

  // Sensor values should be read-only (immutable) because the 
  // user shouldn't be able to change the value
  EUI_UINT16_RO( "adc", adc_reading ),
};

// Instantiate the communication interface's management object
eui_interface_t serial_comms; 

void setup() 
{
  // Setup the serial port and status LED
  Serial.begin( 115200 );
  pinMode( LED_PIN, OUTPUT );

  // Standard configuration of a single interface with tracked variables
  serial_comms.output_func = &tx_putc;
  setup_interface( &serial_comms );
  EUI_TRACK( dev_msg_store );
  setup_identifier( "hello-ro", 8 );

  // init the blinking timers
  time_on = millis();
  time_off = millis();
}

void loop() 
{
  uart_rx_handler();  //check for new inbound data

  if( blink_enable )
  {
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
  }

  digitalWrite( LED_PIN, led_state );
  adc_reading = analogRead( A0 );
}

void uart_rx_handler()
{
  while( Serial.available() > 0 )  
  {  
    parse_packet( Serial.read(), &serial_comms );
  }
}
  
void tx_putc( uint8_t *data, uint16_t len )
{
  Serial.write( data, len );
}
