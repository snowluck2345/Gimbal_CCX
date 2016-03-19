#include <DynamixelSerial.h>
#include "I2Cdev.h"
#include "MPU9250.h"
#include "String.h"
#include "stdio.h"
#include "Wire.h"

//example message: nn,1234,1234,1234,qq
//example message: nn,1234,1234,1234,1234,1,1,1,1,1,1,qq


int Temperature[2], Voltage[2], Position[2], Velocity[2], Load[2], goalPosition[2], moveVelocity[2], camera1[3] = {0}, camera2[3] = {0};


//--------------------------------------------------
//IMU STUFF BEGINS
//--------------------------------------------------

MPU9250 accelgyro;
I2Cdev   I2C_M;

uint8_t buffer_m[6];


int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t   mx, my, mz;



float heading;
float tiltheading;

float Axyz[3];
float Gxyz[3];
float Mxyz[3];


#define sample_num_mdate  5000

volatile float mx_sample[3];
volatile float my_sample[3];
volatile float mz_sample[3];

static float mx_centre = 0;
static float my_centre = 0;
static float mz_centre = 0;

volatile int mx_max = 0;
volatile int my_max = 0;
volatile int mz_max = 0;

volatile int mx_min = 0;
volatile int my_min = 0;
volatile int mz_min = 0;

char response[50];

//--------------------------------------------------
//IMU STUFF ENDS
//--------------------------------------------------

char incomingByte[37] = {0};   // for incoming serial data
char *p, *i;

void parseString(void);
void fetchDynamixelInfo(void);
void returnDynamixelInfo(void);



void setup() {
  
  Wire.begin();
  accelgyro.initialize();
  delay(1000);

  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  
  goalPosition[0] = 0;
  goalPosition[1] = 0;
  moveVelocity[0] = 0;
  moveVelocity[1] = 0;
  Temperature[0] = 0;
  Temperature[1] = 0;

  Serial.begin(57600);

}

void loop() {

  //I toggle the pin digital pin 2 to make my buffers not screw up my serial communication with the computer
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  if (Serial.available() > 36)
  {
    
    getAccel_Data();
    getGyro_Data();
    getCompassDate_calibrated(); // compass data has been calibrated here
    getHeading();				//before we use this function we should run 'getCompassDate_calibrated()' frist, so that we can get calibrated data ,then we can get correct angle .
    getTiltHeading();

    // read the incoming byte:
    for (int i = 0; i < 37; i++)
    {
      incomingByte[i] = Serial.read();
    }

    digitalWrite(2, LOW);

    String stringOne = String(incomingByte);
    
    parseString();
    Serial.end();

    fetchDynamixelInfo();
    returnDynamixelInfo();

    Serial.begin(57600);

    Serial.print("nn,");
    Serial.print(mx_centre);
    Serial.print(",");
    Serial.print(my_centre);
    Serial.print(",");
    Serial.print(mz_centre);
    Serial.print(",");
    Serial.print(Axyz[0]);
    Serial.print(",");
    Serial.print(Axyz[1]);
    Serial.print(",");
    Serial.print(Axyz[2]);
    Serial.print(",");
    Serial.print(Gxyz[0]);
    Serial.print(",");
    Serial.print(Gxyz[1]);
    Serial.print(",");
    Serial.print(Gxyz[2]); 
    Serial.print(",");
    Serial.print(Mxyz[0]);
    Serial.print(",");
    Serial.print(Mxyz[1]);
    Serial.print(",");
    Serial.print(Mxyz[2]);
    Serial.print(",");
    Serial.print(goalPosition[0]);
    Serial.print(",");
    Serial.print(moveVelocity[0]);
    Serial.print(",");
    Serial.print(goalPosition[1]);
    Serial.print(",");
    Serial.print(moveVelocity[1]);
    Serial.print(",");
    Serial.print(Temperature[0]);
    Serial.print(",");
    Serial.print(Voltage[0]);
    Serial.print(",");
    Serial.print(Position[0]);
    Serial.print(",");
    Serial.print(Velocity[0]);
    Serial.print(",");
    Serial.print(Load[0]);
    Serial.print(",");
    Serial.print(Temperature[1]);
    Serial.print(",");
    Serial.print(Voltage[1]);
    Serial.print(",");
    Serial.print(Position[1]);
    Serial.print(",");
    Serial.print(Velocity[1]);
    Serial.print(",");
    Serial.print(Load[1]);
    Serial.print(",");
    Serial.print("qq");

    
    digitalWrite(3, camera1[0]);
    digitalWrite(4, camera1[1]);
    digitalWrite(5, camera1[2]);
    digitalWrite(6, camera2[0]);
    digitalWrite(7, camera2[1]);
    digitalWrite(8, camera2[2]);

    

  }








}

