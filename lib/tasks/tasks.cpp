/**
 * Joel Brigida
 * CDA 4102: Computer Architecture
 * This is the implementation file for all task intantiation.
 * Supporting variables & functions are in `tasks.h`
*/

#include "allLibs.h"
#include "tasks.h"

/** Lowest Level Function: Read Raw User Input **/
/** Always scanning the keyboard **/

void userCLITask(void *param)                                                   // Function definition for user CLI task
{
    Message sendMsg;                                                            // Declare user message
    char input;                                                                 // Each char of user input                                             
    char buffer[BUF_LEN];                                                       // buffer to hold user input
    uint8_t index = 0;                                                          // character count

    memset(buffer, 0, BUF_LEN);                                                 // Clear user input buffer

    for(;;)
    {       
        if(Serial.available() > 0)
        {
            input = Serial.read();                                              // read each character of user input

            if(index < (BUF_LEN - 1))
            {
                buffer[index] = input;                                          // write received character to buffer
                index++;
            }
            if(input == '\n')                                                   // Check when user presses ENTER key
            {
                Serial.print("\n");
                strcpy(sendMsg.msg, buffer);                                    // copy input to Message node
                xQueueSend(msgQueue, (void *)&sendMsg, 10);                     // Send to msgQueue for interpretation
                memset(buffer, 0, BUF_LEN);                                     // Clear input buffer
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

/** MidLevel Function: msgTask() 
 *  RX: Input Validation
 *  TX: Determine what goes to which queue & SEND 
 *  ELSE: Yield to other tasks
 *  Description: receives packets of user data and determines if they
 *  are valid CLI ipput statements.
 *  If(valid): can be passed on, IF(!Valid): Throw it out STAT.
 *  Perform simple functions only: `lscmd`, `cpu ` &  functions. They require
 *  No add'l resources.
**/


void msgTask(void* param)        /*** CLI Input Validation / Handling ***/
{
    Message someMsg;                                                                // Each object rec'd from User Input
    Command ledCmd;
    SDCommand sdCardCmd;                                                            // New object for SD Card Comms
    uint8_t localCPUFreq;                                                           // 80, 160 or 240Mhz
    char buffer[BUF_LEN];                                                           // string buffer for Terminal Message
    char matchingString[15];
    char userInput[BUF_LEN];
    short ledValue = 0;
    int goodInput = -1;

    memset(buffer, 0, BUF_LEN);                                                     // Clear input buffer

    for(;;)
    {
        if(xQueueReceive(msgQueue, (void *)&someMsg, 0) == pdTRUE)                  // If a `Message` is rec'd from queue
        {
            Serial.println("User Input Received...Checking Validity...");
            goodInput = testInput(someMsg.msg);                                     // -1 = NO MATCH, otherwise return INDEX#
            int j = int(goodInput);
            
            if(goodInput >= 0)                                                      // found a Matching string if goodInput >= 0
            {
                strcpy(matchingString, sdCardCmd.cmd);                                    // copy matching string
                sprintf(buffer, "\nMatch Found: %s", matchingString);
                Serial.print(buffer);
                memset(buffer, 0, BUF_LEN);                                             // Clear input buffer

                if(CPU_L <= goodInput && goodInput <= CPU_H)                                // Only for "EASY" stuff: `lscmd`, `cpu`, `freq`
                {
                    if(memcmp(someMsg.msg, allCommands[0], strlen(allCommands[0])) == 0)                 // `lscmd` command
                    {
                        Serial.print("\n\nLED Commands:\n\n");
                        Serial.print("Enter \'delay xxx\' to change RGB Fade Speed.\n");
                        Serial.print("Enter \'fade xxx\' to change RGB Fade Amount.\n");
                        Serial.print("Enter \'pattern xxx\' to change RGB Pattern.\n");
                        Serial.print("Enter \'bright xxx\' to change RGB Brightness (Only Pattern 3).\n");
                        Serial.print("Enter \'cpu xxx\' to change CPU Frequency.\n");
                        Serial.print("Enter \'values\' to retrieve current delay, fade, pattern & bright values.\n");
                        Serial.print("Enter \'freq\' to retrieve current CPU, XTAL & APB Frequencies.\n");
                        Serial.print("\nSD Card Commands:\n\n");
                        Serial.print("Enter \'something\' to change SD Card something.\n");
                        Serial.print("Enter \'another thing\' to change SD Card another thing.\n");
                        Serial.print("Enter \'third thing\' to change SD Card third thing.\n");

                    }
                    else if(memcmp(someMsg.msg, allCommands[1], strlen(allCommands[1])) == 0)            // if `cpu` command rec'd (complare to global var)
                    {
                        localCPUFreq = ledCmd.amount;
                        if(localCPUFreq != 240 && localCPUFreq != 160 && localCPUFreq != 80)
                        {
                            Serial.println("Invalid Input: Must Be 240, 160, or 80Mhz");
                            Serial.println("Returning....\n");
                            continue;
                        }
                        setCpuFrequencyMhz(localCPUFreq);                           // Set New CPU Freq
                        vTaskDelay(10 / portTICK_PERIOD_MS);                        // yield for a brief moment

                        sprintf(buffer, "\nNew CPU Frequency is: %dMHz\n\n", getCpuFrequencyMhz());
                        Serial.print(buffer);
                        memset(buffer, 0, BUF_LEN);
                    }
                    else if(memcmp(someMsg.msg, allCommands[2], strlen(allCommands[2])) == 0)   // if `freq` command rec'd (compare to global var)
                    {
                        sprintf(buffer, "\nCPU Frequency is:  %d MHz", getCpuFrequencyMhz());
                        Serial.print(buffer);
                        memset(buffer, 0, BUF_LEN);
                        sprintf(buffer, "\nXTAL Frequency is: %d MHz", getXtalFrequencyMhz());
                        Serial.print(buffer);
                        memset(buffer, 0, BUF_LEN); 
                        sprintf(buffer, "\nAPB Freqency is:   %d MHz\n\n", (getApbFrequency() / 1000000));
                        Serial.print(buffer);
                        memset(buffer, 0, BUF_LEN);
                    }
                } // END if(0 <= goodInput && goodInput <= 2)
                else if(LED_L <= goodInput && goodInput <= LED_H)                   // LED COMMANDS: SEND TO LED QUEUE
                {
                    char* tailPtr = someMsg.msg + strlen(allCommands[goodInput]);
                    ledValue = atoi(tailPtr);
                    ledValue = abs(ledValue);
                    strcpy(ledCmd.cmd, allCommands[goodInput]);
                    ledCmd.amount = ledValue;
                    xQueueSend(ledQueue, (void *)&ledCmd, 10);                      // Send to ledQueue for interpretation
                }
                else if(SD_L <= goodInput && goodInput <= SD_H)                     // SD Commands: SEND TO SD QUEUE
                {
                    //char* tailPtr = someMsg.msg + strlen(allCommands[goodInput])
                    char* spacePos = strchr(someMsg.msg, ' ');
                    strcpy(sdCardCmd.cmd, allCommands[goodInput]);
                    strcpy(sdCardCmd.msg, spacePos + 1);                            // Retrieve 2nd portion of SC Command
                    xQueueSend(sdQueue, (void *)&sdCardCmd, 10);                    // Send to sdQueue for interpretation
                }
                else // Is this case valid??
                {
                    sprintf(buffer, "\n\nThis should not happen: goodInput = %d\n\n", goodInput);
                    Serial.print(buffer);
                    memset(buffer, 0, BUF_LEN);
                }
            } // END if(goodInput > 0)
            else // No input match (returned -1)
            {
                strcpy(someMsg.msg, userInput);
                sprintf(buffer, "\nInvalid Command: %s", userInput);
                Serial.print(buffer);
                memset(buffer, 0, BUF_LEN);
            }
        }
        vTaskDelay(25 / portTICK_PERIOD_MS);                                        // Yield to other tasks
    }// END for(;;)
}
/** led2And13Task() listens for changes from the msgTask() **/

void led2And13Task(void *param)
{
    /** Init LEDs & Functions **/
    
    CRGB leds[NUM_LEDS];                                                            // Array for RGB LED on GPIO_2
    FastLED.addLeds <CHIPSET, RGB_LED, COLOR_ORDER> (leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(75);
    leds[0] = CRGB::White;                                                          // Power up all Pin 2 LEDs for Power On Test
    FastLED.show();
    
    ledcSetup(LEDCchan, LEDCfreq, LEDCtimer);                                       // Setup LEDC timer (For LED_BUILTIN)
    ledcAttachPin(BLUE_LED, LEDCchan);                                              // Attach timer to LED pin
    vTaskDelay(2000 / portTICK_PERIOD_MS);                                          // 2 Second Power On Delay

    Serial.println("Power On Test Complete...");
    leds[0] = CRGB::Black;
    FastLED.show();
    vTaskDelay(500 / portTICK_PERIOD_MS);                                           // 0.5 Second off before Starting Tasks

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
    leds[0] = CRGB::Red;                                                            // Start with Red LED when instantiated
    FastLED.show();

    for(;;)
    {
        /*** Command Handling ***/
        if(xQueueReceive(ledQueue, (void *)&ledCmd, 0) == pdTRUE)                   // if command received from MSG QUEUE
        {
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
        vTaskDelay(delayInterval / portTICK_PERIOD_MS);                         // CLI adjustable delay (non blocking)
    }
}

void sdRXTask(void *param)
{
    SDCommand sdCardCmd;                                                        // New object for SD Card Comms
    char buffer[BUF_LEN];                                                       // string buffer for Terminal Message
    uint64_t cardSize;
    short ledDelay;                                                             // blink delay in ms
    short fadeAmt;
    short pattern;
    short bright;

    memset(buffer, 0, BUF_LEN);                                                 // Clear input buffer

    for (;;)
    {
        if(xQueueReceive(sdQueue, (void *)&sdCardCmd, 0) == pdTRUE)              // If an `sdCardCmd` is rec'd from queue
        { 

            if(memcmp(sdCardCmd.cmd, allCommands[SD_L], strlen(allCommands[SD_L])) == 0) // if `lssd` command rec'd (compare to global var)
            {    
                    Serial.println("lssd: Listing All SD Card Commands: \n");
                    /* Start with 'List all SD commands' command */
                    int j = 1;  // Command Number
                    for(int i = SD_L; i <= SD_H; i++)
                    {
                        sprintf(buffer, "Command #%d", j);
                        Serial.println(buffer);
                        memset(buffer, 0, BUF_LEN);
                        sprintf(buffer, allCommands[i]);
                        Serial.println(buffer);
                        memset(buffer, 0, BUF_LEN);
                    }
            }
            else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 1], strlen(allCommands[SD_L + 1])) == 0)  // if `lsdir` command rec'd
            {   
                Serial.println("LSDIR COMMAND....");
            }
            else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 2], strlen(allCommands[SD_L + 2])) == 0)// if `mkdir ` command rec'd (compare to global var)
            {
                Serial.println("MKDIR COMMAND....");
            }
            else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 3], strlen(allCommands[SD_L + 3])) == 0)// if `rmdir ` command rec'd (compare to global var)
            {
                Serial.println("RMDIR COMMAND....");
            }
            else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 4], strlen(allCommands[SD_L + 4])) == 0)// if `readfile ` command rec'd (compare to global var)
            {
                Serial.println("READFILE COMMAND...");
            }
            else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 5], strlen(allCommands[SD_L + 5])) == 0)// if `writefile ` command rec'd (compare to global var)
            {
                Serial.println("WRITEFILE COMMAND....");
            }       
            else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 6], strlen(allCommands[SD_L + 6])) == 0)// if `append ` command rec'd
            {
                Serial.println("APPEND COMMAND....");
            }
            else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 7], strlen(allCommands[SD_L + 7])) == 0)// if `rename ` command rec'd
            {
                Serial.println("RENAMECOMMAND....");
            }
            else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 8], strlen(allCommands[SD_L + 8])) == 0)// if `rmfile ` command rec'd
            {
                Serial.println("RMFILE COMMAND....");
            }
            else if(memcmp(sdCardCmd.cmd, allCommands[SD_H], strlen(allCommands[SD_H])) == 0)// if `lsbytes` command rec'd
            {
                cardSize = SD.cardSize() / (1024 * 1024);
                sprintf(buffer, "SD Card Size: %lluMB\n", cardSize);
                Serial.print(buffer);
                memset(buffer, 0, BUF_LEN);                                     // Clear input buffer
                
                sprintf(buffer, "Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
                Serial.print(buffer);
                memset(buffer, 0, BUF_LEN);                                     // Clear input buffer

                sprintf(buffer, "Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
                Serial.print(buffer);
                memset(buffer, 0, BUF_LEN);                                     // Clear input buffer
            }
            else // Not a command: Print the message to the terminal
            {
                sprintf(buffer, "Invalid Command: %s\n", sdCardCmd.cmd);          // print user message
                Serial.print(buffer);
                memset(buffer, 0, BUF_LEN);                                     // Clear input buffer
            }
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);                         // CLI adjustable delay (non blocking)
    }
}