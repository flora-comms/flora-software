#include <SPI.h>
#include <SD.h>

// Define the Chip Select (CS) pin
const int SD_CS_PIN = 8; // Use GPIO 10 or any available GPIO for CS

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for the serial port to connect (optional)
    }

    Serial.println("Initializing SD card using default FSPI pins...");

    // Initialize SD card with the specified CS pin
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD card initialization failed!");
        return;
    }
    Serial.println("SD card initialized successfully.");

    // List files on the SD card
    listFiles(SD.open("/"), 0);
}

void loop() {
    // Empty loop
}

// Function to list files on the SD card
void listFiles(File dir, int numTabs) {
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            break; // Exit the loop if there are no more entries
        }
        // Print tabs for indentation based on the depth of the directory
        for (uint8_t i = 0; i < numTabs; i++) {
            Serial.print('\t');
        }
        Serial.print(entry.name()); // Print the file/directory name
        if (entry.isDirectory()) {
            Serial.println("/"); // Indicate that it is a directory
            listFiles(entry, numTabs + 1); // Recursive call to list files in the directory
        } else {
            Serial.print("\t\t");
            Serial.print(entry.size()); // Print file size
            Serial.println(" bytes"); // Print "bytes" unit
        }
        entry.close(); // Close the file entry to free resources
    }
}

