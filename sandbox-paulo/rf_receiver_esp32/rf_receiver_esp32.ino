#include <Arduino.h>

// HC-12 configuration
#define HC12_TX_PIN 17
#define HC12_RX_PIN 16
#define HC12_SET_PIN 2

HardwareSerial HC12(1); // Use UART1 for HC-12

const uint16_t HEADER = 0xAA55; // Unique header to identify the packet


struct SensorData {
  uint32_t millis;
  // uint32_t rawPressure;
  float pressureValue;
  float speed;
  float height;
  float accel_x, accel_y, accel_z;
};

// SensorData receivedData;
void receiveSensorData() {
  // Check if enough data is available
  if (HC12.available() >= (sizeof(uint16_t) + sizeof(SensorData))) {
    uint16_t receivedHeader;
    HC12.readBytes((char*)&receivedHeader, sizeof(receivedHeader)); // Read the header

    // Validate the header
    if (receivedHeader != 0xAA55) {
      Serial.println("Invalid header received!");
      return;
    }

    // Read the SensorData structure
    SensorData receivedData;
    HC12.readBytes((char*)&receivedData, sizeof(SensorData));

    // Debug output
    // Serial.print("Received Data:");
    Serial.print(">Millis:"); Serial.println(receivedData.millis);
    Serial.print(">Pressure:"); Serial.println(receivedData.pressureValue);
    Serial.print(">Speed:"); Serial.println(receivedData.speed);
    Serial.print(">Height:"); Serial.println(receivedData.height);
    Serial.print(">Accel X:"); Serial.println(receivedData.accel_x);
    Serial.print(">Accel Y:"); Serial.println(receivedData.accel_y);
    Serial.print(">Accel Z:"); Serial.println(receivedData.accel_z);
  }
}

void setup() {
  Serial.begin(115200);         // For debugging
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

  // Exit HC-12 configuration mode
  digitalWrite(HC12_SET_PIN, HIGH);
  delay(100);

  Serial.println("HC-12 configured and ready.");
}

void loop() {
  // if (HC12.available()) {
  //   String message = HC12.readStringUntil('\n');
  //   Serial.println("Received: " + message);
  //   // Serial.println(String(millis()));
  // }
  receiveSensorData();
}


