/*
======   AVAlinkWeb.h  ======

Version: v1.0
Last Update: Nov 6 2024

======   DESCRIPTION   =====

AVAlink web server interface

*/

#ifndef AVALINK_WEB_H
#define AVALINK_WEB_H

#include <FloraNetUtils.h>

// handles the web server
class FloraNetWeb {
private:
    AsyncWebServer * _server;
    AsyncWebSocket * _ws;
    QueueHandle_t _inbox;       // the queue where messages from the protocol task are received
    QueueHandle_t _outbox;

    /// @brief Initializes web server stuff
    void initWebServer();

    /// @brief Runs the web server until timeout
    void runServer();

public:
    FloraNetWeb(QueueHandle_t inbox, QueueHandle_t outbox) {
        _server = new AsyncWebServer(80);
        _ws = new AsyncWebSocket(WEBSOCKET_ENDPOINT);
        _inbox = inbox;
    }
    /// @brief Runs the web service handler.
    void run();
};

/// @brief Web socket event handler
/// @param socket the web socket
/// @param client the client
/// @param type the type of event
/// @param arg any argumetns passed in
/// @param data the data included in the message
/// @param len the length of the data buffer
void onWsEvent(AsyncWebSocket *socket, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len);

/// @brief Web task function
extern "C" void webTask(void * pvParameter);



#endif