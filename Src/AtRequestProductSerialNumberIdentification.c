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
#include <unistd.h>

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

void *ReqProductSerialNumberIdentification(int evt, void *arg);
static void ATGSNTimerHandler(void *arg);
int AtRequestProductSerialNumberIdentification(const char *cmd, char *parameter, char *result, int timeout);

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
static TimerHandle_t timerIDGSN = NULL;

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
 * Local Command Handler
 */
void *ReqProductSerialNumberIdentification(int evt, void *arg) {
    char *ptr;
    char lineEnding[3]={0};
    
    // stop & clear timer
    xTimerStop(timerIDGSN, 0);
    
    ptr = (*atcInfo)->atc->cmd_inf->result;
    
    if (!(arg) || evt == CMD_NG) {
        if ((*atcInfo)->dce_response_format) {
            sprintf(ptr, "%s%s%s", lineEnding, AT_STRING_ERROR, lineEnding);
        } else {
            sprintf(ptr, "%s%s", "4", lineEnding);
        }
    } else {
        // update serial number
        memcpy( (*atcInfo)->serial_number, (char *)&arg, strlen( (char *)&arg));
        
        if ((*atcInfo)->dce_response_format) {
            sprintf(ptr, "%s%s%s%s", lineEnding, (char *)&arg, AT_STRING_OK, lineEnding);
        } else {
            sprintf(ptr, "%s%s%s", (char *)&arg,AT_STRING_OK, lineEnding);
        }
    }
    sendRsp(ptr);
    
    return NULL;
}

/*
 * Timer Handler
 */
static void ATGSNTimerHandler(void *arg) {
    // stop timer
    xTimerStop(timerIDGSN, 0);

    // call CBF
    timerCBF = ReqProductSerialNumberIdentification(CMD_TIMER_EXPIRED, NULL);
    timerCBF(arg);
}

/* 
 * AT Command Receiver
 *   command : AT+GSN
 */
int AtRequestProductSerialNumberIdentification(const char *cmd, char *parameter, char *result, int timeout) {
    struct EventMessage msg;

    // Check parameter ? or ignore ?
    if (*parameter) {
        ReqProductSerialNumberIdentification(CMD_NG, NULL);
        return -1;
    }

    /* req  */
    msg.event = EVT_PRODUCT_SERIAL_NUMBER_INDENTIFICATION_REQ;
    msg.len = 0;
    msg.fptr = ReqProductSerialNumberIdentification;
    memcpy(msg.cmdName, cmd, strlen(cmd));
    msg.queueInfo.from = atTaskQueue;
    msg.queueInfo.to = productTaskQueue;
    msg.queueInfo.returnto = atTaskQueue;
    
    xQueueSend(productTaskQueue, &msg, portMAX_DELAY);
    
    // create timer
    if (timeout > 0) {
        timerIDGSN = xTimerCreate("GSNTimer", (timeout / portTICK_PERIOD_MS), pdFALSE, 0, ATGSNTimerHandler);
    }

    return 0;
}

/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/
