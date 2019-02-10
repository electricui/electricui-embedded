#include "electricui.h"

#if defined(ESP32) || defined(ESP8266)

#else
  #ifndef HAVE_HWSERIAL1
    #include <SoftwareSerial.h>

    SoftwareSerial Serial1(10, 11);
  #endif
#endif

// Example demonstrates connections to the UI over several serial links
// The UI can switch between the links for failover or as part of load-balancing.

// Simple variables to modify the LED behaviour
uint8_t   blink_enable = 0; //if the blinker should be running
uint8_t   led_state  = 0;   //track if the LED is illuminated
uint16_t  glow_time  = 200; //in milliseconds
uint16_t  dark_time  = 200; //in milliseconds

// Keep track of how much time we've spent on or off
uint16_t time_on = 0;
uint16_t time_off = 0;

char demo_string[] = "HelloElectric";

// Function declarations for the serial output callbacks
void serial0_tx( uint8_t *data, uint16_t len );
void serial1_tx( uint8_t *data, uint16_t len );

// Tracked variables
eui_message_t dev_msg_store[] = {

    EUI_UINT8( "led_blink",  blink_enable ),
    EUI_UINT8( "led_state",  led_state ),
    EUI_UINT16("lit_time",   glow_time ),
    EUI_UINT16("unlit_time", dark_time ),
};

// List of interfaces passed to Electric UI with the output pointers
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
  setup_identifier("hello many", 10);

  //led timers
  time_on = millis();
  time_off = millis();
}

uint8_t led_status_counter = 0;

void loop() 
{
  rx_handler();  //check serial rx fifo

  // Interact with the real world
  if( blink_enable )
  {
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
  }

  digitalWrite( LED_BUILTIN, led_state );
}

void rx_handler()
{
  // USB CDC VCP
  while(Serial.available() > 0)  //rx has data
  {  
    parse_packet(Serial.read(), &transport_methods[0]);
  }

  // Second Serial Port
  while(Serial1.available() > 0)
  {  
    parse_packet(Serial1.read(), &transport_methods[1]);
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