#include <LogList.h>
#include <driver/rtc_io.h>

/// @brief Power Managment handler instance
class FloraNetPower
{
private:
    TaskHandle_t _tskLora;
    TaskHandle_t _tskWeb;
    TaskHandle_t _tskProto;

    /// @brief Handles a ready to sleep event
    void handleSleep();
public:
    /// @brief Powermanager handler constructor
    /// @param lora the lora task handle
    /// @param web the web task handle
    /// @param proto the protcol task handle
    /// @param radio pointer to the lora radio module.
    FloraNetPower(TaskHandle_t lora, TaskHandle_t web, TaskHandle_t proto) { _tskLora = lora; _tskWeb = web; _tskProto = proto; }

    /// @brief Runs the power manager handler
    void run();
    
};

/// @brief The power managment task function
/// @param pvParameter Pass in pointer to a FloraNetPower instance here.
extern "C" void powerTask( void * pvParameter);