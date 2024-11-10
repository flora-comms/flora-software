/*
======   AVAlinkRadio.h  ======

Version: v1.0
Last Update: Nov 6 2024

======   DESCRIPTION   =====

AVAlink Layer 1 and 2 interfacing

*/

#ifndef AVALINK_RADIO_H
#define AVALINK_RADIO_H
#pragma once

#include <RadioLib.h>
#include <AVAlinkWeb.h>

// ----- GLOBALS ----- //

extern volatile bool operationDone;
extern int txState;
extern SPIClass lora_spi;
extern SX1262 radio;

// ---- TYPEDEFS ----- //

// Errors possible in lora
enum LoraError {
    LORA_ERR_NONE,
    LORA_ERR_INIT,
};

enum LoraState
{
    LORA_TX,
    LORA_RX,
    LORA_CAD,
};

class LoraPacket
{
public:
    uint8_t destination;
    uint8_t senderId;
    uint8_t packetId;
    uint8_t ttl;
    char *payload;

    /// @brief Converts a queue message into a lora packet
    /// @param msg The QueueMessage object to convert into a Lora Packet.
    LoraError fromMsg(Message *msg);

    /// @brief Converts a lora packet into a que message
    /// @param msg The Queue Message object to write into
    /// @return A QParseError status code.
    LoraError toMsg(Message *msg);

    /// @brief Converts a lora packet into a csv string that can be written to history.csv in the sd card
    /// @return The SD-compatible formatted csv string.
    String toSdFormat();

    /// @brief Converts a byte array to a lora packet
    /// @param bytes Buffer containing the payload bytes
    /// @return The length of the packet payload.
    uint16_t fromBytes(uint8_t *bytes, uint16_t len);

    /// @brief Converts a lora packet to a byte string
    /// @param bytes Pointer to buffer to read into
    /// @return Length of the packet.
    uint16_t toBytes(uint8_t *bytes);

    /// @brief determines if packet needs repeating
    bool needsRepeating();
};

/// @brief Lora interrupt handler
void onLoraIrq(void);

/// @brief Lora task function
/// @param qFromWeb Recieves messages from Web task
/// @param qToWeb Sends messages to web task.
void loraTask( void *pvParameters);

/// @brief Initilizes Lora stuff
LoraError initLora(SX1262 *radio);

#endif