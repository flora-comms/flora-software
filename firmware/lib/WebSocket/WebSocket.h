// Web Socket lib for AVAlink

#include <ESPAsyncWebServer.h>
#include "ApiHandlers.h"

#define WEB_SOCKET_PATH "/chat"

// global web socket variable
extern AsyncWebSocket ws;

// websocket initialization function
void wsInit();

// event handler function
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);


// gets message history
void populate(uint8_t offset, uint32_t id);

// deals with message sent by client
void handleTx(uint8_t nodeId, const char *payload);

// deals with message recieved from network
void handleRx(uint8_t nodeId, const char *payload);

// deals with closed web socket

