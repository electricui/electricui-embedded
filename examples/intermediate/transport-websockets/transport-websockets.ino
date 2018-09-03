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

// Define default network credentials
char * wifi_ssid = "Name";
char * wifi_pass = "password";

uint8_t ws_connected 	= 0;	//state indication
uint8_t ws_port 		= 80;
char 	ws_path[] 		= "ws(s)://255.255.255.255:81";

//example variables
uint8_t   example_uint8   = 21;
uint16_t  example_uint16  = 321;
uint32_t  example_uint32  = 654321;
float     example_float   = 3.14159;
char 	  demo_string[] = "ESP32 Test Board";

euiMessage_t dev_msg_store[] = {
    EUI_UINT8( "wsc", ws_connected),
    EUI_CHAR( "ws", ws_path ),

    EUI_UINT8(  "ui8", example_uint8 ),
    EUI_UINT16( "i16", example_uint16 ),
    EUI_UINT32( "i32", example_uint32 ),
    EUI_FLOAT(  "fPI", example_float ),
    EUI_RO_CHAR( "dst", demo_string ),
};

WiFiMulti WiFiMulti;
WebSocketsServer webSocket = WebSocketsServer(ws_port);

eui_interface usb_comms;
eui_interface ws_comms;

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
                parse_packet(*iter++, &ws_comms);
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

void setup() 
{  
    Serial.begin(115200);

    //eUI setup
    usb_comms.output_char_fnPtr = &cdc_tx_putc;
    ws_comms.output_char_fnPtr = &ws_tx_putc;
    setup_dev_msg(dev_msg_store, ARR_ELEM(dev_msg_store));
    setup_identifier("esp32", 5);

    WiFiMulti.addAP(wifi_ssid, wifi_pass);
}

void loop() 
{
    wifi_handle();

    while(Serial.available() > 0)
    {  
      parse_packet(Serial.read(), &usb_comms);
    }
}

void cdc_tx_putc(uint8_t data)
{
  Serial.write(data);
}

void ws_tx_putc(uint8_t data)
{
  webSocket.broadcastBIN(&data, 1);
}