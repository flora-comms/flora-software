

#ifndef FLORANET_H
#define FLORANET_H
#pragma once

#include <FloraNetRadio.h>
#include <FloraNetWeb.h>

class FloraNet {
    QueueHandle_t        qToMesh();    
    QueueHandle_t        qToWeb;     
    bool                 bApIsUp;    
    SX1262               radio;
    SPIClass             sdSPI;
    EventGroupHandle_t   xEventGroup;
    AsyncWebServer       server;
    AsyncWebSocket       ws;
    TaskHandle_t         xLoraTask;
    TaskHandle_t         xWebTask;
};

/// @brief Initializes FLORANET hardware
void initFloraNet();

#endif