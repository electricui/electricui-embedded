/*
 *	Error handling and status returns are provided by the parser function,
 *	handling these error cases is typically not required, but can aid debugging
 *  or allow for added functionality to the existing library.
 *  
 *  Interface callbacks can be used to handle untracked messages, flag an event while 
 *  tracked data is processed, or inspect the inbound buffer when an error was flagged.
 *
 *  report_error( ... ) demonstrates runtime variable creation for sending arbitrary strings.
 *  Heartbeat tracking demo code is shown inside the callback's EUI_CB_TRACKED section.
*/

#include "electricui.h"

// Follows the same behaviour as the hello-blink
uint8_t   blink_enable = 1;
uint8_t   led_state    = 0;
uint16_t  glow_time    = 200;
uint32_t  led_timer    = 0;

char device_name[] = "hello-callback";

#define HEARTBEAT_EXPECTED_MS 800
uint32_t last_heartbeat = 0;
uint8_t heartbeat_ok_count = 0;

uint32_t heartbeat_dt = 0;

// Use the EUI_INTERFACE_CB macro to include the diagnostics callback 
eui_interface_t serial_interface = EUI_INTERFACE_CB( &serial_write, &eui_callback ); 

eui_message_t tracked_variables[] = 
{
  EUI_UINT16( "led_state",  led_state ),
  EUI_UINT16( "lit_time",   glow_time ),
  EUI_CHAR_ARRAY( "name",   device_name ),
};

void setup() 
{
  Serial.begin( 115200 );
  pinMode( LED_BUILTIN, OUTPUT );

  // Setup as normal
  eui_setup_interface( &serial_interface );
  EUI_TRACK( tracked_variables );
  eui_setup_identifier( "status", 6 );

  led_timer = millis();
}

void loop() 
{
  serial_rx_handler();
  is_connection_ok();

  if( heartbeat_dt < 256 )
  {
    led_state = 255 - heartbeat_dt;
    analogWrite( LED_BUILTIN, led_state );
  }

}

bool is_connection_ok( void )
{
  heartbeat_dt = millis() - last_heartbeat;

  // Check if the most recent heartbeat has timed out
  if( heartbeat_dt > HEARTBEAT_EXPECTED_MS )
  {
      heartbeat_ok_count = 0;
  }
  
  // Connection is considered OK if more than 3 heartbeats have arrived within 
  // the threshold duration
  return ( heartbeat_ok_count > 3 );
}

void serial_rx_handler()
{
  while( Serial.available() > 0 )  
  {  
  	uint8_t inbound_byte = Serial.read();

  	// The inbound data handler returns status flags indicating errors or successes
    eui_errors_t parse_status = eui_parse( inbound_byte, &serial_interface );

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
        report_error("Parser Fail");
      break;
    }

    switch( parse_status.action )
    {
      case EUI_ACTION_CALLBACK_ERROR:
        // Couldn't call a developer tracked callback function due to null pointer reference
        report_error("Callback Fail");
      break;

      case EUI_ACTION_WRITE_ERROR:
      {
        // Inbound packet has invalid offset data or invalid length which would result in 
        // out-of-bounds data operations. When this occurs, data is not written
        eui_header_t header   = serial_interface.packet.header;
        uint8_t      *name_rx = serial_interface.packet.id_in;
        eui_message_t *tracked = find_tracked_object( (char *)name_rx );
        uint8_t size_expected = tracked->size;

        char error_buf[50] = { 0 };
        snprintf(error_buf, 50, "Write Err: got %d, exp %d", header.data_len, size_expected );

        report_error( error_buf );
      }
      break;

      case EUI_ACTION_TYPE_MISMATCH_ERROR:
        // Data type in the inbound packet doesn't match the internal type
        report_error("Type Fail");
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
      report_error("Query Fail");
      break;

      case EUI_QUERY_SEND_OFFSET_ERROR:
      // Couldn't reply to the ranged query
      report_error("Offset Query Fail");
      break;
    }

  }
}

// Send the text to the UI for display to user
// this message is generated on the fly to match the string length
void report_error( char * error_string )
{
  eui_message_t err_message = { .id = "err",
                                .type = TYPE_CHAR,
                                .size = strlen( error_string ),
                              { .data = error_string }       };

  eui_send_untracked( &err_message );
}

// The library calls this callback after valid packet processing
void eui_callback( uint8_t message )
{
  switch(message)
  {
    case EUI_CB_TRACKED:
    {
      // UI received a tracked message ID and has completed processing

      // Watch for the internal heartbeat message to come in
      uint8_t *name_rx = serial_interface.packet.id_in;

      if( strcmp( (char*)name_rx, EUI_INTERNAL_HEARTBEAT ) == 0 )
      {
        if( millis() - last_heartbeat < HEARTBEAT_EXPECTED_MS )
        {
          // Increment the counter, but clamp to 20
          heartbeat_ok_count = min(heartbeat_ok_count++, 20);
        }

        last_heartbeat = millis();
      }

    }
    break;

    case EUI_CB_UNTRACKED:
    {
      // UI passed in an untracked message ID

      // Grab parts of the inbound packet which are are useful
      eui_header_t header   = serial_interface.packet.header;
      uint8_t      *name_rx = serial_interface.packet.id_in;
      void         *payload = serial_interface.packet.data_in;

      // See if the inbound packet name matches our intended variable
      if( strcmp( (char*)name_rx, "test" ) == 0 )
      {
        // Ensure the UI is sending a variable of the right type
        if( header.type == TYPE_UINT16 )
        {
          // Do something with the inbound payload here
          report_error("u16 Test CB");
        }
      }
    }
    break;

    case EUI_CB_PARSE_FAIL:
      // Inbound message parsing failed, this callback help while debugging

    break;
  }
}

// EUI output over serial
void serial_write( uint8_t *data, uint16_t len )
{
  Serial.write( data, len );
}