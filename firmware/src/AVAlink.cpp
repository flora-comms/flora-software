#include <AVAlink.h>

SPIClass sdSPI(FSPI);
QueueHandle_t qToWeb = xQueueCreate(QUEUE_LENGTH, sizeof(Message *));
QueueHandle_t qToMesh = xQueueCreate(QUEUE_LENGTH, sizeof(Message *));
EventGroupHandle_t xLoraEventGroup = xEventGroupCreate();
bool bApIsUp = true;

/// @brief Initializes AVAlink hardware
void initAvalink()
{
#ifdef DEBUG
    Serial.begin(SERIAL_BAUD);
#endif

#ifdef ENV_PLATFORMIO
    delay(5000); // to give time to restart serial monitor task
#endif
    sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    SD.begin(SD_CS);
    return;
}