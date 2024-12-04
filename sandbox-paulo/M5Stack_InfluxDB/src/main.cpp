#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif

#include <Arduino.h>
#include <M5Stack.h>
#include <Adafruit_MPU6050.h>
#include "MPU6050Handler.h"
#include "AccelerometerHandler.h"

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#include <WiFi.h>
#include <HTTPClient.h>

// WiFi AP SSID
#define WIFI_SSID "PAPRI 2.4"
// WiFi password
#define WIFI_PASSWORD "92034590"

#define INFLUXDB_URL "http://192.168.18.28:8086"
#define INFLUXDB_TOKEN "admin123"
#define INFLUXDB_ORG "docs"
#define INFLUXDB_BUCKET "home"

// Time zone info
#define TZ_INFO "UTC-3"

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Declare Data point
Point sensor("rocket");

const int ADC_PIN = 36;
unsigned long startTime;
unsigned long duration = 1000; // Measure for 1 second
int readCount = 0;

Adafruit_MPU6050 mpu;

float baselinePressure = 0.0, baselineHeight = 0.0;
float baselineX = 0.0, baselineY = 0.0, baselineZ = 0.0;

void setup()
{
  M5.begin();
  Serial.begin(115200);
  delay(1000); // Short delay for setup stability

  // Initialize Accelerometer
  if (!initializeMPU6050(mpu))
  {
    while (1)
      ;
  }
  configureMPU6050(mpu);
  Serial.println("Setting up accelerometer");
  setupAccelerometer(mpu, baselineX, baselineY, baselineZ);

  delay(1000); // Short delay for setup stability

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection())
  {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  }
  else
  {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  sensor.addTag("device", DEVICE);

  startTime = millis(); // Capture the start time
}

void loop()
{
  unsigned long timestamp = millis() - startTime;

  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float accelX = a.acceleration.x - baselineX;
  float accelY = a.acceleration.y - baselineY;
  float accelZ = a.acceleration.z - baselineZ;

  // Clear fields for reusing the point. Tags will remain the same as set above.
  sensor.clearFields();

  // Store measured value into point
  time_t now = time(nullptr); // Get system time in seconds
  // sensor.addField("timestamp", (unsigned long)now * 1000); // Milliseconds
  sensor.addField("timestamp", (unsigned long)timestamp); // Milliseconds
  sensor.addField("accelX", round(accelX * 100) / 100);
  sensor.addField("accelY", round(accelY * 100) / 100);
  sensor.addField("accelZ", round(accelZ * 100) / 100);

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(sensor.toLineProtocol());

  // Write point
  if (!client.writePoint(sensor))
  {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  if (timestamp > 8000) {
    Serial.print("Done!");
    while(1) {
      delay(1000);
    }
  }

  delay(250);

}
