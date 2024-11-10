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

    // Set up Wi-Fi (AP mode)
    DBG_PRINTLN("Setting up Access Point with SSID: ");
    DBG_PRINTLN(WIFI_SSID);
    if (WiFi.softAP(WIFI_SSID))
    {
        DBG_PRINTLN("Access Point setup complete");
    }
    else
    {
        DBG_PRINTLN("Failed to set up Access Point");
    }

    DBG_PRINTLN("Access Point IP address: ");
    DBG_PRINTLN(WiFi.softAPIP());

    // Setup SPI busses
    sd_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    lora_spi.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);

    // Initialize the SD card

#ifdef LILYGO_T3_S3_V1_2
#ifdef ENV_ARDUINO
#define REASSIGN_PINS
    int sck = SD_SCK;
    int mosi = SD_MOSI;
    int miso = SD_MISO;
#endif
#endif

    if (!SD.begin(SD_CS))
    {
        DBG_PRINTLN("Failed to mount SD card!");
        return;
    }
    else
    {
        DBG_PRINTLN("SD card mounted successfully.");
    }

    // initialize SX1262 with default settings
    Serial.print(F("[SX1262] Initializing ... "));
    int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC, LORA_POWER, LORA_PREAMB);
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {
        Serial.print(F("failed, code "));
        Serial.println(state);
    }
    radio.setCurrentLimit(60.0);
    radio.setDio2AsRfSwitch(true);
    radio.explicitHeader();
    radio.setCRC(2);
    radio.setDio1Action(onLoraIrq);

    // Start server

    server.begin();
    DBG_PRINTLN("Web server started!");

    server.serveStatic("/", SD, "/").setDefaultFile("index.html");
    server.addHandler(&ws);

    // start DNS

    if (!MDNS.begin(WEBSERVER_DNS))
    {
        DBG_PRINTLN("Error setting up MDNS responder!");
    }

    MDNS.addService("http", "tcp", 80);

    // add websocket service
    ws.onEvent(onWsEvent);
    return;
}