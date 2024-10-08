#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SD.h>  
#include <esp_task_wdt.h>
#include <ESPmDNS.h>

const char *ssid = "AVAlink";

// SD card CS (Chip Select) pin
#define SD_CS_PIN 13

#define REASSIGN_PINS
int sck = 14;
int miso = 2;
int mosi = 11;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup() {
    Serial.begin(115200);

    // Set up Wi-Fi (AP mode)
    Serial.print("Setting up Access Point with SSID: ");
    if (WiFi.softAP(ssid)) {
        Serial.println("Access Point setup complete");
    } else {
        Serial.println("Failed to set up Access Point");
    }

    Serial.print("Access Point IP address: ");
    Serial.println(WiFi.softAPIP());

    // Initialize the SD card
    Serial.println("Initializing SD card...");
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("Failed to mount SD card!");
        return;
    } else {
        Serial.println("SD card mounted successfully.");
    }

    // Serve files from the SD card
    server.serveStatic("/", SD, "/").setDefaultFile("index.html");

    // Start the server
    server.begin();
    Serial.println("Server started");
}


void loop() {
    //nothing nothing nothing 
}
