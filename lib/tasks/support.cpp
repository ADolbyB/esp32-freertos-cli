
#include <Arduino.h>
#include "tasks.h"

int testInput(const char* userInput)                                             // VALID / INVALID: If valid. which command?
{
    unsigned cLength = 0;
    for (int i = 0; i < sizeof(allCommands) / sizeof(allCommands[0]); ++i)
    {
        cLength = strlen(allCommands[i]);
        if (memcmp(userInput, allCommands[i], cLength) == 0)
        {
            return i;
        }
    }
    return -1;                                                                          // Indicate no match
};

void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax)   // 'value' must be between 0 & 'valueMax'
{
    uint32_t duty = (4095 / valueMax) * min(value, valueMax);                           // calculate duty cycle: 2^12 - 1 = 4095
    ledcWrite(channel, duty);                                                           // write duty cycle to LEDC
};