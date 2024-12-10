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

    /// @brief Initializes web server stuff
    void initWebServer();

    /// @brief Runs the web server until timeout
    void runServer();

public:
    FloraNetWeb() {};
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