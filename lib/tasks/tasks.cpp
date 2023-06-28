/**
 * Joel Brigida
 * CDA 4102: Computer Architecture
 * 
*/
#include <Arduino.h>
#include "tasks.h"

void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax)  // 'value' must be between 0 & 'valueMax'
{
    uint32_t duty = (4095 / valueMax) * min(value, valueMax);                   // calculate duty cycle: 2^12 - 1 = 4095
    ledcWrite(channel, duty);                                                   // write duty cycle to LEDC
}

void userCLITask(void *param)                                                   // Function definition for user CLI task
{
    Message sendMsg;                                                            // Declare user message
    char input;                                                                 // Each char of user input                                             
    char buffer[255];                                                       // buffer to hold user input
    uint8_t index = 0;                                                          // character count

    memset(buffer, 0, 255);                                                 // Clear user input buffer

    for(;;)
    {       
        if(Serial.available() > 0)
        {
            input = Serial.read();                                              // read each character of user input

            if(index < (255 - 1))
            {
                buffer[index] = input;                                          // write received character to buffer
                index++;
            }
            if(input == '\n')                                                   // Check when user presses ENTER key
            {
                Serial.print("\n");
                strcpy(sendMsg.msg, buffer);                                    // copy input to Message node
                xQueueSend(msgQueue, (void *)&sendMsg, 10);                     // Send to msgQueue for interpretation
                memset(buffer, 0, 255);                                     // Clear input buffer
                index = 0;                                                      // Reset index counter.
            }
            else // echo each character back to the serial terminal
            {
                Serial.print(input);
            }
        }
        vTaskDelay(25 / portTICK_PERIOD_MS);                                    // yield to other tasks
    }
}

void msgRXTask(void *param) /*** CLI Input Validation / Handling ***/           /*** Analyze Each Node **/
{
    Message someMsg;                                                            // Each object given from the user
    Command someCmd;
    SDCommand sdCardCmd;                                                        // New object for SD Card Comms
    uint8_t localCPUFreq;                                                       // 80, 160 or 240Mhz
    char buffer[255];                                                       // string buffer for Terminal Message
    short ledDelay;                                                             // blink delay in ms
    short fadeAmt;
    short pattern;
    short bright;

    memset(buffer, 0, 255);                                                 // Clear input buffer

    for(;;)
    {
        if(xQueueReceive(msgQueue, (void *)&someMsg, 0) == pdTRUE)              // If a `Message` is rec'd from queue
        {   
            /* LED Commands */
            if(memcmp(someMsg.msg, fadeCmd, 5) == 0)                            // Check for `fade ` command: Ref: https://cplusplus.com/reference/cstring/memcmp/
            {
                char* tailPtr = someMsg.msg + 5;                                // pointer arithmetic: move pointer to integer value
                fadeAmt = atoi(tailPtr);                                        // retreive integer value at end of string
                fadeAmt = abs(fadeAmt);                                         // fadeAmt can't be negative
                if(fadeAmt <= 0 || fadeAmt > 128)
                {
                    Serial.println("Value Must Be Between 1 & 128");
                    Serial.println("Returning....");
                    continue;
                }
                strcpy(someCmd.cmd, "fade");
                someCmd.amount = fadeAmt;                                       // copy input to Command node
                xQueueSend(ledQueue, (void *)&someCmd, 10);                     // Send to ledQueue for interpretation
            }
            else if(memcmp(someMsg.msg, delayCmd, 6) == 0)                      // Check for `delay ` command
            {
                char* tailPtr = someMsg.msg + 6;                                // pointer arithmetic: move pointer to integer value
                ledDelay = atoi(tailPtr);                                       // retreive integer value at end of string
                ledDelay = abs(ledDelay);                                       // ledDelay can't be negative
                if(ledDelay <= 0)
                {
                    Serial.println("Value Must Be > 0");
                    Serial.println("Returning....");
                    continue;
                }
                strcpy(someCmd.cmd, "delay");
                someCmd.amount = ledDelay;                                      // copy input to Command node
                xQueueSend(ledQueue, (void *)&someCmd, 10);                     // Send to ledQueue for interpretation                    // Clear input buffer
            }
            else if(memcmp(someMsg.msg, patternCmd, 8) == 0)                    // Check for `pattern ` command
            {
                char* tailPtr = someMsg.msg + 8;                                // pointer arithmetic: move pointer to integer value
                pattern = atoi(tailPtr);                                        // retreive integer value at end of string
                pattern = abs(pattern);                                         // patternType can't be negative

                strcpy(someCmd.cmd, "pattern");
                someCmd.amount = pattern;                                       // copy input to Command node
                xQueueSend(ledQueue, (void *)&someCmd, 10);                     // Send to ledQueue for interpretation
            }
            else if(memcmp(someMsg.msg, brightCmd, 7) == 0)                     // Check for `bright ` command
            {
                char* tailPtr = someMsg.msg + 7;                                // pointer arithmetic: move pointer to integer value
                bright = atoi(tailPtr);                                         // retreive integer value at end of string
                bright = abs(bright);                                           // ledDelay can't be negative
                
                strcpy(someCmd.cmd, "bright");
                someCmd.amount = bright;                                        // copy input to Command node
                xQueueSend(ledQueue, (void *)&someCmd, 10);                     // Send to ledQueue for interpretation
            }
            else if(memcmp(someMsg.msg, cpuCmd, 4) == 0)                        // check for `cpu ` command
            {
                char* tailPtr = someMsg.msg + 4;
                localCPUFreq = atoi(tailPtr);
                localCPUFreq = abs(localCPUFreq);

                strcpy(someCmd.cmd, "cpu");
                someCmd.amount = localCPUFreq;                                  // copy input to Message node
                xQueueSend(ledQueue, (void *)&someCmd, 10);                     // Send to ledQueue for interpretation
            }
            else if(memcmp(someMsg.msg, getValues, 6) == 0)
            {
                strcpy(someCmd.cmd, "values");
                someCmd.amount = 0;                                             // copy input to Message node
                xQueueSend(ledQueue, (void *)&someCmd, 10);                     // Send to ledQueue for interpretation
            }
            else if(memcmp(someMsg.msg, getFreq, 4) == 0)
            {
                strcpy(someCmd.cmd, "freq");
                someCmd.amount = 0;                                             // copy input to Message node
                xQueueSend(ledQueue, (void *)&someCmd, 10);                     // Send to ledQueue for interpretation
            }
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);                                    // Yield to other tasks
    }
}

