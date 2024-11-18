/* 
======   AVAlink Firmware  ======

Version: v1.0 (beta)
Last Update: under development

======   DESCRIPTION   =====

Firmware for AVAlink nodes. 
To configure the firmware, follow the instructions in lib/AVAlinkConfig/ConfigOptions.h

*/

// ---------------- INCLUDES ---------------- //

#include <AVAlink.h>

// ---------------- CLASSES ---------------- //


// ---------------- GLOBALS ---------------- //
TaskHandle_t xLoraTask;
TaskHandle_t xWebTask;
// ---------------- PROTOTYPES ---------------- //


// ---------------- MAIN ---------------- //

void setup() {
  
  initAvalink(); // initialize hardware

  xTaskCreatePinnedToCore(loraTask, "Lora", STACK_SIZE, (void *)1, 0, &xLoraTask, CORE_LORA); 
  xTaskCreatePinnedToCore(webTask, "Web", STACK_SIZE, (void *) 1, 0, &xWebTask, CORE_WEB);
}
void loop() {
  
}