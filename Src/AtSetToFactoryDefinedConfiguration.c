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
#define MAX_ATF_SUPPORTED_RANGE     2

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
void *SetToFactoryDefinedConf(int evt, void *arg);
static void ATFTimerHandler(void *arg);
int AtSetToFactoryDefinedConfiguration(const char *cmd, char *parameter, char *result, int timeout);

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
extern QueueHandle_t productTaskQueue;

/****************************************************************************/
/** **/
/** LOCAL VARIABLES **/
/** **/
/****************************************************************************/
static TimerHandle_t timerIDATF = NULL;
static char supportedATFRange[MAX_ATF_SUPPORTED_RANGE + 1] = {'0', '1', '\0'};

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

    for (idx = 0; idx < MAX_ATF_SUPPORTED_RANGE; idx++) {
        if (value == supportedATFRange[idx]) {
            return 1;
        }
    }

    return 0;
}

/*
 * Local Command Handler : Q
 */
void *SetToFactoryDefinedConf(int evt, void *arg) {
    char *ptr;
    char lineEnding[3]={0};
    
    // stop & clear timer
    xTimerStop(timerIDATF, 0);

    (void)getLineEnding(lineEnding);
    ptr = (*atcInfo)->atc->cmd_inf->result;
    
    if (evt == CMD_OK || CMD_DONE ) {
        if ((*atcInfo)->result_code_suppression == 0) {
            if ((*atcInfo)->dce_response_format) {
                sprintf(ptr,"%s%s%s%s",lineEnding,(char *)&arg,AT_STRING_OK,lineEnding);
            } else {
                sprintf(ptr,"%s%s%s",(char *)&arg,AT_STRING_OK,lineEnding);
            }
        }
    } else {
        if ((*atcInfo)->result_code_suppression == 0) {
            if ((*atcInfo)->dce_response_format) {
                sprintf(ptr,"%s%s%s%s",lineEnding,(char *)&arg,AT_STRING_ERROR,lineEnding);
            } else {
                sprintf(ptr,"%s%s%s",(char *)&arg,AT_STRING_ERROR,lineEnding);
            }
        }
    }
    
    return NULL;
}

/*
 * Timer Handler
 */
static void ATFTimerHandler(void *arg) {
    // stop timer
    xTimerStop(timerIDATF, 0);

    // call CBF
    timerCBF = SetToFactoryDefinedConf(CMD_TIMER_EXPIRED, NULL);
    timerCBF(arg);
}


/* 
 * AT Command Receiver
 *   command : AT&F
 */
int AtSetToFactoryDefinedConfiguration(const char *cmd, char *parameter, char *result, int timeout) {
    int nParam = 0;
    struct EventMessage msg;

if (isSupportedValue(*parameter) || (!(*parameter))) {
        if (*parameter == '1') {
            nParam = 1;
        } else {
            nParam = 0;
        }
    } else {
        SetToFactoryDefinedConf(CMD_NG, NULL);
        return -1;    
    }

    /*  */  
    msg.event = EVT_RESET_TO_FACTORY_DEFINED_CONFIGURATION_REQ;
    msg.parameters.paramEvent1 = nParam;
    msg.len = 0;   
    msg.fptr = SetToFactoryDefinedConf;
    memcpy(msg.cmdName, cmd, strlen(cmd));
    msg.queueInfo.from = atTaskQueue;
    msg.queueInfo.to = productTaskQueue;
    msg.queueInfo.returnto = atTaskQueue;
    
    xQueueSend(productTaskQueue, &msg, portMAX_DELAY);

    // create timer
    if (timeout > 0) {
        timerIDATF = xTimerCreate("FTimer", (timeout / portTICK_PERIOD_MS), pdFALSE, 0, ATFTimerHandler);
    }

    return 0;
}

/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/
