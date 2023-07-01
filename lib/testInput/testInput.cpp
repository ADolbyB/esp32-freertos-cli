/**
 * Joel Brigida
 * CDA 4102: Computer Architecture
 * This function checks the validity of any command found in the msg queue
 * It iterates through all indices in the `allcommands[]` array.
*/
#include "testInput.h"

int testInput(const char* userInput)
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
    return -1;                                                      // Indicate no match
}