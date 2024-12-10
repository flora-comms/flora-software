#include <FloraNetUtils.h>

class RetryTimerID
{
public:
    LogEntry *entry;

    RetryTimerID(LogEntry *logEntry) { entry = logEntry; };
};

extern "C" void RetryTimerCallback(TimerHandle_t xTimer);