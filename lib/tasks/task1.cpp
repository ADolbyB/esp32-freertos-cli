

#include <Arduino.h>
#include "tasks.h"

/** TASK 1: Lowest Level Function: Read Raw User Input **/
/** Always scanning the keyboard **/

void userCLITask(void *param)                                                   // Function definition for user CLI task
{
    Serial.println("Starting Task 1...");
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
                xQueueSend(msgQueue, (void *)&sendMsg, 20);                     // Send to msgQueue for interpretation
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