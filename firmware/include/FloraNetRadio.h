/*
======   FloraNetRadio.h  ======

Version: v1.0
Last Update: Nov 6 2024

======   DESCRIPTION   =====

FLORANET Layer 1 and 2 interfacing

*/

#ifndef FLORANET_RADIO_H
#define FLORANET_RADIO_H
#pragma once

#include <LogList.h>

/// @brief Handles the LoRa hardware.
class FloraNetRadio {
private:

    

    /// @brief Radio starts recieving
    void startRx();

    /// @brief Handles a TX_READY event
    void handleTx();

    /// @brief Initializes the LoRa hardware
    void initLora();

    /// @brief Handles an RX_READY event
    void handleRx();

    /// @brief Pepares the task for MPU light sleep.
    void prepForSleep();

public:
    /// @brief Constructor
    /// @param radio the lora radio hardware to use
    /// @param inbox The queue to transmit messages from
    /// @param outbox the queue to send received messages to
    FloraNetRadio()
    {    }

    /// @brief Runs the radio handler instance.
    void run();
};

/// @brief LoRa hardware handling task function.
/// @param pvParameters Pass in a pointer to a FloraNetRadio instance here
extern "C" void loraTask(void * pvParameter);
#endif