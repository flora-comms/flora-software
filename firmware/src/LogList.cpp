#include <LogList.h>
LogList *pxHistoryLogs[256] = {nullptr};
// Retry Timer
LogEntry::RetryTimer::RetryTimer()
{
    _entry = nullptr;
    timer = NULL;
}
LogEntry::RetryTimer::RetryTimer(LogEntry *entry)
{
    _entry = entry;
    // create the timer handle
    long retryDelay = random(RETRY_INTERVAL) * 1000;
    timer = xTimerCreate(
        "",
        pdMS_TO_TICKS(retryDelay),
        false,
        entry,
        RetryTimerCallback);
}

void RetryTimerCallback(TimerHandle_t xTimer)
{
    // get the timer id
    LogEntry *entry = (LogEntry *)pvTimerGetTimerID(xTimer);

    // otherwise, add it to the retry queue and notify the protocol task
    xQueueSend(qRetries, &(entry->msg), MAX_TICKS_TO_WAIT);
    xEventGroupSetBits(xEventGroup, EVENTBIT_RETRY_READY);
}

// Log Entry
void LogList::removeLast()
{
    // save the pointer of the entry at the tail
    LogEntry *pOldTail = tail;
    // make tail's previous the tail with a _next value of null
    tail = pOldTail->prev;
    tail->next = nullptr;
    delete (pOldTail);
    // decrement length
    len--;
    return;
}

LogEntry::LogEntry(Message *message)
{
    msg = message;
    id = message->packetId;
    ack = false;
    next = nullptr;
    prev = nullptr;
    retryTimer = RetryTimer(this);
}

LogEntry::~LogEntry()
{
    delete msg;
    xTimerDelete(retryTimer.timer, MAX_TICKS_TO_WAIT);
}

bool LogList::checkId(uint8_t packetId)
{
    LogEntry *pCheckEntry = root; // start at the beginning
    while (pCheckEntry != nullptr)
    { // while the entry is valid
        uint8_t id = pCheckEntry->id;
        bool ack = pCheckEntry->ack;
        if ((packetId == id) & ack) // if the packet ids match and the message has already been acknowledged
        {
            return true;
        }

        if ((packetId == id) & !ack) { // if the packet ids match but the message hasn't been acknowledged
            // delete the retry timer and acknowledge
            xTimerStop(pCheckEntry->retryTimer.timer, MAX_TICKS_TO_WAIT);
            pCheckEntry->ack = true;
            return true;
        }
        pCheckEntry = pCheckEntry->next; // go to the next entry
    }
    return false;
}

// public
bool LogList::needsRepeating(Message *message)
{   
    // if no more hops remaining
    if (message->ttl == 0) {
        return false;
    }
    // check if the packet id has been seen recently
    return !checkId(message->packetId);
}

void LogList::update(Message *message) {
    
    LogEntry *entry = new LogEntry(message);
    // if root is null, then root and tail become entry
    if (root == nullptr) {
        root = entry;
        tail = root;
    } // if root is not null
    else {
        entry->next = root;    // entry _next is root
        root->prev = entry;    // the root's previous is entry
        root = entry;          // then the root becomes entry
    }
    // increase length by one
    len++;

    // make sure the list isn't too long
    while (len > ACKNOWLEDGE_WINDOW_SIZE) {
        removeLast();
    }
    
    return;
}