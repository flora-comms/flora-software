/* 
======   FloraNet Firmware  ======

Version: v1.0 (beta)
Last Update: under development

======   DESCRIPTION   =====

Firmware for Petal Radio nodes. 
To configure the firmware, follow the instructions in include/ConfigOptions.h

*/

// ---------------- INCLUDES ---------------- //

#include <FloraNet.h>
// ---------------- CLASSES ---------------- //


// ---------------- GLOBALS ---------------- //
TaskHandle_t xLoraTask;
TaskHandle_t xWebTask;
// ---------------- PROTOTYPES ---------------- //


// ---------------- MAIN ---------------- //

void setup() {
  
  initFloraNet(); // initialize hardware

  xTaskCreatePinnedToCore(loraTask, "Lora", STACK_SIZE, (void *)1, 1, &xLoraTask, CORE_LORA); 
  xTaskCreatePinnedToCore(webTask, "Web", STACK_SIZE, (void *) 1, 0, &xWebTask, CORE_WEB);
}
void loop() {
  
}