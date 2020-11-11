/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include "electricui.h"
#include <bluefruit.h>

// Set this to the CID provided by BT SIG after certification
#define BT_COMPANY_ID 0xFFFF  // FFFF is the accepted test CID

// BLE Service
BLEDis  bledis;     // Device information
BLEUart bleuart;    // uart over ble

uint8_t   blink_enable  = 1;
uint8_t   led_state     = 0;
uint16_t  glow_time     = 200;
uint32_t  led_timer     = 0;

void usb_tx(  uint8_t *data, uint16_t len );
void uart_tx( uint8_t *data, uint16_t len );
void ble_tx(  uint8_t *data, uint16_t len );

char central_name[32] = { 0 };

eui_message_t dev_msg_store[] = {
  EUI_UINT8( "led_blink",  blink_enable ),
  EUI_UINT8( "led_state",  led_state ),
  EUI_UINT16("lit_time",   glow_time ),
  EUI_UINT32("time",   led_timer ),
  EUI_CHAR_ARRAY( "central", central_name ),
};

eui_interface_t transport_methods[] = {
  EUI_INTERFACE( &usb_tx ),
  EUI_INTERFACE( &uart_tx ),
  EUI_INTERFACE( &ble_tx ),
};

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);  

  pinMode( LED_BUILTIN, OUTPUT );

  EUI_LINK( transport_methods );
  EUI_TRACK( dev_msg_store );
  eui_setup_identifier("bluefruit-demo", 14);

  configure_bluetooth();
  bleuart.begin();

  // Set up and start advertising
  start_advertisements();

  led_timer = millis();
}

void configure_bluetooth( void )
{
  Bluefruit.configPrphBandwidth( BANDWIDTH_MAX );
  Bluefruit.begin();

  // Set max power. -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower( 4 );
  Bluefruit.setName( "eUI BLE" );

  Bluefruit.setConnectCallback( connect_callback );
  Bluefruit.setDisconnectCallback( disconnect_callback );

  // Configure and Start Device Information Service
  bledis.setModel( "BLE Dev Board" );
  bledis.setHardwareRev("hwTest");
  bledis.setSoftwareRev("0.1.1");
  bledis.setManufacturer( "Electric UI Pty Ltd" );
  bledis.begin();
}

void start_advertisements(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags( BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE );
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService( bleuart );
  Bluefruit.Advertising.addName();
  // Bluefruit.Advertising.addAppearance();

  // Demonstrates setting the company data section of the advertisement.
  // This requires the CID to be valid or you will fail BT SIG certification
  uint8_t manf_data[8] = { 0 };
  memcpy( manf_data, (uint8_t*)BT_COMPANY_ID, 2);

  uint8_t manf_payload[] = { 0xDE, 0xAD, 0xBE, 0XEF };
  memcpy( manf_data+2, manf_payload, 4);

  Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, manf_data, sizeof(manf_data));

  // getMcuUniqueID()

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void loop()
{
  // Check if any data has come in
  inbound_data_handler();

  if( blink_enable )
  {
    // Check if the LED has been on for the configured duration
    if( millis() - led_timer >= glow_time )
    {
      led_state = !led_state; // Invert led state
      led_timer = millis();

      if( !digitalRead(13) && Bluefruit.connected() )
      {
        send_tracked_on( "time", &transport_methods[2] );
      }

    }    
  }

  // Make it easier to see when a central is setup for the uart RX
  glow_time = ( bleuart.notifyEnabled() )? 100 : 250;

  digitalWrite( LED_BUILTIN, led_state );

  // Put CPU to sleep until an event occurs
  waitForEvent();
}

void inbound_data_handler()
{
  // USB Serial
  while( Serial.available() > 0 )  
  {  
    eui_parse( Serial.read(), &transport_methods[0] );
  }

  // Hardware Serial
  while( Serial1.available() > 0 )  
  {  
    eui_parse( Serial.read(), &transport_methods[1] );
  }

  // Bluetooth LE "uart"
  while( bleuart.available() )
  {
    eui_parse( (uint8_t) bleuart.read(), &transport_methods[2] );
  } 
}
  
void usb_tx( uint8_t *data, uint16_t len )
{
  Serial.write( data, len ); // Usb serial connection
}

void uart_tx( uint8_t *data, uint16_t len )
{
  Serial1.write( data, len ); // Hardware serial output
}

void ble_tx( uint8_t *data, uint16_t len )
{
  bleuart.write( data, len );  // Bluetooth serial
}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  Bluefruit.Gap.getPeerName(conn_handle, central_name, sizeof(central_name));
}

// callback invoked when central connects
// Reason is a BLE_HCI_STATUS_CODE which can be found at
// https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/cores/nRF5/nordic/softdevice/s140_nrf52_6.1.1_API/include/ble_hci.h
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;
}