void RGBcolorWheelTask(void *param)
{
    /** Init LEDs & Functions **/
    
    CRGB leds[NUM_LEDS];                                                            // Array for RGB LED on GPIO_2
    FastLED.addLeds <CHIPSET, RGB_LED, COLOR_ORDER> (leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(75);
    leds[0] = CRGB::White;                                                      // Power up all Pin 2 LEDs for Power On Test
    FastLED.show();
    
    ledcSetup(LEDCchan, LEDCfreq, LEDCtimer);                                   // Setup LEDC timer (For LED_BUILTIN)
    ledcAttachPin(BLUE_LED, LEDCchan);                                          // Attach timer to LED pin
    vTaskDelay(2000 / portTICK_PERIOD_MS);                                      // 2 Second Power On Delay

    Serial.println("Power On Test Complete...");
    leds[0] = CRGB::Black;
    FastLED.show();
    vTaskDelay(500 / portTICK_PERIOD_MS);                                       // 0.5 Second off before Starting Tasks

    Command someCmd;                                                            // Received from `msgRXTask`
    uint8_t localCPUFreq;
    char buffer[255];
     
    int fadeInterval = 5;                                                       // LED fade interval
    int delayInterval = 30;                                                     // Delay between changing fade intervals
    int patternType = 1;                                                        // default LED pattern: case 1
    int brightVal = 250;                                                        // Brightness Command (Pattern 3) initial value
    int brightness = 65;                                                        // Initial Brightness value

    short hueVal = 0;                                                           // add 32 each time for each color...
    bool swap = false;                                                          // Swap Red/Blue colors
    bool lightsOff = false;
    uint8_t accessLEDCAnalog = 1;
    leds[0] = CRGB::Red;                                                        // Start with Red LED when instantiated
    FastLED.show();

    for(;;)
    {
        /*** Command Handling ***/
        if(xQueueReceive(ledQueue, (void *)&someCmd, 0) == pdTRUE)              // if command received from MSG QUEUE
        {
            if(memcmp(someCmd.cmd, fadeCmd, 4) == 0)                            // if `fade` command rec'd (compare to global var)
            {
                fadeInterval = someCmd.amount;
                sprintf(buffer, "New Fade Value: %d\n\n", someCmd.amount);      // BUGFIX: sometimes displays negative number
                Serial.print(buffer);                
                memset(buffer, 0, 255); 
            }
            else if(memcmp(someCmd.cmd, delayCmd, 5) == 0)                      // if `delay` command rec'd (compare to global var)
            {
                delayInterval = someCmd.amount;
                sprintf(buffer, "New Delay Value: %dms\n\n", someCmd.amount);
                Serial.print(buffer);
                memset(buffer, 0, 255);  
            }
            else if((memcmp(someCmd.cmd, patternCmd, 6) == 0))                  // if `pattern` command rec'd (compare to global var)
            {
                patternType = someCmd.amount;
                if(int(abs(patternType)) <= NUM_PATTERNS && int(patternType) != 0) // BUGFIX: "New Pattern: 0" with invalid entry
                {
                    sprintf(buffer, "New Pattern: %d\n\n", someCmd.amount);
                    Serial.print(buffer);
                    memset(buffer, 0, 255);
                }
            }
            else if(memcmp(someCmd.cmd, brightCmd, 5) == 0)                     // if `bright` command rec'd (compare to global var)
            {
                brightVal = someCmd.amount;                
                if(brightVal >= 255)
                {
                    Serial.println("Maximum Value 255...");
                    brightVal = 255;
                }               
                sprintf(buffer, "New Brightness: %d / 255\n\n", someCmd.amount);
                Serial.print(buffer);
                memset(buffer, 0, 255);
            }
            else if(memcmp(someCmd.cmd, cpuCmd, 3) == 0)                        // if `cpu` command rec'd (complare to global var)
            {
                localCPUFreq = someCmd.amount;
                if(localCPUFreq != 240 && localCPUFreq != 160 && localCPUFreq != 80)
                {
                    Serial.println("Invalid Input: Must Be 240, 160, or 80Mhz");
                    Serial.println("Returning....\n");
                    continue;
                }
                setCpuFrequencyMhz(localCPUFreq);                               // Set New CPU Freq
                vTaskDelay(10 / portTICK_PERIOD_MS);                            // yield for a brief moment

                sprintf(buffer, "\nNew CPU Frequency is: %dMHz\n\n", getCpuFrequencyMhz());
                Serial.print(buffer);
                memset(buffer, 0, 255);
            }
            else if(memcmp(someCmd.cmd, getValues, 6) == 0)                     // if `values` command rec'd (complare to global var)
            {
                sprintf(buffer, "\nCurrent Delay = %dms.           (default = 30ms)\n", delayInterval);
                Serial.print(buffer);
                memset(buffer, 0, 255);
                sprintf(buffer, "Current Fade Interval = %d.      (default = 5)\n", abs(fadeInterval));
                Serial.print(buffer);
                memset(buffer, 0, 255);
                sprintf(buffer, "Current Pattern = %d.            (default = 1)\n", patternType);
                Serial.print(buffer);
                memset(buffer, 0, 255);
                sprintf(buffer, "Current Brightness = %d / 255. (default = 250)\n\n", brightVal);
                Serial.print(buffer);
                memset(buffer, 0, 255);
            }
            else if(memcmp(someCmd.cmd, getFreq, 4) == 0)                       // if `freq` command rec'd (compare to global var)
            {
                sprintf(buffer, "\nCPU Frequency is:  %d MHz", getCpuFrequencyMhz());
                Serial.print(buffer);
                memset(buffer, 0, 255);
                sprintf(buffer, "\nXTAL Frequency is: %d MHz", getXtalFrequencyMhz());
                Serial.print(buffer);
                memset(buffer, 0, 255); 
                sprintf(buffer, "\nAPB Freqency is:   %d MHz\n\n", (getApbFrequency() / 1000000));
                Serial.print(buffer);
                memset(buffer, 0, 255);
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);                                // yield briefly (only if command rec'd)
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
        vTaskDelay(delayInterval / portTICK_PERIOD_MS);                         // CLI adjustable delay (non blocking)
    }
}

void sdRXTask(void *param) /*** CLI Input Validation / Handling ***/           /*** Analyze Each Node **/
{
    Message someMsg;                                                            // Each object given from the user
    Command someCmd;
    SDCommand sdCardCmd;                                                        // New object for SD Card Comms
    uint8_t localCPUFreq;                                                       // 80, 160 or 240Mhz
    char buffer[255];                                                       // string buffer for Terminal Message
    short ledDelay;                                                             // blink delay in ms
    short fadeAmt;
    short pattern;
    short bright;

    memset(buffer, 0, 255);                                                 // Clear input buffer

    for(;;)
    {
        if(xQueueReceive(msgQueue, (void *)&someMsg, 0) == pdTRUE)              // If a `Message` is rec'd from queue
        {   

            if(memcmp(someMsg.msg, sdListCmds, 5) == 0)                         // if `lscmd` command rec'd (compare to global var)
            {    
                /* LED Commands */
                if(memcmp(someMsg.msg, fadeCmd, 5) == 0)                            // Check for `fade ` command: Ref: https://cplusplus.com/reference/cstring/memcmp/
                {
                    char* tailPtr = someMsg.msg + 5;                                // pointer arithmetic: move pointer to integer value
                    fadeAmt = atoi(tailPtr);                                        // retreive integer value at end of string
                    fadeAmt = abs(fadeAmt);                                         // fadeAmt can't be negative
                    if(fadeAmt <= 0 || fadeAmt > 128)
                    {
                        Serial.println("Value Must Be Between 1 & 128");
                        Serial.println("Returning....");
                        continue;
                    }
                    strcpy(someCmd.cmd, "fade");
                    someCmd.amount = fadeAmt;                                       // copy input to Command node
                    xQueueSend(ledQueue, (void *)&someCmd, 10);                     // Send to ledQueue for interpretation
                }
                else if(memcmp(someMsg.msg, delayCmd, 6) == 0)                      // Check for `delay ` command
                {
                    char* tailPtr = someMsg.msg + 6;                                // pointer arithmetic: move pointer to integer value
                    ledDelay = atoi(tailPtr);                                       // retreive integer value at end of string
                    ledDelay = abs(ledDelay);                                       // ledDelay can't be negative
                    if(ledDelay <= 0)
                    {
                        Serial.println("Value Must Be > 0");
                        Serial.println("Returning....");
                        continue;
                    }
                    strcpy(someCmd.cmd, "delay");
                    someCmd.amount = ledDelay;                                      // copy input to Command node
                    xQueueSend(ledQueue, (void *)&someCmd, 10);                     // Send to ledQueue for interpretation                    // Clear input buffer
                }
                else if(memcmp(someMsg.msg, patternCmd, 8) == 0)                    // Check for `pattern ` command
                {
                    char* tailPtr = someMsg.msg + 8;                                // pointer arithmetic: move pointer to integer value
                    pattern = atoi(tailPtr);                                        // retreive integer value at end of string
                    pattern = abs(pattern);                                         // patternType can't be negative

                    strcpy(someCmd.cmd, "pattern");
                    someCmd.amount = pattern;                                       // copy input to Command node
                    xQueueSend(ledQueue, (void *)&someCmd, 10);                     // Send to ledQueue for interpretation
                }
                else if(memcmp(someMsg.msg, brightCmd, 7) == 0)                     // Check for `bright ` command
                {
                    char* tailPtr = someMsg.msg + 7;                                // pointer arithmetic: move pointer to integer value
                    bright = atoi(tailPtr);                                         // retreive integer value at end of string
                    bright = abs(bright);                                           // ledDelay can't be negative
                    
                    strcpy(someCmd.cmd, "bright");
                    someCmd.amount = bright;                                        // copy input to Command node
                    xQueueSend(ledQueue, (void *)&someCmd, 10);                     // Send to ledQueue for interpretation
                }
                else if(memcmp(someMsg.msg, cpuCmd, 4) == 0)                        // check for `cpu ` command
                {
                    char* tailPtr = someMsg.msg + 4;
                    localCPUFreq = atoi(tailPtr);
                    localCPUFreq = abs(localCPUFreq);

                    strcpy(someCmd.cmd, "cpu");
                    someCmd.amount = localCPUFreq;                                  // copy input to Message node
                    xQueueSend(ledQueue, (void *)&someCmd, 10);                     // Send to ledQueue for interpretation
                }
                else if(memcmp(someMsg.msg, getValues, 6) == 0)
                {
                    strcpy(someCmd.cmd, "values");
                    someCmd.amount = 0;                                             // copy input to Message node
                    xQueueSend(ledQueue, (void *)&someCmd, 10);                     // Send to ledQueue for interpretation
                }
                else if(memcmp(someMsg.msg, getFreq, 4) == 0)
                {
                    strcpy(someCmd.cmd, "freq");
                    someCmd.amount = 0;                                             // copy input to Message node
                    xQueueSend(ledQueue, (void *)&someCmd, 10);                     // Send to ledQueue for interpretation
                }
                
                /*** SD Card Commands ***/                                          /* Start with 'List all commands' command */
                
                else if(memcmp(someMsg.msg, sdListCmds, 5) == 0)                    // if `lscmd` command rec'd (compare to global var)
                {   
                    // send sdCardCmd over the sdQueue to perform an operation on the SD Card
                    strcpy(sdCardCmd.cmd, "lscmd");                                 // pass "lscmd" on to the SD card queue? Handle this over there instead.
                    strcpy(sdCardCmd.msg, "");                                      // send an empty string? maybe just call a txt that lists what the commands are?
                    xQueueSend(sdQueue, (void *)&someCmd, 10);                      // send `lscmd` to `sdQueue` ... wait 10ms if busy
                }
                else if(memcmp(someMsg.msg, sdListDir, 6) == 0)                     // if `lsdir ` command rec'd (compare to global var)
                {
                    // send sdCardCmd over the sdQueue to perform an operation on the SD Card
                    strcpy(sdCardCmd.cmd, "lsdir");                                 // pass "lsdir" on to the SD card queue...Handle this over there instead.
                    strcpy(sdCardCmd.msg, "");                                      // send empty string (no message)
                    xQueueSend(sdQueue, (void *)&someCmd, 10);                      // send `lscmd` to `sdQueue`
                    // memset(buffer, 0, 255);  
                }
                else if(memcmp(someMsg.msg, sdCreateDir, 6) == 0)                   // if `mkdir ` command rec'd (compare to global var)
                {

                    // need to pass to the SD Queue...
                    // each element rec'd, handle in the proper queue...
                    // Found a Command to create dir...
                    // FW to sdCardQueue....handle it over there...
                }
                else if(memcmp(someMsg.msg, sdDeleteDir, 6) == 0)                   // if `rmdir ` command rec'd (compare to global var)
                {
                    // need a directory for a choice...parse string again....
                    // memset(buffer, 0, 255);
                }
                else if(memcmp(someMsg.msg, sdReadFile, 9) == 0)                    // if `readfile ` command rec'd (compare to global var)
                {

                }       
                else if(memcmp(someMsg.msg, sdWriteFile, 10) == 0)                  // if `writefile ` command rec'd
                {
                    // memset(buffer, 0, 255);
                }
                else if(memcmp(someMsg.msg, sdAppendFile, 7) == 0)                  // if `append ` command rec'd
                {
                    // memset(buffer, 0, 255);
                }
                else if(memcmp(someMsg.msg, sdRenameFile, 7) == 0)                  // if `rename ` command rec'd
                {

                }
                else if(memcmp(someMsg.msg, sdDeleteFile, 7) == 0)                  // if `rmfile ` command rec'd
                {

                }
                else if(memcmp(someMsg.msg, sdUsedSpace, 7) == 0)                   // if `lsbytes` command rec'd
                {
                
                }
                else // Not a command: Print the message to the terminal
                {
                    sprintf(buffer, "Invalid Command: %s\n", someMsg.msg);          // print user message
                    Serial.print(buffer);
                    memset(buffer, 0, 255);                                     // Clear input buffer
                }
            }
            vTaskDelay(20 / portTICK_PERIOD_MS);                                    // Yield to other tasks
        }
    }
}