//Import the ElectricUI Library
#include "electricui.h"

// Simple variables to modify the LED behaviour
uint8_t   blink_enable = 1; //if the blinker should be running
uint8_t   led_state  = 0;   //track if the LED is illuminated
uint16_t  glow_time  = 200; //in milliseconds

// Keep track of when the light turns on or off
uint32_t led_timer = 0;

// Instantiate the communication interface's management object
eui_interface_t serial_comms; 

// Electric UI manages variables referenced in this array
eui_message_t dev_msg_store[] = 
{
  EUI_UINT8(  "led_blink",  blink_enable ),
  EUI_UINT8(  "led_state",  led_state ),
  EUI_UINT16( "lit_time",   glow_time ),
};

void setup() 
{
  // Setup the serial port and status LED
  Serial.begin( 115200 );
  pinMode( LED_BUILTIN, OUTPUT );

  // Each communications interface uses a settable output function
  serial_comms.output_cb = &tx_putc;

  // Provide the library with the interface we just setup
  setup_interface( &serial_comms );

  // Provide the tracked variables to the library
  EUI_TRACK( dev_msg_store );

  // Provide a identifier to make this board easy to find in the UI
  setup_identifier( "hello", 5 );


  led_timer = millis();
}

void loop() 
{
  serial_rx_handler();  //check for new inbound data

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

void serial_rx_handler()
{
  // While we have data, we will pass those bytes to the ElectricUI parser
  while( Serial.available() > 0 )  
  {  
    parse_packet( Serial.read(), &serial_comms );  // Ingest a byte
  }
}
  
void tx_putc( uint8_t *data, uint16_t len )
{
  Serial.write( data, len ); //output on the main serial port
}
