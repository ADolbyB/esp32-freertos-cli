#ifndef LEDCLI_H
#define LEDCLI_H

/** Header File for the led CLI **/

static const int LEDCchan = 0;                                                  // use LEDC Channel 0 for Blue LED
static const int LEDCtimer = 12;                                                // 12-bit precision LEDC timer
static const int LEDCfreq = 5000;                                               // 5000 Hz LEDC base freq.

static const char delayCmd[] = "delay ";                                        // STRLEN = 6: delay command definition
static const char fadeCmd[] = "fade ";                                          // STRLEN = 5: fade command definition
static const char patternCmd[] = "pattern ";                                    // STRLEN = 8: pattern command definition
static const char brightCmd[] = "bright ";                                      // STRLEN = 7: brightness command (pattern 3 only)
static const char cpuCmd[] = "cpu ";                                            // STRLEN = 4: cpu speed control command
static const char getValues[] = "values";                                       // STRLEN = 6: show values of all user variables
static const char getFreq[] = "freq";                                           // STRLEN = 4: show values for freq


struct Message                                                                  // Struct for CLI input
{
    char msg[80];                                                               // User Input
};

struct Command                                                                  // Sent from `msgRXTask` to `RGBcolorWheelTask`
{
    char cmd[25];
    int amount;
};

#endif