void parseString(void)
{
  //  First strtok iteration
  p = strtok_r(incomingByte, ",", &i);
  //Serial.println(p);

  //  Second strtok iteration
  p = strtok_r(NULL, ",", &i);
  //Serial.print(p);
  //Serial.println("");

  goalPosition[0] = atoi(p);

  //  Third strtok iteration
  p = strtok_r(NULL, ",", &i);
  //Serial.print(p);
  //Serial.println("");

  moveVelocity[0] = atoi(p);

  //  Fourth strtok iteration
  p = strtok_r(NULL, ",", &i);
  //Serial.print(p);
  //Serial.println("");

  goalPosition[1] = atoi(p);

  //  Fifth strtok iteration
  p = strtok_r(NULL, ",", &i);
  //Serial.print(p);
  //Serial.println("");

  moveVelocity[1] = atoi(p);

  //  Sixth strtok iteration
  p = strtok_r(NULL, ",", &i);
  camera1[0] = atoi(p);
   // Serial.println(p);

  
  //seventh
  p = strtok_r(NULL, ",", &i);
  camera1[1] = atoi(p);
   // Serial.println(p);

  //eighth
  p = strtok_r(NULL, ",", &i);
  camera1[2] = atoi(p);
   // Serial.println(p);

  //ninth
  p = strtok_r(NULL, ",", &i);
  camera2[0] = atoi(p);
   // Serial.println(p);

  //tenth
  p = strtok_r(NULL, ",", &i);
  camera2[1] = atoi(p);
   // Serial.println(p);

  //eleventh
  p = strtok_r(NULL, ",", &i);
  camera2[2] = atoi(p);
 // Serial.println(p);
  //Serial.println("");
}

void fetchDynamixelInfo(void) {
  Dynamixel.begin(1000000, 2);        // Begin Servo Comunication
  delay(50);

  Temperature[0] = Dynamixel.readTemperature(1); // Request and Print the Temperature
  Voltage[0] = Dynamixel.readVoltage(1);         // Request and Print the Voltage
  Position[0] = Dynamixel.readPosition(1);       // Request and Print the Position
  Velocity[0] = Dynamixel.readSpeed(1);
  Load[0] = Dynamixel.readLoad(1);


  Temperature[1] = Dynamixel.readTemperature(2); // Request and Print the Temperature
  Voltage[1] = Dynamixel.readVoltage(2);         // Request and Print the Voltage
  Position[1] = Dynamixel.readPosition(2);       // Request and Print the Position
  Velocity[1] = Dynamixel.readSpeed(2);
  Load[1] = Dynamixel.readLoad(2);


  Dynamixel.moveSpeed(1, goalPosition[0], moveVelocity[0]); // Move the Servo radomly from 200 to 800
  Dynamixel.moveSpeed(2, goalPosition[1], moveVelocity[1]); // Move the Servo radomly from 200 to 800

  delay(50);
  Dynamixel.end();                 // End Servo Comunication
  delay(50);
}

void returnDynamixelInfo(void)
{
  Serial.begin(9600);              // Begin Serial Comunication
  delay(50);

  Serial.print("nn,");   // Print the variables in the Serial Monitor
  Serial.print(Voltage[0]);
  Serial.print(",");
  Serial.print(Temperature[0]);
  Serial.print(",");
  Serial.print(Position[0]);
  Serial.print(",");
  Serial.print(Velocity[0]);
  Serial.print(",");
  Serial.print(Load[0]);

  Serial.print(",pp,");

  Serial.print(Voltage[1]);
  Serial.print(",");
  Serial.print(Temperature[1]);
  Serial.print(",");
  Serial.print(Position[1]);
  Serial.print(",");
  Serial.print(Velocity[1]);
  Serial.print(",");
  Serial.print(Load[1]);
  Serial.print(",qq");

  Serial.end();                     // End the Serial Comunication
  delay(50);
}


void getHeading(void)
{
  heading = 180 * atan2(Mxyz[1], Mxyz[0]) / PI;
  if (heading < 0) heading += 360;
}

void getTiltHeading(void)
{
  float pitch = asin(-Axyz[0]);
  float roll = asin(Axyz[1] / cos(pitch));

  float xh = Mxyz[0] * cos(pitch) + Mxyz[2] * sin(pitch);
  float yh = Mxyz[0] * sin(roll) * sin(pitch) + Mxyz[1] * cos(roll) - Mxyz[2] * sin(roll) * cos(pitch);
  float zh = -Mxyz[0] * cos(roll) * sin(pitch) + Mxyz[1] * sin(roll) + Mxyz[2] * cos(roll) * cos(pitch);
  tiltheading = 180 * atan2(yh, xh) / PI;
  if (yh < 0)    tiltheading += 360;
}



