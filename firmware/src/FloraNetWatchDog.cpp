#include <FloraNetWatchDog.h>
void IRAM_ATTR buttonISR() {
  xEventGroupSetBits(xAvalinkEventGroup, EVENTBIT_WEB_REQUESTED);
}

void sleepRoutine() {
  // Remove Interupt from button
  detachInterrupt(USER_BTN);
  // setup LORA_IRQ
  pinMode(LORA_IRQ, INPUT);
  gpio_wakeup_enable((gpio_num_t)LORA_IRQ,
                     GPIO_INTR_HIGH_LEVEL); // wake from semtech using GPIO
  esp_sleep_enable_gpio_wakeup();
  // setup USER_BTN
  rtc_gpio_pullup_en((gpio_num_t)USER_BTN);
  rtc_gpio_pulldown_dis((gpio_num_t)USER_BTN);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)USER_BTN,
                               0); // wake from button using EXT0
}
void wakeRoutine() {
  // Check wake source
  esp_sleep_wakeup_cause_t wakeCause = esp_sleep_get_wakeup_cause();
  if (wakeCause = ESP_SLEEP_WAKEUP_EXT0) // Button wakeup
  {
    xEventGroupClearBits(xAvalinkEventGroup, EVENTBIT_WEB_SLEEP_READY |
                                                 EVENTBIT_PROTO_SLEEP |
                                                 EVENTBIT_LORA_SLEEP);
    if (digitalRead(LORA_IRQ) == HIGH) {
      xEventGroupSetBits(xAvalinkEventGroup, EVENTBIT_RX_READY);
    } else {
      radio.setDio1Action(onRxIrq); // Re-attach interupt
    }
    vTaskResume(xLoraTask);
    vTaskResume(xWebTask);
    vTaskResume(xProtoTask);
  } else
    (wakeCause = ESP_SLEEP_WAKEUP_GPIO) // IRQ WAkeup
    {
      xEventGroupClearBits(xAvalinkEventGroup,
                           EVENTBIT_PROTO_SLEEP | EVENTBIT_LORA_SLEEP);
      xEventGroupSetBits(xAvalinkEventGroup, EVENTBIT_RX_READY);
      vTaskResume(xLoraTask);
      vTaskResume(xProtoTask);
    }
}

void watchDogTask(void *) {

  EventBits_t xSleepBits;

  while (true) {
    // wait for sleep ready events on WEB_SLEEP, PROTO_SLEEP and LORA_SLEEP
    xEventGroupWaitBits(xAvalinkEventGroup,
                        EVENTBIT_WEB_SLEEP_READY | EVENTBIT_PROTO_SLEEP_READY,
                        pdFALSE, pdTRUE, portMAX_DELAY);
    // Attach interupt to handle button press in this period
    attachInterrupt(digitalPinToInterrupt(USER_BTN), buttonISR, FALLING);
    // Prep LORA_TASK for sleep
    xEventGroupSetBits(xAvalinkEventGroup, EVENTBIT_PREP_SLEEP);
    // WAIT ON LORA_TASK, ONLY when WEB and PROTO are ready
    xSleepBits = xEventGroupWaitBits(xAvalinkEventGroup, EVENTBIT_LORA_SLEEP,
                                     pdFALSE, pdTRUE, portMAX_DELAY);
    // Ensure WEB and PROTO are still ready to sleep
    if ((xSleepBits &
         (EVENTBIT_WEB_SLEEP_READY | EVENTBIT_PROTO_SLEEP_READY)) ==
        (EVENTBIT_WEB_SLEEP_READY | EVENTBIT_PROTO_SLEEP_READY)) {
      if ((digitalRead(LORA_IRQ) == HIGH)) {
        // MESSAGE CAME IN
        xEventGroupClearBits(xAvalinkEventGroup,
                             EVENTBIT_PROTO_SLEEP_READY | EVENTBIT_LORA_SLEEP);
        // set RX ready
        xEventGroupSetBits(xAvalinkEventGroup, EVENTBIT_LORA_RX_READY);
      } else {
        // get ready for sleep
        sleepRoutine();
        DBG_PRINTLN("Going to sleep");
        esp_light_sleep_start();
        DBG_PRINTLN("I woke UP!");
        wakeRoutine();
      }
    } else // WEB OR PROTO NOT READY
    {
      xEventGroupClearBits(xAvalinkEventGroup, EVENTBIT_LORA_SLEEP);
      if ((digitalRead(LORA_IRQ) == HIGH)) {
        xEventGroupSetBits(xAvalinkEventGroup, EVENTBIT_LORA_RX_READY);
      } else {
        radio.setDio1Action(onRxIrq);
      }
    }
  }
}