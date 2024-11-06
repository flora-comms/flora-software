/*
======   DataParsing.h  ======

Version: v1.0
Last Update: Nov 6 2024

======   DESCRIPTION   =====

Provides data parser error codes and message types.

*/

#ifndef DATA_PARSING_H
#define DATA_PARSING_H

enum ParseError {
    PARSE_ERR_NONE,
};

enum MsgType {
    TEXT,
    CONTROL,
    SOS
};

class LoraPacket {
    public:
        uint8_t destination;
        uint8_t senderId;
        uint8_t packetId;
        uint8_t ttl;
        String  *payload;

        ParseError fromBytes(uint8_t *bytes);
};

// message type that can be pulled into a queue
class WebPacket {
    public:
        MsgType type;
        String  *payload;
        int     senderId;
};

#endif