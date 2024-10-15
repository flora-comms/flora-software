/* 
======   AVAlink Firmware  ======

Version: v1.0 (beta)
Last Update: under development

======   DESCRIPTION   =====

Firmware for AVAlink nodes

*/

// ---------------- INCLUDES ---------------- //

#include <Arduino.h>
#include <SD.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// ---------------- DEFINES ---------------- //

// LILYGO T3 S3 V1.2
#define SD_CS   13    // SD chip select pin
#define SD_SCK  14    // SD sck pin
#define SD_MOSI 11    // SD mosi pin
#define SD_MISO 2     // SD miso

// ---------------- GLOBALS ---------------- //

AsyncWebServer server(80);    // web server on port 80
AsyncWebSocket ws("/chat");   // web socket at "ws://avalink.local/chat"

// ---------------- CLASSES ---------------- //



// ---------------- PROTOTYPES ---------------- //

/// @brief avalink.local/chat websocket event handler
/// @param server 
/// 
/// @param client 
/// @param type 
/// @param arg 
/// @param data 
/// @param len 
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

// ---------------- MAIN ---------------- //

void setup() {
  
}

void loop() {
  // nothing nothing
}

// ---------------- DEFINITIONS ---------------- //
