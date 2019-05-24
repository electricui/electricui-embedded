/*
 * Based on the ESP32 BLE UART example sketch
 * Provides Electric UI interfaces over the main serial link (usually on the usb connector), hardware uart
 * and BLE with a read and write characteristic.
 * 
 * This device is equivalent to the bluefruit-bleuart example sketch, but specifically for the ESP32.
*/

#include "electricui.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Behaves the same way as the hello-blink example, but operates on serial and BLE
uint8_t   blink_enable  = 1;
uint8_t   led_state     = 0;
uint16_t  glow_time     = 200;
uint32_t  led_timer     = 0;

void usb_tx(  uint8_t *data, uint16_t len );
void uart_tx( uint8_t *data, uint16_t len );
void ble_tx(  uint8_t *data, uint16_t len );

eui_message_t dev_msg_store[] = 
{
    EUI_UINT8( "led_blink",  blink_enable ),
    EUI_UINT8( "led_state",  led_state ),
    EUI_UINT16("lit_time",   glow_time ),
};

eui_interface_t transport_methods[] = 
{
    EUI_INTERFACE( &usb_tx ),
    EUI_INTERFACE( &uart_tx ),
    EUI_INTERFACE( &ble_tx ),
};

// UART characteristic (matches the bluefruit-bleuart example sketch)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;

class ble_server_callbacks: public BLEServerCallbacks
{
  void onConnect( BLEServer* pServer ) 
  {
    deviceConnected = true;
  };

  void onDisconnect( BLEServer* pServer )
  {
    deviceConnected = false;
  }
};

class ble_characteristic_callbacks: public BLECharacteristicCallbacks 
{
  void onWrite(BLECharacteristic *pCharacteristic) 
  {
    std::string rxValue = pCharacteristic->getValue();

    if ( rxValue.length() > 0 ) 
    {
      for (int i = 0; i < rxValue.length(); i++)
      {
        parse_packet( (uint8_t)rxValue[i], &transport_methods[2] );
      }
    }
  }
};

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);  

  pinMode( LED_BUILTIN, OUTPUT );

  EUI_LINK( transport_methods );
  EUI_TRACK( dev_msg_store );
  eui_setup_identifier("espBLE-demo", 14);

  // Create the BLE Device and configure the services
  BLEDevice::init("eUI BLESP");
  setup_bluetooth();

  led_timer = millis();
}

void setup_bluetooth()
{
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks( new ble_server_callbacks() );

  // Create the BLE Service
  BLEService *pService = pServer->createService( SERVICE_UUID );

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
                      
  pTxCharacteristic->addDescriptor( new BLE2902() );

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks( new ble_characteristic_callbacks() );

  // Start the service, and start advertising
  pService->start();
  pServer->getAdvertising()->start();
}

void loop()
{
  // Check if any data has come in
  inbound_data_handler();

  if( !deviceConnected )
  {
      delay(250);                         // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising();        // restart advertising
  }

  if( blink_enable )
  {
    // Check if the LED has been on for the configured duration
    if( millis() - led_timer >= glow_time )
    {
      led_state = !led_state; // Invert led state
      led_timer = millis();
    }    
  }

  digitalWrite( LED_BUILTIN, led_state );

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

  // BLE inbound data is handled in the callback, not polled
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
  if( deviceConnected ) 
  {
    pTxCharacteristic->setValue( data, len );
    pTxCharacteristic->notify();
  } 
}