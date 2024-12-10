#include <RetryTimer.h>

void RetryTimerCallback(TimerHandle_t xTimer)
{
    // get the timer id
    RetryTimerID* id = (RetryTimerID *) pvTimerGetTimerID( xTimer );

    // if the message has been acknowledged
    if (id->entry->ack) {
        // delete the timer and return
        xTimerDelete( xTimer, MAX_TICKS_TO_WAIT );
        return;
    }

    // otherwise, add it to the retry queue and notify the protocol task
    xQueueSend(qRetries, &(id->entry->msg), MAX_TICKS_TO_WAIT);
    xTimerDelete( xTimer, MAX_TICKS_TO_WAIT);
    xEventGroupSetBits(xEventGroup, EVENTBIT_RETRY_READY);
}