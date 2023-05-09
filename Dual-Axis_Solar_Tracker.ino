/*
 * An Algorithm based Dual-Axis Solar Tracker
 * 
 * Able to read date from RTC clock and convert it to day of year and then use it
 * to calculate solar declination angle. THis shuld be done once a day say 7.30 a.m.
 * 
 * Take declination angle and subtract latitude of a place say Nairobi = -1.29 degress,
 * store this value as tilt angle
 * 
 * Servo motor 1 to read tilt angle value and adjust accordingly.
 * 
 * Servo motor 2 to follow sun from East to West. This should be at speed of 15 degrees per hour
 * or rotate after every 10 minutes (15/60) that is 0.25 degrees. The servo motor should reset
 * the panel ready for the next morning.
 * 
 * Incorporate gyroscope to confirm tilt angle of the panel
*/


#include <Wire.h> // library for I2C communication

#include <Servo.h>

#include <DS3232RTC.h> // library for the DS3232 RTC module


// RTC Module pin inputs
// CONNECTIONS:
// DS1302 CLK/SCLK --> 5
// DS1302 DAT/IO --> 4
// DS1302 RST/CE --> 2
// DS1302 VCC --> 3.3v - 5v
// DS1302 GND --> GND
#define RTC_SDA_PIN A4  // Serial Data  Pin (I2C interface)
#define RTC_SCL_PIN A5  // Serial Clock Pin (I2c interface)
#define RTC_RST_PIN 2  // RST/CE Reset


// SERVO MOTOR pin inputs
#define SERVO_PWM_PIN1 9
#define SERVO_PWM_PIN2 10


const float LATITUDE = -1.29;
const float EARTH_TILT = 23.45;
const float TOTAL_DAYS = 360.25;

DS3232RTC rtc;
Servo servo1;
Servo servo2;

float angle_servo1;
float angle_servo2;

float tilt_angle;
float declination_angle;
int day_of_year;


void setup() {

  Serial.begin(115200);        // Starts serial communication in 115200 baud rate.

  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);
  Serial.println();

  // servo motor config setup
  servo1.attach(SERVO_PWM_PIN1, 500, 2500);
  servo2.attach(SERVO_PWM_PIN2, 500, 2500);

  // rtc config setup
  rtc.begin();

  setSyncProvider(rtc.get);  // Library function to get the time from the RTC module.
    
  if(timeStatus() != timeSet) 
      Serial.println("System Time Cannot be Set. Check Connections.");
  else
      Serial.println("System Time is Set.");

  Serial.println(now());
  rtc.set(now());

}

void loop() {

  displayTime();     // Prints the time to serial monitor.
  Serial.println();


  // Adjust motor 1 to tilt angle at around 7am
  if(hour() == 7){
    servo1.write(tilt_angle); 
  }

  // Adjust motor 2 to follow sun
  if(hour() > 7 && hour() < 18){
    // 0.25 degrees per second
    angle_servo2 += 0.25;
    // above value gets rounded off to an integer
    servo2.write(angle_servo2);
  } else{
    angle_servo2 = 0;
    servo2.write(angle_servo2);
  }

  delay(1000);       // 1 second interval between prints.
}


void displayTime()  // Function that prints the time to serial monitor.
{
    
    Serial.print("=> ");
    Serial.print(day());
    Serial.print("/");
    Serial.print(month());
    Serial.print("/");
    Serial.print(year());
    Serial.print(" ");
    Serial.print(hour());
    displayDigits(minute());
    displayDigits(second());

    day_of_year = getDayOfYear(day(), month(), year());
    
    declination_angle = getDeclinationAngleFromDOY(day_of_year);

    tilt_angle = getTiltAngle();

    Serial.print(" - Day ");
    Serial.print(day_of_year);
    Serial.print(" of Year ");
    Serial.print(year());
    Serial.print(" => Declination Angle = ");
    Serial.print(declination_angle);
    Serial.print(" => Tilt angle = ");
    Serial.print(tilt_angle);
}


void displayDigits(int digits)    // Function that prints preceding colon and leading 0.
{
    Serial.print(':');
    if(digits < 10) 
        Serial.print('0');
    Serial.print(digits);
}


int getDayOfYear(int day, int month, int year) {
  int daysInMonth[] = {0,31,59,90,120,151,181,212,243,273,304,334};
  int leapDays = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0 ? 1 : 0;
  return daysInMonth[month - 1] + day + leapDays;
}


float getDeclinationAngleFromDOY(int DOY){
  return -EARTH_TILT * cos( ( (2 * PI) /TOTAL_DAYS) * (DOY + 10) );
}


float getTiltAngle(){
  return declination_angle - LATITUDE;
}
