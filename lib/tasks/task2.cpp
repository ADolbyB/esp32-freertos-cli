/** TASK 2: MidLevel Function: msgTask() 
 *  RX: Input Validation
 *  TX: Determine what goes to which queue & SEND 
 *  ELSE: Yield to other tasks
 *  Description: receives packets of user data and determines if they
 *  are valid CLI ipput statements.
 *  If(valid): can be passed on, IF(!Valid): Throw it out STAT.
 *  Perform simple functions only: `lscmd`, `cpu ` &  functions. They require
 *  No add'l resources.
**/

#include <Arduino.h>
#include "tasks.h"

void msgTask(void* param)        /*** CLI Input Validation / Handling ***/
{
    Serial.println("Starting Task 2...");
    Message someMsg;                                                            // Each object rec'd from User Input
    Command ledCmd;
    SDCommand sdCardCmd;                                                        // New object for SD Card Comms
    uint8_t localCPUFreq;                                                       // 80, 160 or 240Mhz
    char buffer[BUF_LEN];                                                       // string buffer for Terminal Message
    //char userInput[BUF_LEN];
    //char matchingString[15];
    short ledValue = 0;
    int goodInput = -1;

    memset(buffer, 0, BUF_LEN);                                                 // Clear input buffer
    //memset(userInput, 0, BUF_LEN);                                            // Clear input buffer
    Serial.println("Task 2 Entering For Loop...");
    for(;;)
    {
        if(xQueueReceive(msgQueue, (void *)&someMsg, 0) == pdTRUE)              // If a `Message` is rec'd from queue
        {
            Serial.println("User Input Received...Checking Validity...");
            strcpy(buffer, someMsg.msg);
            goodInput = testInput(buffer);                                      // -1 = NO MATCH, otherwise return INDEX#
            //int j = int(goodInput);
            memset(buffer, 0, BUF_LEN);                                         // Clear input buffer

            if(goodInput >= 0)                                                  // found a Matching string if goodInput >= 0
            {
                strcpy(buffer, someMsg.msg);                                    // copy matching string
                sprintf(buffer, "\nMatch Found: %s", buffer);
                Serial.print(buffer);
                memset(buffer, 0, BUF_LEN);                                     // Clear input buffer

                if(CPU_L <= goodInput && goodInput <= CPU_H)                    // Only for "EASY" stuff: `lscmd`, `cpu`, `freq`
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
                // else if(SD_L <= goodInput && goodInput <= SD_H)                     // SD Commands: SEND TO SD QUEUE
                // {
                //     //char* tailPtr = someMsg.msg + strlen(allCommands[goodInput])
                //     char* spacePos = strchr(someMsg.msg, ' ');
                //     strcpy(sdCardCmd.cmd, allCommands[goodInput]);
                //     strcpy(sdCardCmd.msg, spacePos + 1);                            // Retrieve 2nd portion of SC Command
                //     xQueueSend(sdQueue, (void *)&sdCardCmd, 10);                    // Send to sdQueue for interpretation
                // }
                else // Is this case valid??
                {
                    sprintf(buffer, "\n\nThis should not happen: goodInput = %d\n\n", goodInput);
                    Serial.print(buffer);
                    memset(buffer, 0, BUF_LEN);
                }
            } // END if(goodInput > 0)
            else // No input match (returned -1)
            {
                sprintf(buffer, "\nInvalid Command: %s", someMsg.msg);
                Serial.print(buffer);
                memset(buffer, 0, BUF_LEN);
            }
        }
        vTaskDelay(25 / portTICK_PERIOD_MS);                                        // Yield to other tasks
    } // END for(;;)
}