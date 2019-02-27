/*
 * This example shows how structures can be used to pack and use structured data more effectively.
 * Demonstrates a simple PWM fan controller with multiple temperature sensors.
 * The emulated temperature sensors are all sampled and sent to the UI in a structure.
 * Fan speed control is achieved with user-configurable curves.
 * Fan control is capable of identifying a stall condition, and restarting. Blips to 100% during start.
*/

#include "electricui.h"

#define FAN_STALL_MAX_RPM       2200
#define FAN_STALL_FAULT_RPM     400
#define FAN_STARTUP_TIME_MS     1000
#define FAN_STALL_WAIT_TIME_MS  3000

#define SIM_TEMPERATURE_BASE_C  30

// Temperature readings with simple metadata about their use/location
typedef struct
{
    char* label;
    float temperature;
} TempSensor_t;

TempSensor_t sensors[] = 
{
    { .label = "Compute",   .temperature = 0 },
    { .label = "Chipset",   .temperature = 0 },
    { .label = "Video",     .temperature = 0 },
    { .label = "Ambient",   .temperature = 0 },
};

// Fan lookup table
typedef struct
{
    uint8_t temperature;
    uint8_t percentage;
} FanCurve_t;

FanCurve_t fan_curve[] =
{
    { .temperature =  0, .percentage =  20 },
    { .temperature = 20, .percentage =  20 },
    { .temperature = 35, .percentage =  45 },
    { .temperature = 45, .percentage =  90 },
    { .temperature = 60, .percentage = 100 },
};

// Fan state machine manages fan behaviour
typedef enum
{
    FAN_STATE_OFF,
    FAN_STATE_STALL_DETECT,
    FAN_STATE_START,
    FAN_STATE_ON,
} FanState_t;

typedef struct
{
    FanState_t state;
    
    uint8_t  output_percentage; //percentage the fan is set to run at
    uint16_t rpm;               // the speed of the fan
    float    *temp_source;      //pointer to the sensor we run on

    uint32_t timer;
} Fan_t;

Fan_t computer_fan;

// User selectable 'active' temperature sensor
// TODO add support
uint8_t selected_source = 0;

// Simulation variables, these wouldn't be needed with real fan hardware
uint8_t user_stall_test = 0;        // set to true to simulate a stall condition
uint8_t fan_pwm_setting = 0;        // track the simulated fan's input dutycycle
float   user_thermal_bias = 0.0f; // mutate the simulated temperature readings

// Track these 'custom' variables with ElectricUI
eui_message_t dev_msg_store[] = 
{
    EUI_CUSTOM_RO( "sense", sensors ),
    EUI_CUSTOM(    "curve", fan_curve ),
    EUI_CUSTOM_RO( "fan",   computer_fan ),
    EUI_UINT8(     "source", selected_source ),

    EUI_UINT8( "stall", user_stall_test ),
    EUI_FLOAT( "heat", user_thermal_bias ),
};

eui_interface_t serial_comms;


void setup()
{
    Serial.begin(115200);
    pinMode( LED_BUILTIN, OUTPUT);

    serial_comms.output_func = &tx_putc;
    setup_interface(&serial_comms);
    EUI_TRACK(dev_msg_store);
    setup_identifier( "structs", 7 );
}

void loop()
{
    while( Serial.available() > 0 )
    {  
        parse_packet( Serial.read(), &serial_comms );
    }

    simulate_read_temp_sensors();               // read the temperature sensors and convert to celsius
    simulate_read_fan_speed( &computer_fan );   // read the fan's speed from hall effect sensor
    
    // allow user to control which temperature sensor is being used for fan control
    // todo allow UI to change this
    computer_fan.temp_source = &sensors[0].temperature;

    // Process the fan's state machine
    fan_control( &computer_fan );

    // Blink the LED to match the fan's behaviour
    if( computer_fan.state == FAN_STATE_ON )
    {
        digitalWrite( LED_BUILTIN, HIGH );
    }
    else
    {
        digitalWrite( LED_BUILTIN, LOW );
    }

}

void tx_putc( uint8_t *data, uint16_t len )
{
  Serial.write( data, len );
}

// Simulate temperature sensors
void simulate_read_temp_sensors( void )
{
    // Add a fake temperature reading to each sensor, and allow the user to manipulate an offset
    for( uint8_t i = 0; i < (( sizeof(sensors) / sizeof(sensors[0])) - 1); i++ )
    {
        //TODO add sinusoidal slow changes to this value
        sensors[i].temperature = (float)SIM_TEMPERATURE_BASE_C - (i*4) + user_thermal_bias;

        // Add (up to) +-0.1degC noise on the reading
        sensors[i].temperature += (float)(random(0, 1000)-500)/1000;
    }
}

