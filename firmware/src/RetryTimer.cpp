#include <RetryTimer.h>


void RetryTimerCallback(TimerHandle_t xTimer)
{
    // get the timer id
    RetryTimer* id = (RetryTimer *) pvTimerGetTimerID( xTimer );

    // otherwise, add it to the retry queue and notify the protocol task
    xQueueSend(qRetries, &(id->entry->msg), MAX_TICKS_TO_WAIT);
    xTimerDelete( xTimer, 0 );
    xEventGroupSetBits(xEventGroup, EVENTBIT_RETRY_READY);
}