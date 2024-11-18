#include <LogList.h>

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

    // now that the list has the new message on top, see if we need to retry any messages
    if (_len > RETRY_THRESHOLD) {
        checkForRetries();
    }

    // make sure the list isn't too long
    while (_len >= ACKNOWLEDGE_WINDOW_SIZE) {
        removeLast();
    }
    return;
}

void LogList::removeLast() {
    
    // save the pointer of the entry at the tail
    LogEntry *pOldTail = _tail;
    // make tail's previous the tail with a _next value of null
    _tail = pOldTail->_prev;
    _tail->_next = nullptr;
    delete(pOldTail);
    // decrement length
    _len--;
    return;
}

bool LogList::checkId(uint8_t packetId) {
    LogEntry *pCheckEntry = _root;  // start at the beginning
    while (pCheckEntry != nullptr) { // while the entry is valid
        if (packetId ==  pCheckEntry->_id) {    // if the packet ids match
            pCheckEntry->_ack = true;
            return true;
        }
        pCheckEntry = pCheckEntry->_next;   // go to the next entry
    }
    return false;
}

void LogList::checkForRetries() {
    uint8_t index = 0;
    LogEntry* pCheckEntry = _root;  // start at the top of the list
    while (index != RETRY_THRESHOLD) {  // while we havent reached the index threshold
        pCheckEntry = pCheckEntry->_next;   // go to the next index
        index++;
    }
    if (!pCheckEntry->_ack) {    // if the message hasnt been acknowledged
        pCheckEntry->_msg->isRetry = true;      // set the isRetry value of the message to True
        xQueueSendToFront(qToMesh, &pCheckEntry->_msg, 0);  // send to the front for a retry
    }
    return;
}