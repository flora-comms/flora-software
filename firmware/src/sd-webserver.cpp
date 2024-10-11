#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SD.h>
#include <ESPmDNS.h>
// #include "WebSocket.h"
#include <time.h>
#include <ArduinoJson.h>
#include <SPI.h>

// SD card CS (Chip Select) pin
#define SD_CS_PIN 13
#define TIME_STR_LEN 29 // len plus one
#define MAX_PAYLOAD_LEN 255 // len plus one
#define NODE_ID 1
#define MSG_JSON_SIZE 81 // round number. gonna be mostly \0
#define MSG_HISTORY_WINDOW_SIZE MSG_JSON_SIZE*10

int sck = 14;
int miso = 2;
int mosi = 11;

// ------------ STRUCTS ---------------- //

class Message {
  public:
    char payload[MAX_PAYLOAD_LEN];
    time_t timestamp;
    uint8_t nodeId;

    Message(char *msgpayload)
      :payload({0})
      ,timestamp(millis())
      ,nodeId(NODE_ID) {
        memcpy(payload, msgpayload, strlen(msgpayload));
      }

    void serialize() {
      JsonDocument msg;
      msg["Type"] = "Msg";
      msg["Payload"] = payload;
      char strtime[TIME_STR_LEN];
      strftime(strtime, TIME_STR_LEN, "%c %Z", gmtime(&timestamp));
      msg["Timestamp"] = strtime;
      msg["NodeID"] = NODE_ID;

      // create json string and send to serial port for now
      serializeJson(msg, Serial);
    }
};

// ------------- GLOBALS ---------------- //
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/chat");

// WiFi SSID
const char *ssid = "AVAlink";

// ------------- PROTOTYPES -------------- //
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void populate(uint8_t offset, uint32_t id);

    // ---------------- MAIN -----------------//
    void setup()
{
  Serial.begin(115200);

  delay(5000);
  // Set up Wi-Fi (AP mode)
  Serial.println("Setting up Access Point with SSID: ");
  if (WiFi.softAP(ssid)) {
      Serial.println("Access Point setup complete");
  } else {
      Serial.println("Failed to set up Access Point");
  }

  Serial.println("Access Point IP address: ");
  Serial.println(WiFi.softAPIP());

  // Initialize the SD card
  Serial.println("Initializing SD card...");
  SPI.begin(sck, miso, mosi, SD_CS_PIN);
  SPI.setDataMode(SPI_MODE0);
  if (!SD.begin(SD_CS_PIN)) {
      Serial.println("Failed to mount SD card!");
      return;
  } else {
      Serial.println("SD card mounted successfully.");
  }

  // Serve files from the SD card
  server.serveStatic("/", SD, "/").setDefaultFile("index.html");

     // Set up mDNS responder:
    // - first argument is the domain name, in this example
    //   the fully-qualified domain name is "esp32.local"
    // - second argument is the IP address to advertise
    //   we send our IP address on the WiFi network
    if (!MDNS.begin("avalink")) {
      Serial.println("Error setting up MDNS responder!");
      while (1) {
        delay(1000);
      }
    }
    Serial.println("mDNS responder started");


    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    // Start the server
    server.begin();
    Serial.println("Server started");
    MDNS.addService("http", "tcp", 80);
}

void loop() {
}

// ------------------ DEFINITIONS ------------------ //
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  
  if(type == WS_EVT_CONNECT){
  
    Serial.println("Websocket client connection received");
    client->ping();
     
  } else if(type == WS_EVT_DISCONNECT){
 
    Serial.println("Client disconnected");
  
  } else if(type == WS_EVT_DATA){

    ws.textAll(data, sizeof(data));
    
    // populate(0, client->id());
    
  }
}

void populate(uint8_t offset, uint32_t id) {
  File msgHistory = SD.open("/mock_history.txt", "r");
  unsigned long len = msgHistory.size();

  // create buffer to store data before sending
  char windowBuf[MSG_HISTORY_WINDOW_SIZE];

  // TODO check offset*MSG_BUF_SIZE < len

  // go to start of message history window
  unsigned long start = len - (1+offset)*MSG_HISTORY_WINDOW_SIZE;
  msgHistory.seek(start);

  // read the message history into the buffer
  msgHistory.readBytes(windowBuf, MSG_HISTORY_WINDOW_SIZE);

  // send it over the websocket
  ws.text(id, windowBuf);

  //

}