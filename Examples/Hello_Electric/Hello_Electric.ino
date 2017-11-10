extern "C"
{
  #include "electricui.h"
};

//example interactive data
uint8_t led_brightness = 2;
uint8_t btn1_state = 0;
uint8_t btn2_state = 0;

void toggleLed()
{
  digitalWrite(4, !digitalRead(4));
}

//type example variables
uint8_t   example_uint8   = 21;
uint16_t  example_uint16  = 321;
uint32_t  example_uint32  = 654321;
float     example_float   = 3.1415;

uint8_t   example_uint8_arr[]  = { 21, 20, 19 };
uint16_t  example_uint16_arr[] = { 321, 320, 319 };
uint32_t  example_uint32_arr[] = { 654321, 654320, 654319 };
float     example_float_arr[]  = { 1.60217, 9.1093, -1.0015 };


//internal index of developer-space message metadata
const euiMessage_t dev_msg_store[] = {
    {.msgID = "led", .type = TYPE_UINT8, .payload = &led_brightness },
    {.msgID = "tgl", .type = TYPE_CALLBACK, .payload = &toggleLed },
    {.msgID = "btA", .type = TYPE_UINT8, .payload = &btn1_state },
    {.msgID = "btB", .type = TYPE_UINT8, .payload = &btn2_state },
};

eui_parser_state usb_comms; //parser data storage by struct (todo cleanup)


void setup() 
{
  Serial.begin(115200);   //USB CDC instance
  Serial1.begin(115200);  //hardware uart instance
  pinMode(4, OUTPUT);     //set led to output to increase brightness

  //pass parser callback ptr and developer msg array to eUI lib.
  setupParser(&uart_tx_putc); 
  setupDevMsg(dev_msg_store, ARR_ELEM(dev_msg_store));
}

void loop() 
{
  uart_rx_handler();  //check serial rx fifo

  analogWrite(9, led_brightness);  //draw to led
  btn1_state = digitalRead(5); //buttonA on helloboard
  btn2_state = digitalRead(8); //buttonB

  //printDevArray();
  //printTypeTests();
  //loopbackTests();

  delay(250);
  Serial.println(""); //helps keep terminal a bit cleaner during debugging
}

void printDevArray()
{
  for(int i = 0; i < ARR_ELEM(dev_msg_store); i++)
  {
      generatePacket( dev_msg_store[i].msgID, 
                  generateHeader(MSG_DEV, MSG_ACK_NOTREQ, MSG_RES_L, MSG_TYPE_TYP, dev_msg_store[i].type),
                  sizeof(example_uint8), 
                  dev_msg_store[i].payload);
  }
}

// //I use these to test behaviour against myself mostly.
void loopbackTests()
{
  //randomise the brightness of the led variable
  generatePacket( dev_msg_store[0].msgID, 
              generateHeader(MSG_DEV, MSG_ACK_NOTREQ, MSG_RES_L, MSG_TYPE_TYP, dev_msg_store[0].type),
              sizeof(example_uint8), 
              (uint8_t)rand()%255);

  //this is a callback test (the fuction toggles LED5)
  generatePacket( dev_msg_store[1].msgID, 
              generateHeader(MSG_DEV, MSG_ACK_NOTREQ, MSG_RES_L, MSG_TYPE_TYP, dev_msg_store[1].type),
              sizeof(toggleLed),  //this is a bit shit
              dev_msg_store[1].payload);
}

void printTypeTests()
{
  //test single element vars with encoder
  generatePacket( "ui8", 
                  generateHeader(MSG_INTERNAL, MSG_ACK_REQ, MSG_RES_L, MSG_TYPE_TYP, TYPE_UINT8),
                  sizeof(example_uint8), 
                  &example_uint8);
  delay(50);
  generatePacket( "i16", 
                  generateHeader(MSG_INTERNAL, MSG_ACK_REQ, MSG_RES_L, MSG_TYPE_TYP, TYPE_UINT16),
                  sizeof(example_uint16), 
                  &example_uint16);
  delay(50);
  generatePacket( "i32", 
                  generateHeader(MSG_INTERNAL, MSG_ACK_REQ, MSG_RES_L, MSG_TYPE_TYP, TYPE_UINT32),
                  sizeof(example_uint32), 
                  &example_uint32);
  delay(50);
  generatePacket( "fPI", 
                  generateHeader(MSG_INTERNAL, MSG_ACK_REQ, MSG_RES_L, MSG_TYPE_TYP, TYPE_FLOAT),
                  sizeof(example_float), 
                  &example_float);
  delay(200);
  //test multi-element array types with encoder
  generatePacket( "ua8", 
                  generateHeader(MSG_INTERNAL, MSG_ACK_REQ, MSG_RES_L, MSG_TYPE_TYP, TYPE_UINT8),
                  sizeof(example_uint8_arr), 
                  &example_uint8_arr);
  delay(50);
  generatePacket( "ia6", 
                  generateHeader(MSG_INTERNAL, MSG_ACK_REQ, MSG_RES_L, MSG_TYPE_TYP, TYPE_UINT16),
                  sizeof(example_uint16_arr), 
                  &example_uint16_arr);
  delay(50);
  generatePacket( "ia2", 
                  generateHeader(MSG_INTERNAL, MSG_ACK_REQ, MSG_RES_L, MSG_TYPE_TYP, TYPE_UINT32),
                  sizeof(example_uint32_arr), 
                  &example_uint32_arr);
  delay(50);
  generatePacket( "fpA", 
                  generateHeader(MSG_INTERNAL, MSG_ACK_REQ, MSG_RES_L, MSG_TYPE_TYP, TYPE_FLOAT),
                  sizeof(example_float_arr), 
                  &example_float_arr);
}

void uart_rx_handler()
{
  while(Serial.available() > 0)  //while rx fifo has data
  {  
    parsePacket(Serial.read(), &usb_comms);  //eat a byte
  }
}

//helps us pretend what most other microcontrollers use as an output function
void uart_tx_putc(uint8_t data)
{
  Serial1.write(data); //write to hardware uart
  Serial.write(data);  //output on usb cdc virtual com
  //parsePacket(data, &usb_comms); //loopback
}

//handles buffer inputs (unused currently)
void uart_tx_handler(uint8_t *outputBuffer, uint8_t bufLen)
{
  for (uint8_t i = 0; i < bufLen; i++) 
  {
    Serial1.write(outputBuffer[i]); //write to hardware uart
    Serial.write(outputBuffer[i]);  //output on usb cdc virtual com
    //parsePacket(outputBuffer[i], &usb_comms);  //loopback
  }
  Serial.println("\n");
}