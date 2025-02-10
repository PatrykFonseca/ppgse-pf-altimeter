/*
 * Project: ADC Reader for M5Stack ESP32
 * Author: Gabriel Seleme
 * Date: October, 2024
 * Description: Real time reading of a ADC pin value from the M5Stack ESP32
 * Platform: M5Stack Core, PlatformIO, Visual Studio Code
 * Required Libraries:
 * - M5Stack library (https://github.com/m5stack/M5Stack)
 */

#include <M5Stack.h>
#include <Wire.h>
#include <math.h>
#include <SD.h>

const int MPU_Ace_Address = 0x68;
int16_t Acx, Acy, Acz, Tmp, GyX, GyY, GyZ;
int AcXcal = -950, AcYcal = -300, AcZcal = 0, GyXcal = 480, GyYcal = 170, GyZcal = 210, tcal = -1600;
double t, tx, tf, pitch, roll;

File GY_521_Data;

void setup(){
  M5.Lcd.begin();
  Wire.begin();
  Wire.beginTransmission(MPU_Ace_Address); // Start communication with the GY-521.
  Wire.write(0x6B); // This address is the one responsible for Pwr Management.
  Wire.write(0); // Turns off sleep mode of the Pwr Mngmt (default) and awaken the GY-521.
  Wire.endTransmission(true); 
  Serial.begin(9600);
  SD.begin();
  GY_521_Data = SD.open("/GY521_Data.txt", FILE_WRITE);
}

void loop(){
  M5.Lcd.clear(DARKGREEN);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setCursor(0, 0);

  Wire.beginTransmission(MPU_Ace_Address); // Start communication with the GY-521.
  Wire.write(0x3B); // Goes to the address where the High Byte of X Axis Acceleration reading is stored.
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_Ace_Address, 14, true); // Retrieve 14 Bytes starting from the X Axis Acceleration.
  /*
  The bits retrieved are:
  03B = Ace_X_High, 03C = Ace_X_Low, Ace_Y_High = 03D, Ace_Y_Low = 3E, Ace_Z_High = 3F, Ace_Z_Low = 40
  Temp_High = 41, Temp_Low = 42
  Gyro_X_High = 43, Gyro_X_Low = 44; Gyro_Y_High = 45, Gyro_Y_Low = 46, Gyro_Z_High = 47, Gyro_Z_Low = 48;
  */
  Acx = Wire.read() << 8 | Wire.read(); // Combines High and Low bytes of Ace_X and stores it in the Acx variable, same for the next variables.
  Acy = Wire.read() << 8 | Wire.read();
  Acz = Wire.read() << 8 | Wire.read();
  Tmp = Wire.read() << 8 | Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read(); 
  GyZ = Wire.read() << 8 | Wire.read(); 

  if(GY_521_Data){
    GY_521_Data.print("Acx:  "); GY_521_Data.print(Acx);
    GY_521_Data.print("Acy:  "); GY_521_Data.print(Acy);
    GY_521_Data.print("Acz:  "); GY_521_Data.print(Acz);
    GY_521_Data.flush();
  }
  else{
    M5.Lcd.printf("Failed to upload readed data.");
  }

  M5.Lcd.printf("X = ");
  M5.Lcd.printf("%d", Acx + AcXcal);
  M5.Lcd.printf(" Y = ");
  M5.Lcd.printf("%d", Acy + AcYcal);
  M5.Lcd.printf(" Z = ");
  M5.Lcd.printf("%d", Acz + AcZcal);
  M5.Lcd.printf( "Temperature = ");
  M5.Lcd.printf("%d", Tmp);
  M5.Lcd.printf(" Celsius");
  delay(1000);
}