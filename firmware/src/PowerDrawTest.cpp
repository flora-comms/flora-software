#include <PowerDrawTest.h>
void PowerDrawTask(void * pvParameter )
{
    uint8_t packetId = 0;
    uint32_t payload = 0;

    while (true)
    {
        Message * msg = new Message();
        msg->packetId = packetId;
        msg->payload = String(payload);
        msg->senderId = 5;
        msg->dest = 0xFF;
        msg->ttl = 4;

        xQueueSend(qFromWeb, &msg, MAX_TICKS_TO_WAIT);

        packetId++;
        payload++;
        xEventGroupSetBits(xEventGroup, EVENTBIT_WEB_RX_DONE);
        vTaskDelay(pdMS_TO_TICKS(600000)); // wait 10 min
    }
}