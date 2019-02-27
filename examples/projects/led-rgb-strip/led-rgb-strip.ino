/*
 * Demonstrates interacting with a strip of WS2812 RGB LEDs.
 * Dependant on the Adafruit library https://github.com/adafruit/Adafruit_NeoPixel
*/

#include "electricui.h"
#include <Adafruit_NeoPixel.h>

#define PIN_WS2812 7
#define STRIP_LENGTH 8

Adafruit_NeoPixel strip = Adafruit_NeoPixel( STRIP_LENGTH, PIN_WS2812, NEO_GRB + NEO_KHZ800 );
eui_interface_t serial_comms;

typedef struct {
    int red;
    int green;
    int blue;
} rgb_t;

//example hardware data
uint8_t rgb_brightness  = 50;
rgb_t   led_colour[ STRIP_LENGTH ] = { 0, 0, 0 };

const uint16_t num_leds_connected = STRIP_LENGTH;

eui_message_t dev_msg_store[] = 
{
    EUI_UINT8(        "bright",   rgb_brightness ),
    EUI_CUSTOM_ARRAY( "rgb",      led_colour ),
    EUI_UINT16_RO(    "numleds",  num_leds_connected ),
};

void setup() 
{
    Serial.begin(115200);

    //eUI setup
    serial_comms.output_func = &tx_putc;
    setup_interface(&serial_comms);
    EUI_TRACK(dev_msg_store);
    setup_identifier("rgbstrip", 8);

    // LEDs should start off
    strip.begin();
    strip.clear();
    strip.show();
}

void loop() 
{
  while( Serial.available() > 0 )
  {  
      parse_packet( Serial.read(), &serial_comms );
  }

  //Set each led in the strip
  for( int i = 0; i < STRIP_LENGTH; i++ )
  {
      strip.setPixelColor( i, strip.Color(led_colour[i].red, led_colour[i].green, led_colour[i].blue) );
  }

  strip.setBrightness( rgb_brightness );
  strip.show();

  delay(1);
}

void tx_putc( uint8_t *data, uint16_t len )
{
  Serial.write( data, len );
}