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
#if defined(USE_CS)
#define CRITICAL_SECTION(args...) taskDISABLE_INTERRUPTS(); args; taskENABLE_INTERRUPTS();
#else
#define CRITICAL_SECTION(args...) args;
#endif

// attaches the buttonISR to the user button
#define ATTACH_BUTTONISR() pinMode(USER_BUTTON, INPUT_PULLUP); vTaskDelay(3); attachInterrupt(digitalPinToInterrupt(USER_BUTTON), buttonISR, LOW)
// detaches the buttonISR from the user button
#define DETACH_BUTTONISR() detachInterrupt(digitalPinToInterrupt(USER_BUTTON))

// ensures that only valid items are read in from the queue
#define QUEUE_RECEIVE(queue, buf)   if(!xQueueReceive(queue, &buf, MAX_TICKS_TO_WAIT)) { return; }

#define YIELD() vTaskDelay(1)
#define SPI sdSPI
// Globals
extern EventGroupHandle_t xEventGroup;
extern SX1262 radio;
extern SPIClass sdSPI;
extern SPIClass loraSPI;
extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern portMUX_TYPE csToken;
// for timer task
extern QueueHandle_t qRetries;          // the retry queue
extern LogList* pxHistoryLogs[256];     // array of LogList pointers containing message history from each sender id.
extern QueueHandle_t qToMesh ;
extern QueueHandle_t qFromMesh ;
extern QueueHandle_t qToWeb ;

// for web socket event handler
extern QueueHandle_t qFromWeb;
extern uint8_t currentId;       // the current packet id

extern long maxTimeOnAir;

// ISRs
/// @brief LoRa RX interrupt handler
extern "C" void RxISR(void);

/// @brief LoRa TX interrupt handler
extern "C" void TxISR(void);

/// @brief User button press ISR
extern "C" void buttonISR(void);

extern "C" void ledBlinker( void * pvParameter);
#endif // FLORANET_UTILS_H