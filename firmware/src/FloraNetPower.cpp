#include <FloraNetPower.h>

void FloraNetPower::handleSleep()
{
    xEventGroupSetBits(xEventGroup, EVENTBIT_PREP_SLEEP);
    EventBits_t eb = xEventGroupWaitBits(xEventGroup, EVENTBIT_LORA_SLEEP_READY, false, true, portMAX_DELAY);
    // if web and proto tasks are no longer ready to sleep
    if((eb & (EVENTBIT_WEB_SLEEP_READY | EVENTBIT_PROTO_SLEEP_READY)) != (EVENTBIT_WEB_SLEEP_READY | EVENTBIT_PROTO_SLEEP_READY))
    {
        // clear lora sleep, and deal with rx isr
        xEventGroupClearBits(xEventGroup, EVENTBIT_LORA_SLEEP_READY);
        if (digitalRead(LORA_IRQ) == HIGH)
        {
            xEventGroupSetBits(xEventGroup, EVENTBIT_LORA_RX_READY);
        } else {
            _radio->setDio1Action(RxISR);
        }
        return;
    }
    // make sure we haven't receieved a message after we removed the rx isr
    if(digitalRead(LORA_IRQ) == HIGH)
    {  
        // Restart the lora and proto tasks
        xEventGroupClearBits(xEventGroup, EVENTBIT_LORA_SLEEP_READY | EVENTBIT_PROTO_SLEEP_READY);
        xEventGroupSetBits(xEventGroup, EVENTBIT_LORA_RX_READY);
        taskYIELD();
        return;
    }

    // if we are still good to sleep,

    // remove buttonISR, set wakeup sources and go to sleep
    // Remove Interupt from button
    detachInterrupt(USER_BUTTON);

    // suspend all tasks
    vTaskSuspend(_tskLora);
    vTaskSuspend(_tskProto);
    vTaskSuspend(_tskWeb);
    
    // setup LORA_IRQ
    pinMode(LORA_IRQ, INPUT);
    gpio_wakeup_enable((gpio_num_t)LORA_IRQ, GPIO_INTR_HIGH_LEVEL); // wake from semtech using GPIO
    esp_sleep_enable_gpio_wakeup();
    // setup USER_BTN
    rtc_gpio_pullup_en((gpio_num_t)USER_BUTTON);
    rtc_gpio_pulldown_dis((gpio_num_t)USER_BUTTON);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)USER_BUTTON, 0); // wake from button using EXT0

    // GO TO SLEEP
    DBG_PRINTLN("GOING TO SLEEP!");
    #ifdef DEBUG
    vTaskDelay(pdMS_TO_TICKS(200));
    #endif
    esp_light_sleep_start();
    DBG_PRINTLN("Woke up!");
    #ifdef DEBUG
        vTaskDelay(pdMS_TO_TICKS(200));
    #endif
    // determine wakeup cause
    esp_sleep_wakeup_cause_t wakeCause = esp_sleep_get_wakeup_cause();

    // if caused by button
    if (wakeCause = ESP_SLEEP_WAKEUP_EXT0) // Button wakeup
    {
        xEventGroupClearBits(xEventGroup, 
        EVENTBIT_WEB_SLEEP_READY | EVENTBIT_PROTO_SLEEP_READY | EVENTBIT_LORA_SLEEP_READY);

        // check if we've received a lora message
        if (digitalRead(LORA_IRQ) == HIGH)
        {
            xEventGroupSetBits(xEventGroup, EVENTBIT_LORA_RX_READY);
        }
        else
        {
            _radio->setDio1Action(RxISR); // Re-attach interupt
        }
        // resume tasks
        CRITICAL_SECTION(
            vTaskResume(_tskProto);
            vTaskResume(_tskWeb);
            vTaskResume(_tskLora))
        taskYIELD();
        return;
    }

    // otherwise
    xEventGroupClearBits(       // clear proto and lora sleep event bits
        xEventGroup,
        (EVENTBIT_LORA_SLEEP_READY | EVENTBIT_PROTO_SLEEP_READY));
    xEventGroupSetBits(xEventGroup, EVENTBIT_LORA_RX_READY);    // let lora know there's a new message
    
    // resume the lora and protocol tasks
    CRITICAL_SECTION(
        vTaskResume(_tskLora);
        vTaskResume(_tskProto)
    )
    taskYIELD();
    return;
}
void FloraNetPower::run()
{
    while (true)
    {
        xEventGroupWaitBits(
            xEventGroup,
            (EVENTBIT_PROTO_SLEEP_READY | EVENTBIT_WEB_SLEEP_READY),
            false,
            true,
            portMAX_DELAY);
        handleSleep();
    }
}