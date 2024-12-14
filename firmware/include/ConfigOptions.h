/*
======   FloraNet Firmware Config  ======

Version: v2.0
Last Update: Dec 09 2024

======   DESCRIPTION   =====

Configuration options for building and flashing FloraNet firmware.
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


/*

================================

FIRMWARE

Variables for firmware configuration
*/

#define WIFI_SSID "FloraNet" // WiFi network name

/*

======================================

DEBUGGING

Comment out to remove debug functionality

*/
//#define DEBUG
//#define USE_CS
//#define POWER_SAVER
//#define TEST_SLEEP
//#define FLASH_ON_NEW_MESSAGE
#define USE_NVS
//#define WS_KEEP_ALIVE
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
