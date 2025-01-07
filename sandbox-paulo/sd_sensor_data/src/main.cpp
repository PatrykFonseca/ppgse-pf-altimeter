#include <Arduino.h>
#include <m5stack.h>
#include <Adafruit_MPU6050.h>
#include "MPU6050Handler.h"
#include "AltimeterHandler.h"
#include "AccelerometerHandler.h"
#include "Constants.h"
#include <SD.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

MS5611 ms5611(0x77);
Adafruit_MPU6050 mpu;

SPIClass SPISD(VSPI); // Custom SPI object for SD card
const String logFileName = "/log.txt"; // Log file name

float baselinePressure = 0.0, baselineHeight = 0.0;
float baselineX = 0.0, baselineY = 0.0, baselineZ = 0.0;

int currentLine = 0; // Current line position
const int lineHeight = 20; // Line height in pixels

// Buffers for stats
String buffer1 = "";
String buffer2 = "";
String buffer3 = "";

// Pointers for active and write buffers
String *activeBuffer = &buffer1;
String *writeBuffer = nullptr;

// Constants
const int loopThreshold = 2000;  // Total number of loops
const int saveThreshold = 100;   // Save data every 100 loops
int count = 0;                   // Loop counter

void printToScreen(String message) {
  if (currentLine >= M5.Lcd.height()) {
    M5.Lcd.clear(); // Clear screen when the bottom is reached
    currentLine = 0; // Reset to the top
  }
  M5.Lcd.setCursor(0, currentLine); // Set cursor to the current line
  M5.Lcd.println(message); // Print the message
  currentLine += lineHeight; // Move to the next line
}


// Function to add a log message to the file
void addLogToFile(String *buffer) {
  File logFile = SD.open("/log.txt", FILE_APPEND);
  if (logFile) {
    logFile.print(*buffer); // Write the buffer to the SD card
    logFile.close();
    buffer->clear(); // Clear the buffer after saving
  } else {
    printToScreen("Error writing to log file.");
  }
}


// Task for saving data to the SD card
void saveDataTask(void *param) {
  unsigned long startTime = millis(); // Record the start time
  String *bufferToWrite = (String *)param;
  addLogToFile(bufferToWrite);
  unsigned long endTime = millis(); // Record the end time
  unsigned long elapsedTime = endTime - startTime; // Calculate elapsed time
  String logMessage = "Time to save data to SD card: " + String(elapsedTime) + " ms";
  Serial.println(logMessage); // Log the time taken
  vTaskDelete(NULL); // Delete the task after completion
}


void setup() {
  M5.begin();
  M5.Power.begin();
  M5.Lcd.clear();
  M5.Lcd.setTextSize(2); // Adjust text size if needed
  M5.Lcd.setCursor(0, 0); // Start at the top
  printToScreen("Initializing...");
  delay(5000);

  Serial.begin(115200);         // For debugging

  // Initialize SD card with custom SPI pins
  SPISD.begin(18, 19, 23, 4); // SCK = 18, MISO = 19, MOSI = 23, CS = 4

  if (!SD.begin(4, SPISD, 1000000)) { // Frequency of 1 MHz
    printToScreen("Card failed, or not present");
    while (1); // Halt if SD card initialization fails
  }
  printToScreen("TF card initialized.");

  // Create or open the log file
  File logFile = SD.open(logFileName, FILE_WRITE);
  if (logFile) {
    logFile.println("Log file created successfully.");
    logFile.close();
    printToScreen("Log file created: " + logFileName);
  } else {
    printToScreen("Failed to create log file.");
    while (1); // Halt if log file creation fails
  }

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

  String initialStats = "baselinePressure:" + String(baselinePressure, 4) + 
                        ";baselineHeight:" + String(baselineHeight, 4) + 
                        ";baselineX:" + String(baselineX, 4) + 
                        ";baselineY:" + String(baselineY, 4) + 
                        ";baselineZ:" + String(baselineZ, 4);
  // addLogToFile(initialStats);
  
  delay(2000);
  printToScreen("Starting");
}

void loop() {

  String millisString = (String)millis();

  // Read raw data from the MS5611 sensor
  uint32_t rawPressure = ms5611.readRawData(0x40); // Read raw pressure
  uint32_t rawTemperature = ms5611.readRawData(0x50); // Read raw temperature
  
  /* Get new sensor events with the readings */
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  // Calculate temperature and absolute pressure
  ms5611.calculateTemperature(rawTemperature);
  float pressure = ms5611.calculatePressure(rawPressure, rawTemperature);

  // Calculate relative pressure (difference from baseline)
  float relativePressure = pressure - baselinePressure;

  // Calculate absolute and relative heights
  float absoluteHeight = (T0 / g) * R * log(P0 / pressure); // Absolute height
  float relativeHeight = absoluteHeight - baselineHeight;   // Relative height

  // float accelX = accel.acceleration.x - baselineX;
  // float accelY = accel.acceleration.y - baselineY;
  // float accelZ = accel.acceleration.z - baselineZ;
  float accelX = accel.acceleration.x;
  float accelY = accel.acceleration.y;
  float accelZ = accel.acceleration.z;

  // Convert float to String
  String absoluteHeightStr = String(absoluteHeight, 4); // 2 decimal places
  String relativeHeightStr = String(relativeHeight, 4);
  String accelXStr = String(accelX, 4);
  String accelYStr = String(accelY, 4);
  String accelZStr = String(accelZ, 4);

  String stats = "\nmillis:" + millisString +
                        ";absoluteHeightStr:" + absoluteHeightStr + 
                        ";relativeHeightStr:" + relativeHeightStr + 
                        ";accelXStr:" + accelXStr + 
                        ";accelYStr:" + accelYStr + 
                        ";accelZStr:" + accelZStr;

  // Append stats to the active buffer
  *activeBuffer += stats;
  count++;

  // Every saveThreshold loops, switch buffers and save data
  if (count % saveThreshold == 0) {
    if (activeBuffer == &buffer1) {
      writeBuffer = &buffer1;
      activeBuffer = &buffer2;
    } else if (activeBuffer == &buffer2) {
      writeBuffer = &buffer2;
      activeBuffer = &buffer3;
    } else if (activeBuffer == &buffer3) {
      writeBuffer = &buffer3;
      activeBuffer = &buffer1;
    }

    // Create a task to save the write buffer to the SD card
    xTaskCreatePinnedToCore(saveDataTask, "SaveDataTask", 4096, writeBuffer, 1, NULL, 1);
  }

  // Stop after loopThreshold loops
  if (count >= loopThreshold) {
    printToScreen("Completed data collection.");
    while (1); // Halt program
  }
  Serial.println(count);
}
