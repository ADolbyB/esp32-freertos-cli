/** 
 * Joel Brigida
 * CDA 4610: Computer Architecture
 * June 30, 2023
 * These are the top level global 
 * variable constants: Every cpp/h file includes this
 * to reduce the clutterance of program overhead.
*/

#ifndef ALLLIBS_H
#define ALLLIBS_H

#include <SPI.h>
#include <Arduino.h>
#include <FastLED.h>
#include "FS.h"
#include "SD.h"
#include "testInput.h"
#include "tasks.h"

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
    
#endif