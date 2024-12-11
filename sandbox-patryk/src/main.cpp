#include <M5Stack.h>

// Variables to manage user interactions
bool fileSelectionMode = false; // Whether the user is in the file selection menu
String selectedFileName;        // The name of the selected file

SPIClass SPISD(VSPI);// Custom SPI object to use specific pins.

void listFiles(fs::FS &fs, const char *dirname, uint8_t levels) {
    File root = fs.open(dirname);
    if (!root) {
        M5.Lcd.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        M5.Lcd.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            M5.Lcd.print("FILE: ");
            M5.Lcd.println(file.name());
        }
        file = root.openNextFile();
    }
}

void setup() {
    M5.begin();
    

    SPISD.begin(18,19,23,4);// For the SD card: SCK = 18 ; MISO = 19 ; MOSI = 23 , CS = 4;
   
    if (!SD.begin(4, SPISD ,1000000)) { // Freq of 1Mhz, worked on my TFCard
        M5.Lcd.println("Card failed, or not present");
        while (1);
    }
    M5.Lcd.println("TF card initialized.");
}

void openFile(const String &fileName) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("Opening file: ");
    M5.Lcd.println(fileName);

    File file = SD.open(fileName, FILE_READ);
    if (file) {
        M5.Lcd.println("Content:");
        while (file.available()) {
            M5.Lcd.write(file.read());
        }
        file.close();
    } else {
        M5.Lcd.println("Error opening file.");
    }
    delay(3000);
}

void loop() {
    M5.update();

    if (!fileSelectionMode) {
        // Main Menu
        if (M5.BtnA.wasPressed()) {
            M5.Lcd.println("Creating hello.txt");
            File myFile = SD.open("/hello.txt", FILE_WRITE);
            if (myFile) {
                M5.Lcd.println("File created successfully.");
                myFile.close();
            } else {
                M5.Lcd.println("Error creating hello.txt");
            }
        }

        if (M5.BtnB.wasPressed()) {
            M5.Lcd.println("Writing to hello.txt...");
            File myFile = SD.open("/hello.txt", FILE_APPEND);
            if (myFile) {
                myFile.println("Button 2 pushed");
                myFile.close();
                M5.Lcd.println("Write successful.");
            } else {
                M5.Lcd.println("Error opening hello.txt");
            }
        }

        if (M5.BtnC.wasPressed()) {
            M5.Lcd.println("Listing files on SD card...");
            listFiles(SD, "/", 0);
            M5.Lcd.println("Press BtnA to select a file.");
            fileSelectionMode = true;
        }
    } else {
        // File Selection Mode
        if (M5.BtnA.wasPressed()) {
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.println("Enter file name to open:");
            selectedFileName = "/hello.txt"; // Replace this with user input or selection logic
            M5.Lcd.print("Selected: ");
            M5.Lcd.println(selectedFileName);
            M5.Lcd.println("Press BtnA to open or BtnC to cancel.");
        }

        if (M5.BtnA.wasPressed()) {
            openFile(selectedFileName);
            fileSelectionMode = false; // Exit file selection mode
        }

        if (M5.BtnC.wasPressed()) {
            M5.Lcd.println("Operation canceled.");
            fileSelectionMode = false; // Exit file selection mode
        }
    }
}
