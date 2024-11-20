#include "AVAlinkWeb.h"

// init globals
AsyncWebServer server(80);
AsyncWebSocket ws(WEBSOCKET_ENDPOINT); // websocket
uint8_t currentId = 0;

/// @brief Web socket event handler
/// @param server the web socket
/// @param client the client
/// @param type the type of event
/// @param arg any argumetns passed in
/// @param data the data included in the message
/// @param len the length of the data buffer
void onWsEvent(AsyncWebSocket *socket, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    DBG_PRINTLN("Websocket client connection received");
  } else if (type == WS_EVT_DISCONNECT) {
    DBG_PRINTLN("Client disconnected");
  } else if (type == WS_EVT_DATA) {
    ws.textAll(data, len);
    Message *rx_message = new Message(data, currentId++);

    DBG_PRINT("WS Data received: ");

    // Write Message to SD card
    rx_message->appendHistory("/data/history.csv");

    for (int i = 0; i < len; i++) {
      DBG_PRINT((char)data[i]);
    }

    DBG_PRINTLN();

    xQueueSend(     // send the message pointer to the lora task
        qToMesh,
        (void *)&rx_message,
        (TickType_t)0);
    xEventGroupSetBits(xAvalinkEventGroup, EVENTBIT_LORA_Q);
    return;
  }
}

/// @brief Web task function
void webTask(void *) {
  bApIsUp = true;
  initWebServer(); // initialize the hardware
  while (true) {
    xEventGroupWaitBits(xAvalinkEventGroup, EVENTBIT_WEB_READY, pdTRUE, pdFALSE, portMAX_DELAY);
    Message *rx_msg; // create pointer to message object

    xQueueReceive(qToWeb, &rx_msg, 0); // read in mesage pointer from queue

    String data = rx_msg->toSerialJson(); // create serialized json object

    ws.textAll(data); // send the data over the web socket to all the clients

    // write to sd card
    rx_msg->appendHistory(HISTORY_FILENAME);
  };
}
WebError initWebServer() // Initializes web server stuff
{

  // Set up Wi-Fi (AP mode)
  DBG_PRINTLN("Setting up Access Point with SSID: ");
  DBG_PRINTLN(WIFI_SSID);
  if (WiFi.softAP(WIFI_SSID)) {
    DBG_PRINTLN("Access Point setup complete");
  } else {
    DBG_PRINTLN("Failed to set up Access Point");
    return WEB_ERR_WIFI_AP;
  }

  DBG_PRINTLN("Access Point IP address: ");
  DBG_PRINTLN(WiFi.softAPIP());

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

  currentId = 0;
  return WEB_ERR_NONE;
}
