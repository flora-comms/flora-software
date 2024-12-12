#include <FloraNet.h>
uint8_t currentId;
FloraNet::FloraNet() {

    // create handlers
    _loraHandler = new FloraNetRadio();

    _webHandler = new FloraNetWeb();

    _protoHandler = new FloraNetProto();
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

    #ifdef POWER_SAVER
    _powerHandler = new FloraNetPower(tskLora, tskWeb, tskProto);

    xTaskCreatePinnedToCore(powerTask, "Power", STACK_SIZE, _powerHandler, TASK_PRIORITY_POWER, &tskPower, 0);
    #else
    xEventGroupSetBits(xEventGroup, EVENTBIT_WEB_REQUESTED);
    #endif
}