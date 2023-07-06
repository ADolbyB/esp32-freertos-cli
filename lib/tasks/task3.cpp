/**
 * Joel Brigida
 * CDA 4102: Computer Architecture
 * Implementation File for Task3: This controls all LED operations.
 * June 28, 2023
*/

#include <Arduino.h>
#include "tasks.h"

void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax)   // 'value' must be between 0 & 'valueMax'
{
    uint32_t duty = (4095 / valueMax) * min(value, valueMax);                           // calculate duty cycle: 2^12 - 1 = 4095
    ledcWrite(channel, duty);                                                           // write duty cycle to LEDC
}

void led2And13Task(void* param)
{
    Serial.println("Starting Task 3....");
    Command ledCmd;                                                                 // Received from `msgTask`
    char buffer[BUF_LEN];
     
    int fadeInterval = 5;                                                           // LED fade interval
    int delayInterval = 30;                                                         // Delay between changing fade intervals
    int patternType = 1;                                                            // default LED pattern: case 1
    int brightVal = 250;                                                            // Brightness Command (Pattern 3) initial value
    int brightness = 65;                                                            // Initial Brightness value
    int ledDelay;                                                                   // blink delay in ms
    int fadeAmt;
    int pattern;
    int bright;

    short hueVal = 0;                                                               // add 32 each time for each color...
    bool swap = false;                                                              // Swap Red/Blue colors
    bool lightsOff = false;
    uint8_t accessLEDCAnalog = 1;

    CRGB leds[NUM_LEDS];
    FastLED.addLeds <CHIPSET, RGB_LED, COLOR_ORDER> (leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(75);
    leds[0] = CRGB::White;                                                          // Power up all Pin 2 LEDs for Power On Test
    FastLED.show();

    Serial.println("Inside Task3: FastLED Setup Done.");

    ledcSetup(LEDCchan, LEDCfreq, LEDCtimer);                                       // Setup LEDC timer (For LED_BUILTIN)
    ledcAttachPin(BLUE_LED, LEDCchan);                                              // Attach timer to LED pin

    Serial.println("Inside Task 3: LEDC Setup done...Pause for 1 sec....");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    leds[0] = CRGB::Black;
    FastLED.show();
    
    Serial.println("Task 3: Entering for(;;) loop...");

    while(1)
    {
        Serial.println("Inside Task 3: for(;;) loop...");
        /*** Command Handling ***/
        if(ledQueue != NULL)
        {
            if(xQueueReceive(ledQueue, (void *)&buffer, 0) == pdTRUE)                   // if LED command received from MSG QUEUE
            {
                strcpy(ledCmd.cmd, buffer);
                memset(buffer, 0, BUF_LEN); 
                /* LED Commands */
                if(memcmp(ledCmd.cmd, allCommands[LED_L], strlen(allCommands[LED_L])) == 0)  // Check for `delay ` command: Ref: https://cplusplus.com/reference/cstring/memcmp/
                {
                    ledDelay = ledCmd.amount;
                    // char* tailPtr = ledCmd.cmd + strlen(allCommands[LED_L]);            // pointer arithmetic: move pointer to integer value
                    // ledDelay = atoi(tailPtr);                                           // retreive integer value at end of string
                    // ledDelay = abs(ledDelay);                                           // ledDelay can't be negative
                    if(ledDelay <= 0)
                    {
                        Serial.println("Value Must Be > 0");
                        Serial.println("Returning....");
                        continue;
                    }
                    delayInterval = ledDelay;
                    sprintf(buffer, "New Delay Value: %dms\n\n", ledCmd.amount);
                    Serial.print(buffer);
                    memset(buffer, 0, BUF_LEN);  
                }
                else if(memcmp(ledCmd.cmd, allCommands[LED_L + 1], strlen(allCommands[LED_L + 1])) == 0) // Check for `fade ` command
                {
                    fadeAmt = ledCmd.amount;
                    // char* tailPtr = ledCmd.cmd + strlen(allCommands[LED_L  + 1]);       // pointer arithmetic: move pointer to integer value
                    // fadeAmt = atoi(tailPtr);                                            // retreive integer value at end of string
                    fadeAmt = abs(fadeAmt);                                             // fadeAmt can't be negative
                    if(fadeAmt <= 0 || fadeAmt > 128)
                    {
                        Serial.println("Value Must Be Between 1 & 128");
                        Serial.println("Returning....");
                        continue;
                    }
                    fadeInterval = fadeAmt;
                    sprintf(buffer, "New Fade Value: %d\n\n", ledCmd.amount);           // BUGFIX: sometimes displays negative number
                    Serial.print(buffer);                
                    memset(buffer, 0, BUF_LEN); 
                }
                else if(memcmp(ledCmd.cmd, allCommands[LED_L + 2], strlen(allCommands[LED_L + 2])) == 0)  // Check for `pattern ` command
                {
                    // char* tailPtr = ledCmd.cmd + strlen(allCommands[LED_L + 2]);        // pointer arithmetic: move pointer to integer value
                    // pattern = atoi(tailPtr);                                            // retreive integer value at end of string
                    // pattern = abs(pattern);                                             // patternType can't be negative
                    
                    
                    patternType = ledCmd.amount;
                    patternType = abs(patternType);
                    if(int(abs(patternType)) <= NUM_PATTERNS && int(patternType) != 0)  // BUGFIX: "New Pattern: 0" with invalid entry
                    {
                        sprintf(buffer, "New Pattern: %d\n\n", ledCmd.amount);
                        Serial.print(buffer);
                        memset(buffer, 0, BUF_LEN);
                    }
                }
                else if(memcmp(ledCmd.cmd, allCommands[LED_L + 3], strlen(allCommands[LED_L + 3])) == 0)    // Check for `bright ` command
                {
                    bright = ledCmd.amount;
                    bright = abs(bright);
                    // char* tailPtr = ledCmd.cmd + strlen(allCommands[LED_L + 3]));       // pointer arithmetic: move pointer to integer value
                    // bright = atoi(tailPtr);                                             // retreive integer value at end of string
                    // bright = abs(bright);                                               // ledDelay can't be negative
                    brightVal = bright;                
                    if(brightVal >= 255)
                    {
                        Serial.println("Maximum Value 255...");
                        brightVal = 255;
                    }               
                    sprintf(buffer, "New Brightness: %d / 255\n\n", ledCmd.amount);
                    Serial.print(buffer);
                    memset(buffer, 0, BUF_LEN);
                }
                else if(memcmp(ledCmd.cmd, allCommands[LED_H], strlen(allCommands[LED_H])) == 0)                        // check for `cpu ` command
                {
                    // List all LED Values
                    sprintf(buffer, "Listing All Current LED Values: \n");
                    Serial.print(buffer);
                    memset(buffer, 0, BUF_LEN);
                    sprintf(buffer, "Current Delay = %dms.           (default = 30ms)\n", delayInterval);
                    Serial.print(buffer);
                    memset(buffer, 0, BUF_LEN);
                    sprintf(buffer, "Current Fade Interval = %d.      (default = 5)\n", abs(fadeInterval));
                    Serial.print(buffer);
                    memset(buffer, 0, BUF_LEN);
                    sprintf(buffer, "Current Pattern = %d.            (default = 1)\n", patternType);
                    Serial.print(buffer);
                    memset(buffer, 0, BUF_LEN);
                    sprintf(buffer, "Current Brightness = %d / 255. (default = 250)\n\n", brightVal);
                    Serial.print(buffer);
                    memset(buffer, 0, BUF_LEN);
                }
            }
            else /*** Show LED Patterns ***/
            {
                if(patternType == 1)                                                // Fade On/Off & cycle through 8 colors
                {
                    lightsOff = false;
                    if(accessLEDCAnalog == 1)
                    {
                        ledcAnalogWrite(LEDCchan, 0);                               // Only Need to do this ONCE
                        accessLEDCAnalog = 0;
                    }
                    brightness += fadeInterval;                                     // Adjust brightness by fadeInterval
                    if(brightness <= 0)                                             // Only change color if value <= 0
                    {
                        brightness = 0;
                        fadeInterval = -fadeInterval;                               // Reverse fade effect
                        hueVal += 32;                                               // Change color
                        if(hueVal >= 255)
                        {
                            hueVal = 0;                         
                        }
                        leds[0] = CHSV(hueVal, 255, 255);                           // Rotate: Rd-Orng-Yel-Grn-Aqua-Blu-Purp-Pnk
                    }
                    else if(brightness >= 255)
                    {
                        brightness = 255;
                        fadeInterval = -fadeInterval;                               // Reverse fade effect
                    }
                    FastLED.setBrightness(brightness);
                    FastLED.show();
                }
                else if(patternType == 2)                                           // Fade On/Off Red/Blue Police Pattern
                {
                    lightsOff = false;
                    if(accessLEDCAnalog == 1)
                    {
                        ledcAnalogWrite(LEDCchan, 0);                               // Only Need to do this ONCE
                        accessLEDCAnalog = 0;
                    }
                    brightness += fadeInterval;                                     // Adjust brightness by fadeInterval
                    if(brightness <= 0)                                             // Only change color if value <= 0
                    {
                        brightness = 0;
                        fadeInterval = -fadeInterval;                               // Reverse fade effect
                        swap = !swap;                                               // swap colors
                        if(swap)
                        {
                            leds[0] = CRGB::Blue;
                        }
                        else
                        {
                            leds[0] = CRGB::Red;
                        }
                    }
                    else if(brightness >= 255)
                    {
                        brightness = 255;
                        fadeInterval = -fadeInterval;                               // Reverse fade effect
                    }
                    FastLED.setBrightness(brightness);
                    FastLED.show();
                }
                else if(patternType == 3)                                           // Rotate Colors w/o fade
                {
                    lightsOff = false;
                    if(accessLEDCAnalog == 1)
                    {
                        ledcAnalogWrite(LEDCchan, 0);                               // Only Need to do this ONCE
                        accessLEDCAnalog = 0;
                    }
                    brightness = brightVal;                                         // Pull value from global integer
                    hueVal += fadeInterval;                                         // Change color based on global value
                    if(hueVal >= 255)
                    {
                        hueVal = 0;                         
                    }
                    leds[0] = CHSV(hueVal, 255, 255);                               // Rotate Colors 0 - 255
                    FastLED.setBrightness(brightness);
                    FastLED.show();
                }
                else if(patternType == 4)                                           // Blue LED (Pin 13) Fades on/off
                {
                    lightsOff = false;
                    if(accessLEDCAnalog == 0)
                    {
                        leds[0] = CRGB::Black;                                      // Turn Off RGB LED
                        FastLED.show();
                        accessLEDCAnalog = 1;                                       // Only need to do this ONCE
                    }
                    brightness += fadeInterval;                                     // Adjust brightness by fadeInterval
                    if(brightness <= 0)                                             // Reverse fade effect at min/max values
                    {
                        brightness = 0;
                        fadeInterval = -fadeInterval;
                    }
                    else if(brightness >= 255)
                    {
                        brightness = 255;
                        fadeInterval = -fadeInterval;
                    }
                    ledcAnalogWrite(LEDCchan, brightness);                          // Set brightness on LEDC channel 0
                }
                else if(patternType == 5)                                           // Blue LED (pin 13) Cycles on/off
                {
                    lightsOff = false;
                    if(accessLEDCAnalog == 0)
                    {
                        leds[0] = CRGB::Black;                                      // Turn Off RGB LED
                        FastLED.show();
                        accessLEDCAnalog = 1;                                       // Only need to do this ONCE
                    }
                    swap = !swap;
                    if(swap)
                    {
                        ledcAnalogWrite(LEDCchan, 255);
                    }
                    else
                    {
                        ledcAnalogWrite(LEDCchan, 0);
                    }
                }
                else                                                                // Turn Off Everything
                {
                    if(lightsOff == false)
                    {
                        lightsOff = true;
                        if(accessLEDCAnalog == 0)
                        {
                            leds[0] = CRGB::Black;                                  // Turn Off RGB LED
                            FastLED.show();
                        }
                        else // RGB LED already off
                        {
                            ledcAnalogWrite(LEDCchan, 0);                           // Turn off Blue LED
                        }
                        Serial.println("Invalid Pattern...Turning Lights Off!!\n");
                    }
                }
            }
        }
        else
        {
            Serial.println("Error! ledQueue == NULL");
        }
        vTaskDelay(delayInterval / portTICK_PERIOD_MS);                         // CLI adjustable delay (non blocking)
    }
}