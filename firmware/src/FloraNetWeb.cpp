#include <FloraNetWeb.h>
// private
void FloraNetWeb::initWebServer() // Initializes web server stuff
{

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
    return;
  }

  DBG_PRINTLN("Access Point IP address: ");
  DBG_PRINTLN(WiFi.softAPIP());

  // init sd card

  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS))
  {
    DBG_PRINTLN("Failed to initialize the SD card.");
  }

  server.begin();
  DBG_PRINTLN("Web server started!");

  server.serveStatic("/", SD, "/").setDefaultFile("index.html");
  server.addHandler(&ws);

  // start DNS

  if (!MDNS.begin(WEBSERVER_DNS))
  {
    DBG_PRINTLN("Error setting up MDNS responder!");
  }

  MDNS.addService("http", "tcp", 80);

  // add websocket service
  ws.onEvent(onWsEvent);
  return;
}

void FloraNetWeb::runServer()
{
  while (true)
  {
    // wait for an action or 5 min timeout
    EventBits_t eventBits = xEventGroupWaitBits(xEventGroup, EVENTBIT_WEB_TX_READY | EVENTBIT_SOCKET_ACTION, false, true, pdMS_TO_TICKS(300000));

    // if timeout, return
    if ((eventBits & (EVENTBIT_WEB_TX_READY | EVENTBIT_SOCKET_ACTION)) == 0)
    {
      return;
    }

    // if websocket action, just clear the flag
    if ((eventBits & EVENTBIT_SOCKET_ACTION) != 0)
    {
      xEventGroupClearBits(xEventGroup, EVENTBIT_SOCKET_ACTION);
      YIELD();
    }

    // if a message is ready
    if ((eventBits & EVENTBIT_WEB_TX_READY) != 0)
    {
      // read in the message from the protocol task
      Message *msg;
      QUEUE_RECEIVE(qToWeb, msg)
      msg->appendHistory();
      // convert to json string and send over the web socket
      ws.textAll(msg->toSerialJson());
    }

    if (uxQueueMessagesWaiting(qToWeb) == 0)
    {
      xEventGroupClearBits(xEventGroup, EVENTBIT_WEB_TX_READY);
    } else {
      YIELD();
    }
  }
}

// public
void FloraNetWeb::run() {
  pinMode(USER_BUTTON, INPUT_PULLUP);
  attachInterrupt(USER_BUTTON, buttonISR, LOW);
  #ifdef POWER_SAVER
  xEventGroupSetBits(xEventGroup, EVENTBIT_WEB_SLEEP_READY);
  #endif
  while (true) {
    xEventGroupWaitBits(xEventGroup, EVENTBIT_WEB_REQUESTED, false, false, portMAX_DELAY);
    xEventGroupClearBits(xEventGroup, EVENTBIT_WEB_REQUESTED | EVENTBIT_WEB_SLEEP_READY);
    initWebServer();
    runServer();
    // upon timeout, clean up servers and attach button interrupt
    MDNS.end();
    server.end();
    SD.end();
    sdSPI.end();
    pinMode(USER_BUTTON, INPUT_PULLUP);
    attachInterrupt(USER_BUTTON, buttonISR, LOW);
    xEventGroupSetBits(xEventGroup, EVENTBIT_WEB_SLEEP_READY);
  }
}

// external
void onWsEvent(AsyncWebSocket *socket, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    DBG_PRINTLN("Websocket client connection received");
  } else if (type == WS_EVT_DISCONNECT) {
    DBG_PRINTLN("Client disconnected");
  } else if (type == WS_EVT_DATA) {
    ws.textAll(data, len);
    Message *rx_message = new Message(data, currentId++);

    rx_message->appendHistory();
    DBG_PRINT("WS Data received: ");

    #ifdef DEBUG
    for (int i = 0; i < len; i++) {
      DBG_PRINT((char)data[i]);
    }
    #endif

    DBG_PRINTLN();

    xQueueSend(     // send the message pointer to the protocol task
        qFromWeb,
        &rx_message,
        MAX_TICKS_TO_WAIT);

    // let the protocol task know a message is ready and the web task know something happened on the socket
    xEventGroupSetBits(xEventGroup, EVENTBIT_WEB_RX_DONE | EVENTBIT_SOCKET_ACTION);
    return;
  }
}

void webTask(void * pvParameter) {
  FloraNetWeb *handler = static_cast<FloraNetWeb *>(pvParameter);
  handler->run(); // run the handler. should never return
  delete handler; // if it does, delete it and the task?
  vTaskDelete(NULL);
  return;
}