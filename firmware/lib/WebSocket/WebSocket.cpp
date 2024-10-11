#include "WebSocket.h"

AsyncWebSocket ws(WEB_SOCKET_PATH);

class Message {
    char * payload;
    tm timestamp;
    uint8_t nodeId;
};

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{

    if (type == WS_EVT_CONNECT)
    {

        Serial.println("Websocket client connection received");
    }
    else if (type == WS_EVT_DISCONNECT)
    {

        Serial.println("Client disconnected");
    }
    else if (type == WS_EVT_DATA)
    {
        Serial.print("Data received: ");

        for (int i = 0; i < len; i++)
        {
            Serial.print((char)data[i]);
        }

        Serial.println();

        JsonDocument params;

        // TODO: check for empty data string
        deserializeJson(params, data);

        // TODO: check for absence of type
        ApiType type = params["Type"];
        switch(type) {
            case API_TYPE_HISTORY:
                populate(params["Offset"], client->id());
                break;
            case API_TYPE_MSG:
                // handle message
                handleTx(params["Node"], params["Payload"]);
                break;
            case API_TYPE_ACK:
                // do we need this? maybe stop a retry timer?
                break;
        }
    }
}

// handlers

// data event
void populate(uint8_t offset) {
    // create message buffer

    // open file

    // calculate offset

    // go to offset

    // read next 10 messages into buffer

    // close file

    // send message buffer over socket

}

// deals with message sent by client
void handleTx(uint8_t nodeId, const char *payload) {
    // Deserialize json

    // create message object from json doc

    // add timestamp

    // Put into tx queue for lora

    // Acknowledge client

    // add to session message buffer
}

// deals with message recieved from network
void handleRx(uint8_t nodeId, const char *payload) {
    // Read from Rx queue
    
    // Add timestamp

    // create message json

    // serialize and send to client

    // if no ack, retry send x 3

    // add to session Message buffer

}

// close event
void onClose() {
    // write session message buffer to history file
}