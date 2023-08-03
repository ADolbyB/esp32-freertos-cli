/**
 * Joel Brigida
 * CDA 4102: Computer Architecture
 * Implementation File for Task4: SD card handler that performs operations on SD card data.
 * June 28, 2023
*/

#include <Arduino.h>
#include "tasks.h"

void sdRXTask(void *param)
{
    Serial.println("Starting Task 4...");
    SDCommand sdCardCmd;                                                        // New object for SD Card Comms
    char buffer[BUF_LEN];                                                       // string buffer for Terminal Message
    uint64_t cardSize;
    short ledDelay;                                                             // blink delay in ms
    short fadeAmt;
    short pattern;
    short bright;

    memset(buffer, 0, BUF_LEN);                                                 // Clear input buffer

    Serial.println("Task 4 Entering for(;;);");
    for(;;)
    {
        Serial.println("Task  4 Inside for(;;)");
        if(sdQueue != NULL)
        {
            if(xQueueReceive(sdQueue, (void *)&buffer, 0) == pdTRUE)              // If an `sdCardCmd` is rec'd from queue
            { 
                strcpy(sdCardCmd.cmd, buffer);                                     // copy data to object
                memset(buffer, 0, BUF_LEN);                                     // clear the buffer
                if(memcmp(sdCardCmd.cmd, allCommands[SD_L], strlen(allCommands[SD_L])) == 0) // if `lssd` command rec'd (compare to global var)
                {    
                        Serial.println("lssd: Listing All SD Card Commands: \n");
                        /* Start with 'List all SD commands' command */
                        int j = 1;  // Command Number
                        for(int i = SD_L; i <= SD_H; i++)
                        {
                            sprintf(buffer, "Command #%d  ", j);
                            Serial.print(buffer);
                            memset(buffer, 0, BUF_LEN);
                            sprintf(buffer, allCommands[i]);
                            Serial.println(buffer);
                            memset(buffer, 0, BUF_LEN);
                        }
                }
                else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 1], strlen(allCommands[SD_L + 1])) == 0)  // if `lsdir` command rec'd
                {   
                    Serial.println("LSDIR COMMAND....Placeholder");
                }
                else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 2], strlen(allCommands[SD_L + 2])) == 0)// if `mkdir ` command rec'd (compare to global var)
                {
                    Serial.println("MKDIR COMMAND....Placeholder");
                }
                else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 3], strlen(allCommands[SD_L + 3])) == 0)// if `rmdir ` command rec'd (compare to global var)
                {
                    Serial.println("RMDIR COMMAND....Placeholder");
                }
                else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 4], strlen(allCommands[SD_L + 4])) == 0)// if `readfile ` command rec'd (compare to global var)
                {
                    Serial.println("READFILE COMMAND...Placeholder");
                }
                else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 5], strlen(allCommands[SD_L + 5])) == 0)// if `writefile ` command rec'd (compare to global var)
                {
                    Serial.println("WRITEFILE COMMAND....Placeholder");
                }       
                else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 6], strlen(allCommands[SD_L + 6])) == 0)// if `append ` command rec'd
                {
                    Serial.println("APPEND COMMAND....Placeholder");
                }
                else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 7], strlen(allCommands[SD_L + 7])) == 0)// if `rename ` command rec'd
                {
                    Serial.println("RENAMECOMMAND....Placeholder");
                }
                else if(memcmp(sdCardCmd.cmd, allCommands[SD_L + 8], strlen(allCommands[SD_L + 8])) == 0)// if `rmfile ` command rec'd
                {
                    Serial.println("RMFILE COMMAND....Placeholder");
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
        }
        else
        {
            Serial.println("ERROR! Could Not Create sdQueue!!");
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);                         // CLI adjustable delay (non blocking)
    }
}