/* 
======   AVAlink Firmware  ======

Version: v1.0 (beta)
Last Update: under development

======   DESCRIPTION   =====

Firmware for AVAlink nodes. 
To configure the firmware, follow the instructions in lib/AVAlinkConfig/ConfigOptions.h

*/

// ---------------- INCLUDES ---------------- //

#ifdef ENV_PLATFORMIO
#include <Arduino.h>
#endif

#include <SD.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <RadioLib.h>
#include <ArduinoJson.h>
#include <AVAlink.h>

// ---------------- CLASSES ---------------- //
enum LoraState
{
  LORA_TX,
  LORA_RX,
  STDBY,
  SLEEP
};

// ---------------- GLOBALS ---------------- //

AsyncWebServer server(80);              // web server on port 80
AsyncWebSocket ws(WEBSOCKET_ENDPOINT);  // websocket
SPIClass sd_spi(FSPI);                  // SPI=`sd card spi bus
SPIClass lora_spi(HSPI);                // SPI3 lora module spi bus
SX1262 radio = new Module(              // lora radio
    LORA_NSS,
    LORA_IRQ,
    LORA_NRST,
    LORA_BUSY,
    lora_spi,
    SPISettings(2000000, MSBFIRST, SPI_MODE0));

volatile bool lora_flag = false;
int           tx_state = RADIOLIB_ERR_NONE;

enum LoraState lora_state = STDBY;

// ---------------- PROTOTYPES ---------------- //

/// @brief websocket event handler
/// @param server The WebSocket server/// 
/// @param client The webSocket client
/// @param type   The type of WebSocket event
/// @param arg    
/// @param data   Data buffer included in the websocket frame
/// @param len    Length of the data buffer
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

/// @brief Lora interrupt event handler
void onLoraIrq(void) {
  lora_flag = true;
}

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


  // Setup SPI busses
  sd_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  lora_spi.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);

  // Initialize the SD card

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

  // initialize SX1262 with default settings
  Serial.print(F("[SX1262] Initializing ... "));
  int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC, LORA_POWER, LORA_PREAMB);
  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
  radio.setCurrentLimit(60.0);
  radio.setDio2AsRfSwitch(true);
  radio.explicitHeader();
  radio.setCRC(2);
  radio.setDio1Action(onLoraIrq);
  radio.startReceive();
  lora_state = LORA_RX;

  // Start server

  server.begin();
  DBG_PRINTLN("Web server started!");

  server.serveStatic("/", SD, "/").setDefaultFile("index.html");
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
  if (lora_flag) {
    lora_flag = false;    // reset flag

    if (lora_state == LORA_TX) {
      if (!(tx_state == RADIOLIB_ERR_NONE)) {
        DBG_PRINTLN("LoRa transmission failure");
      } else {
        lora_state = LORA_RX;
        radio.startReceive();
      }
    } else if (lora_state == LORA_RX) {
      int radio_state;
      String rx_data;
      radio_state = radio.readData(rx_data);
      if (!(radio_state == RADIOLIB_ERR_NONE)) {
        DBG_PRINTLN("Lora RX failure");
      } else {
        DBG_PRINTLN("Recieved LoRa data:");
        DBG_PRINTLN(rx_data);
        DBG_PRINTLN("");
        JsonDocument json;
        json["Payload"] = rx_data;
        json["NodeID"] = 4;
        String data;
        serializeJson(json, data);
        ws.textAll(data);
      }
    } else {
        DBG_PRINTLN(
          "lora state invalid????"
        );
    }

  }
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
    JsonDocument json;

    deserializeJson(json, data);

    String payload = json["Payload"];

    ws.textAll(data, len);

    // File history = SD.open("/history.JSON", FILE_WRITE);  // open history file
    // history.write(*data);                     // write data
    // history.close();                          // close the file

    #ifdef DEBUG
      Serial.print("WS Data received: ");

      for (int i = 0; i < len; i++)
      {
        Serial.print((char)data[i]);
      }

      Serial.println();
    #endif

    tx_state = radio.startTransmit(payload);

    lora_state = LORA_TX;

  }
}

