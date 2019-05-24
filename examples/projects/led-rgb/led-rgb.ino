/*
 * This demo uses a single WS2812 RGB led, use with the colour picker UI component.
 * Dependant on the Adafruit library https://github.com/adafruit/Adafruit_NeoPixel
*/

#include "electricui.h"
#include <Adafruit_NeoPixel.h>

#define PIN_WS2812 7

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN_WS2812, NEO_GRB + NEO_KHZ800);
eui_interface_t serial_comms;

typedef struct {
  int red;
  int green;
  int blue;
} rgb_t;

//example hardware data
uint8_t rgb_brightness  = 50;
rgb_t   led_colour      = { 10, 128, 60 };

char device_name[] = "led-rgb test";

eui_message_t dev_msg_store[] = 
{
    EUI_UINT8(        "bright", rgb_brightness ),
    EUI_CHAR_ARRAY(   "name",   device_name ),
    EUI_UINT16_ARRAY( "rgb",    led_colour ),
};

void setup() 
{
  Serial.begin(115200);

  // eUI setup
  serial_comms.output_cb = &tx_putc;
  eui_setup_interface(&serial_comms);
  EUI_TRACK(dev_msg_store);
  eui_setup_identifier("rgbled", 6);

  // LEDs should start off
  strip.begin();
  strip.clear();
  strip.show();
}

void loop() 
{
  while(Serial.available() > 0)
  {  
    eui_parse(Serial.read(), &serial_comms);
  }

  //RGB LED
  strip.setPixelColor( 0, strip.Color(led_colour.red, led_colour.green, led_colour.blue) );
  strip.setBrightness( rgb_brightness );
  strip.show();

  delay(1);
}

void tx_putc( uint8_t *data, uint16_t len )
{
  Serial.write( data, len );
}