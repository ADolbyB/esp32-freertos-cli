/**
 * Joel Brigida
 * CDA-4102: Computer Architecture
 * Header file for task functions (inf loops)
 * June 28, 2023
*/

#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>
#include <FastLED.h>

// Pins:
#define SD_CS 5                                                             
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23
#define RGB_LED 2                                                               // Pin 2 on Thing Plus C is connected to WS2812 LED
#define BLUE_LED 13                                                             // Pin 13 is On-Board Blue LED

#define COLOR_ORDER GRB                                                         // RGB LED in top right corner
#define CHIPSET WS2812                                                          // Chipset for On-Board RGB LED
#define NUM_LEDS 1                                                              // Only 1 RGB LED on the ESP32 Thing Plus
#define NUM_PATTERNS 5                                                          // Total number of LED patterns

static const uint8_t BUF_LEN = 255;                                             // Buffer Length setting for user CLI terminal

static const int LEDCchan = 0;                                                  // use LEDC Channel 0
static const int LEDCtimer = 12;                                                // 12-bit precision LEDC timer
static const int LEDCfreq = 5000;                                               // 5000 Hz LEDC base freq.
static const int LEDpin = LED_BUILTIN;                                          // Use pin 13 on-board LED for SW fading
static const int QueueSize = 5;                                                 // 5 elements in any Queue

static QueueHandle_t msgQueue;                                                  // Queue for CLI messages
static QueueHandle_t ledQueue;                                                  // Queue to LED commands
static QueueHandle_t sdQueue;                                                   // Queue to handle SD card commands
    
    /** LED Commands **/

static const char delayCmd[] = "delay ";                                        // STRLEN = 6: delay command definition
static const char fadeCmd[] = "fade ";                                          // STRLEN = 5: fade command definition
static const char patternCmd[] = "pattern ";                                    // STRLEN = 8: pattern command definition
static const char brightCmd[] = "bright ";                                      // STRLEN = 7: brightness command (pattern 3 only)
static const char getValues[] = "values";                                       // STRLEN = 6: show values of all user variables

    /** SD Card Commands **/

static const char sdListCmds[] = "lssd";                                       // STRLEN = 5: prints a list of SD commands (from msgQueue)
static const char sdListDir[] = "lsdir ";                                       // STRLEN = 6: List subdirectories under given argument
static const char sdCreateDir[] = "mkdir ";                                     
static const char sdDeleteDir[] = "rmdir ";
static const char sdReadFile[] = "readfile ";
static const char sdWriteFile[] = "writefile ";
static const char sdAppendFile[] = "append ";
static const char sdRenameFile[] = "rename ";
static const char sdDeleteFile[] = "rmfile ";
static const char sdUsedSpace[] = "lsbytes";

    /** Other Commands **/

static const char mainCmds[] = "lscmd";
static const char cpuCmd[] = "cpu ";                                            // STRLEN = 4: cpu speed control command
static const char getFreq[] = "freq";                                           // STRLEN = 4: show values for freq


struct Message                                                                  // Struct for CLI input
{
    char msg[80];                                                               // User Input
};

struct Command                                                                  // Sent from `msgRXTask` to `RGBcolorWheelTask`
{
    char cmd[25];
    int amount;
};

struct SDCommand                                                                // Sent from `msgRXTask` to `SDCardTask`
{
    char cmd[25];
    char msg[80];                                                               // default length of Bash Terminal Line
};

void sdRXTask(void *param);
void userCLITask(void *param);
void msgRXTask(void *param);
void led2And13Task(void *param);

inline void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255)        // 'value' must be between 0 & 'valueMax'
{
    uint32_t duty = (4095 / valueMax) * min(value, valueMax);                   // calculate duty cycle: 2^12 - 1 = 4095
    ledcWrite(channel, duty);                                                   // write duty cycle to LEDC
};

#endif