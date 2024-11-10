/*
======   AVAlinkWeb.h  ======

Version: v1.0
Last Update: Nov 6 2024

======   DESCRIPTION   =====

AVAlink web server interface

*/

#ifndef AVALINK_WEB_H
#define AVALINK_WEB_H
#pragma once

#include <WiFi.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <AVAlinkConfig.h>
#include <SD.h>
#include <FreeRTOS.h>

// ---- GLOBALS ---- //

extern QueueHandle_t qFromLora;         // cueue handles. hold 10 pointers to message objects each
extern QueueHandle_t qToLora;
extern AsyncWebServer server;             // web server on port 80
extern AsyncWebSocket ws; // websocket

extern bool RxMsgAvailable;
extern bool TxMsgAvaialble;
extern SPIClass sd_spi; // SPI2 sd card spi bus

// ---- TYPEDEF ----- //

enum WebError {
    WEB_ERR_NONE,
    WEB_ERR_SD,
    WEB_ERR_WIFI_AP
};

enum MsgType
{
    TEXT,
    SOS
};

/// @brief Message class. Intermediate between a web chat and a lora packet for use by the web task to pass around. 
class Message
{
public:
    MsgType type;       // the message type
    char *payload;    // message payload array
    uint8_t senderId;   // the sender ID (0-255). 0 reserved for gateway

    Message();

    /// @brief Converts a serial json message  into a Message type.
    /// @param data The data buffer containing the serial data
    /// @return Web error status code
    Message(uint8_t *data);

    /// @brief Converts a the message type into a JSON string ready for sending over the web socket.
    /// @param data The data string to write into.
    /// @return A Web Error status code.
    String toSerialJson();
};



/// @brief Web socket event handler
/// @param socket the web socket
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
    size_t len);

/// @brief Web task function
void webTask( void * );

/// @brief Initializes web server stuff
WebError initWebServer();

#endif