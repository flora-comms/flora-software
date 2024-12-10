#include <FloraNetRadio.h>

// private

void FloraNetRadio::startRx() {
    // clear the rx-ready event bit
    xEventGroupClearBits(xEventGroup, EVENTBIT_LORA_RX_READY);
    // attach the rx isr to DIO1 and start recieving
    CRITICAL_SECTION(
        radio.setDio1Action(RxISR);
        radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_NONE)   // single rx mode
        )
}
void FloraNetRadio::handleTx() {

    // recieve the message
    Message * msg;
    xQueueReceive(qToMesh, &msg, MAX_TICKS_TO_WAIT);
    DBG_PRINTF("RhT:%i-%i", msg->senderId, msg->packetId);
    uint8_t buf[256];
    uint16_t len = msg->toLoraPacket(buf);

    // start transmitting
    CRITICAL_SECTION(
        int status = radio.startTransmit(buf, len)
    )

    // check for issues
    if (status != RADIOLIB_ERR_NONE) {
        DBG_PRINTF("LTX failed. Code %i", status);
        return;
    } 

    DBG_PRINTLN("LTX suc");

    // wait for the transmission to finish
    xEventGroupWaitBits(xEventGroup, EVENTBIT_LORA_TX_DONE, true, true, MAX_TICKS_TO_WAIT);
    xEventGroupClearBits(xEventGroup, EVENTBIT_LORA_TX_DONE);
}

void FloraNetRadio::initLora() {
    Serial.println("Initializing SX1262...");
    loraSPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
    CRITICAL_SECTION
    (
        int status = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC, LORA_POWER, LORA_PREAMB)
    )

    if (status == RADIOLIB_ERR_NONE)
    {
        DBG_PRINT("success!\n");
    }
    else
    {
        DBG_PRINTF("Failed: code %i\n", status);
        return;
    }
    radio.setCurrentLimit(60.0);
    radio.setDio2AsRfSwitch(true);
    radio.explicitHeader();
    radio.setCRC(2);
    randomSeed(radio.getRSSI());
    
}

void FloraNetRadio::handleRx() {

    uint8_t rx_data[256] = {0};              // create buffer of zeros

    // read the received data into the buffer
    CRITICAL_SECTION 
    (
        int status = radio.readData(rx_data, 0)
    )

    

    // start receiving again
    startRx();

    if (!(status == RADIOLIB_ERR_NONE))
    {
        DBG_PRINTLN("Lora RX failure");
    }

    Message *pxRxMsg = new Message(rx_data);

    DBG_PRINTF("RhR:%i-%i",pxRxMsg->senderId, pxRxMsg->packetId);

    xQueueSend(qFromMesh, &pxRxMsg, MAX_TICKS_TO_WAIT);

    xEventGroupSetBits(xEventGroup, EVENTBIT_WEB_RX_DONE); // notify protocol task

    return;
}    

void FloraNetRadio::prepForSleep() {
    while (uxQueueMessagesWaiting(qToMesh) != 0) 
    {
        handleTx();
    }

    // clear flags and start receiving with no dio1 action
    xEventGroupClearBits(xEventGroup, EVENTBIT_LORA_RX_READY);
    radio.clearDio1Action();
    CRITICAL_SECTION(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_NONE))
    // set lora sleep
    xEventGroupSetBits(xEventGroup, EVENTBIT_LORA_SLEEP_READY);
}

// public

void FloraNetRadio::run() {
    initLora();
    maxTimeOnAir = radio.getTimeOnAir(255) / 1000;
    xEventGroupClearBits(xEventGroup, (EVENTBIT_LORA_RX_READY | EVENTBIT_LORA_TX_READY));
    while (true) {
        // wait for an event
        EventBits_t eventBits = xEventGroupWaitBits(xEventGroup,
                                    (EVENTBIT_LORA_RX_READY | EVENTBIT_LORA_TX_READY | EVENTBIT_PREP_SLEEP),
                                    false,
                                    false,
                                    portMAX_DELAY);
        
        // handle events
        if ((eventBits & EVENTBIT_LORA_RX_READY) != 0){
            xEventGroupClearBits(xEventGroup, EVENTBIT_LORA_RX_READY);
            handleRx();
        }

        if ((eventBits & EVENTBIT_LORA_TX_READY) != 0) {
            // handle the transmission
            handleTx();

            // if there are no more messages in the queue, clear tx ready event flag and start receiving
            if (uxQueueMessagesWaiting(qToMesh) == 0)
            {
                xEventGroupClearBits(xEventGroup, EVENTBIT_LORA_TX_READY);
                startRx();
            }
        }
        if ((eventBits & EVENTBIT_PREP_SLEEP) != 0) {
            xEventGroupClearBits(xEventGroup, EVENTBIT_PREP_SLEEP);
            prepForSleep();
        }
    }
}

void loraTask(void * pvParameter) {
    FloraNetRadio * handler = static_cast<FloraNetRadio*> ( pvParameter );
    handler->run(); // run the handler. should never return
    delete handler; // if it does, delete it and the task?
    vTaskDelete(NULL);
    return;
}