/**
 * Joel Brigida
 * CDA 4102: Computer Architecture
 * Header file for task functions. This is a barebones 
 * example to avoid crowding,
 * June 28, 2023
*/

#ifndef TASKS_H
#define TASKS_H

#include "allLibs.h"
#include "testInput.h"

struct Message                                                                  // Struct for CLI input
{
    char msg[80];                                                               // User Input
};

struct Command                                                                  // Sent from `msgRXTask` to `msgRXTask`
{
    char cmd[25];
    int amount;
};

struct SDCommand                                                                // Sent from `msgRXTask` to `SDCardTask`
{
    char cmd[25];
    char msg[80];                                                               // default length of Bash Terminal Line
};

void sdRXTask(void* );
void userCLITask(void* );
void msgTask(void* );
void led2And13Task(void* );

inline void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255)        // 'value' must be between 0 & 'valueMax'
{
    uint32_t duty = (4095 / valueMax) * min(value, valueMax);                   // calculate duty cycle: 2^12 - 1 = 4095
    ledcWrite(channel, duty);                                                   // write duty cycle to LEDC
};

#endif