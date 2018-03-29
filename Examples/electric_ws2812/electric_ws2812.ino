extern "C"
{
  #include "electricui.h"
};

#include <Adafruit_NeoPixel.h>    //dep for WS2812 RGB led

#define PIN_WS2812 7
#define PIN_PHOTO A3
#define PIN_BUTTONA 5
#define PIN_BUTTONB 8

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN_WS2812, NEO_GRB + NEO_KHZ800);
eui_interface usb_comms; //eui Transport

typedef struct {
  int red;
  int green;
  int blue;
} rgb_t;

uint16_t delta_time     = 0;
uint16_t loop_time      = 0;

//example hardware data
uint8_t btn1_state      = 0;
uint8_t btn2_state      = 0;
uint8_t rgb_brightness = 50;
uint16_t photosensor_raw = 0;
rgb_t example_rgb = { 182, 236, 20 };

const euiMessage_t dev_msg_store[] = {
    {.msgID = "lop", .type = TYPE_UINT16,   .size = sizeof(delta_time),       .payload = &delta_time      },
    {.msgID = "btA", .type = TYPE_UINT8,    .size = sizeof(btn1_state),       .payload = &btn1_state      },
    {.msgID = "btB", .type = TYPE_UINT8,    .size = sizeof(btn2_state),       .payload = &btn2_state      },
    
    {.msgID = "pts", .type = TYPE_UINT16,   .size = sizeof(photosensor_raw),  .payload = &photosensor_raw },
    {.msgID = "bright", .type = TYPE_UINT8, .size = sizeof(rgb_brightness),   .payload = &rgb_brightness  },
    {.msgID = "rgb", .type = TYPE_CUSTOM_MARKER, .size = sizeof(example_rgb), .payload = &example_rgb     },
};

void cdc_tx_putc(uint8_t data)
{
  Serial.write(data);
}

void setup() 
{
  Serial.begin(115200); //USB
  pinMode(PIN_PHOTO, INPUT);

  //eUI setup
  usb_comms.output_char_fnPtr = &cdc_tx_putc;
  setup_dev_msg(dev_msg_store, ARR_ELEM(dev_msg_store));
  setup_identifier();

  strip.begin();
  strip.clear();
  strip.show();

  loop_time = micros(); //loop counter in microseconds
}

void loop() 
{
  while(Serial.available() > 0)   //handle inbound serial data
  {  
    parse_packet(Serial.read(), &usb_comms);
  }

  //Interact with the real world
  btn1_state = digitalRead(PIN_BUTTONA);      //buttonA on helloboard
  btn2_state = digitalRead(PIN_BUTTONB);      //buttonB
  photosensor_raw = analogRead(PIN_PHOTO);    //onboard phototransistor

  //RGB LED
  strip.setPixelColor( 0, strip.Color(example_rgb.red, example_rgb.green, example_rgb.blue) );
  strip.setBrightness(rgb_brightness);
  strip.show();

  //loop timing
  delta_time = micros() - loop_time;  //counter diff between last loop, and now
  loop_time = micros();
  delay(1); //randomly select loop delay for 1-10ms to simulate varying cpu load
}