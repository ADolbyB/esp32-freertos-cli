/**
 * Joel Brigida
 * CDA 4102: Computer Architecture
 * 5/31/2023
 * This is an RTOS Example that implements semi-atomic tasks and controls the 2 LEDs
 * on the ESP32 Thing Plus C (GPIO_2 RGB LED & GPIO 13 Blue LED).
 * The user types commands into the Serial CLI handled by `msgTask`. There it is
 * checked if it is a valid command or not. If not a valid command, the message is printed 
 * to the terminal. If it is a valid command, it's sent to the `leds2and13` to be parsed
 * and the variables controlling LED output are changed inside that task.
 * This program only runs/requires 1 CPU core, but after it tunes up well on a single core,
 * I will activate both cores.
 */

#include <Arduino.h>
#include <FastLED.h>
#include "tasks.h"

#if CONFIG_FREERTOS_UNICORE
    static const BaseType_t app_cpu = 0;
#else
    static const BaseType_t app_cpu = 1;                                            // Only use CPU Core 1
#endif

/**
 * MultiCore: Change Core used when instantiating tasks
static const BaseType_t PRO_CPU = 0;                                                // Core 0 = Protocol CPU (WiFi/BT Stack)
static const BaseType_t APP_CPU = 1;  
*/

/** SETUP BEGIN **/
void setup()
{
    //msgQueue = xQueueCreateStatic(QueueSize, sizeof(Message), msgQueueStorage, &xMsgQueue);                            // Instantiate message queue
    //ledQueue = xQueueCreateStatic(QueueSize, sizeof(Command), ledQueueStorage, &xLedQueue);                            // Instantiate command queue
    //sdQueue = xQueueCreateStatic(QueueSize, sizeof(SDCommand), sdQueueStorage, &xSdQueue);                           // Instantiate SD Card Queue

    msgQueue = xQueueCreate(QueueSize, sizeof(Message));                            // Instantiate message queue
    ledQueue = xQueueCreate(QueueSize, sizeof(Command));                            // Instantiate command queue
    sdQueue = xQueueCreate(QueueSize, sizeof(SDCommand));                           // Instantiate SD Card Queue

    Serial.begin(115200);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.println("\n\n=>> ESP32 FreeRTOS Command Line Demo: LEDs & SD Card <<=");

    xTaskCreatePinnedToCore(
        userCLITask,
        "readInput",
        4096,
        NULL,
        1,
        &Task1,
        app_cpu
    );

    Serial.println("userCLITask Instantiation Complete...");

    xTaskCreatePinnedToCore(
        msgTask,
        "rxMsgs",
        4096,
        NULL,
        1,
        &Task2,
        app_cpu
    );

    Serial.println("msgTask Instantiation Complete...");

    xTaskCreatePinnedToCore(                                                        // Instantiate LED fade task
        led2And13Task,
        "leds2and13",
        4096,
        NULL,
        1,
        &Task3,
        app_cpu
    );

    Serial.println("led2Andled13 Task Instantiation Complete...");

    xTaskCreatePinnedToCore(                                                        // Instantiate LED fade task
        sdRXTask,
        "rxSdCard",
        4096,
        NULL,
        1,
        &Task4,
        app_cpu
    );
    
    Serial.println("sdRXTask Instantiation Complete...");                           // debug

    vTaskDelete(NULL);                                                              // Self Delete setup() & loop()
}

void loop() {}