void Mxyz_init_calibrated ()
{

  Serial.println(F("Before using 9DOF,we need to calibrate the compass frist,It will takes about 2 minutes."));
  Serial.print("  ");
  Serial.println(F("During  calibratting ,you should rotate and turn the 9DOF all the time within 2 minutes."));
  Serial.print("  ");
  Serial.println(F("If you are ready ,please sent a command data 'ready' to start sample and calibrate."));
  while (!Serial.find("ready"));
  Serial.println("  ");
  Serial.println("ready");
  Serial.println("Sample starting......");
  Serial.println("waiting ......");

  get_calibration_Data ();

  Serial.println("     ");
  Serial.println("compass calibration parameter ");
  Serial.print(mx_centre);
  Serial.print("     ");
  Serial.print(my_centre);
  Serial.print("     ");
  Serial.println(mz_centre);
  Serial.println("    ");
}


void get_calibration_Data ()
{
  for (int i = 0; i < sample_num_mdate; i++)
  {
    get_one_sample_date_mxyz();
    /*
    Serial.print(mx_sample[2]);
    Serial.print(" ");
    Serial.print(my_sample[2]);                            //you can see the sample data here .
    Serial.print(" ");
    Serial.println(mz_sample[2]);
    */



    if (mx_sample[2] >= mx_sample[1])mx_sample[1] = mx_sample[2];
    if (my_sample[2] >= my_sample[1])my_sample[1] = my_sample[2]; //find max value
    if (mz_sample[2] >= mz_sample[1])mz_sample[1] = mz_sample[2];

    if (mx_sample[2] <= mx_sample[0])mx_sample[0] = mx_sample[2];
    if (my_sample[2] <= my_sample[0])my_sample[0] = my_sample[2]; //find min value
    if (mz_sample[2] <= mz_sample[0])mz_sample[0] = mz_sample[2];

  }

  mx_max = mx_sample[1];
  my_max = my_sample[1];
  mz_max = mz_sample[1];

  mx_min = mx_sample[0];
  my_min = my_sample[0];
  mz_min = mz_sample[0];



  mx_centre = (mx_max + mx_min) / 2;
  my_centre = (my_max + my_min) / 2;
  mz_centre = (mz_max + mz_min) / 2;

}






void get_one_sample_date_mxyz()
{
  getCompass_Data();
  mx_sample[2] = Mxyz[0];
  my_sample[2] = Mxyz[1];
  mz_sample[2] = Mxyz[2];
}


void getAccel_Data(void)
{
  accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
  Axyz[0] = (double) ax / 16384;//16384  LSB/g
  Axyz[1] = (double) ay / 16384;
  Axyz[2] = (double) az / 16384;
}

void getGyro_Data(void)
{
  accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
  Gxyz[0] = (double) gx * 250 / 32768;//131 LSB(��/s)
  Gxyz[1] = (double) gy * 250 / 32768;
  Gxyz[2] = (double) gz * 250 / 32768;
}

void getCompass_Data(void)
{
  I2C_M.writeByte(MPU9150_RA_MAG_ADDRESS, 0x0A, 0x01); //enable the magnetometer
  delay(10);
  I2C_M.readBytes(MPU9150_RA_MAG_ADDRESS, MPU9150_RA_MAG_XOUT_L, 6, buffer_m);

  mx = ((int16_t)(buffer_m[1]) << 8) | buffer_m[0] ;
  my = ((int16_t)(buffer_m[3]) << 8) | buffer_m[2] ;
  mz = ((int16_t)(buffer_m[5]) << 8) | buffer_m[4] ;

  //Mxyz[0] = (double) mx * 1200 / 4096;
  //Mxyz[1] = (double) my * 1200 / 4096;
  //Mxyz[2] = (double) mz * 1200 / 4096;
  Mxyz[0] = (double) mx * 4800 / 8192;
  Mxyz[1] = (double) my * 4800 / 8192;
  Mxyz[2] = (double) mz * 4800 / 8192;
}

void getCompassDate_calibrated ()
{
  getCompass_Data();
  Mxyz[0] = Mxyz[0] - mx_centre;
  Mxyz[1] = Mxyz[1] - my_centre;
  Mxyz[2] = Mxyz[2] - mz_centre;
}

