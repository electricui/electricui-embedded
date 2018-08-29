#include "electricui.h"

// This demo was conducted using the i2c based LSM9DS1 IMU.
// We used the Sparkfun ESP32 and ESP32 Motion Shield, it's got battery support too!
// Dependant on the Sparkfun library https://github.com/sparkfun/SparkFun_LSM9DS1_Arduino_Library
// Reference: https://learn.sparkfun.com/tutorials/lsm9ds1-breakout-hookup-guide#installing-the-arduino-library
#include "Wire.h"
#include "SparkFunLSM9DS1.h"

#define LSM9DS1_M 0x1E // Mag address must be 0x1E, would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG  0x6B // Accel/gyro address must be 0x6B, would be 0x6A if SDO_AG is LOW

LSM9DS1 imu;  // Create an LSM9DS1 object

eui_interface serial_comms;   //create the ElectricUI interface

// Define a 3-axis structure to make data representation easier
typedef struct {
    float x;
    float y;
    float z;
} imu_t;

// Declare a imu_t for the accelerometer, gyro and magnetometer
imu_t accelometer = { 0.0f, 0.0f, 0.0f };
imu_t gyroscope  = { 0.0f, 0.0f, 0.0f };
imu_t magnetometer   = { 0.0f, 0.0f, 0.0f };

float internal_temp = 0.0f;
uint16_t sensor_status = 0;

const euiMessage_t dev_msg_store[] = 
{
    EUI_UINT16( "status", sensor_status ),
    EUI_FLOAT( "temp",  internal_temp ),
    EUI_CUSTOM( "acc",  accelometer ),
    EUI_CUSTOM( "gyro", gyroscope ),
    EUI_CUSTOM( "mag",  magnetometer ),
};

void setupGyro()
{
  // [enabled] turns the gyro on or off.
  imu.settings.gyro.enabled = true;  // Enable the gyro
  // [scale] sets the full-scale range of the gyroscope.
  // scale can be set to either 245, 500, or 2000
  imu.settings.gyro.scale = 245; // Set scale to +/-245dps
  // [sampleRate] sets the output data rate (ODR) of the gyro
  // sampleRate can be set between 1-6
  // 1 = 14.9    4 = 238
  // 2 = 59.5    5 = 476
  // 3 = 119     6 = 952
  imu.settings.gyro.sampleRate = 3; // 59.5Hz ODR
  // [bandwidth] can set the cutoff frequency of the gyro.
  // Allowed values: 0-3. Actual value of cutoff frequency
  // depends on the sample rate. (Datasheet section 7.12)
  imu.settings.gyro.bandwidth = 0;
  // [lowPowerEnable] turns low-power mode on or off.
  imu.settings.gyro.lowPowerEnable = false; // LP mode off
  // [HPFEnable] enables or disables the high-pass filter
  imu.settings.gyro.HPFEnable = true; // HPF disabled
  // [HPFCutoff] sets the HPF cutoff frequency (if enabled)
  // Allowable values are 0-9. Value depends on ODR.
  // (Datasheet section 7.14)
  imu.settings.gyro.HPFCutoff = 1; // HPF cutoff = 4Hz
  // [flipX], [flipY], and [flipZ] are booleans that can
  // automatically switch the positive/negative orientation
  // of the three gyro axes.
  imu.settings.gyro.flipX = false; // Don't flip X
  imu.settings.gyro.flipY = false; // Don't flip Y
  imu.settings.gyro.flipZ = false; // Don't flip Z
}

void setupAccel()
{
  // [enabled] turns the acclerometer on or off.
  imu.settings.accel.enabled = true; // Enable accelerometer
  // [enableX], [enableY], and [enableZ] can turn on or off
  // select axes of the acclerometer.
  imu.settings.accel.enableX = true; // Enable X
  imu.settings.accel.enableY = true; // Enable Y
  imu.settings.accel.enableZ = true; // Enable Z
  // [scale] sets the full-scale range of the accelerometer.
  // accel scale can be 2, 4, 8, or 16
  imu.settings.accel.scale = 8; // Set accel scale to +/-8g.
  // [sampleRate] sets the output data rate (ODR) of the
  // accelerometer. ONLY APPLICABLE WHEN THE GYROSCOPE IS
  // DISABLED! Otherwise accel sample rate = gyro sample rate.
  // accel sample rate can be 1-6
  // 1 = 10 Hz    4 = 238 Hz
  // 2 = 50 Hz    5 = 476 Hz
  // 3 = 119 Hz   6 = 952 Hz
  imu.settings.accel.sampleRate = 1; // Set accel to 10Hz.
  // [bandwidth] sets the anti-aliasing filter bandwidth.
  // Accel cutoff freqeuncy can be any value between -1 - 3. 
  // -1 = bandwidth determined by sample rate
  // 0 = 408 Hz   2 = 105 Hz
  // 1 = 211 Hz   3 = 50 Hz
  imu.settings.accel.bandwidth = 0; // BW = 408Hz
  // [highResEnable] enables or disables high resolution 
  // mode for the acclerometer.
  imu.settings.accel.highResEnable = false; // Disable HR
  // [highResBandwidth] sets the LP cutoff frequency of
  // the accelerometer if it's in high-res mode.
  // can be any value between 0-3
  // LP cutoff is set to a factor of sample rate
  // 0 = ODR/50    2 = ODR/9
  // 1 = ODR/100   3 = ODR/400
  imu.settings.accel.highResBandwidth = 0;  
}

