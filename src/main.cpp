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
    
    msgQueue = xQueueCreate(QueueSize, sizeof(Message));                            // Instantiate message queue
    ledQueue = xQueueCreate(QueueSize, sizeof(Command));                            // Instantiate command queue
    sdQueue = xQueueCreate(QueueSize, sizeof(SDCommand));                           // Instantiate SD Card Queue
    
    CRGB leds[NUM_LEDS];                                                            // Array declaration for RGB LED on GPIO_2

    Serial.begin(115200);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.println("\n\n=>> ESP32 FreeRTOS Command Line Demo: LEDs & SD Card <<=");

    /** Init LEDs & Functions **/

    //CRGB leds[NUM_LEDS];

    // FastLED.addLeds <CHIPSET, RGB_LED, COLOR_ORDER> (leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    // FastLED.setBrightness(75);

    // Serial.println("Brightness Set....Turning On RGB LED now.");
    
    // leds[0] = CRGB::White;                                                          // Power up all Pin 2 LEDs for Power On Test
    // FastLED.show();
    
    // ledcSetup(LEDCchan, LEDCfreq, LEDCtimer);                                       // Setup LEDC timer (For LED_BUILTIN)
    // ledcAttachPin(BLUE_LED, LEDCchan);                                              // Attach timer to LED pin
    // vTaskDelay(2000 / portTICK_PERIOD_MS);                                          // 2 Second Power On Delay

    // Serial.println("Power On Test Complete...");
    // leds[0] = CRGB::Black;
    // FastLED.show();
    // vTaskDelay(500 / portTICK_PERIOD_MS);                                           // 0.5 Second off before Starting Tasks

    // Serial.println("...Beginning Task Creation....");

    xTaskCreatePinnedToCore(
        userCLITask,
        "Read Raw User Input",
        4096,
        NULL,
        1,
        NULL,
        app_cpu
    );

    Serial.println("userCLITask Instantiation Complete...");

    xTaskCreatePinnedToCore(
        msgTask,
        "rxusermesgs",
        4096,
        NULL,
        1,
        NULL,
        app_cpu
    );

    Serial.println("msgTask Instantiation Complete...");

    xTaskCreatePinnedToCore(                                                    // Instantiate LED fade task
        led2And13Task,
        "led2andled13",
        4096,
        NULL,
        1,
        NULL,
        app_cpu
    );

    Serial.println("led2Andled13 Task Instantiation Complete...");

    xTaskCreatePinnedToCore(                                                    // Instantiate LED fade task
        sdRXTask,
        "rxsdelsesleep",
        4096,
        NULL,
        1,
        NULL,
        app_cpu
    );
    
    Serial.println("sdRXTask Instantiation Complete...");                      // debug

    vTaskDelete(NULL);                                                          // Self Delete setup() & loop()
}

void loop() {}