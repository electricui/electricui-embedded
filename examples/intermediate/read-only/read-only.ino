#include "electricui.h"

uint8_t  readonly_uint8  = 42;
uint8_t  writable_uint8  = 22;

eui_message_t dev_msg_store[] = 
{
    //the RO macros ensure the variable cannot be written over
    EUI_UINT8_RO( "ro", readonly_uint8 ),
    EUI_UINT8(    "wr", writable_uint8 ),
};

eui_interface_t serial_comms;

void setup() 
{
    Serial.begin(115200);

    //eUI setup
    serial_comms.output_func = &tx_putc;
    setup_interface(&serial_comms, 1);

    EUI_TRACK(dev_msg_store);
    setup_identifier("readonly", 8);
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
