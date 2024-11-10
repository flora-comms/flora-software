/* 
======   AVAlink Firmware  ======

Version: v1.0 (beta)
Last Update: under development

======   DESCRIPTION   =====

Firmware for AVAlink nodes. 
To configure the firmware, follow the instructions in lib/AVAlinkConfig/ConfigOptions.h

*/

// ---------------- INCLUDES ---------------- //

#ifdef ENV_PLATFORMIO
#include <Arduino.h>
#endif

#include <AVAlink.h>

// ---------------- CLASSES ---------------- //


// ---------------- GLOBALS ---------------- //

volatile bool lora_flag = false;
int           tx_state = RADIOLIB_ERR_NONE;
// ---------------- PROTOTYPES ---------------- //


// ---------------- MAIN ---------------- //

void setup() {
  
  initAvalink(); // initialize hardware

  TaskHandle_t *tskLora;
  TaskHandle_t *tskWeb;

  xTaskCreatePinnedToCore(loraTask, "Lora", STACK_SIZE, (void *)1, 0, tskLora, 0); 
  xTaskCreatePinnedToCore(webTask, "Web", STACK_SIZE, (void *) 1, 0, tskWeb, 0);
}
void loop() {
  
}


