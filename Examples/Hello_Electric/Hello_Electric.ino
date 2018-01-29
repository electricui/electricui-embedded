extern "C"
{
  #include "electricui.h"
};

//example interactive data
uint8_t led_brightness = 2;
uint8_t btn1_state = 0;
uint8_t btn2_state = 0;

//example function called by UI
void toggleLed()
{
  digitalWrite(4, !digitalRead(4));
}

//example variable types
uint8_t   example_uint8   = 21;
uint16_t  example_uint16  = 321;
uint32_t  example_uint32  = 654321;
float     example_float   = 3.14159;

uint8_t   example_uint8_arr[]  = { 21, 20, 19 };
uint16_t  example_uint16_arr[] = { 321, 320, 319 };
uint32_t  example_uint32_arr[] = { 654321, 654320, 654319 };
float     example_float_arr[]  = { 1.60217, 9.1093, -1.0015 };

//example custom type
typedef struct {
  int red;
  int green;
  int blue;
} rgb_t;

typedef struct {
  float x;
  float y;
  float z;
} imu_t;

typedef enum {
    TYPE_RGB = TYPE_CUSTOM_MARKER,
    TYPE_IMU,
} custom_euiType_t;

rgb_t example_rgb = { 182, 236, 20 };
imu_t example_imu = { 0.002, 0.003, -9.782 };

//internal index of developer-space message metadata
const euiMessage_t dev_msg_store[] = {
    {.msgID = "led", .type = TYPE_UINT8,    .size = sizeof(led_brightness), .payload = &led_brightness  },
    {.msgID = "tgl", .type = TYPE_CALLBACK, .size = sizeof(toggleLed),      .payload = &toggleLed       },
    {.msgID = "btA", .type = TYPE_UINT8,    .size = sizeof(btn1_state),     .payload = &btn1_state      },
    {.msgID = "btB", .type = TYPE_UINT8,    .size = sizeof(btn2_state),     .payload = &btn2_state      },

    //type examples
    {.msgID = "ui8", .type = TYPE_UINT8,  .size = sizeof(example_uint8),  .payload = &example_uint8       },
    {.msgID = "i16", .type = TYPE_UINT16, .size = sizeof(example_uint16), .payload = &example_uint16      },
    {.msgID = "i32", .type = TYPE_UINT32, .size = sizeof(example_uint32), .payload = &example_uint32      },
    {.msgID = "fPI", .type = TYPE_FLOAT,  .size = sizeof(example_float),  .payload = &example_float       },
    {.msgID = "ua8", .type = TYPE_UINT8,  .size = sizeof(example_uint8_arr),  .payload = &example_uint8_arr   },
    {.msgID = "ia6", .type = TYPE_UINT16, .size = sizeof(example_uint16_arr), .payload = &example_uint16_arr  },
    {.msgID = "ia2", .type = TYPE_UINT32, .size = sizeof(example_uint32_arr), .payload = &example_uint32_arr  },
    {.msgID = "fpA", .type = TYPE_FLOAT,  .size = sizeof(example_float_arr),  .payload = &example_float_arr   },

    //custom type examples
    {.msgID = "rgb", .type = TYPE_RGB, .size = sizeof(example_rgb), .payload = &example_rgb },
    {.msgID = "imu", .type = TYPE_IMU, .size = sizeof(example_imu), .payload = &example_imu },
};

eui_interface_state usb_comms; //eui Transport interface holding obj


void setup() 
{
  Serial.begin(115200);   //USB CDC instance
  Serial1.begin(115200);  //hardware uart instance
  pinMode(4, OUTPUT);     //set led to output


  //pass parser callback ptr and developer msg array to eUI lib.
  usb_comms.output_char_fnPtr = &uart_tx_putc;
  setupDevMsg(dev_msg_store, ARR_ELEM(dev_msg_store));
  setupIdentifier();
}

void loop() 
{
  uart_rx_handler();  //check serial rx fifo

  //Interact with the real world
  analogWrite(9, led_brightness);   //draw to led
  btn1_state = digitalRead(5);      //buttonA on helloboard
  btn2_state = digitalRead(8);      //buttonB

  //printDevArray();  //print messages without being prompted

  delay(50);
}

void printDevArray()
{
  for(int i = 0; i < ARR_ELEM(dev_msg_store); i++)
  {
      generatePacket( dev_msg_store[i].msgID, 
                      generateHeader(MSG_DEV, MSG_ACK_NOTREQ, MSG_RES_L, MSG_TYPE_TYP, dev_msg_store[i].type),
                      dev_msg_store[i].size, 
                      dev_msg_store[i].payload,
                      &uart_tx_putc);
  }
}

void uart_rx_handler()
{
  while(Serial.available() > 0)  //rx has data
  {  
    parsePacket(Serial.read(), &usb_comms);  //eat a byte
  }
}

//helps us pretend what most other microcontrollers use as an output function
void uart_tx_putc(uint8_t data)
{
  Serial1.write(data); //write to hardware uart
  Serial.write(data);  //output on usb cdc virtual com
}