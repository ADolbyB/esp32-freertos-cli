#ifndef TESTINPUT_H
#define TESTINPUT_H

#include "allLibs.h"
#include "tasks.h"

enum Limits : int {                                                                 // Change Limits when adding / subtracting tasks
    CPU_L = 0, CPU_H = 2, LED_L = 3, LED_H = 7, SD_L = 8, SD_H = 17 
};

static const char allCommands[][15] = {
    "lscmd", "cpu ", "freq",                                                        // CPU Functions [0, 2]
    "delay ", "fade ", "pattern ", "bright ", "values",                             // LED Functions [3, 7]
    "lssd", "lsdir ", "mkdir ", "rmdir ", "readfile ",                              // SD Functions
    "writefile ", "append ", "rename ", "rmfile ", "lsbytes"                        // SD Functions [8, 17]
};

int testInput(const char* );                                                        // VALID / INVALID: If valid. which command?

#endif // end TESTINPUT_H