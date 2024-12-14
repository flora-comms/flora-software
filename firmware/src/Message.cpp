#include <Message.h>

Message::Message()
{
    dest = 0x00;
    payload = String();
    senderId = 0x00;
    packetId = 0x00;
    ttl = 0;
}

Message::Message(uint8_t *bytes)
{
    dest = *bytes++;
    senderId = *bytes++;
    packetId = *bytes++;
    ttl = *bytes++;
    payload = String((char *)bytes);
}

Message::Message(uint8_t *serialJson, uint8_t id)
{
    // if id is not null, then the data is from the webside and requires a packet id and fresh ttl
    JsonDocument json;

    deserializeJson(json, serialJson);

    payload = String((const char *)json["Payload"]);

    senderId = json["NodeID"];

    packetId = id;

    ttl = MAX_LORA_TTL;

    if (json["SOS"] == 0) // if it's not an SOS message
    {
        dest = 0xFF;
    }
    else
    {
        dest = 0x00;
    }
}

String Message::toSerialJson()
{
    JsonDocument json;
    String data;
    json["Payload"] = payload;
    json["NodeID"] = senderId;
    if (dest != 0x00)
    {
        json["SOS"] = 0;
    }
    else
    {
        json["SOS"] = 1;
    }

    serializeJson(json, data);
    return data;
}

uint16_t Message::toLoraPacket(uint8_t *buf)
{
    uint16_t len = 0;
    buf[len++] = dest;     // byte 0
    buf[len++] = senderId; // byte 1
    buf[len++] = packetId; // byte 2
    buf[len++] = ttl;      // byte 3
    const char *cstr = payload.c_str();
    while (*cstr != '\0')
    {
        buf[len++] = *cstr++;
    }
    buf[len] = '\0'; // get total packet length
    return len;
}

void Message::appendHistory()
{
    uint8_t type;
    if (dest == 0x00)
    {
        type = 1;
    }
    else
    {
        type = 0;
    }
    String combinedString = "\"" + payload + "\"" + "," + String(senderId) + "," +
                            String(type); // "payload",nodeID,SOS

    File file = SD.open(HISTORY_FILENAME, FILE_APPEND);

    if (!file)
    {
        DBG_PRINTLN("Failed to open file for writing!");
        return;
    }
    else
    {
        file.println(combinedString);
        file.close();
    }

    #ifdef USE_NVS
    JsonDocument json;
    json["ID"] = currentId;
    String newPacketId;
    serializeJson(json, newPacketId);
    file = SD.open(PACKET_ID_FILENAME, FILE_WRITE);
    file.print(newPacketId);
    file.close();
    #endif
}