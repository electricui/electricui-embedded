/*
 * This demo was conducted using the i2c based LSM9DS1 IMU.
 * We used the Sparkfun ESP32 and ESP32 Motion Shield, it's got battery support too!
 * Dependant on the Adafruit library https://github.com/adafruit/Adafruit_LSM9DS1
 * The adafruit library needs "Adafruit Unified Sensor Library" https://github.com/adafruit/Adafruit_Sensor
*/

#include "electricui.h"

#include "Wire.h"
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>

// Create LSM9DS1 object, without arguments it defaults to i2c mode
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

eui_interface_t serial_comms;

// We use Adafruit's Unified Sensor Library's main structure here as an example
// It contains version, sensor identifier and type, timestamp and a 16-byte payload which uses unions
// The 16-byte payload is 4 floats for x,y,z then a int8 status and 3 reserved int8 padding bytes.
sensors_event_t acc;
sensors_event_t gyr;
sensors_event_t mag;
sensors_event_t internal_temp;

// default sensor ranges
uint8_t accelerometer_setting = lsm.LSM9DS1_ACCELRANGE_2G;
uint8_t gyro_setting = lsm.LSM9DS1_GYROSCALE_245DPS;
uint8_t mag_setting = lsm.LSM9DS1_MAGGAIN_4GAUSS;

uint8_t sensor_configured = 0;

uint8_t loop_frequency_hz = 50;

eui_message_t dev_msg_store[] = 
{
    EUI_UINT8_RO( "status", sensor_configured ),
    EUI_CUSTOM_RO( "acc",   acc.acceleration ),
    EUI_CUSTOM_RO( "gyro",  gyr.gyro ),
    EUI_CUSTOM_RO( "mag",   mag.magnetic ),
    EUI_CUSTOM_RO( "temp",  internal_temp.temperature ),

    EUI_UINT8( "acc_range",   accelerometer_setting ),
    EUI_UINT8( "gyro_range",  gyro_setting ),
    EUI_UINT8( "mag_range",   mag_setting ),
    EUI_FUNC( "reconfig",     setup_sensor ),

    EUI_UINT16( "rate", loop_frequency_hz ),
};

void setup_sensor()
{
  // The Adafruit driver has the accelerometer enum ordering from 0b00 to 0b11 in order of
  // 2G, 16G, 4G, 8G. We use the UI configurable setting, but clamp within the driver ranges
  lsm.setupAccel( constrain( accelerometer_setting, lsm.LSM9DS1_ACCELRANGE_2G, lsm.LSM9DS1_ACCELRANGE_8G ) );
  
  // Gyroscope and mag sensors have normal configuration enum ordering
  lsm.setupGyro( constrain( gyro_setting, lsm.LSM9DS1_GYROSCALE_245DPS, lsm.LSM9DS1_GYROSCALE_2000DPS ) );
  lsm.setupMag( constrain( mag_setting, lsm.LSM9DS1_MAGGAIN_4GAUSS, lsm.LSM9DS1_MAGGAIN_16GAUSS ) );
}

void setup() 
{
  Serial.begin( 115200 );
  
  // Basic eUI setup for serial interface
  serial_comms.output_func = &tx_putc;
  setup_interface( &serial_comms );
  EUI_TRACK( dev_msg_store );
  setup_identifier( "imu", 3 );

}

void loop() 
{  
  while( Serial.available() > 0 )
  {  
      parse_packet(Serial.read(), &serial_comms);
  }

  if( sensor_configured )
  {
    lsm.read();
    lsm.getEvent(&acc, &mag, &gyr, &internal_temp); 
  }
  else
  {
    if( !lsm.begin() )
    {
      //the sensor didn't respond, hopefully trying again will help?
      //todo try a more substantial reset process
    }
    else
    {
      setup_sensor();
      sensor_configured = 1;  //flag a successful setup
    }
  }

}

void tx_putc( uint8_t *data, uint16_t len )
{
  Serial.write( data, len );
}