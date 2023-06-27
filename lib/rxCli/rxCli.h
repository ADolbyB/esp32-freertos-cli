#ifndef RXCLI_H
#define RXCLI_H

/** Header File for the led CLI **/

struct Message                                                                  // Struct for CLI input
{
    char msg[80];                                                               // User Input
};

struct Command                                                                  // Sent from `msgRXTask` to `RGBcolorWheelTask`
{
    char cmd[25];
    int amount;
};

struct SDCommand                                                                // Sent from `msgRXTask` to `SDCardTask`
{
    char cmd[25];
    char msg[80];                                                               // default length of Bash Terminal Line
};


#endif