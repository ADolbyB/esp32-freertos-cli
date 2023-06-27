#ifndef SDCLI_H
#define SDCLI_H 

/** SD CLI Header File **/

static const char sdListCmds[] = "lscmd";                                       // STRLEN = 5: prints a list of SD commands (from msgQueue)
static const char sdListDir[] = "lsdir ";                                       // STRLEN = 6: List subdirectories under given argument
static const char sdCreateDir[] = "mkdir ";                                     
static const char sdDeleteDir[] = "rmdir ";
static const char sdReadFile[] = "readfile ";
static const char sdWriteFile[] = "writefile ";
static const char sdAppendFile[] = "append ";
static const char sdRenameFile[] = "rename ";
static const char sdDeleteFile[] = "rmfile ";
static const char sdUsedSpace[] = "lsbytes";

/** No Function Definitions Needed for Global SD functions **/

struct SDCommand                                    // Sent from `msgRXTask` to `SDCardTask`
{
    char cmd[25];
    char msg[80];                                   // default length of Bash Terminal Line
};

#endif