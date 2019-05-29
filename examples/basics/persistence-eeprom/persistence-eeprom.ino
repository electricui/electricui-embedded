/* 
 * This example shows how to add persistence to variables using EEPROM.
 * 
 * Only works with microcontrollers which are supported by Arduino's bundled
 * EEPROM library such as 8-bit AVR boards. ARM boards commonly need flash-emulation.
 * 
 * 1. Load data from EEPROM storage into variables at startup.
 * 2. Save variables into EEPROM when the UI requests a save action.
 * 
 * This example builds off the baseline hello-blink flashing light example 
 * where the blink timing and enabled state are preserved across reboots.
 * Electric UI's tracked variables provide useful boilerplate reducing write/read complexity.
*/

#include "electricui.h"
#include <EEPROM.h>

#define EEPROM_BASE_ADDR 	0x00
#define EEPROM_MAGIC_WORD 	0x42

void save_settings();			// Define the callback used to save data
void erase_settings();			// Define the callback for flash erase
uint16_t unique_device_id = 0;	// Persistent UUID useful for device identification


// Same as hello-blink.ino example
uint8_t   blink_enable = 1;
uint8_t   led_state    = 0;
uint16_t  glow_time    = 200;
uint32_t  led_timer    = 0;

char device_name[17]     = "persistent-blink";

// Standard interface and tracked variables
eui_interface_t serial_comms; 
eui_message_t 	dev_msg_store[] = 
{
  EUI_UINT8(  "led_blink",  blink_enable ),
  EUI_UINT8(  "led_state",  led_state ),
  EUI_UINT16( "lit_time",   glow_time ),
  EUI_CHAR_ARRAY("name", 	device_name ),

  // Add a callback the UI can use to request a save
  EUI_FUNC("save", 	save_settings ),
  EUI_FUNC("clear", erase_settings ),

  EUI_UINT16( "uuid",   unique_device_id ),
};


void setup()
{
  Serial.begin( 115200 );
  pinMode( LED_BUILTIN, OUTPUT );

  // Fetch any saved variables from EEPROM
  retrieve_settings();

  serial_comms.output_cb = &tx_putc;
  eui_setup_interface( &serial_comms );
  EUI_TRACK( dev_msg_store );

  //not going to be random for first use
  eui_setup_identifier( (char*)unique_device_id, 2 );	

  led_timer = millis();
}

void loop()
{
  serial_rx_handler();

  if( blink_enable )
  {
    if( millis() - led_timer >= glow_time )
    {
      led_state = !led_state; //invert led state
      led_timer = millis();
    }    
  }

  digitalWrite( LED_BUILTIN, led_state ); //update the LED to match the intended state
}

// Pull the saved settings from eeprom into tracked variables where suitable
void retrieve_settings( void )
{
	uint16_t eeprom_address = EEPROM_BASE_ADDR;

	if( EEPROM.read( eeprom_address ) == EEPROM_MAGIC_WORD )
	{
		eeprom_address++;

		for( uint8_t i = 0; i < ARR_ELEM(dev_msg_store); i++ )
		{
			// we haven't saved immutable data, so don't try and fetch it
			if(    dev_msg_store[i].type != TYPE_CALLBACK 
				&& dev_msg_store[i].type >> 7 != READ_ONLY_FLAG )
			{
				uint8_t *variable_ptr = (uint8_t*)dev_msg_store[i].payload;

				for( uint16_t j = 0; j < dev_msg_store[i].size; j++)
				{
					variable_ptr[j] = EEPROM.read(eeprom_address);
					eeprom_address++;
					Serial.print(variable_ptr[j], HEX);
				}
			}
		}
	}
}

// Save the mutable variable data
void save_settings( void )
{
	uint16_t eeprom_address = EEPROM_BASE_ADDR;

	//write a magic word at the base address which indicates data is written
	EEPROM.write(eeprom_address, 0x42 );
	eeprom_address++;

	//write the payloads into memory from each of the tracked objects in order
	for( uint8_t i = 0; i < ARR_ELEM(dev_msg_store); i++ )
	{
		// we don't want to save immutable data
		if(    dev_msg_store[i].type != TYPE_CALLBACK 
			&& dev_msg_store[i].type >> 7 != READ_ONLY_FLAG )
		{
			uint8_t *variable_ptr = (uint8_t*)dev_msg_store[i].payload;
			for( uint16_t j = 0; j < dev_msg_store[i].size; j++)
			{
				EEPROM.write(eeprom_address, variable_ptr[j] );
				eeprom_address++;
			}
		}
	}
}

void erase_settings( void )
{
	// Zero out the eeprom
	for (int i = 0 ; i < EEPROM.length() ; i++) 
	{
    	EEPROM.write(i, 0);
  	}
}

void serial_rx_handler()
{
  while( Serial.available() > 0 )  
  {  
    eui_parse( Serial.read(), &serial_comms );
  }
}
  
void tx_putc( uint8_t *data, uint16_t len )
{
  Serial.write( data, len );
}
