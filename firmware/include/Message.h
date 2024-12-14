#include <FloraNetConfig.h>

/// @brief Message class. Intermediate between a web chat and a lora packet for inter-task communications
class Message
{
public:
    String payload;   // message payload string
    uint8_t senderId; // the sender ID (0-255). 0 reserved for gateway
    uint8_t packetId; // the packetId associated with the message
    uint8_t ttl;      // the message time to live
    uint8_t dest;     // the Node ID of the destination


    /// @brief Default Message constructor
    Message();
    
    /// @brief Converts serial data that already contains a packet id into a message
    /// @param bytes The lora pacekt byte buffer to convert from
    Message(uint8_t *bytes);

    /// @brief Converts a serial json message  into a Message type.
    /// @param data The data buffer containing the serial data
    /// @param id The packetId to assign to the message. Defaults to NULL which means the data already contains the packet ID.
    /// @return Web error status code
    Message(uint8_t *data, uint8_t id);

    /// @brief Converts a the message type into a JSON string ready for sending
    /// over the web socket.
    /// @return The serialized JSON string
    String toSerialJson();

    /// @brief Converts to a serialized LoRa packet with correct headers, etc.
    /// @param buf the byte buffer to write into
    /// @return The length of the serialized lora packet
    uint16_t toLoraPacket(uint8_t *buf);

    /// @brief Appends the payload to the chat history file
    void appendHistory();
};