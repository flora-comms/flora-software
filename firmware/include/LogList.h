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

#include <FloraNetUtils.h>



/// @brief Retry timer callback. Only called if the message is not acknowledged
/// @param xTimer
extern "C" void RetryTimerCallback(TimerHandle_t xTimer);

/// @brief A log entry containing a message and acknowledge info
class LogEntry
{
public:
    /// @brief Retry timer for a message
    class RetryTimer
    {
    public:
        LogEntry *_entry;
        TimerHandle_t timer;
        RetryTimer();
        RetryTimer(LogEntry *logEntry);
    };
    Message *msg;
    uint8_t id;
    bool ack;
    LogEntry *next;
    LogEntry *prev;
    RetryTimer retryTimer;
    /// @brief Constructs a LogEntry from a Message object
    /// @param message The Message to construct the log entry from
    LogEntry(Message *message);

    /// @brief Destructor
    ~LogEntry();
};

/// @brief A doubly-linked list of LogEntries
class LogList {
public:
    
    LogEntry *root;    // the first entry in the log list
    LogEntry *tail;
    uint8_t len;

    LogList()
    {
        root = nullptr;
        tail = nullptr;
        len = 0;
    }

    /// @brief Pushes an entry to the top of the list
    /// @param message The message to push to the top
    void update(Message *message);

    /// @brief Determines if a message needs to be repeated
    /// @param message The message to check.
    /// @return True if the message needs to be repeated. False if not
    bool needsRepeating(Message *message);

private:
    /// @brief Removes the last entry from the list
    void removeLast();
    /// @brief Checks if a message needs to be retried. If yes, it puts the message at the top of the tx queue.
    void checkForRetries();
    /// @brief finds a packetId in the list and marks it as acknowledged if found
    /// @param packetId The packetId to find
    /// @return True if the packetId is in the list. False if the packet id does not exist
    bool checkId(uint8_t packetId);
};

extern LogList *pxHistoryLogs[256];

#endif  // LOGLIST_H