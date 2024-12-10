#include <FloraNetProto.h>

//private
void FloraNetProto::init()
{
    // initialize the history logs
    for( int i = 0; i < 256; i++) {
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
            DBG_PRINTF("RTY:%i-%i", msg->senderId, msg->packetId);
            xQueueSend(qToMesh, &msg, MAX_TICKS_TO_WAIT);
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
    xQueueSend(qToMesh, &msg, MAX_TICKS_TO_WAIT);
    xEventGroupSetBits(xEventGroup, EVENTBIT_LORA_TX_READY);
    xTimerStart(timer, MAX_TICKS_TO_WAIT);
}

void FloraNetProto::handleLora()
{
    // check if there's actually something to read
    if (uxQueueMessagesWaiting(qFromMesh) == 0)
    {
        xEventGroupClearBits(xEventGroup, EVENTBIT_LORA_RX_DONE);
    }
    // get the message from the queue
    Message *msg;
    QUEUE_RECEIVE(qFromMesh, msg)
    if (uxQueueMessagesWaiting(qFromMesh) == 0)
    {
        xEventGroupClearBits(xEventGroup, EVENTBIT_LORA_RX_DONE);
    }
    DBG_PRINTF("PhL:%s", msg->payload);
    LogList *log;
    if (pxHistoryLogs[msg->senderId] == nullptr)
    {
        DBG_PRINTLN("lognf");
        log = new LogList();
        pxHistoryLogs[msg->senderId] = log;
    }
    else
    {
        log = pxHistoryLogs[msg->senderId];
    }

    // if it doesn't need repeating, return
    if (!log->needsRepeating(msg)) { DBG_PRINTLN("no rpt"); return; }

    // if the web app is down, write to the sd card, otherwise send to web task
    if ((xEventGroupGetBits(xEventGroup) & EVENTBIT_WEB_SLEEP_READY) == EVENTBIT_WEB_SLEEP_READY)
    {
        sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
        SD.begin(SD_CS);
        msg->appendHistory();
        SD.end();
        sdSPI.end();
    } else {
        xQueueSend(qToWeb, &msg, MAX_TICKS_TO_WAIT);
        xEventGroupSetBits(xEventGroup, EVENTBIT_WEB_TX_READY);
    }

    handleTx(msg, log);
}

void FloraNetProto::handleWeb()
{
    // check if there's actually something to read garbage call
    if (uxQueueMessagesWaiting(qFromWeb) == 0)
    {
        xEventGroupClearBits(xEventGroup, EVENTBIT_WEB_RX_DONE);
        return;
    }
    
    // read in message
    Message *msg;
    QUEUE_RECEIVE(qFromWeb, msg)

    // clear event bit if necessary
    if (uxQueueMessagesWaiting(qFromWeb) == 0)
    {
        xEventGroupClearBits(xEventGroup, EVENTBIT_WEB_RX_DONE);
    }
    DBG_PRINTF("PhW:%i-%i", msg->senderId, msg->packetId);
    LogList* log;
    if (pxHistoryLogs[msg->senderId] == nullptr){
        DBG_PRINTLN("lognf");
        log = new LogList();
        pxHistoryLogs[msg->senderId] = log;
    } else {
        log = pxHistoryLogs[msg->senderId];
    }

    
    // transmit to mesh
    handleTx(msg, log);
}

bool FloraNetProto::readyToSleep()
{
    YIELD(); // for wdt
    // check if all queues are clear
    uint8_t queuesClear = 0;

    if(uxQueueMessagesWaiting(qFromMesh) == 0)
    {
        xEventGroupClearBits(xEventGroup, EVENTBIT_LORA_RX_DONE);
        queuesClear++;
    }
    if (uxQueueMessagesWaiting(qFromWeb) == 0)
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
    #ifdef POWER_SAVER
    xEventGroupSetBits(xEventGroup, EVENTBIT_PROTO_SLEEP_READY);    // ok ready to sleep again
    #endif
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