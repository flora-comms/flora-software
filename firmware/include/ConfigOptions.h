/*
======   AVAlink Firmware Config  ======

Version: v1.0
Last Update: Nov 6 2024

======   DESCRIPTION   =====

Configuration options for building and flashing AVAlink firmware.
For hardware details such as pin numbers, bandwidth, etc., or to add your own
custom hardware values, see HardwareConfig.h
*/

#ifndef CONFIG_OPTIONS_H

#define CONFIG_OPTIONS_H

/*

=============================

ENVIRONMENT

Uncomment define for the IDE uses. MAKE SURE ONLY ONE IS UNCOMMENTED

*/

#define ENV_PLATFORMIO
// #define ENV_ARDUINO

#ifdef ENV_PLATFORMIO
#include <Arduino.h>
#endif
/*

================================

FIRMWARE

Variables for firmware configuration
*/
#define WIFI_SSID "FloraNet Noyse" // WiFi network name
#define WEBSERVER_DNS                                                          \
  "floranet" // puts domain at "http://{WEBSERVER_DNS}.local"
#define WEBSOCKET_ENDPOINT                                                     \
  "/chat" // puts websocket at "ws://{WEBSERVER_DNS}/{WEBSOCKET_ENDPOINT}"
#define MAX_LORA_TTL 4  // maximum TTL hop count
#define QUEUE_LENGTH 10 // freertos queue length
#define STACK_SIZE 8192 // stack size for each task

/*

======================================

DEBUGGING

Comment out to remove debug functionality

*/

#define DEBUG

/*

=================================

HARDWARE

Options for Hardware
*/

// ----- BOARD OPTIONS ------ //

//#define LILYGO_T3_S3_V1_2
#define PETAL_V0_0

// ------ TRANSCIEVER DEVICE OPTIONS ----- //

#define LORA_DEV_SX1262_HF

// ------ LORA MODE OPTIONS ------- //

//#define LORA_MODE_SHORT_TURBO // 21.88 kbps

// #define LORA_MODE_SHORT_FAST     // 10.94 kbps

// #define LORA_MODE_MED_FAST       // 3.52 kbps

// #define LORA_MODE_MED_SLOW       // 1.95 kbps

#define LORA_MODE_LONG_FAST      // 1.07 kbps

// #define LORA_MODE_LONG_MOD       // 0.34 kbps

// #define LORA_MODE_LONG_SLOW      // 0.18 kbps

// #define LORA_MODE_V_LONG_SLOW    // 0.09 kbps

#endif
