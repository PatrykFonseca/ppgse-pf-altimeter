#include <Arduino.h>

// HC-12 configuration
#define HC12_TX_PIN 17
#define HC12_RX_PIN 16
#define HC12_SET_PIN 2

HardwareSerial HC12(1); // Use UART1 for HC-12

const int LED_PIN = 2;  // Built-in LED on GPIO2

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
  if (HC12.available()) {
    String message = HC12.readStringUntil('\n');
    Serial.println("Received: " + message);
    // Serial.println(String(millis()));
    digitalWrite(LED_PIN, HIGH);  // Turn LED ON
    delay(200);                  // Wait 0.5 seconds
    digitalWrite(LED_PIN, LOW);  // Turn LED OFF
  }
}



