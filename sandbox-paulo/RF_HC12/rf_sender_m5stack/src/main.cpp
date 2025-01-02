#include <Arduino.h>
#include <m5stack.h>

// HC-12 configuration
#define HC12_TX_PIN 17
#define HC12_RX_PIN 16
#define HC12_SET_PIN 25

HardwareSerial HC12(1); // Use UART1 for HC-12

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
  
  delay(2000);
  M5.Lcd.clear(); // Clear screen when the bottom is reached
}

void loop() {
  HC12.println("Hello World");
  printToScreen("Sent: Hello World");
  // Serial.println("Sent: Hello World"); // Debugging
  delay(2000); // Send every 2 seconds
}
