#include <FloraNetUtils.h>

class FloraNetProto {
private:
    QueueHandle_t qToMesh;
    QueueHandle_t qFromMesh;
    QueueHandle_t qToWeb;
    QueueHandle_t qFromWeb;

    /// @brief Appends a message to the chat history in the sd card.
    /// @param msg The message to send to the sd card
    void appendHistory(Message *msg);

    /// @brief Initializes the SD card and any other necessary things...
    void init();

    void handleTx();

    void handleEvent();

    bool readyToSleep();

public:
    /// @brief Runs the protocol handler instance.
    void run();
};