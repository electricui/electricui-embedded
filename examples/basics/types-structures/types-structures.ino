#include "electricui.h"

// Some simple custom types
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} rgb_t;

typedef struct {
    float x;
    float y;
    float z;
} imu_t;

// Initalise some example variables using those custom types
rgb_t example_rgb = { 182, 236, 20 };
imu_t example_imu = { 0.002, 0.003, -9.782 };

// Track these 'custom' variables with ElectricUI
eui_message_t dev_msg_store[] = 
{
    EUI_CUSTOM( "rgb", example_rgb ),
    EUI_CUSTOM( "imu", example_imu ),
};

eui_interface_t serial_comms;

void setup() 
{
    Serial.begin(115200);

    //eUI setup
    serial_comms.output_func = &tx_putc;
    setup_interface(&serial_comms);

    EUI_TRACK(dev_msg_store);
    setup_identifier("structs", 7);
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

void tx_putc( uint8_t *data, uint16_t len )
{
  for( uint16_t i = 0; i < len; i++ )
  {
      Serial.write( data[i] );
  }
}