void setupMag()
{
  // [enabled] turns the magnetometer on or off.
  imu.settings.mag.enabled = true; // Enable magnetometer
  // [scale] sets the full-scale range of the magnetometer
  // mag scale can be 4, 8, 12, or 16
  imu.settings.mag.scale = 12; // Set mag scale to +/-12 Gs
  // [sampleRate] sets the output data rate (ODR) of the
  // magnetometer.
  // mag data rate can be 0-7:
  // 0 = 0.625 Hz  4 = 10 Hz
  // 1 = 1.25 Hz   5 = 20 Hz
  // 2 = 2.5 Hz    6 = 40 Hz
  // 3 = 5 Hz      7 = 80 Hz
  imu.settings.mag.sampleRate = 5; // Set OD rate to 20Hz
  // [tempCompensationEnable] enables or disables 
  // temperature compensation of the magnetometer.
  imu.settings.mag.tempCompensationEnable = false;
  // [XYPerformance] sets the x and y-axis performance of the
  // magnetometer to either:
  // 0 = Low power mode      2 = high performance
  // 1 = medium performance  3 = ultra-high performance
  imu.settings.mag.XYPerformance = 3; // Ultra-high perform.
  // [ZPerformance] does the same thing, but only for the z
  imu.settings.mag.ZPerformance = 3; // Ultra-high perform.
  // [lowPowerEnable] enables or disables low power mode in
  // the magnetometer.
  imu.settings.mag.lowPowerEnable = false;
  // [operatingMode] sets the operating mode of the
  // magnetometer. operatingMode can be 0-2:
  // 0 = continuous conversion
  // 1 = single-conversion
  // 2 = power down
  imu.settings.mag.operatingMode = 0; // Continuous mode
}

uint16_t initLSM9DS1()
{
  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;

  setupGyro();
  setupAccel();
  setupMag();
  imu.settings.temp.enabled = true;

  return imu.begin();
}

void setup() 
{
  Serial.begin(115200);
  
  // Basic eUI setup for serial interface
  serial_comms.output_char_fnPtr = &tx_putc;
  setup_dev_msg(dev_msg_store, ARR_ELEM(dev_msg_store));
  setup_identifier("imu", 3);

  // Setup the Motion Shield
  sensor_status = initLSM9DS1();
  
}

void loop() 
{  
  sample_sensor();
  store_readings();

  while(Serial.available() > 0)
  {  
      parse_packet(Serial.read(), &serial_comms);
  }
}

void tx_putc(uint8_t data)
{
    Serial.write(data);
}

void sample_sensor()
{
  // imu.accelAvailable() returns 1 if new accelerometer
  // data is ready to be read. 0 otherwise.
  if (imu.accelAvailable())
  {
    imu.readAccel();
  }
  
  if (imu.gyroAvailable())
  {
    imu.readGyro();
  }
  
  if (imu.magAvailable())
  {
    imu.readMag();
  }
  
  if (imu.tempAvailable())
  {
    imu.readTemp();
  }
}

void store_readings()
{
  //its a bit annoying the Sparkfun library doesn't provide
  //structures for each sensor, so we copy them into our structures.
  accelometer.x = imu.calcAccel(imu.ax);
  accelometer.y = imu.calcAccel(imu.ay);
  accelometer.z = imu.calcAccel(imu.az);

  gyroscope.x = imu.calcGyro(imu.gx);
  gyroscope.y = imu.calcGyro(imu.gy);
  gyroscope.z = imu.calcGyro(imu.gz);

  magnetometer.x = imu.calcMag(imu.mx);
  magnetometer.y = imu.calcMag(imu.my);
  magnetometer.z = imu.calcMag(imu.mz);

  internal_temp = imu.temperature;
}
