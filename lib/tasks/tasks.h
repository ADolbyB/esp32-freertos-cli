/**
 * Joel Brigida
 * CDA 4102: Computer Architecture
 * Header file for task functions.
 * June 28, 2023
*/

#ifndef TASKS_H
#define TASKS_H

#include <SPI.h>
#include <Arduino.h>
#include <FastLED.h>
#include "FS.h"
#include "SD.h"

#define SD_CS 5                                                             
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23
#define RGB_LED 2                                                                       // Pin 2 on Thing Plus C is connected to WS2812 LED
#define BLUE_LED 13                                                                     // Pin 13 is On-Board Blue LED

#define COLOR_ORDER GRB                                                                 // RGB LED in top right corner
#define CHIPSET WS2812                                                                  // Chipset for On-Board RGB LED
#define NUM_LEDS 1                                                                      // Only 1 RGB LED on the ESP32 Thing Plus
#define NUM_PATTERNS 5                                                                  // Total number of LED patterns

static const uint8_t BUF_LEN = 80;                                                      // Buffer Length setting for user CLI terminal
static const int LEDCchan = 0;                                                          // use LEDC Channel 0
static const int LEDCtimer = 12;                                                        // 12-bit precision LEDC timer
static const int LEDCfreq = 5000;                                                       // 5000 Hz LEDC base freq.
static const int LEDpin = LED_BUILTIN;                                                  // Use pin 13 on-board LED for SW fading
static const int QueueSize = 5;                                                         // 5 elements in any Queue

//static StaticQueue_t xMsgQueue;                                                       // Queue for CLI messages
//static StaticQueue_t xLedQueue;                                                       // Queue to LED commands
//static StaticQueue_t xSdQueue;                                                        // Queue to LED commands

static QueueHandle_t msgQueue;
static QueueHandle_t ledQueue;
static QueueHandle_t sdQueue;                                                           // Queue to handle SD card commands   

static TaskHandle_t Task1;
static TaskHandle_t Task2;
static TaskHandle_t Task3;
static TaskHandle_t Task4;

enum Limits : int {                                                                     // Change Limits when adding / subtracting tasks
    CPU_L = 0, CPU_H = 2, LED_L = 3, LED_H = 7, SD_L = 8, SD_H = 17 
};

static const char allCommands[][15] = {
    "lscmd", "cpu ", "freq",                                                            // CPU Functions [0, 2]
    "delay ", "fade ", "pattern ", "bright ", "values",                                 // LED Functions [3, 7]
    "lssd", "lsdir ", "mkdir ", "rmdir ", "readfile ",                                  // SD Functions
    "writefile ", "append ", "rename ", "rmfile ", "lsbytes"                            // SD Functions [8, 17]
};

typedef struct Message {                                                                // Struct for CLI input
    char msg[15];                                                                       // same size as each allCommands[] entry
} Message;

typedef struct Command {                                                                // Sent from `msgRXTask()` to `led2And13Task()`
    char cmd[12];
    int amount;
} Command;

typedef struct SDCommand {                                                              // Sent from `msgRXTask()` to `sdCardTask()`
    char cmd[10];
    char msg[5];                                                                        // default length of Bash Terminal Line
} SDCommand;

//static uint8_t msgQueueStorage[QueueSize * sizeof(Message)];
//static uint8_t ledQueueStorage[QueueSize * sizeof(Command)];
//static uint8_t sdQueueStorage[QueueSize * sizeof(SDCommand)];

void userCLITask(void* );
void msgTask(void* );
void led2And13Task(void* );
void sdRXTask(void* );

int testInput(const char* userInput);
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255);

#endif