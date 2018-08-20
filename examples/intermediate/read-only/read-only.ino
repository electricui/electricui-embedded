extern "C"
{
    #include "electricui.h"
};

uint8_t  readonly_uint8  = 42;
uint8_t  writable_uint8  = 22;

euiMessage_t dev_msg_store[] = 
{
    //the RO macros ensure the variable cannot be written over
    EUI_RO_UINT8( "ro", readonly_uint8 ),
    
    EUI_UINT8(    "wr", writable_uint8 ),
};

eui_interface serial_comms;

void setup() 
{
    Serial.begin(115200);

    //eUI setup
    serial_comms.output_char_fnPtr = &tx_putc;
    setup_dev_msg(dev_msg_store, ARR_ELEM(dev_msg_store));
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
