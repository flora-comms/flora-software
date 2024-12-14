#include <ConfigOptions.h>

#ifndef FLORANET_DEBUG_H
#define FLORANET_DEBUG_H

#ifdef DEBUG
#define SERIAL_BAUD 115200
#define DBG_PRINTLN(MSG) Serial.println(MSG)
#define DBG_PRINT(MSG) Serial.print(MSG)
#define DBG_PRINTF(format, args...) Serial.printf(format, args)
#else
#define DBG_PRINTLN(MSG)
#define DBG_PRINT(MSG)
#define DBG_PRINTF(format, args...)
#endif

#endif // FLORANET_DEBUG_H