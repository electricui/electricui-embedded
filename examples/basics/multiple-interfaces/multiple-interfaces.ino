// Example demonstrates connections to the UI over several serial links
// The UI can switch between the links for failover or as part of load-balancing.

#include "electricui.h"

// Depending on your microcontroller's 'second' UART, you may need to configure
// SoftwareSerial for the second output, or SERCOM on Atmel 32-bit boards.
// Needs an external UART to USB adaptor or similar for PC connection
#if defined (__AVR__) || (__avr__)
  #ifndef HAVE_HWSERIAL1
    #include <SoftwareSerial.h>

    SoftwareSerial Serial1(10, 11);
  #endif
#endif


// Simple variables to modify the LED behaviour
uint8_t   blink_enable = 1; //if the blinker should be running
uint8_t   led_state  = 0;   //track if the LED is illuminated
uint16_t  glow_time  = 200; //in milliseconds

// Keep track of when the light turns on or off
uint32_t led_timer = 0;

char demo_string[] = "HelloMultiple";

// Function declarations for the serial output callbacks
void serial0_tx( uint8_t *data, uint16_t len );
void serial1_tx( uint8_t *data, uint16_t len );

// Tracked variables
eui_message_t dev_msg_store[] = 
{
  EUI_UINT8(  "led_blink",  blink_enable ),
  EUI_UINT8(  "led_state",  led_state ),
  EUI_UINT16( "lit_time",   glow_time ),
  EUI_CHAR_ARRAY( "name",   demo_string ),
};

// List of interfaces passed to Electric UI with the output pointers
eui_interface_t transport_methods[] = {
    EUI_INTERFACE( &serial0_tx ),
    EUI_INTERFACE( &serial1_tx ),
};

void setup() 
{
  Serial.begin( 115200 );   // USB Connector

  // Most arduino targets will use the next hardware serial (or softserial from top of file)
  Serial1.begin( 115200 );  

  pinMode( LED_BUILTIN, OUTPUT );

  //eUI setup
  EUI_LINK( transport_methods );
  EUI_TRACK( dev_msg_store );
  eui_setup_identifier("hello many", 10);

  //led timers
  led_timer = millis();
}

void loop() 
{
  rx_handler();  //check serial rx fifo

  // Interact with the real world
  if( blink_enable )
  {
    // Check if the LED has been on for the configured duration
    if( millis() - led_timer >= glow_time )
    {
      led_state = !led_state; //invert led state
      led_timer = millis();
    }    
  }

  digitalWrite( LED_BUILTIN, led_state ); //update the LED to match the intended state

}

void rx_handler()
{
  // USB CDC VCP
  while( Serial.available() > 0 )  //rx has data
  {  
    eui_parse( Serial.read(), &transport_methods[0] );
  }

  // Second Serial Port
  while( Serial1.available() > 0 )
  {  
    eui_parse( Serial1.read(), &transport_methods[1] );
  }
}

void serial0_tx( uint8_t *data, uint16_t len )
{
  // Output over main serial connection
  Serial.write( data, len );
}

void serial1_tx( uint8_t *data, uint16_t len )
{
  // Write to second serial port (or software serial on 10/11 if none exists)
  Serial1.write( data, len );
}