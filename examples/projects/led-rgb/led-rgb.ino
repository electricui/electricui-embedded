#include "electricui.h"
#include <Adafruit_NeoPixel.h>    // Required library for the WS2812 RGB led

#define PIN_WS2812 7

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN_WS2812, NEO_GRB + NEO_KHZ800);
eui_interface_t serial_comms; //eui Transport

typedef struct {
  int red;
  int green;
  int blue;
} rgb_t;

//example hardware data
uint8_t rgb_brightness  = 50;
rgb_t   led_colour      = { 182, 236, 20 };

eui_message_t dev_msg_store[] = 
{
    EUI_UINT8( "bright", rgb_brightness ),
    EUI_CUSTOM("rgb", led_colour ),
};

void tx_putc(uint8_t data)
{
  Serial.write(data);
}

void setup() 
{
  Serial.begin(115200);

  //eUI setup
  serial_comms.output_func = &tx_putc;
  setup_interface(&serial_comms, 1);
  EUI_TRACK(dev_msg_store);
  setup_identifier("rgbled", 6);

  strip.begin();
  strip.clear();
  strip.show();
}

void loop() 
{
  while(Serial.available() > 0)   //handle inbound serial data
  {  
    parse_packet(Serial.read(), &serial_comms);
  }

  //RGB LED
  strip.setPixelColor( 0, strip.Color(led_colour.red, led_colour.green, led_colour.blue) );
  strip.setBrightness( rgb_brightness );
  strip.show();

  delay(1);
}