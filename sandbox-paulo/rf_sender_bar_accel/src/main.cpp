#include <Arduino.h>
#include <m5stack.h>
#include <Adafruit_MPU6050.h>
#include "MS5611.h"
#include "MPU6050Handler.h"
#include "AltimeterHandler.h"
#include "AccelerometerHandler.h"
#include "Constants.h"

// HC-12 configuration
#define HC12_TX_PIN 17
#define HC12_RX_PIN 16
#define HC12_SET_PIN 25

HardwareSerial HC12(1); // Use UART1 for HC-12
MS5611 ms5611(0x77);
Adafruit_MPU6050 mpu;

float baselinePressure = 0.0, baselineHeight = 0.0;
float baselineX = 0.0, baselineY = 0.0, baselineZ = 0.0;

int currentLine = 0; // Current line position
const int lineHeight = 20; // Line height in pixels

void printToScreen(String message) {
  if (currentLine >= M5.Lcd.height()) {
    M5.Lcd.clear(); // Clear screen when the bottom is reached
    currentLine = 0; // Reset to the top
  }
  M5.Lcd.setCursor(0, currentLine); // Set cursor to the current line
  M5.Lcd.println(message); // Print the message
  currentLine += lineHeight; // Move to the next line
}

void setup() {
  M5.begin();
  M5.Lcd.clear();
  M5.Lcd.setTextSize(2); // Adjust text size if needed
  M5.Lcd.setCursor(0, 0); // Start at the top
  printToScreen("Initializing...");

  Serial.begin(115200);         // For debugging
  HC12.begin(9600, SERIAL_8N1, HC12_RX_PIN, HC12_TX_PIN);
  HC12.begin(9600, SERIAL_8N1, HC12_RX_PIN, HC12_TX_PIN);
  pinMode(HC12_SET_PIN, OUTPUT);

  // Enter HC-12 configuration mode
  digitalWrite(HC12_SET_PIN, LOW);
  delay(100);

  // Reset HC-12 to default settings
  HC12.println("AT+DEFAULT");
  delay(100);
  HC12.println("AT");
  delay(100);
  HC12.println("AT+RX");
  delay(100);

  while (HC12.available()) {
    String message = HC12.readStringUntil('\n');
    printToScreen("Received: " + message);
  }

  // Exit HC-12 configuration mode
  digitalWrite(HC12_SET_PIN, HIGH);

  printToScreen("HC-12 configured and ready.");
  // Serial.println("HC-12 configured and ready.");

  // Initialize Altimeter
  if (!ms5611.begin()) {
    // Serial.println("MS5611 initialization failed!");
    printToScreen("MS5611 initialization failed!");
    while (1);
  }
  Serial.println("Setting up altimeter");
  printToScreen("Setting up altimeter");
  setupAltimeter(ms5611, baselinePressure, baselineHeight);

  // Initialize Accelerometer
  if (!initializeMPU6050(mpu)) {
    while (1);
  }
  configureMPU6050(mpu);
  // Serial.println("Setting up accelerometer");
  printToScreen("Setting up accelerometer");
  setupAccelerometer(mpu, baselineX, baselineY, baselineZ);
  
  delay(2000);
  M5.Lcd.clear(); // Clear screen when the bottom is reached
}

void loop() {
  HC12.println("Hello World");
  printToScreen("Sent: Hello World");
  // Serial.println("Sent: Hello World"); // Debugging


  // Read raw data from the MS5611 sensor
  uint32_t rawPressure = ms5611.readRawData(0x40); // Read raw pressure
  uint32_t rawTemperature = ms5611.readRawData(0x50); // Read raw temperature

  // Calculate temperature and absolute pressure
  ms5611.calculateTemperature(rawTemperature);
  float pressure = ms5611.calculatePressure(rawPressure, rawTemperature);

  // Calculate relative pressure (difference from baseline)
  float relativePressure = pressure - baselinePressure;

  // Calculate absolute and relative heights
  float absoluteHeight = (T0 / g) * R * log(P0 / pressure); // Absolute height
  float relativeHeight = absoluteHeight - baselineHeight;   // Relative height

  Serial.print("Absolute Pressure: ");
  Serial.print(pressure);
  Serial.println(" mbar");

  Serial.print("Relative Pressure: ");
  Serial.print(relativePressure);
  Serial.println(" mbar");

  Serial.print("Absolute Height: ");
  Serial.print(absoluteHeight);
  Serial.println(" m");

  Serial.print("Relative Height: ");
  Serial.print(relativeHeight);
  Serial.println(" m");
  
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float accelX = a.acceleration.x - baselineX;
  float accelY = a.acceleration.y - baselineY;
  float accelZ = a.acceleration.z - baselineZ;

  /* Print out the values */
  Serial.print("Acceleration X: ");
  Serial.print(accelX);
  Serial.print(", Y: ");
  Serial.print(accelY);
  Serial.print(", Z: ");
  Serial.print(accelZ);
  Serial.println(" m/s^2");

  Serial.println("");



  delay(100); // Send every 2 seconds
}
