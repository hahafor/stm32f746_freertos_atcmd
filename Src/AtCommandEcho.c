/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/****************************************************************************/
/** **/
/** MODULES USED **/
/** **/
/****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <FreeRTOS.h>
#include <timers.h>


#include "../Inc/atTask.h"
#include "../Inc/atcmd.h"


/****************************************************************************/
/** **/
/** DEFINITIONS AND MACROS **/
/** **/
/****************************************************************************/
#define MAX_ATE_SUPPORTED_RANGE     2

/****************************************************************************/
/** **/
/** TYPEDEFS AND STRUCTURES **/
/** **/
/****************************************************************************/
/****************************************************************************/
/** **/
/** PROTOTYPES OF LOCAL FUNCTIONS **/
/** **/
/****************************************************************************/
static int isSupportedValue(char value);
int AtCommandEcho(const char *cmd, char *parameter, char *result, int timeout);
void *CommandEchoHandler(int evt, void *arg);

/****************************************************************************/
/** **/
/** EXPORTED VARIABLES **/
/** **/
/****************************************************************************/
/****************************************************************************/
/** **/
/** GLOBAL VARIABLES **/
/** **/
/****************************************************************************/
/****************************************************************************/
/** **/
/** LOCAL VARIABLES **/
/** **/
/****************************************************************************/
static char supportedATERange[MAX_ATE_SUPPORTED_RANGE + 1] = {'0', '1', '\0'};

/****************************************************************************/
/** **/
/** EXPORTED FUNCTIONS **/
/** **/
/****************************************************************************/
/****************************************************************************/
/** **/
/** GLOBAL FUNCTIONS **/
/** **/
/****************************************************************************/
/****************************************************************************/
/** **/
/** LOCAL FUNCTIONS **/
/** **/

/****************************************************************************/
/*
 * Check parameter
 */
static int isSupportedValue(char value) {
    int idx = 0;

    if (value == '\0')
        return 1;

    for (idx = 0; idx < MAX_ATE_SUPPORTED_RANGE; idx++) {
        if (value == supportedATERange[idx]) {
            return 1;
        }
    }

    return 0;
}

/*
 * Local Command Handler
 */
void *CommandEchoHandler(int evt, void *arg) {
    char lineEnding[3]={0};
    char *ptr;
    
    (void) getLineEnding(lineEnding);

    if (evt == CMD_NG) {
        ptr = (*atcInfo)->atc->cmd_inf->result;
        if ((*atcInfo)->dce_response_format) {
            sprintf(ptr, "%s%s%s", lineEnding, AT_STRING_ERROR, lineEnding);
        } else {
            sprintf(ptr, "%s%s", AT_STRING_ERROR, lineEnding);
        }
        sendRsp(ptr);
    }

    return NULL;
}

/* 
 * AT Command Receiver
 *   command : ATE
 */
int AtCommandEcho(const char *cmd, char *parameter, char *result, int timeout) {
    if (isSupportedValue(*parameter) || ( !(*parameter)) ) {
        if (*parameter == '1') {
            (*atcInfo)->echo = 1;
        } else {
            (*atcInfo)->echo = 0;
        }
    } else {
        CommandEchoHandler(CMD_NG, NULL);
        return -1;    
    }

    return 0;
}

/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/
