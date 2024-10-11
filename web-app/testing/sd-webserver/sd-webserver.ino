#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SD.h>
#include <ESPmDNS.h>

// SD card CS (Chip Select) pin
#define SD_CS_PIN 13

#define REASSIGN_PINS
int sck = 14;
int miso = 2;
int mosi = 11;


// ------------- GLOBALS ---------------- //
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/chat");

// WiFi SSID
const char *ssid = "AVAlink";

// ------------- PROTOTYPES ---------------//

// web socket event handler
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

// ---------------- MAIN -----------------//
void setup() {
  Serial.begin(115200);

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
    //nothing nothing nothing 
}

// ------------------ DEFINITIONS ------------------ //
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  
  if(type == WS_EVT_CONNECT){
  
    Serial.println("Websocket client connection received");
     
  } else if(type == WS_EVT_DISCONNECT){
 
    Serial.println("Client disconnected");
  
  } else if(type == WS_EVT_DATA){
  
    ws.textAll(data, len);
 
    Serial.print("Data received: ");
  
    for(int i=0; i < len; i++) {
          Serial.print((char) data[i]);
    }
  
    Serial.println();
  }
}


