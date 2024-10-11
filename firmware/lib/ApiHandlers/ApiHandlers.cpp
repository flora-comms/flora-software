#include "ApiHandlers.h"

ApiHandler::ApiHandler()
    :type(API_TYPE_NULL)
    ,params(nullptr)
    {

    }

// requires that params exists
ApiHandler::ApiHandler(ApiType apitype, JsonDocument * apiparams)
    :type(apitype)
    ,params(nullptr)
{
    memcpy(params, apiparams, apiparams->size());
}

// returns NO DATA error if data is empty
// returns DESERIALIZATION error if unable to deserialize
ApiError ApiHandler::fromJson(uint8_t *data) {
    if(!data) {
        return API_ERROR_NO_DATA;
    }

    JsonDocument apiparams;

    DeserializationError error = deserializeJson(apiparams, data);

    if(error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return API_ERROR_DESERIALIZATION;
    }
    ApiType apitype = apiparams["type"];

    memcpy(params, &apiparams, apiparams.size());
};

// ApiError ApiHandler::handle() {
//     switch(type) {
//         case API_TYPE_HISTORY:
//             populate();
//             break;
//         case API_TYPE_MSG:
//             break;
//         case API_TYPE_ACK:
//             break;
//         case API_TYPE_NULL:
//             return API_ERROR_NO_TYPE;
//     }
// }

// void populate() {};