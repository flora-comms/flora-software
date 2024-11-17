#include <AVAlinkRadio.h>

// globals

SX1262 radio = new Module(
    LORA_NSS,
    LORA_IRQ,
    LORA_NRST,
    LORA_BUSY,
    loraSPI);

SPIClass loraSPI(HSPI);

LogList* pxHistoryLogs[256];

// function definitions
void initLora() {
    loraSPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
    DBG_PRINT("[SX1262] Initializing ... ");
    int status = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC, LORA_POWER, LORA_PREAMB);
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

    xLoraEventGroup = xEventGroupCreate();
    if (xLoraEventGroup == NULL)
    {
        DBG_PRINTLN("Lora Event group creation was unsuccessful");
    }

    // init linked lists on heap
    for (int i = 0; i < 256; i++)
    {
        pxHistoryLogs[i] = new LogList();
    }
}

/// @brief Lora task function
void loraTask(void * pvParameters) 
{
    initLora(); // initialize the ahrdwawer


    // random number generator
    randomSeed(radio.getRSSI());

    startRx();      // TODO define this w irq and startReceive()

    EventBits_t xLoraEventBits;

    while (true)
    {
        // wait for notification from radio or web server
        xLoraEventBits = xEventGroupWaitBits(
            xLoraEventGroup, 
            (EventBits_t)(EVENTBIT_LORA_RX | EVENTBIT_LORA_TX | EVENTBIT_LORA_Q),
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);   // Wait for notification from webserver or radio

        if (xLoraEventBits & EVENTBIT_LORA_TX)             // if a tx has finished
        {
            handleTx();                                    // handle the tx finished, start rxing again
        }

        if (xLoraEventBits & EVENTBIT_LORA_RX)        // if an rx has finished
        {
            handleRx();                               // handle the recieved message
        }
        
        if (xLoraEventBits & EVENTBIT_LORA_Q)    // if there's a message in the queue
        {

            Message *pxTxMsg;

            xQueueReceive(qToMesh, &pxTxMsg, 0);

            // if the queue is empty, 
            if(uxQueueMessagesWaiting(qToMesh) == 0) {
                xEventGroupClearBits(xLoraEventGroup, EVENTBIT_LORA_Q); // clear the queue event bit
            }

            uint16_t status = startTx(pxTxMsg);

            if (status != RADIOLIB_ERR_NONE) {
                DBG_PRINTF("Tx Failure - Code: %i", status);
                startRx();
            }
        }

    }
}

void startRx() {
    xEventGroupClearBits(xLoraEventGroup, EVENTBIT_LORA_RX);    // clear the event
    radio.setDio1Action(onRxIrq);                               // set the irq
    radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_NONE);        // start in single recieve mode
}

void handleRx() {
    int status;
    uint8_t rx_data[256] = {0};          // create buffer of zeros
    status = radio.readData(rx_data, 0); // read in data
    if (!(status == RADIOLIB_ERR_NONE))
    {
        DBG_PRINTLN("Lora RX failure");
    }

    Message *pxRxMsg = new Message(rx_data);

    if (needsRepeating(pxRxMsg)) // if this message needs repeating
    { 
        // write to sd card if necessary
        if (!bApIsUp) {
            pxRxMsg->appendHistory(HISTORY_FILENAME);
        } else {
            xQueueSend(qToWeb, &pxRxMsg, 0); // send to the web server through web server queue
            xEventGroupSetBits(xLoraEventGroup, EVENTBIT_WEB_READY);
        }

        xQueueSend(qToMesh, &pxRxMsg, 0);   // send to lora mesh queue for sending when ready

        // raise lora Queue event
        xEventGroupSetBits(xLoraEventGroup, EVENTBIT_LORA_Q);
    }
    return;
}

