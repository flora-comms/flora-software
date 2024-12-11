#include <FloraNetUtils.h>

class RetryTimer
{
public:
    LogEntry *entry;

    RetryTimer(LogEntry *logEntry) { entry = logEntry; };
};

/// @brief Retry timer callback. Only called if the message is not acknowledged
/// @param xTimer 
extern "C" void RetryTimerCallback(TimerHandle_t xTimer);