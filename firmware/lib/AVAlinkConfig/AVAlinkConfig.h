// Config preprocessor logic based on AVAlinkConfig.h

#ifndef AVALINK_CONFIG_H
#define AVALINK_CONFIG_H
#include <ConfigOptions.h>


// DEBUGGING

#ifdef DEBUG
#define SERIAL_BAUD 115200
#define DBG_PRINTLN(MSG) Serial.println(MSG)
#else
#define DBG_PRINTLN(MSG)
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
#define LORA_IRQ 39
#define LORA_NRST 48
#define LORA_BUSY 33
#define LORA_MOSI 35
#define LORA_MISO 37
#define LORA_SCK 36

#endif

// Lora

#ifdef LORA_DEV_SX1262_HF

#define LORA_FREQ 915.0 // MHz
#define LORA_BW 250.0   // kHz
#define LORA_SF 11
#define LORA_CR 5
#define LORA_SYNC 0x34
#define LORA_POWER 17  // dBm
#define LORA_PREAMB 16 // symbols
#endif

#endif