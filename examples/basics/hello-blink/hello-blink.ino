//Import the ElectricUI Library
#include "electricui.h"

#define LED_PIN 13

// Simple variables to modify the LED behaviour
uint8_t   blink_enable = 0; //if the blinker should be running
uint8_t   led_state  = 0;   //track if the LED is illuminated
uint16_t  glow_time  = 200; //in milliseconds
uint16_t  dark_time  = 200; //in milliseconds

// Keep track of how much time we've spent on or off
uint16_t time_on = 0;
uint16_t time_off = 0;

// ElectricUI tracks variables referenced in this array
eui_message_t dev_msg_store[] = 
{
    EUI_UINT8(  "led_blink",  blink_enable ),
    EUI_UINT8(  "led_state",  led_state ),
    EUI_UINT16( "lit_time",   glow_time ),
    EUI_UINT16( "unlit_time", dark_time ),
};

// Instantiate the communication interface
eui_interface_t serial_comms; 

void setup() 
{
  Serial.begin(115200);   // Microcontroller's primary serial interface (usually the USB cable)

  // Setup the output interface
  serial_comms.output_func = &tx_putc;
  setup_interface(&serial_comms, 1);

  // Provide the tracked variables to the library
  EUI_TRACK(dev_msg_store);

  // Provide a identifier to make it easy to find in the UI
  setup_identifier("hello", 5);

  time_on = millis();
  time_off = millis();

}

void loop() 
{
  uart_rx_handler();  //check for new inbound data

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
      digitalWrite( LED_PIN, LOW );
  }
  else
  {
      digitalWrite( LED_PIN, led_state ); //allow the led to blink
  }

}

void uart_rx_handler()
{
    while(Serial.available() > 0)  // While we have data, we will pass those bytes to the ElectricUI parser
    {  
        parse_packet(Serial.read(), &serial_comms);  // Ingest a byte
    }
}

void tx_putc(uint8_t data)
{
    Serial.write(data);  //output on the main serial port
}
