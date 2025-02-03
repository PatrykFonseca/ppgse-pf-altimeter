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

// HC-12 configuration
#define HC12_TX_PIN 17
#define HC12_RX_PIN 16
#define HC12_SET_PIN 5

HardwareSerial HC12(1); // Use UART1 for HC-12
MS5611 ms5611(0x77);
Adafruit_MPU6050 mpu;
SPIClass SPISD(VSPI); // Custom SPI object for SD card

const String logFileName = "/log.txt"; // Log file name

float baselinePressure = 0.0, baselineHeight = 0.0;
float baselineX = 0.0, baselineY = 0.0, baselineZ = 0.0;

int currentLine = 0;       // Current line position
const int lineHeight = 20; // Line height in pixels

// Buffers for stats
String buffer1 = "";
String buffer2 = "";
String buffer3 = "";

// Pointers for active and write buffers
String *activeBuffer = &buffer1;
String *writeBuffer = nullptr;

// Constants
const int loopThreshold = 500; // Total number of loops
const int saveThreshold = 100; // Save data every 100 loops
int count = 0;                 // Loop counter

uint32_t systemMillis = 0;

sensors_event_t accel, gyro, temp;
uint32_t rawPressure;    // Read raw pressure
uint32_t rawTemperature; // Read raw temperature
float pressureValue;

struct SensorData
{
  uint32_t millis;      // Timestamp in milliseconds
  uint32_t rawPressure; // Raw pressure
  float pressureValue;
  float accel_x, accel_y, accel_z; // Accelerometer data
};

uint32_t loopCounter = 0;       // Global loop counter
const uint16_t HEADER = 0xAA55; // Unique header to identify the packet

void sendSensorData()
{
  // Increment the loop counter
  loopCounter++;

  // Populate the structure
  SensorData data = {
      systemMillis, // Current timestamp
      rawPressure,
      pressureValue,
      accel.acceleration.x, // Accelerometer X
      accel.acceleration.y, // Accelerometer Y
      accel.acceleration.z, // Accelerometer Z
  };

  // Send the header first
  HC12.write((uint8_t *)&HEADER, sizeof(HEADER));

  // Send the structure as bytes
  HC12.write((uint8_t *)&data, sizeof(SensorData)); // Cast structure to byte array

  *activeBuffer += String(systemMillis) + ";" +
                   String(data.rawPressure) + ";" +
                   String(data.accel_x) + ";" +
                   String(data.accel_y) + ";" +
                   String(data.accel_z) + ";" +
                   String(data.pressureValue) + "\n";
}

void printToScreen(String message)
{
  if (currentLine >= M5.Lcd.height())
  {
    M5.Lcd.clear();  // Clear screen when the bottom is reached
    currentLine = 0; // Reset to the top
  }
  M5.Lcd.setCursor(0, currentLine); // Set cursor to the current line
  M5.Lcd.println(message);          // Print the message
  currentLine += lineHeight;        // Move to the next line
}

// Function to add a log message to the file
void addLogToFile(String *buffer)
{
  File logFile = SD.open("/log.txt", FILE_APPEND);
  if (logFile)
  {
    logFile.print(*buffer); // Write the buffer to the SD card
    logFile.close();
    buffer->clear(); // Clear the buffer after saving
  }
  else
  {
    printToScreen("Error writing to log file.");
  }
}

// Task for saving data to the SD card
void saveDataTask(void *param)
{
  String *bufferToWrite = (String *)param;
  addLogToFile(bufferToWrite);
  vTaskDelete(NULL); // Delete the task after completion
}

void setup()
{
  M5.begin();
  M5.Power.begin();
  M5.Lcd.clear();
  M5.Lcd.setTextSize(2);  // Adjust text size if needed
  M5.Lcd.setCursor(0, 0); // Start at the top
  printToScreen("Initializing...");
  delay(5000);

  Serial.begin(115200); // For debugging

  // Initialize SD card with custom SPI pins
  SPISD.begin(18, 19, 23, 4); // SCK = 18, MISO = 19, MOSI = 23, CS = 4

  if (!SD.begin(4, SPISD, 1000000))
  { // Frequency of 1 MHz
    printToScreen("Card failed, or not present");
    while (1)
      ; // Halt if SD card initialization fails
  }
  printToScreen("TF card initialized.");

  // Create or open the log file
  File logFile = SD.open(logFileName, FILE_WRITE);
  if (logFile)
  {
    logFile.println("Log file created successfully.");
    logFile.close();
    printToScreen("Log file created: " + logFileName);
  }
  else
  {
    printToScreen("Failed to create log file.");
    delay(5000);
  }

  // Initialize Altimeter
  if (!ms5611.begin())
  {
    // Serial.println("MS5611 initialization failed!");
    printToScreen("MS5611 initialization failed!");
    while (1)
      ;
  }
  Serial.println("Setting up altimeter");
  printToScreen("Setting up altimeter");
  setupAltimeter(ms5611, baselinePressure, baselineHeight);
  String promValues = ms5611.getPromValues();
  Serial.println("promValues: " + promValues);

  // Initialize Accelerometer
  if (!initializeMPU6050(mpu))
  {
    while (1)
      ;
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

  while (HC12.available())
  {
    String message = HC12.readStringUntil('\n');
    // printToScreen("Received: " + message);
  }

  // Exit HC-12 configuration mode
  digitalWrite(HC12_SET_PIN, HIGH);

  printToScreen("HC-12 configured and ready.");

  delay(2000);
  printToScreen("Starting");
  delay(500);
}

void loop()
{

  // String millisString = (String)millis();
  systemMillis = millis();

  // Read raw data from the MS5611 sensor
  rawPressure = ms5611.readRawData(0x40);    // Read raw pressure
  rawTemperature = ms5611.readRawData(0x50); // Read raw temperature

  /* Get new sensor events with the readings */
  mpu.getEvent(&accel, &gyro, &temp);

  // Calculate temperature and absolute pressure
  // ms5611.calculateTemperature(rawTemperature);
  pressureValue = ms5611.calculatePressure(rawPressure, rawTemperature);

  // // Calculate relative pressure (difference from baseline)
  // float relativePressure = pressure - baselinePressure;

  // // Calculate absolute and relative heights
  // float absoluteHeight = (T0 / g) * R * log(P0 / pressure); // Absolute height
  // float relativeHeight = absoluteHeight - baselineHeight;   // Relative height

  sendSensorData();
  count++;

  // Every saveThreshold loops, switch buffers and save data
  if (count % saveThreshold == 0)
  {
    if (activeBuffer == &buffer1)
    {
      writeBuffer = &buffer1;
      activeBuffer = &buffer2;
    }
    else if (activeBuffer == &buffer2)
    {
      writeBuffer = &buffer2;
      activeBuffer = &buffer3;
    }
    else if (activeBuffer == &buffer3)
    {
      writeBuffer = &buffer3;
      activeBuffer = &buffer1;
    }

    // Create a task to save the write buffer to the SD card
    xTaskCreatePinnedToCore(saveDataTask, "SaveDataTask", 4096, writeBuffer, 1, NULL, 1);
  }

  // Serial.println(count);

  // Stop after loopThreshold loops
  // if (count >= loopThreshold) {
  //   printToScreen("Completed data collection.");
  //   while (1) {
  //     delay(1000);
  //   } // Halt program
  // }
}
