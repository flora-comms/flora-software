#include <SPI.h>
#include <SD.h>

#define petal_v_0_0

#ifdef petal_v_0_0
  #define SD_CS   10
  #define SD_SCK  12
  #define SD_MOSI 11
  #define SD_MISO 13
#endif

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for the serial port to connect (optional)
    }

    Serial.println("Initializing SD card using petal pins...");
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

    // Initialize SD card with the specified CS pin
    if (!SD.begin(SD_CS, SPI)) {
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

