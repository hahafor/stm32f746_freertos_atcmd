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
#define MAX_ATZ_SUPPORTED_RANGE     2

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
void *ResetToDefaultConfigurationHandler(int evt, void *arg);
static void ATZTimerHandler(void *arg);
int AtResetToDefaultConfiguration(const char *cmd, char *parameter, char *result, int timeout);

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
static TimerHandle_t timerIDATZ = NULL;
static char supportedATZRange[MAX_ATZ_SUPPORTED_RANGE + 1] = {'0', '1', '\0'};


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

    for (idx = 0; idx < MAX_ATZ_SUPPORTED_RANGE; idx++) {
        if (value == supportedATZRange[idx]) {
            return 1;
        }
    }

    return 0;
}

/*
 * Local Command Handler : Q
 */
void *ResetToDefaultConfigurationHandler(int evt, void *arg) {
    char lineEnding[3]={0};
    char *ptr;
    int done = 0;
    
    while (!done) {
        (void) getLineEnding( lineEnding );
        ptr = (*atcInfo)->atc->cmd_inf->result;

        if ( evt == CMD_CNF ) {
            if ((*atcInfo)->result_code_suppression == 0) {
                if ((*atcInfo)->dce_response_format) {
                    sprintf(ptr, "%s%s%s%s",lineEnding,(char *)&arg,AT_STRING_OK,lineEnding);
                } else {
                    sprintf(ptr, "%s%s%s",(char *)&arg,AT_STRING_OK,lineEnding);
                }
            }
        } else if ( evt == CMD_DONE ) {
            done = 1;
            if ((*atcInfo)->result_code_suppression == 0) {
                if ((*atcInfo)->dce_response_format) {
                    sprintf(ptr, "%s%s%s%s",lineEnding,(char *)&arg,AT_STRING_DONE,lineEnding);
                } else {
                    sprintf(ptr, "%s%s%s",(char *)&arg,AT_STRING_DONE,lineEnding);
                }
            }
            sendRsp(ptr);
             
            goto cleanUp;
        } else {
            done=1;
            if ((*atcInfo)->dce_response_format) {
                sprintf(ptr, "%s%s%s",lineEnding,AT_STRING_ERROR,lineEnding);
            } else {
                sprintf(ptr, "%s%s",AT_STRING_ERROR,lineEnding);
            }
            sendRsp(ptr);
            goto cleanUp;
        }
    }
    
cleanUp:
    done=0;
    
    return NULL;
}

/*
 * Timer Handler
 */
static void ATZTimerHandler(void *arg) {
    // stop timer
    xTimerStop(timerIDATZ, 0);

    // call CBF
    timerCBF = ResetToDefaultConfigurationHandler(CMD_TIMER_EXPIRED, NULL);
    timerCBF(arg);
}

/* 
 * AT Command Receiver
 *   command : ATZ
 */
int AtResetToDefaultConfiguration(const char *cmd, char *parameter, char *result, int timeout) {
    int nParam = 0;
    struct EventMessage msg;

    if (isSupportedValue(*parameter) || ( !(*parameter))) {
        if (*parameter == '1') {
            nParam = 1;
        } else {
            nParam = 0;
        }
    } else {
        ResetToDefaultConfigurationHandler(CMD_NG, NULL);
        return -1;    
    }
    
    /* req  */
    msg.event = EVT_RESET_TO_DEFAULT_CONFIGURATION_REQ;
    msg.parameters.paramEvent1 = nParam;
    msg.len = 0;   
    msg.fptr = ResetToDefaultConfigurationHandler;
    memcpy(msg.cmdName, cmd, strlen(cmd));
    msg.queueInfo.from = atTaskQueue;
    msg.queueInfo.to = productTaskQueue;
    msg.queueInfo.returnto = atTaskQueue;
    
    xQueueSend(productTaskQueue, &msg, portMAX_DELAY);
    
    // create timer
    if (timeout > 0) {
        timerIDATZ = xTimerCreate("ZTimer", (timeout / portTICK_PERIOD_MS), pdFALSE, 0, ATZTimerHandler);
    }

    return 0;
}


/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/
