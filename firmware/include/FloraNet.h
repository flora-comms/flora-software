

#ifndef FLORANET_H
#define FLORANET_H
#pragma once

#include <FloraNetRadio.h>
#include <FloraNetWeb.h>

class FloraNet
{
private:
    FloraNetRadio   _loraHandler;   // handles the lora hardware
    FloraNetWeb     _webHandler;    // handles the web server
    FloraNetProto   _protoHandler;  // handles the network protocol
    FloraNetPower   _powerHandler;  // handles power management

public:
    /// @brief Default constructor
    FloraNet();

    /// @brief Runs the FloraNet instance.
    void run();
};

#endif