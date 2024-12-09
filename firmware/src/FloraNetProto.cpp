#include <FloraNetProto.h>

void appendHistory()
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

    File file = SD.open(fileName, FILE_APPEND);

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
}