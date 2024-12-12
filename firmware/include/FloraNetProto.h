#include <LogList.h>

class FloraNetProto {
private:

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
    FloraNetProto() {};
    /// @brief Runs the protocol handler instance.
    void run();
};

/// @brief The protocol task function
/// @param pvParameter Pass in a FloraNetProto object here.
extern "C" void protoTask(void * pvParameter);