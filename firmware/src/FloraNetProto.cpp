#include <FloraNetProto.h>

//private
void FloraNetProto::appendHistory(Message * msg)
{
    uint8_t type;
    if (msg->dest == 0x00)
    {
        type = 1;
    }
    else
    {
        type = 0;
    }
    String combinedString = "\"" + msg->payload + "\"" + "," + String(msg->senderId) + "," +
                            String(type); // "payload",nodeID,SOS

    File file = SD.open(HISTORY_FILENAME, FILE_APPEND);

    if (!file)
    {
        DBG_PRINTLN("Failed to open file for writing!");
        return;
    }
    else
    {
        file.println(combinedString);
        file.close();
    }
}

void FloraNetProto::init()
{
    // initialize the history logs
    for( int i; i < 256; i++) {
        pxHistoryLogs[i] = new LogList();
    }
}

void FloraNetProto::handleEvents()
{
    do {
        EventBits_t eventbits = xEventGroupGetBits(xEventGroup);
        if ((eventbits & EVENTBIT_LORA_RX_DONE) != 0) {
            handleLora();
        }
        if ((eventbits & EVENTBIT_WEB_RX_DONE) != 0) {
            handleWeb();
        }
        if ((eventbits & EVENTBIT_RETRY_READY) != 0) {
            Message *msg;
            xQueueReceive(qRetries, &msg, MAX_TICKS_TO_WAIT);
            xQueueSend(_qToMesh, &msg, MAX_TICKS_TO_WAIT);
            xEventGroupSetBits(xEventGroup, EVENTBIT_LORA_TX_READY);
        }
    } while (!readyToSleep());
    
}

void FloraNetProto::handleTx(Message * msg, LogList * log)
{
    log->update(msg);
    long delay = random(1, LORA_TX_WAIT_INTERVAL_MAX) * (maxTimeOnAir);
    long retryDelay = random(RETRY_INTERVAL) * maxTimeOnAir;
    // wait for the delay
    vTaskDelay(pdMS_TO_TICKS(delay));

    // write to sd card
    appendHistory(msg);

    // create and start retry timer
    RetryTimerID *id = new RetryTimerID(log->root);
    TimerHandle_t timer = xTimerCreate(
        "",
        pdMS_TO_TICKS(retryDelay),
        false,
        id,
        RetryTimerCallback
    );

    // send to mesh
    xQueueSend(_qToMesh, &msg, MAX_TICKS_TO_WAIT);
    xEventGroupSetBits(xEventGroup, EVENTBIT_LORA_TX_READY);
    xTimerStart(timer, MAX_TICKS_TO_WAIT);
}

void FloraNetProto::handleLora()
{
    // get the message from the queue
    Message *msg;
    xQueueReceive(_qFromMesh, &msg, MAX_TICKS_TO_WAIT);
    LogList *log = pxHistoryLogs[msg->senderId];

    // if it doesn't need repeating, return
    if (!log->needsRepeating(msg)) { return; }

    // If the web server is up
    if ((xEventGroupGetBits(xEventGroup) & EVENTBIT_WEB_SLEEP_READY) == 0)
    {
        xQueueSend(_qToWeb, &msg, MAX_TICKS_TO_WAIT);
        xEventGroupSetBits(xEventGroup, EVENTBIT_WEB_TX_READY);
    }
    handleTx(msg, log);
}

void FloraNetProto::handleWeb()
{
    // read in message
    Message *msg;
    xQueueReceive(_qFromWeb, &msg, MAX_TICKS_TO_WAIT);
    LogList *log = pxHistoryLogs[msg->senderId];
    // send to sd card
    appendHistory(msg);
    // transmit to mesh
    handleTx(msg, log);
}

bool FloraNetProto::readyToSleep()
{
    // check if all queues are clear
    uint8_t queuesClear = 0;

    if(uxQueueMessagesWaiting(_qFromMesh) == 0)
    {
        xEventGroupClearBits(xEventGroup, EVENTBIT_LORA_RX_DONE);
        queuesClear++;
    }
    if (uxQueueMessagesWaiting(_qFromWeb) == 0)
    {
        xEventGroupClearBits(xEventGroup, EVENTBIT_WEB_RX_DONE);
        queuesClear++;
    }
    if (uxQueueMessagesWaiting(qRetries) == 0)
    {
        xEventGroupClearBits(xEventGroup, EVENTBIT_RETRY_READY);
        queuesClear++;
    }

    // if they aren't return false
    if (queuesClear < 3)
    {
        return false;
    } 

    // wait for any messages needing to be acknowledged or retries that might be expiring
    long timeout = maxTimeOnAir * LORA_TX_WAIT_INTERVAL_MAX * 3;
    EventBits_t eventbits = xEventGroupWaitBits(
        xEventGroup,
        (EVENTBIT_LORA_RX_DONE | EVENTBIT_WEB_RX_DONE | EVENTBIT_RETRY_READY),
        false,
        false,
        pdMS_TO_TICKS(timeout));

    // if timeout
    if ((eventbits & (EVENTBIT_LORA_RX_DONE | EVENTBIT_WEB_RX_DONE | EVENTBIT_RETRY_READY)) == 0)
    {
        // set ready to sleep
        xEventGroupSetBits(xEventGroup, EVENTBIT_PROTO_SLEEP_READY);
        return true;
    } else {
        // not ready to sleep
        return false;
    }
}

//public
void FloraNetProto::run()
{
    // make sure we don't go to sleep
    xEventGroupClearBits(xEventGroup, EVENTBIT_PROTO_SLEEP_READY);
    init();     // init stuff
    xEventGroupSetBits(xEventGroup, EVENTBIT_PROTO_SLEEP_READY);    // ok ready to sleep again
    while (true)
    {
        // wait for an event
        xEventGroupWaitBits(
            xEventGroup,
            (EVENTBIT_LORA_RX_DONE | EVENTBIT_WEB_RX_DONE | EVENTBIT_RETRY_READY),
            false,
            false,
            portMAX_DELAY
        );
        // clear sleep readiness and handle the event when it arrives
        xEventGroupClearBits(xEventGroup, EVENTBIT_PROTO_SLEEP_READY);
        handleEvents();
    }
}

// external
void protoTask(void * pvParameter)
{
    FloraNetProto *handler = static_cast<FloraNetProto *>(pvParameter);
    handler->run(); // run the handler. should never return
    delete handler; // if it does, delete it and the task?
    vTaskDelete(NULL);
    return;
}