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
#include <ESPmDNS.h>

// ---------------- DEFINES ---------------- //

/*
ENVIRONMENT

Uncomment define for the IDE uses. MAKE SURE ONLY ONE IS UNCOMMENTED

*/
#define ENV_PLATFORMIO
// #define ENV_ARDUINO

/*
FIRMWARE

Variables for firmware configuration
*/
#define WIFI_SSID           "AVAlink"   // WiFi network name
#define WEBSERVER_DNS       "avalink"   // puts domain at "http://{WEBSERVER_DNS}.local"
#define WEBSOCKET_ENDPOINT  "/chat"     // puts websocket at "ws://{WEBSERVER_DNS}/{WEBSOCKET_ENDPOINT}"

/*
DEBUGGING

Comment out for debugging
*/
#define DEBUG


#ifdef DEBUG
  #define SERIAL_BAUD       115200
  #define DBG_PRINTLN(MSG)  Serial.println(MSG)
#else
  #define DBG_PRINTLN(MSG) 
#endif

/*
HARDWARE

Options for Hardware
*/

// #define LILYGO_T3_S3_V1_2
#define PETAL_V0_0

// LILYGO T3 S3 V1.2
#ifdef LILYGO_T3_S3_V1_2
  #define SD_CS   13    // SD chip select pin
  #define SD_SCK  14    // SD sck pin
  #define SD_MOSI 11    // SD mosi pin
  #define SD_MISO 2     // SD miso
#endif

// Petal V0.0
#ifdef PETAL_V0_0
  #define SD_CS   10  // SD chip select pin
  #define SD_SCK  12  // SD sck pin
  #define SD_MOSI 11  // SD mosi pin
  #define SD_MISO 13  // SD miso
#endif
// ---------------- GLOBALS ---------------- //

AsyncWebServer server(80);    // web server on port 80
AsyncWebSocket ws(WEBSOCKET_ENDPOINT); 

// ---------------- CLASSES ---------------- //



// ---------------- PROTOTYPES ---------------- //

/// @brief websocket event handler
/// @param server The WebSocket server/// 
/// @param client The webSocket client
/// @param type   The type of WebSocket event
/// @param arg    
/// @param data   Data buffer included in the websocket frame
/// @param len    Length of the data buffer
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

// ---------------- MAIN ---------------- //

void setup() {
  #ifdef DEBUG
  Serial.begin(SERIAL_BAUD);
  #endif

  #ifdef ENV_PLATFORMIO
  delay(5000); // to give time to restart serial monitor task
  #endif

  // Set up Wi-Fi (AP mode)
  DBG_PRINTLN("Setting up Access Point with SSID: ");
  DBG_PRINTLN(WIFI_SSID);
  if (WiFi.softAP(WIFI_SSID))
  {
    DBG_PRINTLN("Access Point setup complete");
  }
  else
  {
    DBG_PRINTLN("Failed to set up Access Point");
  }

  DBG_PRINTLN("Access Point IP address: ");
  DBG_PRINTLN(WiFi.softAPIP());

  // Initialize the SD card

  #ifdef ENV_PLATFORMIO
    DBG_PRINTLN("Initializing SD card...");
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    SPI.setDataMode(SPI_MODE0);
  #endif

  #ifdef LILYGO_T3_S3_V1_2
    #ifdef ENV_ARDUINO
      #define REASSIGN_PINS
      int sck = SD_SCK;
      int mosi = SD_MOSI;
      int miso = SD_MISO;
    #endif
  #endif

  if (!SD.begin(SD_CS))
  {
    DBG_PRINTLN("Failed to mount SD card!");
    return;
  }
  else
  {
    DBG_PRINTLN("SD card mounted successfully.");
  }

  // Start server

  server.begin();
  DBG_PRINTLN("Web server started!");

  server.serveStatic("/", SD, "/").setDefaultFile("/index.html");
  server.addHandler(&ws);

  // start DNS

  if (!MDNS.begin(WEBSERVER_DNS)) {
    DBG_PRINTLN("Error setting up MDNS responder!");
  }

  MDNS.addService("http", "tcp", 80);

  // add websocket service
  ws.onEvent(onWsEvent);

}

void loop() {
  // nothing nothing
}

// ---------------- DEFINITIONS ---------------- //
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT)
  {
    DBG_PRINTLN("Websocket client connection received");
  }
  else if (type == WS_EVT_DISCONNECT)
  {

    DBG_PRINTLN("Client disconnected");
  }
  else if (type == WS_EVT_DATA)
  {

    ws.textAll(data, len);

    // File history = SD.open("/history.JSON", FILE_WRITE);  // open history file
    // history.write(*data);                     // write data
    // history.close();                          // close the file

    #ifdef DEBUG
      Serial.print("Data received: ");

      for (int i = 0; i < len; i++)
      {
        Serial.print((char)data[i]);
      }

      Serial.println();
    #endif
  }
}

