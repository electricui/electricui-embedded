/* 
 * This example implements a simplistic PID controller with simulated water heating behaviour.
 * 
 * UI control of gains for tuning, along with setpoint control and feedback.
 * Different variable types are used, and with read-only exposure to immutable data.
*/

#include "electricui.h"

#define WATER_SPECIFIC_HEAT  4.186  // c in J/gm K
#define HEATER_RATING_W      800    // Watts
#define WATT_TO_CALORIES     0.2388f
#define WATER_VOLUME_ML      2000   // 2L of water

float simulated_water_c = 21.0f;    // starting simulation temperature
uint32_t prev_water_sim = 0;        // thermal sim timestamp

// PID controller state variables
float control_time_delta;
float last_error;
float error_accumulated;
uint32_t timestamp_previous;

// PID controller gains
int32_t kp = 1100;
int32_t ki = 1;
int32_t kd = 8;

int8_t   target_temperature = 40;
float    measured_temp      = 0;
float    output_effort      = 0;
uint8_t  heater_duty        = 0;
uint16_t heater_power_w     = 0;

eui_interface_t serial_comms;

eui_message_t dev_msg_store[] = 
{
    EUI_INT8( "target_t", target_temperature ),
    
    // User configurable gains for PID controller
    EUI_INT32( "gain_p",  kp ),
    EUI_INT32( "gain_i",  ki ),
    EUI_INT32( "gain_d",  kd ),

    // Data coming back from the system
    EUI_FLOAT_RO(   "water_t",  measured_temp  ),
    EUI_UINT8_RO(   "heater_d", heater_duty  ),
    EUI_UINT16_RO(  "heater_w", heater_power_w  ),
    
    EUI_FLOAT_RO( "effort", output_effort),
};

void setup() 
{
    Serial.begin(115200);

    //eUI setup
    serial_comms.output_func = &tx_putc;
    setup_interface(&serial_comms);

    EUI_TRACK(dev_msg_store);
    setup_identifier("types", 8);
}

void loop() 
{
    uart_rx_handler();

    simulate_water();

    // Measure the water temperature, run the control process, and modify the output
    measured_temp = sample_temperature();
    output_effort = pid_controller( (float)target_temperature, measured_temp );

    if( output_effort > 0 )
    {
        control_heater( output_effort / 10 );   //scaling
    }
    else
    {
        control_heater( 0 );
    }

    delay(5);
}

// Simple PID controller
float pid_controller( float setpoint, float input )
{
   // Time since previous PID calculation
   uint32_t timestamp_now = millis();
   control_time_delta  = (float)(timestamp_now - timestamp_previous);
  
   // Calculate errors
   float error           = setpoint - input;
   error_accumulated    += error * control_time_delta;
   float error_deriv    = (error - last_error) / control_time_delta;
    
   // Maintain state info for next step to calculate derivative error and timing
   last_error = error;
   timestamp_previous = timestamp_now;

   float output_effort =   ( scale_gain(kp) * error             ) 
                         + ( scale_gain(ki) * error_accumulated ) 
                         + ( scale_gain(kd) * error_deriv       );

   return output_effort;
}

float scale_gain( int32_t gain )
{
    return (float)gain/100;
}

// Simulate a temperature sensor in a tank of water (with some noise)
float sample_temperature()
{
    return simulated_water_c + ( (float)(random(0, 10)-5) / 1000 );
}

// Simulate thermal source in a tank of water
void control_heater( uint8_t duty_cycle )
{
    // Assume a heater's dutycycle maps linearly to the power rating
    heater_duty    = duty_cycle;
    heater_power_w = map( duty_cycle, 0, 100, 0, HEATER_RATING_W);
}

// Perform naive simulation of water temperature based on heater and losses
void simulate_water()
{
    // Calculate the amount of energy (in cal) added to the fluid since last sim step
    float energy_in  = heater_power_w * ( millis()-prev_water_sim ) * WATT_TO_CALORIES; 

    // Q = m * c * dT means we use dT = Q / m*c to calculate thermal increase due to heating
    float heater_temp_increase = 0;
    
    if( energy_in > 0.0001 )   //avoid division by zero
    {
        heater_temp_increase = energy_in / (float)( WATER_VOLUME_ML * WATER_SPECIFIC_HEAT );
    }
    
    // Fudge a environmental loss to let the pot cool down naturally
    float environment_losses   = 0.05f;

    simulated_water_c += heater_temp_increase - environment_losses;

    prev_water_sim = millis();   //timestamp current time
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
    Serial.write( data, len );
}