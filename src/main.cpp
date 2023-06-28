/**
 * Joel Brigida
 * 5/31/2023
 * This is an RTOS Example that implements semi-atomic tasks and controls the 2 LEDs
 * on the ESP32 Thing Plus C (GPIO_2 RGB LED & GPIO 13 Blue LED).
 * The user types commands into the Serial CLI handled by `userCLITask`. There it is
 * checked if it is a valid command or not. If not a valid command, the message is printed 
 * to the terminal. If it is a valid command, it's sent to the `RGBcolorTask` to be parsed
 * and the variables controlling LED output are changed inside that task.
 * This program only runs/requires 1 CPU core, but after it tunes up well on a single core,
 * I will activate both cores.
 */

#include <Arduino.h>
#include <FastLED.h>
#include "SPI.h" // can be <SPI.h>
#include "FS.h"
#include "SD.h"
#include "tasks.h"


#if CONFIG_FREERTOS_UNICORE
    static const BaseType_t app_cpu = 0;
#else
    static const BaseType_t app_cpu = 1;                                        // Only use CPU Core 1
#endif

/**
 * MultiCore: Change Core used when instantiating tasks
static const BaseType_t PRO_CPU = 0;                                            // Core 0 = Protocol CPU (WiFi/BT Stack)
static const BaseType_t APP_CPU = 1;  
*/

void Init()
{
    msgQueue = xQueueCreate(QueueSize, sizeof(Message));                        // Instantiate message queue
    ledQueue = xQueueCreate(QueueSize, sizeof(Command));                        // Instantiate command queue
    sdQueue = xQueueCreate(QueueSize, sizeof(Command));                         // Instantiate SD Card Queue
    Serial.begin(115200);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.println("\n\n=>> ESP32 FreeRTOS Command Line Demo: LEDs & SD Card <<=");
}

void createTasks()
{
    xTaskCreatePinnedToCore(
        userCLITask,
        "Read User Input",
        2048,
        NULL,
        1,
        NULL,
        app_cpu
    );

    Serial.println("User CLI Task Instantiation Complete...");

    xTaskCreatePinnedToCore(
        msgRXTask,
        "Receive User Input",
        2048,
        NULL,
        1,
        NULL,
        app_cpu
    );

    Serial.println("Message RX Task Instantiation Complete...");

    xTaskCreatePinnedToCore(                                                    // Instantiate LED fade task
        RGBcolorWheelTask,
        "Fade and Rotate RGB LEDs",
        2048,
        NULL,
        1,
        NULL,
        app_cpu
    );

    Serial.println("LED Task Instantiation Complete...");

    xTaskCreatePinnedToCore(                                                    // Instantiate LED fade task
        sdRXTask,
        "Receive SD Commands",
        2048,
        NULL,
        1,
        NULL,
        app_cpu
    );
    
    Serial.println("SD Card Task Instantiation Complete...");                      // debug

}

void showAllCommands()
{
    // print all available CLI commands to the Terminal
    Serial.print("\n\nEnter \'delay xxx\' to change RGB Fade Speed.\n");
    Serial.print("Enter \'fade xxx\' to change RGB Fade Amount.\n");
    Serial.print("Enter \'pattern xxx\' to change RGB Pattern.\n");
    Serial.print("Enter \'bright xxx\' to change RGB Brightness (Only Pattern 3).\n");
    Serial.print("Enter \'cpu xxx\' to change CPU Frequency.\n");
    Serial.print("Enter \'values\' to retrieve current delay, fade, pattern & bright values.\n");
    Serial.print("Enter \'freq\' to retrieve current CPU, XTAL & APB Frequencies.\n\n");
}

/** SETUP BEGIN **/
void setup()
{

    Init();     // Need to run prefunctions in one function call
    
    createTasks();
    showAllCommands();

    // Need to declare an object here to run functions...
    // This is like the `main` but no return: just inf loop
    // can pass the queues in as function argument since the whole operation revolves around sorting commands.

    vTaskDelete(NULL);                                                          // Self Delete setup() & loop()
}

void loop() {}