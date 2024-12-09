/*
======   FloraNetUtils.h  ======

Version: v2.0
Last Update: Dec 6 2024

======   DESCRIPTION   =====

FloraNet utilities. All necessary include files to run FloraNet.

*/
#ifndef FLORANET_UITLS_H
#define FLORANET_UTILS_H

#include <LogList.h>

// critical section macro
#define CRITICAL_SECTION(args...) taskENTER_CRITICAL(&csToken); args; taskEXIT_CRITICAL(&csToken);

// Globals
extern EventGroupHandle_t xEventGroup;
extern portMUX_TYPE csToken;            // spinlock for critical sections

// for timer task
extern QueueHandle_t qRetries;          // the retry queue
extern LogList *pxHistoryLogs[256];     // array of LogList pointers containing message history from each sender id.

// for web socket event handler
extern QueueHandle_t qFromWeb;
extern uint8_t currentId;       // the current packet id

// ISRs
/// @brief LoRa RX interrupt handler
static void RxISR(void);

/// @brief LoRa TX interrupt handler
static void TxISR(void);

/// @brief User button press ISR
static void buttonISR(void);
#endif // FLORANET_UTILS_H