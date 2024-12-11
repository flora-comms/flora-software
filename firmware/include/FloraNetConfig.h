// Config preprocessor logic based on FloraNetConfig.h

// mess with this at your own risk

#ifndef FLORANET_CONFIG_H
#define FLORANET_CONFIG_H

#include <FloraNetDebug.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <FreeRTOS.h>
#include <SD.h>
#include <WiFi.h>
#include <SPI.h>
#include <RadioLib.h>

// Web server
#define WEBSERVER_DNS "floranet"   // puts domain at "http://{WEBSERVER_DNS}.local"
#define WEBSOCKET_ENDPOINT "/chat" // puts websocket at "ws://{WEBSERVER_DNS}/{WEBSOCKET_ENDPOINT}"

// FREERTOS

// Event Group

#define EVENTBIT_LORA_TX_READY      0x1     // 00000000000010
#define EVENTBIT_LORA_TX_DONE       0x2     // 00000000000100
#define EVENTBIT_LORA_RX_READY      0x4     // 00000000001000
#define EVENTBIT_LORA_RX_DONE       0x8     // 00000000010000
#define EVENTBIT_WEB_TX_READY       0x10    // 00000000100000
#define EVENTBIT_WEB_RX_DONE        0x20    // 00000001000000
#define EVENTBIT_LORA_SLEEP_READY   0x40    // 00000010000000
#define EVENTBIT_WEB_SLEEP_READY    0x80    // 00000100000000
#define EVENTBIT_PROTO_SLEEP_READY  0x100   // 00001000000000
#define EVENTBIT_PREP_SLEEP         0x200   // 00010000000000
#define EVENTBIT_WEB_REQUESTED      0x400   // 00100000000000
#define EVENTBIT_SOCKET_ACTION      0x800   // 01000000000000
#define EVENTBIT_RETRY_READY        0x1000  // 01000000000000
#define EVENTBIT_WEB_TIMEOUT        0x2000  // 10000000000000

// queues and stacks
#define QUEUE_LENGTH 10 // freertos queue length
#define STACK_SIZE 8192 // stack size for each task
#define MAX_TICKS_TO_WAIT   10     // maximum ticks to wait when writing to queues

// task priorities
#define TASK_PRIORITY_LORA  configTIMER_TASK_PRIORITY + 4
#define TASK_PRIORITY_PROTO configTIMER_TASK_PRIORITY + 3
#define TASK_PRIORITY_WEB   configTIMER_TASK_PRIORITY + 2
#define TASK_PRIORITY_POWER configTIMER_TASK_PRIORITY + 1

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

#define USER_BUTTON 14

#endif

// S3 processor options

#define CORE_LORA   0

#define CORE_WEB    0

// LORA CONFIG

#define LORA_FREQ 915.0 // MHz
#define LORA_SYNC 0x34  // sync word
#define LORA_POWER 17   // tx power in dBm
#define LORA_PREAMB 16  // # of symbols in preamble
#define MAX_LORA_TTL 4  // maximum TTL hop count

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
#define RETRY_INTERVAL_MAX  60          // maximum number of seconds to wait for a retry
#define RETRY_INTERVAL_MIN  30          // minimum number of seconds to wait for a retry
#define RETRY_INTERVAL  30, 60  // between 30s and 1 min

// WEB CONFIG
#define HISTORY_FILENAME    "/data/history.csv"

#ifdef TEST_SLEEP
#define WEB_TIMEOUT         30000       // the web timeout in ms
#else
#define WEB_TIMEOUT         300000      // the web timeout in ms (default 5 min)
#endif
// GLOBAL VARIABLES


#endif