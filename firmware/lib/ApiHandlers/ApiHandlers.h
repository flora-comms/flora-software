// API Handlers
#include "ArduinoJson.h"

typedef enum { API_TYPE_NULL, API_TYPE_HISTORY, API_TYPE_MSG, API_TYPE_ACK } ApiType;
typedef enum { API_OK, API_ERROR_NO_DATA, API_ERROR_NO_TYPE, API_ERROR_DESERIALIZATION } ApiError;

class ApiHandler {
    public:
        ApiType type;
        JsonDocument * params;

        ApiHandler();
        ApiHandler(ApiType type, JsonDocument * params);

        ApiError fromJson(uint8_t *data);

        ApiError handle();
};