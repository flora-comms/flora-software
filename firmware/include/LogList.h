/*
======   LogList.h  ======

Version: v1.0
Last Update: Nov 6 2024

======   DESCRIPTION   =====

Linked list instantiation for the log lists

*/

#ifndef LOGLIST_H
#define LOGLIST_H
#pragma once

#include <Message.h>

class LogList {
private:
    class LogEntry
    {
    public:
        Message *_msg;
        uint8_t _id;
        bool _ack;
        LogEntry *_next;
        LogEntry *_prev;
        TimerHandle_t _timer;

        /// @brief Constructs a LogEntry from a Message object
        /// @param message The Message to construct the log entry from
        LogEntry(Message *message)
        {
            _msg = message;
            _id = message->packetId;
            _ack = false;
            _next = nullptr;
            _prev = nullptr;
        }
        /// @brief Destructor
        ~LogEntry();
    };
    LogEntry *_root;    // the first entry in the log list
    LogEntry *_tail;
    uint8_t _len;

    /// @brief Removes the last entry from the list
    void removeLast();
    /// @brief Checks if a message needs to be retried. If yes, it puts the message at the top of the tx queue.
    void checkForRetries();
    /// @brief finds a packetId in the list and marks it as acknowledged if found
    /// @param packetId The packetId to find
    /// @return True if the packetId is in the list. False if the packet id does not exist
    bool checkId(uint8_t packetId);

public:

    LogList() { _root = nullptr; _tail = nullptr; _len = 0; }

    unsigned int length() { return _len; }

    /// @brief Pushes an entry to the top of the list
    /// @param message The message to push to the top
    void update(Message *message);

    /// @brief Determines if a message needs to be repeated
    /// @param message The message to check.
    /// @return True if the message needs to be repeated. False if not
    bool needsRepeating(Message *message);
};

#endif  // LOGLIST_H