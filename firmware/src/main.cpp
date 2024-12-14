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


// ---------------- MAIN ---------------- //

void setup() {
  #ifdef DEBUG
  Serial.begin(SERIAL_BAUD);
  #endif
  pinMode(NEW_MESSAGE_LED, OUTPUT);
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(NEW_MESSAGE_LED, HIGH);
    delay(200);
    digitalWrite(NEW_MESSAGE_LED, LOW);
    delay(200);
  }
  FloraNet *floranet = new FloraNet();
  xEventGroupClearBits(xEventGroup, (EVENTBIT_PROTO_SLEEP_READY | EVENTBIT_WEB_SLEEP_READY));
  floranet->run();
}
void loop() {
  
}