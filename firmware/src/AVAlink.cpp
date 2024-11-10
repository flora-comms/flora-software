#include <AVAlink.h>

/// @brief Initializes AVAlink hardware
void initAvalink()
{
#ifdef DEBUG
    Serial.begin(SERIAL_BAUD);
#endif

#ifdef ENV_PLATFORMIO
    delay(5000); // to give time to restart serial monitor task
#endif
    
    
    initWebServer();
    initLora(&radio);

    return;
}