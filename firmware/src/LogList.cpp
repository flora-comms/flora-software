#include <LogList.h>

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
}

LogEntry::~LogEntry()
{
    delete msg;
}

bool LogList::checkId(uint8_t packetId)
{
    LogEntry *pCheckEntry = root; // start at the beginning
    while (pCheckEntry != nullptr)
    { // while the entry is valid
        if (packetId == pCheckEntry->id)
        { // if the packet ids match
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