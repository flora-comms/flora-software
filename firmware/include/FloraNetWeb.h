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

#include <FloraNetUtils.h>

// ---- GLOBALS ---- //

extern uint8_t currentId;   // the current packet id

// ---- TYPEDEF ----- //

enum WebError { WEB_ERR_NONE, WEB_ERR_SD, WEB_ERR_WIFI_AP };

enum MsgType { TEXT, SOS };

enum MsgSource { LORA, JSON };



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
void webTask(void *);

/// @brief Initializes web server stuff
WebError initWebServer();

#endif