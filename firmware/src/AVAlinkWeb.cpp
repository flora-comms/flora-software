#include "AVAlinkWeb.h"
 
//init globals
QueueHandle_t qFromLora = xQueueCreate(QUEUE_LENGTH, sizeof( Message *));
QueueHandle_t qToLora = xQueueCreate(QUEUE_LENGTH, sizeof( Message *));
AsyncWebServer server(80);
AsyncWebSocket ws(WEBSOCKET_ENDPOINT); // websocket

bool RxMsgAvailable = false;
bool TxMsgAvaialble = false;
SPIClass sd_spi(FSPI); // SPI2 sd card spi bus

String Message::toSerialJson()
{
    JsonDocument json;
    String data;
    json["Payload"] = payload;
    json["NodeID"] = senderId;
    if (type == TEXT) {
        json["SOS"] = 0;
    } else {
        json["SOS"] = 1;
    }

    serializeJson(json, data);
    return data;
}

Message::Message() {
    type = TEXT;
    payload = String();
    senderId = 0x00;
}


Message::Message(uint8_t *data) {
    JsonDocument json;

    deserializeJson(json, data);

    payload = String((const char *)json["Payload"]);
    
    senderId = json["NodeID"];

    if (json["SOS"] == 0)
    {
        type = TEXT;
    }
    else
    {
        type = SOS;
    }
}

/// @brief Web socket event handler
/// @param server the web socket
/// @param client the client
/// @param type the type of event
/// @param arg any argumetns passed in
/// @param data the data included in the message
/// @param len the length of the data buffer
void onWsEvent(
    AsyncWebSocket *socket,
    AsyncWebSocketClient *client,
    AwsEventType type,
    void *arg,
    uint8_t *data,
    size_t len
    )
{
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

        Message *rx_message = new Message(data);

        // File history = SD.open("/history.JSON", FILE_WRITE);  // open history file
        // history.write(*data);                     // write data
        // history.close();                          // close the file

        DBG_PRINT("WS Data received: ");

        for (int i = 0; i < len; i++)
        {
            DBG_PRINT((char)data[i]);
        }

        DBG_PRINTLN();

        
        xQueueSend(qToLora, (void *)&rx_message, (TickType_t) 0);     // send the message pointer to the lora task
        return;
    }
}
    /// @brief Web task function
    void webTask( void * )
    {
        initWebServer();    // initialize the hardware
        while (true) {
            if (0 < uxQueueMessagesWaiting(qFromLora)) {       // if message is available
                
                Message *rx_msg;        // create pointer to message object

                xQueueReceive(qFromLora, &rx_msg, 0);   // read in mesage pointer from queue

                String data = rx_msg->toSerialJson();   // create serialized json object

                delete(rx_msg);

                ws.textAll(data);       // send the data over the web socket to all the clients

                // write to sd card
            } else {
                vTaskDelay(1);  // delay five ms
            }
        };
    }
    WebError initWebServer() // Initializes web server stuff
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
            return WEB_ERR_WIFI_AP;
        }

        DBG_PRINTLN("Access Point IP address: ");
        DBG_PRINTLN(WiFi.softAPIP());

        // Setup SPI busses
        sd_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

        if (!SD.begin(SD_CS))
        {
            DBG_PRINTLN("Failed to mount SD card!");
            return WEB_ERR_SD;
        }
        else
        {
            DBG_PRINTLN("SD card mounted successfully.");
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
        return WEB_ERR_NONE;
    }
