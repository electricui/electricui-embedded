/*
 * Example garage door opener
 * Provides simplistic example of using Electric UI with simple statemachines
 * Gives the UI the state of the door, controls to open or close, and progress indicators
*/

#include "electricui.h"

#define MECHANISM_POS_OPEN 		580
#define MECHANISM_POS_CLOSED 	40
#define MOTOR_STEP_SPEED 		1

typedef enum
{
    DOOR_UNKNOWN = 0,
    DOOR_OPENING,
    DOOR_OPEN,
    DOOR_CLOSING,
    DOOR_CLOSED,
} DoorState_t;

DoorState_t control_state;
DoorState_t request_state;

typedef enum
{
    MOTOR_STOP = 0,
    MOTOR_CW,
    MOTOR_CCW,
} MotorDir_t;

int8_t motor_speed;


// Limit switches on mechanism
uint8_t limit_sw_open;
uint8_t limit_sw_closed;

uint16_t mechanism_position_sim = 200;

// Electric UI
eui_interface_t serial_comms;

eui_message_t dev_msg_store[] = 
{
	EUI_UINT8(    "request", 	request_state   ),
	EUI_UINT8_RO( "state", 		control_state   ),

	EUI_UINT8_RO( "sw_up", 		limit_sw_open   ),
	EUI_UINT8_RO( "sw_down", 	limit_sw_closed ),
	EUI_UINT8_RO( "motor", 		motor_speed     ),

	EUI_UINT16_RO( "position", 	mechanism_position_sim ),

};


void setup() 
{
    Serial.begin(115200);

    //eUI setup
    serial_comms.output_cb = &tx_putc;
    eui_setup_interface(&serial_comms);
    EUI_TRACK(dev_msg_store);
    eui_setup_identifier("door", 8);
}

void loop() 
{
    while( Serial.available() > 0 )
    {  
        eui_parse( Serial.read(), &serial_comms );
    }

	simulate_door_mechanism();

	door_controller();

	delay( 10 );	//crudely limit the loop rate to 100hz
}

void door_controller( void )
{
	switch( control_state )
	{
		case DOOR_UNKNOWN:
			// Door is in an unknown state, recover to the 'failsave' state
			drive_motor( MOTOR_STOP );
			control_state = DOOR_CLOSING;
		break;

		case DOOR_OPENING:

			drive_motor( MOTOR_CW );

			if( is_door_open() )
			{
				control_state = DOOR_OPEN;
			}

			if( request_state == DOOR_CLOSED )
			{
				control_state = DOOR_CLOSING;
			}
		break;

		case DOOR_OPEN:

			drive_motor( MOTOR_STOP );

			if( request_state == DOOR_CLOSED )
			{
				control_state = DOOR_CLOSING;
			}
		break;

		case DOOR_CLOSING:

			drive_motor( MOTOR_CCW );

			if( is_door_closed() )
			{
				control_state = DOOR_CLOSED;
			}

			if( request_state == DOOR_OPEN )
			{
				control_state = DOOR_OPENING;
			}
		break;

		case DOOR_CLOSED:

			drive_motor( MOTOR_STOP );

			if( request_state == DOOR_OPEN )
			{
				control_state = DOOR_OPENING;
			}
		break;

		default:
			// Handle invalid state errors here
		break;
	}
}

void drive_motor( MotorDir_t direction )
{
	if( direction == MOTOR_CW )
	{
		motor_speed = MOTOR_STEP_SPEED;
	}
	else if( direction == MOTOR_CCW )
	{
		motor_speed = -1 * MOTOR_STEP_SPEED;
	}
	else // stop the motor (includes MOTOR_STOP in this same state)
	{
		motor_speed = 0;
	}
}

// Return the limit switch state (active or not).
// Commented line shows how a simple switch would be read as part of this check
bool is_door_open()
{
	// limit_sw_open = digitalRead( SWITCH_1_PIN ); 
	return limit_sw_open;
}

bool is_door_closed()
{
	// limit_sw_closed = digitalRead( SWITCH_2_PIN ); 
	return limit_sw_closed;
}

void simulate_door_mechanism()
{
	mechanism_position_sim += motor_speed;

	// Simulate the limit switches indicating if the mechanism is at the end of travel
	limit_sw_open   = ( mechanism_position_sim > MECHANISM_POS_OPEN   );
	limit_sw_closed = ( mechanism_position_sim < MECHANISM_POS_CLOSED );
}

void tx_putc( uint8_t *data, uint16_t len )
{
    Serial.write( data, len );
}
