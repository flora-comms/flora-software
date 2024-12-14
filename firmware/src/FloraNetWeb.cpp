#include <FloraNetWeb.h>
// private
void FloraNetWeb::initWebServer() // Initializes web server stuff
{
  WiFi.mode(WIFI_MODE_AP);
  WiFi.enableAP(true);
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
#ifdef USE_NVS
  if (SD.exists(PACKET_ID_FILENAME))
  {
    // read in packet id from sd card
    File file = SD.open(PACKET_ID_FILENAME, FILE_READ);
    uint8_t buf[50];
    file.readBytesUntil('\0', buf, 50);
    file.close();
    JsonDocument json;
    deserializeJson(json, buf);
    currentId = json["ID"];
  }
  else
  {
    currentId = 0;
  }
#endif

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
  // initialize the server
  initWebServer();
  bool timeout = false;

  #ifdef WS_KEEP_ALIVE
  TaskHandle_t tskKeepAlive;
  xTaskCreate(wsKeepAlive, "keepalive", 2048, (void *)1, TASK_PRIORITY_WEB, 0);
  #endif

// for power saving
#ifdef POWER_SAVER
#define RESET_TIMER() xTimerReset(xWebTimer, pdMS_TO_TICKS(1000)); timeout = false
  TimerHandle_t xWebTimer = xTimerCreate(
      "Web",
      pdMS_TO_TICKS(WEB_TIMEOUT),
      false,
      (void *)1,
      WebTimeoutCallback);
  xTimerStart(xWebTimer, pdMS_TO_TICKS(1000));
#else 
#define RESET_TIMER()
#endif
  

  // make sure the webserver wont timeout right away
  xEventGroupClearBits(xEventGroup, EVENTBIT_WEB_TIMEOUT);

  while (!timeout)
  {
    // wait for an action
    EventBits_t eventBits = xEventGroupWaitBits(
                                  xEventGroup, 
                                  EVENTBIT_WEB_TX_READY | EVENTBIT_SOCKET_ACTION | EVENTBIT_WEB_TIMEOUT,
                                  false, 
                                  false, 
                                  portMAX_DELAY);

    // if timeout, return
    if ((eventBits & EVENTBIT_WEB_TIMEOUT))
    {
      xEventGroupClearBits(xEventGroup, EVENTBIT_WEB_TIMEOUT);
      timeout = true;
    }

    // if websocket action, just clear the flag and reset the timer
    if ((eventBits & EVENTBIT_SOCKET_ACTION))
    {
      xEventGroupClearBits(xEventGroup, EVENTBIT_SOCKET_ACTION);
      
      RESET_TIMER();
    }

    // if a message is ready
    if ((eventBits & EVENTBIT_WEB_TX_READY))
    {
      // read in the message from the protocol task
      Message *msg;
      QUEUE_RECEIVE(qToWeb, msg)
      msg->appendHistory();
      // convert to json string and send over the web socket
      ws.textAll(msg->toSerialJson());

      RESET_TIMER();
    }


    // if there are no more messages in the queue, clear the event bit
    if (uxQueueMessagesWaiting(qToWeb) == 0)
    {
      xEventGroupClearBits(xEventGroup, EVENTBIT_WEB_TX_READY);
    }

#ifdef USE_NVS
    
#endif

    YIELD();  // for wdt
  }

  #ifdef POWER_SAVER
  // delete the timer & return upon timeout
  xTimerDelete(xWebTimer, pdMS_TO_TICKS(1000));
  #endif

  #ifdef WS_KEEP_ALIVE
  vTaskDelete(tskKeepAlive);
  #endif
  return;
}

void cleanWebServer() {
  // upon timeout, clean up servers and attach button interrupt
  MDNS.end();
  server.end();
  SD.end();
  sdSPI.end();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  ATTACH_BUTTONISR();
}

// public
void FloraNetWeb::run() {
  #ifdef POWER_SAVER
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  xEventGroupSetBits(xEventGroup, EVENTBIT_WEB_SLEEP_READY);
  #else
  xEventGroupSetBits(xEventGroup, EVENTBIT_WEB_REQUESTED);
  #endif
  while (true) {
    // wait for the web server to be requested
    xEventGroupWaitBits(xEventGroup, EVENTBIT_WEB_REQUESTED, false, false, portMAX_DELAY);

    // upon request, clear the sleep-ready and request bits
    xEventGroupClearBits(xEventGroup, EVENTBIT_WEB_REQUESTED | EVENTBIT_WEB_SLEEP_READY | EVENTBIT_NEW_MESSAGE);
    
    #ifdef FLASH_ON_NEW_MESSAGE
    TaskHandle_t ledTask = xTaskGetHandle("led");

    if (!(ledTask == NULL))
    {
      vTaskDelete(ledTask);
    }

    digitalWrite(NEW_MESSAGE_LED, LOW);

    xTaskCreatePinnedToCore(wifiBlinker, "wifiBlink", 2048, (void *)1, TASK_PRIORITY_WEB, &ledTask, 1);
    #endif

    // run the server
    runServer();


#ifdef FLASH_ON_NEW_MESSAGE
  vTaskDelete(ledTask);
  digitalWrite(WIFI_LED, LOW);
#endif
    // upon timeout, let power manager know its ready to sleep and yield the processor
    xEventGroupSetBits(xEventGroup, EVENTBIT_WEB_SLEEP_READY);
    YIELD();
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

    
    Message *rx_message = new Message(data, currentId);
    currentId++;
    
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
  } else {
    return;
  }
}

void WebTimeoutCallback ( TimerHandle_t xTimer )
{
  xEventGroupSetBits(xEventGroup, EVENTBIT_WEB_TIMEOUT);
}

void webTask(void * pvParameter) {
  FloraNetWeb *handler = static_cast<FloraNetWeb *>(pvParameter);
  handler->run(); // run the handler. should never return
  delete handler; // if it does, delete it and the task?
  vTaskDelete(NULL);
  return;
}