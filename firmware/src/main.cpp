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
#include <PowerDrawTest.h>

// ---------------- MAIN ---------------- //

void setup() {
  #ifdef DEBUG
  Serial.begin(SERIAL_BAUD);
  #endif
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(18, OUTPUT);
  for (int i = 0; i < 3; i++)
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
  #ifdef POWER_DRAW_TEST_SLAVE
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  SD.begin(SD_CS);
  File file = SD.open("crash-report.csv", FILE_APPEND);
  file.println("crash\n");
  file.close();
  SD.end();
  sdSPI.end();
#endif
  FloraNet *floranet = new FloraNet();
  xEventGroupClearBits(xEventGroup, (EVENTBIT_PROTO_SLEEP_READY | EVENTBIT_WEB_SLEEP_READY));
  floranet->run();
  #ifdef POWER_DRAW_TEST_MASTER
  TaskHandle_t tskDrawTest;
  xTaskCreatePinnedToCore(PowerDrawTask, "draw", STACK_SIZE, (void *)1, TASK_PRIORITY_WEB, &tskDrawTest, 0);
  #endif
}
void loop() {
  
}