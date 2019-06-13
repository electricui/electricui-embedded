/*
 * This demo was conducted using the i2c based LSM9DS1 IMU.
 * We used the Sparkfun ESP32 and ESP32 Motion Shield, it's got battery support too!
 * Dependant on the Adafruit library https://github.com/adafruit/Adafruit_LSM9DS1
 * The Adafruit library needs "Adafruit Unified Sensor Library" https://github.com/adafruit/Adafruit_Sensor
*/

#include "electricui.h"

#include "Wire.h"
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>

// Create LSM9DS1 object, without arguments it defaults to i2c mode
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

eui_interface_t serial_comms = EUI_INTERFACE( &eui_write_data );

// We use Adafruit's Unified Sensor Library's main structure here as an example
// It contains version, sensor identifier and type, timestamp and a 16-byte payload which uses unions
// The 16-byte payload is 4 floats for x,y,z then a int8 status and 3 reserved int8 padding bytes.
sensors_event_t acc;
sensors_event_t gyr;
sensors_event_t mag;
sensors_event_t internal_temp;

// Gyroscope rate default and allowable range
// The Adafruit driver accelerometer enum ordering goes from 0b00 to 0b11 in order of
// 2G, 16G, 4G, 8G. We clamp within the driver ranges, hence 8G as max (still supports 16G)
uint8_t accelerometer_setting = lsm.LSM9DS1_ACCELRANGE_4G;

const uint8_t acc_setting_min = lsm.LSM9DS1_ACCELRANGE_2G;
const uint8_t acc_setting_max = lsm.LSM9DS1_ACCELRANGE_8G;  //16G max

// Gyroscope rate default and allowable range
uint8_t gyro_setting = lsm.LSM9DS1_GYROSCALE_500DPS;

const uint8_t gyro_setting_min = lsm.LSM9DS1_GYROSCALE_245DPS;
const uint8_t gyro_setting_max = lsm.LSM9DS1_GYROSCALE_2000DPS;

// Magnetometer gain default and allowable range
uint8_t mag_setting = lsm.LSM9DS1_MAGGAIN_8GAUSS;

const uint8_t mag_setting_min = lsm.LSM9DS1_MAGGAIN_4GAUSS;
const uint8_t mag_setting_max = lsm.LSM9DS1_MAGGAIN_16GAUSS;

uint8_t sensor_configured = 0;


void setup_sensor();

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
};

void setup_sensor()
{
  // We want to ensure the UI can't force us to use a value that's invalid, 
  // so we clamp between the defined minimum and maximum gain settings.

  uint8_t clamped_acc_scale = constrain(  accelerometer_setting, 
                                          acc_setting_min,
                                          acc_setting_max );
  
  uint8_t clamped_gyro_scale = constrain( gyro_setting, 
                                          gyro_setting_min, 
                                          gyro_setting_max );

  uint8_t clamped_mag_scale = constrain(  mag_setting, 
                                          mag_setting_min, 
                                          mag_setting_max );
  
  lsm.setupAccel( (Adafruit_LSM9DS1::lsm9ds1AccelRange_t)clamped_acc_scale );
  lsm.setupGyro(  (Adafruit_LSM9DS1::lsm9ds1GyroScale_t)clamped_gyro_scale );
  lsm.setupMag(   (Adafruit_LSM9DS1::lsm9ds1MagGain_t)clamped_mag_scale    );
}

void setup() 
{
  Serial.begin( 115200 );
  
  // Basic eUI setup for serial interface
  eui_setup_interface( &serial_comms );
  EUI_TRACK( dev_msg_store );
  eui_setup_identifier( "imu", 3 );

}

void loop() 
{  
  while( Serial.available() > 0 )
  {  
      eui_parse(Serial.read(), &serial_comms);
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

  delay(1);
}

void eui_write_data( uint8_t *data, uint16_t len )
{
  Serial.write( data, len );
}