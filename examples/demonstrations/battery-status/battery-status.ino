/* 
 * This example shows how interface callbacks may be used to:
 * 
 * 1. Provide additional information to the UI's connections screen
 *    before a user connects,
 * 2. Demonstrate handing an untracked payload directly from the UI,
 *    in this case, changing the voltage drain rate with "load_adj".
 * 
 * We simulate a battery and charger, where the charger's supply 
 * can be toggled on and off from the UI.
*/

#include "electricui.h"

#define LED_PIN LED_BUILTIN   //typically 13 on many Arduino boards.


#define SIMULATED_LOAD_mV        2  // device load = discharge rate
#define SIMULATED_CHARGE_RATE_mV 8  // battery charge rate
#define CHARGE_START_PERCENT     35  // SoC % where charger should start
#define CHARGE_FINISH_PERCENT    95  // SoC % where the battery should stop charging

#define SIMULATION_RATE_MS      50  //elapsed time between simulation steps (20hz)

// Lets use a lookup table to calculate the lead-acid percentage from battery voltage 
typedef struct {
  const float voltage;
  const float percentage;
} battery_soc_t;

const battery_soc_t lead_acid_table[] = {
  { .voltage = 12.85f, .percentage = 100.0f },
  { .voltage = 12.65f, .percentage =  75.0f },
  { .voltage = 12.35f, .percentage =  50.0f },
  { .voltage = 12.00f, .percentage =  25.0f },
  { .voltage = 11.80f, .percentage =   0.0f },
};

enum charging_state {
  NOT_CHARGING = 0,
  CHARGING,
};

uint16_t system_v_draw_mV = SIMULATED_LOAD_mV;
uint32_t simulation_timer     = 0;


// Variables to track the battery percentage and charging behaviour
float     battery_voltage = 12.4f;
uint8_t   battery_percentage = 0;
uint8_t   battery_charge_status  = 0;

// Expose a mutable 'device is plugged in' state which can be toggled from the UI.
uint8_t   power_available  = 0;

// Track these with Electric UI
eui_message_t dev_msg_store[] = 
{
  EUI_UINT8(  "plugged_in",  power_available ),

  EUI_FLOAT_RO( "bat_voltage",  battery_voltage ),
  EUI_UINT8_RO( "bat_percent",  battery_percentage ),
  EUI_UINT8_RO( "bat_state",    battery_charge_status ),
};

eui_interface_t serial_comms; 

void setup() 
{
  Serial.begin( 115200 );

  // Setup the output and interface handler callbacks
  serial_comms.output_cb = &tx_putc;
  serial_comms.interface_cb = &eui_callback;

  // Generic setup
  eui_setup_interface( &serial_comms );
  EUI_TRACK( dev_msg_store );
  eui_setup_identifier( "dev-callbacks", 13 );

  // User application setup
  pinMode( LED_PIN, OUTPUT );
  simulation_timer = millis();
}

void loop() 
{
  while( Serial.available() > 0 )
  {  
      eui_parse( Serial.read(), &serial_comms );
  }
    
  // Simulate changing battery level periodically
  if( millis() - simulation_timer >= SIMULATION_RATE_MS )
  {
    simulate_device_load();       // the system load draining the battery
    simulate_battery_charger();   // battery charger behaviour

    simulation_timer = millis();  // Reset the sim timer
  }

  // Calculate the state of charge as a percentage
  battery_percentage = calculate_battery_soc( battery_voltage );

  digitalWrite( LED_PIN, battery_charge_status ); // Turn the LED on while charging

  delay(1);
}

void eui_callback( uint8_t message )
{
  switch( message )
  {
    case EUI_CB_TRACKED:
      // UI received a tracked message ID and has completed processing

    break;

    case EUI_CB_UNTRACKED:
    {
      // UI passed in an untracked message ID
      // Grab parts of the inbound packet which are are useful
      eui_header_t header   = serial_comms.packet.header;
      uint8_t      *name_rx = serial_comms.packet.id_in;
      uint8_t      *payload = (uint8_t*)serial_comms.packet.data_in;

      // See if the inbound packet name matches our intended variable
      if( strcmp( (char*)name_rx, "load_adj" ) == 0 )
      {
        // Ensure the UI is sending a variable of the right type
        if( header.type == TYPE_UINT16 )
        {
          // Grab the data using pointer defererencing
          system_v_draw_mV = *(uint16_t*)payload;

          // Grab the data by grabbing both bytes and shifting them together
          // system_v_draw_mV = (payload[0] << 8) | payload[1];
        }
      }
    }
    break;

    case EUI_CB_PARSE_FAIL:
      // Inbound message parsing failed, this callback can aid debugging

    break;
  }
}
  
void tx_putc( uint8_t *data, uint16_t len )
{
  Serial.write( data, len );
}


// Application-Scope Functions

// Calculates percentage charge for a lead acid battery from lookup table
float calculate_battery_soc( float voltage )
{
  float percentage_charge = 0.0f;
  uint8_t lookup_entries = (sizeof(lead_acid_table) / sizeof(lead_acid_table[0]));

  // If the measured voltage is outside the lookup table, clamp it between 0-100%
  if( voltage < lead_acid_table[ lookup_entries - 1 ].voltage )
  {
      percentage_charge = 0.0f;
  }
  else if ( voltage > lead_acid_table[ 0 ].voltage )
  {
      percentage_charge = 100.0f;
  }

  // Linearly interpolate between points in the lookup table
  for( uint8_t i = 0; i < (lookup_entries - 1); i++ )
  {
    // Find the range of values which the voltage sits between
      if( voltage < lead_acid_table[ i ].voltage && voltage >= lead_acid_table[ i + 1 ].voltage )
      {
        // Base percentage for the lower-end lookup voltage
        percentage_charge = lead_acid_table[ i + 1 ].percentage;

        // Interpolation for the difference to the next higher charge/percentage entry
        percentage_charge += ( ( voltage - lead_acid_table[ i + 1 ].voltage ) 
                               /  ( lead_acid_table[ i ].voltage - lead_acid_table[ i + 1 ].voltage ) )
                             * ( lead_acid_table[ i ].percentage - lead_acid_table[ i + 1 ].percentage );
      }
  }

  return percentage_charge;
}

// This simulates a (dumb) load on the system by draining the battery voltage
void simulate_device_load( void )
{
  // Adds random additional draw to naively simulate varying load condition
  // Allowed to draw the battery to 10V if charging not supplied!
  // Device kicks into a low power mode after 11.5V
  if( battery_voltage > 11.5f )
  {
    battery_voltage -= (float)(system_v_draw_mV + random( 1, 6))/1000;
  } 
  else if ( battery_voltage > 10.0f )
  {
    battery_voltage -= (float)(system_v_draw_mV)/1000;
  }
}

// This simulates the effect a charger has - increasing battery voltage
// In practice, a lead acid under charge will have a 'full' voltage while charging
void simulate_battery_charger( void )
{
  // Work out if the device can/should start charging
  if( power_available )
  {
    // Charge controller starts charging the battery
    if( battery_percentage < CHARGE_START_PERCENT )
    {
      battery_charge_status = CHARGING;
    }
    else if( battery_percentage > CHARGE_FINISH_PERCENT )
    {
      battery_charge_status = NOT_CHARGING;
    }
  }
  else
  {
    battery_charge_status = NOT_CHARGING;
  }

  // Add to the battery voltage to simulate charging behaviour
  if( battery_charge_status == CHARGING)
  {
    battery_voltage += (float)SIMULATED_CHARGE_RATE_mV/1000;
  }
}