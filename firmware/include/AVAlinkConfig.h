// Config preprocessor logic based on AVAlinkConfig.h

#ifndef AVALINK_CONFIG_H
#define AVALINK_CONFIG_H

#include <ConfigOptions.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <FreeRTOS.h>
#include <SD.h>
#include <WiFi.h>
#include <SPI.h>
#include <RadioLib.h>

// FREERTOS

// event bits
#define EVENTBIT_LORA_Q 0x1         // 00000001
#define EVENTBIT_LORA_TX 0x2        // 00000010
#define EVENTBIT_LORA_RX 0x4        // 00000100
#define EVENTBIT_LORA_CAD 0x8       // 00001000
#define EVENTBIT_WEB_READY  0x10    // 00010000

// queues and stacks
#define QUEUE_LENGTH 10 // freertos queue length
#define STACK_SIZE 8192 // stack size for each task

// DEBUGGING

#ifdef DEBUG
#define SERIAL_BAUD 115200
#define DBG_PRINTLN(MSG)            Serial.println(MSG)
#define DBG_PRINT(MSG)              Serial.print(MSG)
#define DBG_PRINTF(format, args...)  Serial.printf(format, args)
#else
#define DBG_PRINTLN(MSG)
#define DBG_PRINT(MSG)
#define DBG_PRINTF(format, args...)
#endif

// HARDWARE

// Boards

// LILYGO T3 S3 V1.2
#ifdef LILYGO_T3_S3_V1_2
#define SD_CS 13   // SD chip select pin
#define SD_SCK 14  // SD sck pin
#define SD_MOSI 11 // SD mosi pin
#define SD_MISO 2  // SD miso

#define LORA_NSS 7
#define LORA_IRQ 33
#define LORA_NRST 8
#define LORA_BUSY 34
#define LORA_MOSI 6
#define LORA_MISO 3
#define LORA_SCK 5
#endif

// Petal V0.0
#ifdef PETAL_V0_0

#define SD_CS 10   // SD chip select pin
#define SD_SCK 12  // SD sck pin
#define SD_MOSI 11 // SD mosi pin
#define SD_MISO 13 // SD miso

#define LORA_NSS 34
#define LORA_IRQ 38
#define LORA_NRST 48
#define LORA_BUSY 33
#define LORA_MOSI 35
#define LORA_MISO 37
#define LORA_SCK 36

#endif

// S3 processor options

#define CORE_LORA   0

#define CORE_WEB    0

// LORA CONFIG

#define LORA_FREQ 915.0 // MHz
#define LORA_SYNC 0x34  // sync word
#define LORA_POWER 17   // tx power in dBm
#define LORA_PREAMB 16  // # of symbols in preamble


// lora modes

#ifdef LORA_MODE_SHORT_TURBO
#define LORA_BW                     500.0   // kHz
#define LORA_SF                     7       
#define LORA_CR                     5
#define LORA_TX_WAIT_INTERVAL_MAX  15 
#endif

#ifdef LORA_MODE_SHORT_FAST
#define LORA_BW                     250.0 // kHz
#define LORA_SF                     7
#define LORA_CR                     5
#define LORA_TX_WAIT_INTERVAL_MAX   15
#endif

#ifdef LORA_MODE_MED_FAST
#define LORA_BW                     250.0 // kHz
#define LORA_SF                     9
#define LORA_CR                     5
#define LORA_TX_WAIT_INTERVAL_MAX   10
#endif

#ifdef LORA_MODE_MED_SLOW
#define LORA_BW                     250.0 // kHz
#define LORA_SF                     10
#define LORA_CR                     5
#define LORA_TX_WAIT_INTERVAL_MAX   10
#endif

#ifdef LORA_MODE_LONG_FAST
#define LORA_BW                     250.0 // kHz
#define LORA_SF                     11
#define LORA_CR                     5
#define LORA_TX_WAIT_INTERVAL_MAX   5
#endif

#ifdef LORA_MODE_LONG_MOD
#define LORA_BW                     125.0 // kHz
#define LORA_SF                     11
#define LORA_CR                     8
#define LORA_TX_WAIT_INTERVAL_MAX   5
#endif

#ifdef LORA_MODE_LONG_SLOW
#define LORA_BW                     125.0 // kHz
#define LORA_SF                     12
#define LORA_CR                     8
#define LORA_TX_WAIT_INTERVAL_MAX   5
#endif

#ifdef LORA_MODE_V_LONG_SLOW
#define LORA_BW                     62.5   // kHz
#define LORA_SF                     12
#define LORA_CR                     8
#define LORA_TX_WAIT_INTERVAL_MAX   5
#endif

// p2p layer config
#define MAX_LORA_TTL 4                  // maximum TTL hop count
#define ACKNOWLEDGE_WINDOW_SIZE 16      // number of messages that we'll keep track of in the log list
#define RETRY_THRESHOLD         1       // The index in the LogList when a message should be retried.

// WEB CONFIG
#define HISTORY_FILENAME "/data/history.csv"

// GLOBAL VARIABLES

extern QueueHandle_t        qToMesh;    // the queue from the web task to the lora task
extern QueueHandle_t        qToWeb;     // the queue from the lora task to the web task
extern bool                 bApIsUp;    // is the wifi ap up?
extern SX1262               radio;
extern SPIClass             sdSPI;
extern SPIClass             loraSPI;
extern EventGroupHandle_t   xAvalinkEventGroup;
extern AsyncWebServer       server;
extern AsyncWebSocket       ws;
extern RadioLibTime_t       xMaxTimeOnAir;
extern TaskHandle_t         xLoraTask;
extern TaskHandle_t         xWebTask;
#endif