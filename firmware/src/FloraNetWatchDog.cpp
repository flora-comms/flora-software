#include <FloraNetWatchDog.h>
void IRAM_ATTR buttonISR() {
  // DO stuff
}

void sleepRoutine() {
  // remove button IRQ and semtech IRQ
  radio.clearDio1Action();
  // button ISR???
  // set wake up sources
  pinMode(LORA_IRQ, INPUT);
  pinMode(USER_BTN, INPUT_PULLUP);
  gpio_wakeup_enable((gpio_num_t)LORA_IRQ, GPIO_INTR_HIGH_LEVEL);
  gpio_wakeup_enable((gpio_num_t)USER_BTN, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
}
void wakeRoutine() {
  // Check wake source
  esp_sleep_get_wakeup_cause();

  // handle button wakeup
  // handle IRQ wakeup
}

void watchDogTask(void *) {

  attachInterrupt(digitalPinToInterrupt(USER_BTN), buttonISR, FALLING);
  EventBits_t xSleepBits;

  while (true) {
    // wait for sleep ready events on WEB_SLEEP, PROTO_SLEEP and LORA_SLEEP
    xSleepBits = xEventGroupWaitBits(xAvalinkEventGroup,
                                     EVENTBIT_WEB_SLEEP_READY |
                                         EVENTBIT_PROTO_SLEEP_READY |
                                         EVENTBIT_LORA_SLEEP,
                                     pdTRUE, pdFALSE, portMAX_DELAY);

    // WEB AND PROTO ready for sleep -> set prep bit
    if ((xSleepBits &
         (EVENTBIT_WEB_SLEEP_READY | EVENTBIT_PROTO_SLEEP_READY)) ==
        (EVENTBIT_WEB_SLEEP_READY | EVENTBIT_PROTO_SLEEP_READY)) {
      // Set Eventbit to prep LORA_TASK for sleep
      xEventGroupSetBits(xAvalinkEventGroup, EVENTBIT_PREP_SLEEP);
    }
    // LORA ready for sleep -> Check WEB and PROTO
    else if ((xSleepBits & EVENTBIT_LORA_SLEEP) != 0) {

      // LORA == true  & ((WEB & PROTO) == true)
      if ((xSleepBits &
           (EVENTBIT_WEB_SLEEP_READY | EVENTBIT_PROTO_SLEEP_READY)) ==
          (EVENTBIT_WEB_SLEEP_READY | EVENTBIT_PROTO_SLEEP_READY)) {
        // ALL Sleep EVENTS READY -> CHECK IRQ
        if (digitalRead(LORA_IRQ) == HIGH) // Check interrupts?
        {
          // reset all sleep EVENT_BITS
          xEventGroupClearBits(xAvalinkEventGroup,
                               EVENTBIT_WEB_SLEEP_READY |
                                   EVENTBIT_PROTO_SLEEP_READY |
                                   EVENTBIT_LORA_SLEEP);
          continue; // Back to start
        } else {
          // GO TO SLEEP
          sleepRoutine();
          esp_light_sleep_start();
          wakeRoutine();
        }
      }

      // LORA == true & ((WEB & PROTO) == false)
      else {
        // clear(LORA_SLEEP)
        // if lora_irq pin == high { set LORA_RX_READY, clear PROTO_SLEEP)
        // re-add rxIrq() to DIO1
        continue; // Back to start
      }
    }
  }
}
