#include "electricui.h"

//Basic set of standard types
int8_t   example_int8   = -21;
uint8_t  example_uint8  = 21;

int16_t  example_int16  = -321;
uint16_t example_uint16 = 321;

int32_t  example_int32  = -654321;
uint32_t example_uint32 = 654321;

float    example_float  = 3.14159;
double   example_double = 6.14159;

char example_char = "a";

euiMessage_t dev_msg_store[] = 
{
    EUI_INT8(   "si8", example_int8  ),
    EUI_UINT8(  "ui8", example_uint8 ),

    EUI_INT16(  "si16", example_int16  ),
    EUI_UINT16( "ui16", example_uint16 ),
    
    EUI_INT32(  "si32", example_int32  ),
    EUI_UINT32( "ui32", example_uint32 ),

    EUI_FLOAT(  "fsp", example_float  ),
    EUI_DOUBLE( "fdp", example_double ),

    EUI_CHAR( "char", example_char),
};

eui_interface serial_comms;

void setup() 
{
    Serial.begin(115200);

    //eUI setup
    serial_comms.output_char_fnPtr = &tx_putc;
    EUI_TRACK(dev_msg_store);
    setup_identifier("stdtypes", 8);
}

void loop() 
{
    uart_rx_handler();

    delay(1);
}

void uart_rx_handler()
{
    while(Serial.available() > 0)
    {  
        parse_packet(Serial.read(), &serial_comms);
    }
}

void tx_putc(uint8_t data)
{
    Serial.write(data);
}
