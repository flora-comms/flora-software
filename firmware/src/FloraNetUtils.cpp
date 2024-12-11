#include <FloraNetUtils.h>

// GLOBALS 

LogList* pxHistoryLogs[256] = {nullptr};
// create queues
QueueHandle_t qToMesh = xQueueCreate(QUEUE_LENGTH, sizeof(Message *));
QueueHandle_t qFromMesh = xQueueCreate(QUEUE_LENGTH, sizeof(Message *));
QueueHandle_t qToWeb = xQueueCreate(QUEUE_LENGTH, sizeof(Message *));
QueueHandle_t qRetries = xQueueCreate(QUEUE_LENGTH, sizeof(Message *));
QueueHandle_t qFromWeb = xQueueCreate(QUEUE_LENGTH, sizeof(Message *));
uint8_t currentId = 0;
EventGroupHandle_t xEventGroup = xEventGroupCreate();
portMUX_TYPE csToken = portMUX_INITIALIZER_UNLOCKED;
AsyncWebServer server(80);
AsyncWebSocket ws(WEBSOCKET_ENDPOINT);

long maxTimeOnAir = 0;

SPIClass sdSPI(FSPI);
SPIClass loraSPI(HSPI);

SX1262 radio = new Module(
    LORA_NSS,
    LORA_IRQ,
    LORA_NRST,
    LORA_BUSY,
    loraSPI);

// ISRs

/// @brief LoRa RX interrupt handler
void RxISR(void)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;

    /* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
    xHigherPriorityTaskWoken = pdFALSE;

    xResult = xEventGroupSetBitsFromISR(
        xEventGroup,
        EVENTBIT_LORA_RX_READY,
        &xHigherPriorityTaskWoken);
    /* Was the message posted successfully? */
    if (xResult != pdFAIL)
    {
        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
           switch should be requested. The macro used is port specific and will
           be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
           the documentation page for the port being used. */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/// @brief LoRa TX interrupt handler
void TxISR(void){
    BaseType_t xHigherPriorityTaskWoken, xResult;

    /* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
    xHigherPriorityTaskWoken = pdFALSE;

    xResult = xEventGroupSetBitsFromISR(
        xEventGroup,
        EVENTBIT_LORA_TX_DONE,
        &xHigherPriorityTaskWoken);
    /* Was the message posted successfully? */
    if (xResult != pdFAIL)
    {
        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
           switch should be requested. The macro used is port specific and will
           be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
           the documentation page for the port being used. */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/// @brief User button press ISR
void buttonISR(void)
{
    DETACH_BUTTONISR();   // remove the interrupt

    BaseType_t xHigherPriorityTaskWoken, xResult;

    /* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
    xHigherPriorityTaskWoken = pdFALSE;

    xResult = xEventGroupSetBitsFromISR(
        xEventGroup,
        EVENTBIT_WEB_REQUESTED,
        &xHigherPriorityTaskWoken);
    /* Was the message posted successfully? */
    if (xResult != pdFAIL)
    {
        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
           switch should be requested. The macro used is port specific and will
           be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
           the documentation page for the port being used. */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}