// Fan management state machine controls the fan speed
void fan_control( Fan_t *fan )
{
    switch( fan->state )
    {
        case FAN_STATE_OFF:
            //make sure fan is not spinning
            fan->output_percentage    = 0;

            // If new running target speed is established, trigger startup blip
            if( fan_speed_at_temp( *fan->temp_source ) > 0 )
            {
                fan->state = FAN_STATE_START;
                fan->timer = millis();
            }

            break;

        case FAN_STATE_STALL_DETECT:
            // Stop the fan output because we think it's stalled
            fan->output_percentage = 0;

            // Wait for a few seconds before attempting a restart
            // The blockage should climb out of the fan during this waiting period
            if( (millis()- fan->timer) > FAN_STALL_WAIT_TIME_MS )
            {
                fan->state = FAN_STATE_START;
                fan->timer = millis();
            }

            break;

        case FAN_STATE_START:
                // Set output PWM to 100% for a short period to ensure reliable start behaviour
                fan->output_percentage = 100;

                // Wait for a short period of time for the fan to start, then go to normal operation
                if( (millis()- fan->timer) > FAN_STARTUP_TIME_MS )
                {
                    fan->state = FAN_STATE_ON;
                }

            break;

        case FAN_STATE_ON:
            // Calculate desired speed from table based on temperature reading
            uint8_t lookup_duty = fan_speed_at_temp( *fan->temp_source );

            // Speed changed while stil running
            if( lookup_duty != fan->output_percentage )
            {
                fan->output_percentage = lookup_duty;
            }

            // Needs to be turned off
            if( lookup_duty == 0 )
            {
                fan->state = FAN_STATE_OFF;
            }

            // Rotor stop detection
            if( fan->rpm < FAN_STALL_FAULT_RPM )
            {
                //restart the fan
                fan->state = FAN_STATE_STALL_DETECT;
                fan->timer = millis();
            }

            break;
    }

    simulate_set_fan_speed( fan->output_percentage );
}

// Returns the desired fan speed for a given input temp based on user curve
uint8_t fan_speed_at_temp( float temperature )
{
    uint8_t fan_lut_size = ( sizeof(fan_curve) / sizeof(fan_curve[0]) );

    if( fan_curve )
    {
        //protect against out-of-bounds temperature inputs
        if( temperature < fan_curve[0].temperature )
        {
            return fan_curve[0].percentage; //temperature is lower than lowest point in LUT
        }
        else if( temperature > fan_curve[ fan_lut_size - 1 ].temperature )
        {
            return 100; //temperature exceeds max LUT value
        }

        for( uint8_t i = 0; i < (fan_lut_size - 1); i++ )
        {
            //within range between two rows of the LUT
            if(    temperature > fan_curve[ i ].temperature 
                && temperature <= fan_curve[ i + 1 ].temperature )
            {
                //linear interpolation for fan speed between the surrounding rows in LUT
                // TODO use the arduino map() here for better clarity
                return fan_curve[ i ].percentage + ( ((temperature - fan_curve[ i ].temperature)/(fan_curve[ i + 1 ].temperature - fan_curve[ i ].temperature)) * ( fan_curve[ i + 1 ].percentage - fan_curve[ i ].percentage ) );
            }
        }
    }

    return 95;  // should have returned with a valid percentage, fail ON for safety.
}

// Behaves like the output function, if using a real fan the output PWM would be set
void simulate_set_fan_speed( uint8_t speed_percentage )
{
    // Output fan duty cycle, with setpoint percentage limited between 0-100%
    uint8_t percentage = constrain( speed_percentage, 0, 100 );
    uint8_t duty_cycle = map( percentage, 0, 100, 0, 255 );

    // digitalWrite( FAN_PWM_PIN, duty_cycle);
    fan_pwm_setting = duty_cycle;
}

// Simulate fan behaviour. If a real fan was used, measure hall pulses and calculate RPM here
void simulate_read_fan_speed( Fan_t *fan )
{
    // Look at the driven duty cycle and linearly interpolate a fan speed from 0 to max-ish rpm
    // also add some noise to the measurement
    uint16_t fan_speed = map( fan_pwm_setting, 0, 255, 0, ( FAN_STALL_MAX_RPM + 20 ) );
    fan_speed += random( -5, 5);

    // If fan has been 'stalled' by the user, set speed to 0
    if( user_stall_test )
    {
        fan_speed = 0;
    }

    fan->rpm = fan_speed;
}