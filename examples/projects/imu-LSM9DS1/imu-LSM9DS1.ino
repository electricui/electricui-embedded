#include "electricui.h"

// This demo was conducted using the i2c based LSM9DS1 IMU.
// We used the Sparkfun ESP32 and ESP32 Motion Shield, it's got battery support too!
// Dependant on the Adafruit library https://github.com/adafruit/Adafruit_LSM9DS1
// Their library needs the Adafruit Unified Sensor Library https://github.com/adafruit/Adafruit_Sensor
#include "Wire.h"
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>

Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(); // Create LSM9DS1 object, without arguments it defaults to i2c mode

eui_interface_t serial_comms;

// We use Adafruit's Unified Sensor Library's main structure here as an example
// It contains version, sensor identifier and type, timestamp and a 16-byte payload which uses unions
// The 16-byte payload is 4 floats for x,y,z then a int8 status and 3 reserved int8 padding bytes.
sensors_event_t acc;
sensors_event_t gyr;
sensors_event_t mag;
sensors_event_t internal_temp;

uint8_t sensor_configured = 0;

eui_message_t dev_msg_store[] = 
{
    EUI_CUSTOM_RO(  "status", sensor_configured ),
    EUI_CUSTOM_RO( "acc",  acc.acceleration ),
    EUI_CUSTOM_RO( "gyro", gyr.gyro ),
    EUI_CUSTOM_RO( "mag",  mag.magnetic ),
    EUI_CUSTOM_RO( "temp", internal_temp.temperature ),
};

void setupSensor()
{
  // Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_8G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_16G);
  
  // Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_8GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_12GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_16GAUSS);

  // Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_500DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_2000DPS);
}

void setup() 
{
  Serial.begin(115200);
  
  // Basic eUI setup for serial interface
  serial_comms.output_func = &tx_putc;
  setup_interface(&serial_comms);
  EUI_TRACK(dev_msg_store);
  setup_identifier("imu", 3);

}

void loop() 
{  

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
      setupSensor();
      sensor_configured = 1;  //flag a successful setup
    }
  }

  //pass inbound serial data to ElectricUI
  while(Serial.available() > 0)
  {  
      parse_packet(Serial.read(), &serial_comms);
  }
}

void tx_putc( uint8_t *data, uint16_t len )
{
  Serial.write( data, len );
}