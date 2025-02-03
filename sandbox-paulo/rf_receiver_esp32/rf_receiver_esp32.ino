#include <Arduino.h>

// HC-12 configuration
#define HC12_TX_PIN 17
#define HC12_RX_PIN 16
#define HC12_SET_PIN 2

HardwareSerial HC12(1); // Use UART1 for HC-12

const uint16_t HEADER = 0xAA55; // Unique header to identify the packet


struct SensorData {
  uint32_t millis;
  uint32_t rawPressure;
  float pressureValue;
  float accel_x, accel_y, accel_z;
};

// struct SensorData {
//   uint32_t millis;
//   uint32_t loopCount;
//   uint32_t rawPressure;
//   uint32_t rawTemperature;
//   float accel_x, accel_y, accel_z;
//   float gyro_x, gyro_y, gyro_z;
//   float temp_c;
// };


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
    Serial.print(">Raw Pressure:"); Serial.println(receivedData.rawPressure);
    Serial.print(">Pressure Value:"); Serial.println(receivedData.pressureValue);
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




















// #include <Arduino.h> // Required for ESP32 projects

// #define HCSet 2 // HC-12 SET pin
// #define chann 10 // Desired channel

// #define HC12_TXD_PIN 17 // HC-12 TX pin
// #define HC12_RXD_PIN 16 // HC-12 RX pin

// HardwareSerial HCSERIAL(2); // Use UART2 for HC-12

// const int LED_PIN = 2;  // Built-in LED on GPIO2

// bool InitHC12() {
//   long BAUDS[4] = {9600, 38400, 115200, 57600};
//   int i = 0;
//   long BAUD = 0;

//   pinMode(HCSet, OUTPUT);
//   digitalWrite(HCSet, LOW); // Enter configuration mode
//   int status = digitalRead(2);
//   Serial.print("Set pin is: ");
//   Serial.println(status);

//   HCSERIAL.begin(9600, SERIAL_8N1, HC12_RXD_PIN, HC12_TXD_PIN);
//   BAUD = 9600;
//   HCSERIAL.println("AT");

//   // Detect current baud rate
//   Serial.println("Detect current baud rate...");
//   while (!HCSERIAL.find("OK")) {
//     HCSERIAL.end();
//     HCSERIAL.begin(BAUDS[i], SERIAL_8N1, HC12_RXD_PIN, HC12_TXD_PIN);
//     BAUD = BAUDS[i];
//     i++;
//     if (i > 3) i = 0;
//     HCSERIAL.println("AT");
//     Serial.println(HCSERIAL.readString());
//     delay(1000);
//   }
//   delay(200);
//   Serial.print("BAUD: ");
//   Serial.println(BAUD);

//   // Change baud rate to 38400 if not already set
//   if (BAUD != 38400) {
//     Serial.println("Changing Baud Rate");
//     HCSERIAL.println("AT+B38400");
//     while (!HCSERIAL.find("OK+B38400")) {
//       delay(1000);
//       HCSERIAL.begin(BAUD);
//       HCSERIAL.println("AT+B38400");
//       Serial.println("Trying to change Baud Rate");
//     }
//     BAUD = 38400;
//   }
//   Serial.println("Baud rate changed");

//   // Set to maximum power
//   Serial.println("Set to maximum power");
//   HCSERIAL.println("AT+P8");
//   while (!HCSERIAL.find("OK+P8")) {
//     Serial.println("Trying to set to max power");
//     HCSERIAL.println("AT+P8");
//     delay(1000);
//   }
//   Serial.println("Maximum power set up!");

//   // Set channel
//   Serial.println("Set the channel");
//   HCSERIAL.print("AT+C");
//   HCSERIAL.print(chann);
//   HCSERIAL.println();
//   char CNF[] = "OK+C000";
//   CNF[4] = ((chann / 100) % 10) + '0';
//   CNF[5] = ((chann / 10) % 10) + '0';
//   CNF[6] = (chann % 10) + '0';
//   while (!HCSERIAL.find("C10")) {
//     Serial.println("Trying to set the channel");
//     HCSERIAL.print("AT+C");
//     HCSERIAL.print(chann);
//     HCSERIAL.println();
//     String result = HCSERIAL.readString();
//     Serial.println("Result = " + result);
//     Serial.println(CNF);
//     delay(1000);
//   }
//   Serial.println("Channel set up!");

//   // Set FU mode
//   HCSERIAL.println("AT+FU3");
//   while (!HCSERIAL.find("OK+FU3")) {
//     delay(20);
//     HCSERIAL.println("AT+FU3");
//   }

//   HCSERIAL.end();
//   HCSERIAL.begin(38400, SERIAL_8N1, HC12_RXD_PIN, HC12_TXD_PIN);
//   digitalWrite(HCSet, HIGH); // Exit configuration mode
//   delay(500);

//   return true;
// }



// void setup() {
//   Serial.begin(115200); // Debugging
//   Serial.println("Starting HC-12 initialization...");
//   while (!InitHC12()) {
//     Serial.println("Initialization failed. Retrying...");
//   }
//   Serial.println("HC-12 Initialized!");
// }


// void loop() {
//   // Check if data is available from the HC-12
//   if (HCSERIAL.available()) {
//     String message = HCSERIAL.readStringUntil('\n'); // Read the incoming message
//     // String message = HCSERIAL.readString(); // Read the incoming message

//     // Debugging: Print the received message
//     Serial.println("Received: " + message);
//     // digitalWrite(LED_PIN, HIGH);  // Turn LED ON
//     // delay(500);                  // Wait 0.5 seconds
//     // digitalWrite(LED_PIN, LOW);  // Turn LED OFF

//   }
// }





