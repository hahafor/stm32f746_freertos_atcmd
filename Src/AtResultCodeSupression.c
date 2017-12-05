
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
#define MAX_ATQ_SUPPORTED_RANGE     2

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
static char supportedATQRange[MAX_ATQ_SUPPORTED_RANGE + 1] = {'0', '1', '\0'};

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
void *ResultCodeSupressionHandler(int evt, void *arg);
int AtResultCodeSupression(const char *cmd, char *parameter, char *result, int timeout);

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

    for (idx = 0; idx < MAX_ATQ_SUPPORTED_RANGE; idx++) {
        if (value == supportedATQRange[idx]) {
            return 1;
        }
    }

    return 0;
}


/*
 * Local Command Handler
 */
void *ResultCodeSupressionHandler(int evt, void *arg) {
    char *ptr;
    char lineEnding[3]={0};
    
    (void)getLineEnding(lineEnding);
    ptr = (*atcInfo)->atc->cmd_inf->result;
    
    if (evt == CMD_NG) {
        if ((*atcInfo)->result_code_suppression == 0) {
            if ((*atcInfo)->dce_response_format) {
                sprintf(ptr, "%s%s%s", lineEnding, AT_STRING_ERROR, lineEnding);
            } else {
                sprintf(ptr, "%s%s", "4", lineEnding);
            }
            sendRsp(ptr);
        } 
    } else {
        if ((*atcInfo)->result_code_suppression == 0) {
            if ((*atcInfo)->dce_response_format) {
                sprintf(ptr, "%s%s%s", lineEnding, AT_STRING_ERROR, lineEnding);
            } else {
                sprintf(ptr, "%s%s", "4", lineEnding);
            }
            sendRsp(ptr);
        }
    }
    
    return NULL;
}

/*
 * Timer Handler
 */


/* 
 * AT Command Receiver
 *   command : ATQ[<value>]
 */
int AtResultCodeSuppression(const char *cmd, char *parameter, char *result, int timeout) {
    struct EventMessage msg;
    
    if ( isSupportedValue(*parameter)) {
        // update value
        (*atcInfo)->result_code_suppression = atoi(parameter);
        
        /* req  */
        msg.event = EVT_REQUEST_RESULT_CODE_SUPRESSION_REQ;
        msg.len = 0;
        msg.fptr = ResultCodeSupressionHandler;
        memcpy(msg.cmdName, cmd, strlen(cmd));
        msg.queueInfo.from = atTaskQueue;
        msg.queueInfo.to = atTaskQueue;
        msg.queueInfo.returnto = NULL;

        xQueueSend(atTaskQueue, &msg, portMAX_DELAY);
    } else {
        ResultCodeSupressionHandler(CMD_NG, &(*atcInfo)->result_code_suppression);
        return -1;
    }
    return 0;
}


/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/