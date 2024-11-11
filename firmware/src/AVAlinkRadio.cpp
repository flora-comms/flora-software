#include <AVAlinkRadio.h>

// init globals

volatile bool operationDone = false;
int txState = RADIOLIB_ERR_NONE;
SPIClass lora_spi(HSPI);   // SPI3 lora module spi bus
SX1262 radio = new Module( // lora radio
    LORA_NSS,
    LORA_IRQ,
    LORA_NRST,
    LORA_BUSY,
    lora_spi,
    SPISettings(2000000, MSBFIRST, SPI_MODE0));


LoraPacket::LoraPacket() {
    destination = 0x00;
    packetId = 0x00;
    ttl = 0x00;
    senderId = 0x00;
    payload = String();
}
/// @brief Converts a queue messagef into a lora packet. consumes the message.
/// @param msg The QueueMessage object to convert into a Lora Packet.
LoraError LoraPacket::fromMsg(Message *msg) 
{
    packetId = 0;                   // TODO add protocol
    destination = 0xFF;             // TODO add SOS stuff
    payload = msg->payload;
    ttl = MAX_LORA_TTL;             // set max ttl
    senderId = msg->senderId;
    return LORA_ERR_NONE;
}

/// @brief Converts a lora packet into a que message
/// @param msg The Queue Message object to write into
/// @return A QParseError status code.
LoraError LoraPacket::toMsg(Message *msg) {
    
    msg->payload = payload;
    msg->senderId = senderId;
    if (destination != 0x00) {
        msg->type = TEXT;
    } else {
        msg->type = SOS;
    }
    return LORA_ERR_NONE;
}

/// @brief Converts a lora packet into a csv string that can be written to history.csv in the sd card
/// @return The SD-compatible formatted csv string.
String LoraPacket::toSdFormat() {
    String sdString;

    return sdString;
}

uint16_t LoraPacket::toBytes(uint8_t *bytes) {
    uint16_t len = 0;
    bytes[len++] = destination;     // byte 0
    bytes[len++] = senderId;        // byte 1
    bytes[len++] = packetId;        // byte 2
    bytes[len++] = ttl;             // byte 3
    const char *cstr = payload.c_str();
    while (*cstr != '\0') {
        bytes[len++] = *cstr++;
    }
    len++;
    bytes[len] = '\0';       // get total packet length
    return len;
}

LoraPacket::LoraPacket(uint8_t *bytes) {
    destination = *bytes++;
    senderId = *bytes++;
    packetId = *bytes++;
    ttl = *bytes++;
    payload = String((char *)bytes);
}


bool LoraPacket::needsRepeating() {
    return true;
}
/// @brief Lora interrupt handler
void onLoraIrq(void)
{
    operationDone = true;
}

/// @brief Lora task function
/// @param qFromWeb Recieves messages from Web task
/// @param qToWeb Sends messages to web task.
void loraTask( void * ) 
{

    LoraState loraState = LORA_RX;

    initLora();     // initialize hardware
    
    // Start Recieve
    radio.startReceive();

    while(true) {
        if (operationDone)
        {
            operationDone = false; // reset flag

            if (loraState == LORA_TX)
            {
                if (!(txState == RADIOLIB_ERR_NONE))
                {
                    DBG_PRINTLN("LoRa transmission failure");
                    DBG_PRINTF("Error code: %i", txState);
                }
                else
                {
                    DBG_PRINTLN("Transmission successful");
                    loraState = LORA_RX;
                    radio.startReceive();
                }
            }
            else if (loraState == LORA_RX)
            {
                int radio_state;
                uint8_t rx_data[256] = {0};     // create buffer of zeros
                radio_state = radio.readData(rx_data, 0);   // read in data
                if (!(radio_state == RADIOLIB_ERR_NONE))
                {
                    DBG_PRINTLN("Lora RX failure");
                }

                LoraPacket packet(rx_data);


                // TODO lora protocol layer - repeating action... see docs
                if (packet.needsRepeating()) {

                }
                Message *forWeb = new Message();

                packet.toMsg(forWeb);   //convert to message type and store on the heap

                // TODO determine if it needs to be written to the SD card

                xQueueSend(qFromLora, &forWeb, 0);   // send to the web server through the queue

                float rssi = radio.getRSSI();
                DBG_PRINTF("Rx Lora Data {\n\tSenderId: %i,\n\tPayload: \"%s\",\n\tRSSI: %.2f\n}", packet.senderId, packet.payload, rssi);
                radio.startReceive();       // start rx again.
            }
        }
        else if (0 < uxQueueMessagesWaiting(qToLora))    // if there's a message in the queue
        {
            Message *txMsg;
            
            xQueueReceive(qToLora, &txMsg, 0);
            LoraPacket packet;
            packet.fromMsg(txMsg);

            DBG_PRINTF("Web Server Message recieved: %s", packet.payload);

            delete(txMsg); // delete from heap.

            
            uint8_t bytes[256] = { 0 }; // TODO make sure web app doesn't let people send messages longer than 251 chars.

            uint16_t length = packet.toBytes(bytes);
            // DBG_PRINTLN("LORA BYTES:");
            // for (uint16_t i = 0; i < 256; i++) {
            //     DBG_PRINT(bytes[i]);
            // }
            // DBG_PRINTLN();
            loraState = LORA_TX;
            txState = radio.startTransmit(bytes, length);
            DBG_PRINTF("\nTx state: %i\n", txState);
        } else {
            vTaskDelay((TickType_t) 1);
        }
    }
}

/// @brief Initilizes Lora stuff
LoraError initLora()
{
    lora_spi.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
    DBG_PRINT("[SX1262] Initializing ... ");
    int status = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC, LORA_POWER, LORA_PREAMB);
    if (status == RADIOLIB_ERR_NONE)
    {
        DBG_PRINT("success!\n");
    }
    else
    {
        DBG_PRINTF("Failed: code %i\n", status);
        return LORA_ERR_INIT;
    }
    radio.setCurrentLimit(60.0);
    radio.setDio2AsRfSwitch(true);
    radio.explicitHeader();
    radio.setCRC(2);
    radio.setDio1Action(onLoraIrq);
    return LORA_ERR_NONE;
}