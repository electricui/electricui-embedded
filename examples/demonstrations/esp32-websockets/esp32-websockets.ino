// This example was written with the ESP8266 and ESP32 as the target hardware.
// Connects to a wifi access point and runs a websockets server as a transport for eUI.
// The ws path is hinted to the UI over the serial connection which ruggedises connection discovery.

// Base wifi libraries from the ESP library pack
#include "WiFi.h"
#include "WiFiMulti.h"
#include "WiFiClientSecure.h"

// Websockets library https://github.com/Links2004/arduinoWebSockets
#include "WebSocketsServer.h"

#include "electricui.h"

#define LED_PIN LED_BUILTIN


// Define default network credentials
char * wifi_ssid = "ssid";
char * wifi_pass = "password";

uint8_t ws_connected 	= 0;	//state indication
uint8_t ws_port 		= 80;
char 	ws_path[] 		= "ws(s)://255.255.255.255:81";

// Simple variables to modify the LED behaviour
uint8_t   blink_enable = 1; //if the blinker should be running
uint8_t   led_state  = 0;   //track if the LED is illuminated
uint16_t  glow_time  = 200; //in milliseconds

// Keep track of when the light turns on or off
uint32_t led_timer = 0;

//example variables
uint8_t   example_uint8   = 21;
uint16_t  example_uint16  = 321;
uint32_t  example_uint32  = 654321;
float     example_float   = 3.141592;
char 	  demo_string[] = "ESP32 Test Board";

eui_message_t dev_msg_store[] = {
    EUI_UINT8( "wsc", ws_connected),
    EUI_CHAR_ARRAY( "ws", ws_path ),

    EUI_UINT8(  "led_blink",  blink_enable ),
    EUI_UINT8(  "led_state",  led_state ),
    EUI_UINT16( "lit_time",   glow_time ),
    
    EUI_UINT8(  "ui8", example_uint8 ),
    EUI_UINT16( "i16", example_uint16 ),
    EUI_UINT32( "i32", example_uint32 ),
    EUI_FLOAT(  "fPI", example_float ),
    EUI_CHAR_RO_ARRAY( "name", demo_string ),
};

WiFiMulti WiFiMulti;
WebSocketsServer webSocket = WebSocketsServer(ws_port);

void tx_putc(uint8_t *data, uint16_t len);
void ws_tx_putc(uint8_t *data, uint16_t len);

eui_interface_t comm_links[] = {
    EUI_INTERFACE(&tx_putc),
    EUI_INTERFACE(&ws_tx_putc),
};

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
    uint8_t * iter = payload;
    uint8_t * end = payload + length;
    
    switch(type) 
    {
        case WStype_DISCONNECTED:
            ws_connected = 2;
            break;
            
        case WStype_CONNECTED:
            ws_connected = 3;
            break;
            
        case WStype_TEXT:
            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
            
        case WStype_BIN:
            while( iter < end )
            {
                parse_packet(*iter++, &comm_links[1]);
            }
            break;
            
        case WStype_ERROR:      
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
        	ws_connected = 4;
        	break;
    }
}

void wifi_handle()
{
    if( WiFiMulti.run() == WL_CONNECTED ) 
    {
    	//we have a wifi connection
    	if(!ws_connected)
    	{
	    	webSocket.begin();
	    	webSocket.onEvent(webSocketEvent);
	    	ws_connected = 1;

		    // The hint is formatted like ws://255.255.255.255:81
		    memset( ws_path, 0, sizeof(ws_path) );	//clear the string first
			snprintf(ws_path, sizeof(ws_path), "ws://%s:%d", WiFi.localIP().toString().c_str(), ws_port);

            glow_time = 200;

			// Using Arduino Strings
		    // String ws_path_string = "ws://" + WiFi.localIP().toString().c_str() + ":" + String(ws_port);
		    // ws_path_string.toCharArray(ws_path, sizeof(ws_path));
    	}
    	else
    	{
    		webSocket.loop();
    	}
    }
    else
    {
        //no connection, try again later
    	ws_connected = 0;
    }
}


void eui_callback( uint8_t message )
{
  switch(message)
  {
    case EUI_CB_TRACKED:
      // UI recieved a tracked message ID and has completed processing

    break;

    case EUI_CB_UNTRACKED:
    {
      // UI passed in an untracked message ID
      // Grab parts of the inbound packet which are are useful
      eui_header_t header   = comm_links[0].packet.header;
      uint8_t      *name_rx = comm_links[0].packet.msgid_in;
      void         *payload = comm_links[0].packet.data_in;

      // See if the inbound packet name matches our intended variable
      if( strcmp( (char *)name_rx, "talk" ) == 0 )
      {
        webSocket.broadcastTXT("hello over websockets");
        glow_time = 50;
      }
    }
    break;

    case EUI_CB_PARSE_FAIL:
    break;
  }
}

void setup() 
{  
    Serial.begin(115200);

    pinMode( LED_BUILTIN, OUTPUT );

    //eUI setup
    comm_links[0].interface_cb = &eui_callback;
    eui_setup_interfaces(comm_links, 2);
    EUI_TRACK(dev_msg_store);
    eui_setup_identifier("esp32", 5);

    WiFiMulti.addAP(wifi_ssid, wifi_pass);

    led_timer = millis();
}

void loop() 
{
  wifi_handle();

  while(Serial.available() > 0)
  {  
    eui_parse(Serial.read(), &comm_links[0]);
  }

  if( blink_enable )
  {
    // Check if the LED has been on for the configured duration
    if( millis() - led_timer >= glow_time )
    {
      led_state = !led_state; //invert led state
      led_timer = millis();
    }    
  }

  digitalWrite( LED_PIN, led_state ); //update the LED to match the intended state
}

void tx_putc( uint8_t *data, uint16_t len )
{
  Serial.write( data, len );
}

void ws_tx_putc( uint8_t *data, uint16_t len )
{
  webSocket.broadcastBIN( data, len);
}