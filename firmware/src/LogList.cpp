#include <LogList.h>

// construction


// private
void LogList::removeLast()
{
    // save the pointer of the entry at the tail
    LogEntry *pOldTail = _tail;
    // make tail's previous the tail with a _next value of null
    _tail = pOldTail->_prev;
    _tail->_next = nullptr;
    delete (pOldTail);
    // decrement length
    _len--;
    return;
}

bool LogList::checkId(uint8_t packetId)
{
    LogEntry *pCheckEntry = _root; // start at the beginning
    while (pCheckEntry != nullptr)
    { // while the entry is valid
        if (packetId == pCheckEntry->_id)
        { // if the packet ids match
            pCheckEntry->_ack = true;
            return true;
        }
        pCheckEntry = pCheckEntry->_next; // go to the next entry
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
    if (_root == nullptr) {
        _root = entry;
        _tail = _root;
    } // if root is not null
    else {
        entry->_next = _root;    // entry _next is root
        _root->_prev = entry;    // the root's previous is entry
        _root = entry;          // then the root becomes entry
    }
    // increase length by one
    _len++;

    // make sure the list isn't too long
    while (_len > ACKNOWLEDGE_WINDOW_SIZE) {
        removeLast();
    }
    return;
}

