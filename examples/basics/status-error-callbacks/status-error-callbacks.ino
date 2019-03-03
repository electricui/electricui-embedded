/*
 *	Error handling and status returns are provided by the parser function,
 *	catching and handling these error cases is typically not required, but can aid debugging
 *  or allow for custom functionality to sit ontop the existing protocol and library.
 *  
 *  Interface callbacks can be used to handle untracked messages, flag an event while 
 *  tracked data is processed, or inspect the inbound buffer when an error was flagged.
*/

#include "electricui.h"

// Follows the same behaviour as the hello-blink
uint8_t   blink_enable = 1;
uint8_t   led_state    = 0;
uint16_t  glow_time    = 200;
uint32_t  led_timer    = 0;

char device_name[] = "hello-errors";


eui_interface_t comm_interface; 

eui_message_t dev_msg_store[] = 
{
  EUI_UINT8(  "led_blink",  blink_enable ),
  EUI_UINT8(  "led_state",  led_state ),
  EUI_UINT16( "lit_time",   glow_time ),
  EUI_CHAR_ARRAY( "name",   device_name ),
};

void setup() 
{
  Serial.begin( 115200 );
  pinMode( LED_BUILTIN, OUTPUT );

  // Provide Electric UI with the output callback
  comm_interface.output_func = &tx_putc;

  // Provide Electric UI with the interface status callback 
  comm_interface.interface_cb = &eui_callback;

  // Setup as normal
  setup_interface( &comm_interface );
  EUI_TRACK( dev_msg_store );
  setup_identifier( "status", 6 );


  led_timer = millis();
}

void loop() 
{
  serial_rx_handler();

  if( blink_enable )
  {
    if( millis() - led_timer >= glow_time )
    {
      led_state = !led_state;
      led_timer = millis();
    }    
  }

  digitalWrite( LED_BUILTIN, led_state );
}

void serial_rx_handler()
{
  while( Serial.available() > 0 )  
  {  
  	uint8_t inbound_byte = Serial.read();

  	// The inbound data handler returns status flags indicating errors or successes
    eui_errors_t parse_status = parse_packet( inbound_byte, &comm_interface );

    switch( parse_status.parser )
    {
      case EUI_PARSER_IDLE:
      // Waiting for more data to complete a valid message
      break;

      case EUI_PARSER_OK:
        // Parsed a message and everything worked as expected
      break;

      case EUI_PARSER_ERROR:
      // The parser encountered an error or failure, e.g. CRC check fails, invalid packet formats
      break;
    }

    switch( parse_status.action )
    {
      case EUI_ACTION_CALLBACK_ERROR:
      // Couldn't call a developer tracked callback function due to null pointer reference
      break;

      case EUI_ACTION_WRITE_ERROR:
      // Inbound packet has invalid offset data or invalid length which would result in 
      // out-of-bounds data operations. When this occurs, data is not written
      break;

      case EUI_ACTION_TYPE_MISMATCH_ERROR:
      // Data type in the inbound packet doesn't match the internal type
      break;
    }

    if( parse_status.ack )
    {
      // Had an issue sending an ack.

    }

    switch( parse_status.query )
    {
      case EUI_QUERY_SEND_ERROR:
        // Couldn't reply to the query
      break;

      case EUI_QUERY_SEND_OFFSET_ERROR:
        // Couldn't reply to the ranged query
      break;
    }

  }
}
  
void tx_putc( uint8_t *data, uint16_t len )
{
  Serial.write( data, len );
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
      eui_header_t header   = comm_interface.packet.header;
      uint8_t      *name_rx = comm_interface.packet.msgid_in;
      void         *payload = comm_interface.packet.data_in;

      // See if the inbound packet name matches our intended variable
      if( strcmp( name_rx, "test" ) == 0 )
      {
        // Ensure the UI is sending a variable of the right type
        if( header.type == TYPE_UINT16 )
        {
        	// Do something with the inbound payload here

        }
      }
    }
    break;

    case EUI_CB_PARSE_FAIL:
      // Inbound message parsing failed, this callback help while debugging

    break;
  }
}
