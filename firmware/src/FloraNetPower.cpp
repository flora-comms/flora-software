#include <FloraNetPower.h>

void FloraNetPower::handleSleep()
{
    xEventGroupSetBits(xEventGroup, EVENTBIT_PREP_SLEEP);
    EventBits_t eb = xEventGroupWaitBits(xEventGroup, EVENTBIT_LORA_SLEEP_READY, false, true, portMAX_DELAY);
    xEventGroupClearBits(xEventGroup, EVENTBIT_LORA_SLEEP_READY);
    // if web and proto tasks are no longer ready to sleep
    if((eb & (EVENTBIT_WEB_SLEEP_READY | EVENTBIT_PROTO_SLEEP_READY)) != (EVENTBIT_WEB_SLEEP_READY | EVENTBIT_PROTO_SLEEP_READY))
    {
        // clear lora sleep, and deal with rx isr
        if (digitalRead(LORA_IRQ) == HIGH)
        {
            xEventGroupSetBits(xEventGroup, EVENTBIT_LORA_RX_READY);
        } else {
            radio.setDio1Action(RxISR);
        }
        return;
    }
    // make sure we haven't receieved a message after we removed the rx isr
    if(digitalRead(LORA_IRQ) == HIGH)
    {  
        // Restart the lora and proto tasks
        xEventGroupClearBits(xEventGroup, EVENTBIT_PROTO_SLEEP_READY);
        xEventGroupSetBits(xEventGroup, EVENTBIT_LORA_RX_READY);
        return;
    }
    // or that the button has been pressed
    if((xEventGroupGetBits(xEventGroup) & EVENTBIT_WEB_REQUESTED) == EVENTBIT_WEB_REQUESTED)
    {
        return;
    }

    // if we are still good to sleep,
    // remove buttonISR, set wakeup sources and go to sleep
    // Remove Interupt from button
    DETACH_BUTTONISR();
    
    // setup LORA_IRQ wakeup
    pinMode(LORA_IRQ, INPUT);
    gpio_wakeup_enable((gpio_num_t)LORA_IRQ, GPIO_INTR_HIGH_LEVEL);     // wake from semtech on high level
    esp_sleep_enable_gpio_wakeup();
    // setup USER_BTN
    esp_sleep_enable_ext0_wakeup((gpio_num_t)USER_BUTTON, 0);
    rtc_gpio_pulldown_dis((gpio_num_t)USER_BUTTON);
    rtc_gpio_pullup_en((gpio_num_t)USER_BUTTON);

    // GO TO SLEEP
    DBG_PRINTLN("GOING TO SLEEP!");

    esp_sleep_wakeup_cause_t wakeCause;

#ifdef FLASH_ON_NEW_MESSAGE

    bool blinkLed = false;

    // if theres a new message, we need to blink the led until the web server is activated.
    if (xEventGroupGetBits(xEventGroup) & EVENTBIT_NEW_MESSAGE)
    {
        blinkLed = true;

        // setup the gpio to drive the led
        pinMode(NEW_MESSAGE_LED, OUTPUT);
        digitalWrite(NEW_MESSAGE_LED, LOW);

        // setup timer wakeup
        esp_sleep_enable_timer_wakeup(LED_BLINK_PERIOD_US);
    }

    
    
    do {

        esp_light_sleep_start();    // go to sleep

        wakeCause = esp_sleep_get_wakeup_cause();   // get the wake cause

        // if it was just the timer wakeup, we want to flash the green led and go back to sleep
        if (wakeCause == ESP_SLEEP_WAKEUP_TIMER)
        {
            digitalWrite(NEW_MESSAGE_LED, HIGH);
            vTaskDelay(pdMS_TO_TICKS(100));         // delay 100 ms
            digitalWrite(NEW_MESSAGE_LED, LOW);
        } else {
            blinkLed = false;
        }
        
    } while (blinkLed);
    #else

    esp_light_sleep_start();
    #endif
    
    DBG_PRINTLN("Woke up!");
    
    // if caused by a button press
    if (wakeCause == ESP_SLEEP_WAKEUP_EXT0)
    {
        xEventGroupClearBits(xEventGroup, EVENTBIT_WEB_SLEEP_READY); // prevent immediate sleep on return.        
        xEventGroupSetBits(xEventGroup, EVENTBIT_WEB_REQUESTED);        // let the web server know its ready to go
        // check if we've received a lora message
        if (digitalRead(LORA_IRQ) == HIGH)
        {
            xEventGroupSetBits(xEventGroup, EVENTBIT_LORA_RX_READY);
        }
        else
        {
            radio.setDio1Action(RxISR); // Re-attach interupt
        }
    } else {        // if caused by lora irq
        ATTACH_BUTTONISR();
        xEventGroupClearBits(xEventGroup, EVENTBIT_PROTO_SLEEP_READY);      // prevent sleep routine call on return
        xEventGroupSetBits(xEventGroup, EVENTBIT_LORA_RX_READY);            // signal lora task to deal with stuff
    }
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
        YIELD();
    }
}

void powerTask( void * pvParameter )
{
    FloraNetPower *handler = static_cast<FloraNetPower *>(pvParameter);
    handler->run(); // run the handler. should never return
    delete handler; // if it does, delete it and the task?
    vTaskDelete(NULL);
    return;
}