int16_t startTx(Message *msg) {
    // if this is the first time sending the message, we want to be able to retry if it doesn't get ack, so retry will be true. 
    // That means we need to deceremnt ttl and save it in the history logs to check for acks
    if (!msg->isRetry)
    {
        msg->ttl--;     // decrement ttl
        pxHistoryLogs[msg->senderId]->update(msg); // put it in the log
    }

    // DBG_PRINTF("Web Server Message recieved: %s", pxRxMsg.payload);

    uint8_t bytes[256] = {0}; // TODO make sure web app doesn't let people send messages longer than 251 chars.

    uint16_t length = msg->toLoraPacket(bytes);

    bool bChannelFree = true;

    // calculate max time on air per pxRxMsg in ms
    RadioLibTime_t xMaxTimeOnAir = radio.getTimeOnAir(255) / 1000;

    do {
        long lDelay = xMaxTimeOnAir * random(1,5);

        startRx();

        EventBits_t xWaitResult = xEventGroupWaitBits(xLoraEventGroup, EVENTBIT_LORA_RX, pdTRUE, pdTRUE, pdMS_TO_TICKS(lDelay));

        if (xWaitResult & EVENTBIT_LORA_RX) {
            handleRx();
            bChannelFree = false;
        } else {
            startCad();
            xEventGroupWaitBits(xLoraEventGroup, EVENTBIT_LORA_CAD, pdTRUE, pdTRUE, pdMS_TO_TICKS(100)); // should not actually timeout
            int16_t iCadResult = handleCad();
            bChannelFree = (iCadResult == RADIOLIB_CHANNEL_FREE);
        }
    } while (!bChannelFree);

    radio.setDio1Action(onTxIrq);
    xEventGroupClearBits(xLoraEventGroup, EVENTBIT_LORA_TX);
    return radio.startTransmit(bytes, length);
}

void handleTx() {
    // clear the event
    xEventGroupClearBits(xLoraEventGroup, EVENTBIT_LORA_TX);
    // start recieving again
    startRx();
}

void startCad() {
    xEventGroupClearBits(xLoraEventGroup, EVENTBIT_LORA_CAD);
    radio.startChannelScan();
}

int16_t handleCad() {
    xEventGroupClearBits(xLoraEventGroup, EVENTBIT_LORA_CAD);
    return radio.getChannelScanResult();
}

bool needsRepeating(Message *msg) {
    LogList* pxLog = pxHistoryLogs[msg->senderId];
    if (msg->ttl == 0) {                    // if the ttl is zero dont repeat
        return false;
    }
    if (pxLog->checkId(msg->packetId)) {    // if the packet id from that sender has been seen before dont repeat
        return false;
    }
    return true;            // otherwise return true
}

static void onRxIrq() {
    BaseType_t xHigherPriorityTaskWoken, xResult;

    /* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
    xHigherPriorityTaskWoken = pdFALSE;

    xResult = xEventGroupSetBitsFromISR(
        xLoraEventGroup,
        EVENTBIT_LORA_RX,
        &xHigherPriorityTaskWoken
    );
    /* Was the message posted successfully? */
    if (xResult != pdFAIL)
    {
        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
           switch should be requested. The macro used is port specific and will
           be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
           the documentation page for the port being used. */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static void onTxIrq() {
    BaseType_t xHigherPriorityTaskWoken, xResult;

    /* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
    xHigherPriorityTaskWoken = pdFALSE;

    xResult = xEventGroupSetBitsFromISR(
        xLoraEventGroup,
        EVENTBIT_LORA_TX,
        &xHigherPriorityTaskWoken);
    /* Was the message posted successfully? */
    if (xResult != pdFAIL)
    {
        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
           switch should be requested. The macro used is port specific and will
           be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
           the documentation page for the port being used. */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static void onCadIrq() {
    BaseType_t xHigherPriorityTaskWoken, xResult;

    /* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
    xHigherPriorityTaskWoken = pdFALSE;

    xResult = xEventGroupSetBitsFromISR(
        xLoraEventGroup,
        EVENTBIT_LORA_CAD,
        &xHigherPriorityTaskWoken);
    /* Was the message posted successfully? */
    if (xResult != pdFAIL)
    {
        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
           switch should be requested. The macro used is port specific and will
           be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
           the documentation page for the port being used. */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}