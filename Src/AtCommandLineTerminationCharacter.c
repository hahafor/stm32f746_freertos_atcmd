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

#include <FreeRTOS.h>
#include <timers.h>

#include "cmsis_os.h" 

#include "../Inc/event.h"
#include "../Inc/atTask.h"
#include "../Inc/atcmd.h"

/****************************************************************************/
/** **/
/** DEFINITIONS AND MACROS **/
/** **/
/****************************************************************************/


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
void *(*timerCBF) (void *);

static int isSupportedValue(char value);


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
void *CommandLineTerminationCharacterHandler(int evt, void *arg);
int AtCommandLineTerminationCharacter(const char *cmd, char *parameter, char *result, int timeout);

/****************************************************************************/
/** **/
/** LOCAL FUNCTIONS **/
/** **/
/****************************************************************************/
/*
 * Check parameter
 */
static int isSupportedValue(char value) {   
    if ( value >=0 && value <= MAX_S3_RANGE ) {
        return 1;
    } else {
        return 0;
    }
}

/*
 * Local Command Handler
 */
void *CommandLineTerminationCharacterHandler(int evt, void *arg) {
    char *ptr;
    char lineEnding[3]={0};
    
    (void)getLineEnding(lineEnding);
    ptr = (*atcInfo)->atc->cmd_inf->result;
    
    if (!(arg) || evt == CMD_NG) {
        if ((*atcInfo)->dce_response_format) {
            sprintf(ptr, "%s%s%s", lineEnding, AT_STRING_ERROR, lineEnding);
        } else {
            sprintf(ptr, "%s%s", AT_STRING_ERROR, lineEnding);
        }
    } else {
        if ((*atcInfo)->dce_response_format) {
            sprintf(ptr, "%s%s%s", lineEnding, (char *)&arg, lineEnding);
        } else {
            sprintf(ptr, "%s%s", (char *)&arg, lineEnding);
        }
    }
    sendRsp(ptr);
    return NULL;
}

/*
 * Timer Handler
 */


/* 
 * AT Command Receiver
 *   command : ATS3=<value>
 */
int AtCommandLineTerminationCharacter(const char *cmd, char *parameter, char *result, int timeout) {
    struct EventMessage msg;
    
    if ( isSupportedValue(*parameter)) {
        // update value
        (*atcInfo)->s3 = *parameter;
        
        /* req  */
        msg.event = EVT_REQUEST_COMMAND_LINE_TERMINATION_CHARACTER_REQ;
        msg.len = 0;
        msg.fptr = CommandLineTerminationCharacterHandler;
        memcpy(msg.cmdName, cmd, strlen(cmd));
        msg.queueInfo.from = atTaskQueue;
        msg.queueInfo.to = atTaskQueue;
        msg.queueInfo.returnto = NULL;

        xQueueSend(atTaskQueue, &msg, portMAX_DELAY);
    } else {
        CommandLineTerminationCharacterHandler(CMD_NG, NULL);
        return -1;
    }
    return 0;
}


/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/