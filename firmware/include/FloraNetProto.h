#include <RetryTimer.h>

class FloraNetProto {
private:
    QueueHandle_t _qToMesh;     // protoTask --> loraTask
    QueueHandle_t _qFromMesh;   // loraTask --> protoTask
    QueueHandle_t _qToWeb;      // protoTask --> webTask
    QueueHandle_t _qFromWeb;    // webTask --> protoTask

    /// @brief Appends a message to the chat history in the sd card.
    /// @param msg The message to send to the sd card
    void appendHistory(Message *msg);
    
    /// @brief Initializes the log list
    void init();

    void handleEvents();

    void handleTx(Message * msg, LogList * log);

    void handleLora();

    void handleWeb();

    bool readyToSleep();

public:

    /// @brief Proto constructor
    /// @param toMesh Queue going from protoTask --> loraTask
    /// @param fromMesh Queue going from loraTask --> protoTask
    /// @param toWeb Queue going from protoTask --> webTask
    /// @param fromWeb Queue going from webTask --> protoTask
    FloraNetProto(QueueHandle_t toMesh, QueueHandle_t fromMesh, QueueHandle_t toWeb, QueueHandle_t fromWeb)
    {
        _qToMesh = toMesh;
        _qFromMesh = fromMesh;
        _qToWeb = toWeb;
        _qFromWeb = fromWeb;
    }
    /// @brief Runs the protocol handler instance.
    void run();
};

/// @brief The protocol task function
/// @param pvParameter Pass in a FloraNetProto object here.
extern "C" void protoTask(void * pvParameter);