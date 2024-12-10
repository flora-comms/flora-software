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
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(18, OUTPUT);
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(16, HIGH);
    digitalWrite(17, HIGH);
    digitalWrite(18, HIGH);
    delay(500);
    digitalWrite(16, LOW);
    digitalWrite(17, LOW);
    digitalWrite(18, LOW);
    delay(500);
  }
  FloraNet *floranet = new FloraNet();
  xEventGroupClearBits(xEventGroup, (EVENTBIT_PROTO_SLEEP_READY | EVENTBIT_WEB_SLEEP_READY));
  floranet->run();
}
void loop() {
  
}