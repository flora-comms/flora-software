#include <FloraNet.h>

FloraNet::FloraNet() {
    // create queues
    QueueHandle_t qToMesh = xQueueCreate(QUEUE_LENGTH, sizeof(Message *));
    QueueHandle_t qFromMesh = xQueueCreate(QUEUE_LENGTH, sizeof(Message *));
    QueueHandle_t qToWeb = xQueueCreate(QUEUE_LENGTH, sizeof(Message *));

    // initialize hardware
    SPIClass sdSPI(FSPI);
    sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    SD.begin(SD_CS);

    // create handlers
    _loraHandler = new FloraNetRadio(qToMesh, qFromMesh);

    _webHandler = new FloraNetWeb(qToWeb);

    _protoHandler = new FloraNetProto(qToMesh, qFromMesh, qToWeb, qFromWeb);


}

void FloraNet::run() {
    // create tasks

    TaskHandle_t tskLora;
    TaskHandle_t tskWeb;
    TaskHandle_t tskProto;
    TaskHandle_t tskPower;

    xTaskCreatePinnedToCore(loraTask, "Lora", STACK_SIZE, _loraHandler, TASK_PRIORITY_LORA, &tskLora, 0);
    xTaskCreatePinnedToCore(webTask, "Web", STACK_SIZE, _webHandler, TASK_PRIORITY_WEB, &tskWeb, 0);
    xTaskCreatePinnedToCore(protoTask, "Proto", STACK_SIZE, _protoHandler, TASK_PRIORITY_PROTO, &tskProto, 0);

    _powerHandler = new FloraNetPower(tskLora, tskWeb, tskProto);

    xTaskCreatePinnedToCore(powerTask, "Power", STACK_SIZE, _powerHandler, TASK_PRIORITY_POWER, &tskPower, 0);
}