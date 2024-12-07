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

#include <FloraNetUtils.h>

/// @brief Handles the LoRa hardware.
class FloraNetRadio {
private:
    SX1262* _pxRadio;
    TaskHandle_t _xTaskHandle;
    QueueHandle_t _qInbox;
    QueueHandle_t _qOutbox;
    EventGroupHandle_t _eventGroup;

public:
    /// @brief Radio starts recieving
    void startRx();

    /// @brief Begins transmitting a message
    /// @param msg The message to transmit
    /// @return The status of the transmission
    int16_t startTx(Message *msg);

    /// @brief The main task functionality to run.
    void mainTask();

    /// @brief Turns on the FLORANET Radio
    void initLora();

    /// @brief Handles a receive event
    void handleRx();

    FloraNetRadio(SX1262* radio, QueueHandle_t qInbox, QueueHandle_t qOutbox, EventGroupHandle_t eventGroup);

    void begin();

    void end();
};

// globals

extern LogList* pxHistoryLogs[256];

extern "C" static void loraTask( void * pvParameter);

/// @brief Handles a TX complete event
void handleTx();

void startCad();

/// @brief Handles a completed CAD event
/// @return The CAD results
int16_t handleCad();

/// @brief Checks if a message needs to be repeated over the LoRa network
/// @param msg The message to check
/// @param log The LogList to check the message against
/// @return True if the message needs repeating. False if it does not.
bool needsRepeating(Message *msg);

/// @brief LoRa RX interrupt handler
static void onRxIrq(void);

/// @brief Lora TX interrupt handler
static void onTxIrq(void);

/// @brief LoRa CAD interrupt handler
static void onCadIrq(void);
#endif