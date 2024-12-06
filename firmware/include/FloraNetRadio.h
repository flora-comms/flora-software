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



// globals

extern LogList* pxHistoryLogs[256];

/// @brief Turns on the FLORANET Radio
void initLora();

/// @brief Lora task function
void loraTask(void * pvParameters);

/// @brief Radio starts recieving
void startRx();

/// @brief Handles a receive event
void handleRx();

/// @brief Begins transmitting a message
/// @param msg The message to transmit
/// @return The status of the transmission
int16_t startTx(Message *msg);

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