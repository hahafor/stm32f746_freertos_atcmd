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

static void ATGMITimerHandler(void *arg);

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
static TimerHandle_t timerIDGMI = NULL;


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
void *RequestManufacturerIdentificationHandler(int evt, void *arg);
int AtRequestManufacturerIdentification(const char *cmd, char *parameter, char *result, int timeout);

/****************************************************************************/
/** **/
/** LOCAL FUNCTIONS **/
/** **/
/****************************************************************************/
/*
 * Check parameter
 */

/*
 * Local Command Handler
 */
void *RequestManufacturerIdentificationHandler(int evt, void *arg) {
    char *ptr;
    char lineEnding[3]={0};
    
    // stop timer
    xTimerStop(timerIDGMI, 0);
    
    (void) getLineEnding(lineEnding);
    ptr = (*atcInfo)->atc->cmd_inf->result;
    
    if ( evt == CMD_OK || CMD_DONE ) {
        // update manufacturer info
        memcpy( (*atcInfo)->manufacturer, (char *)&arg, strlen( (char *)&arg));
        
        if ((*atcInfo)->dce_response_format) {
            sprintf(ptr, "%s%s%s%s",lineEnding,(char *)&arg,AT_STRING_OK,lineEnding);
        } else {
            sprintf(ptr, "%s%s%s",(char *)&arg,AT_STRING_OK,lineEnding);
        }
    } else {
        if ((*atcInfo)->dce_response_format) {
            sprintf(ptr, "%s%s%s",lineEnding,AT_STRING_ERROR,lineEnding);
        } else {
            sprintf(ptr, "%s%s",AT_STRING_ERROR,lineEnding);
        }
    }
    sendRsp(ptr); 
    
    return NULL;
}

/*
 * Timer Handler
 */
static void ATGMITimerHandler(void *arg) {
    // stop timer
    xTimerStop(timerIDGMI, 0);

    // call CBF
    timerCBF = RequestManufacturerIdentificationHandler(CMD_TIMER_EXPIRED, NULL);
    timerCBF(arg);
}

/* 
 * AT Command Receiver
 *   command : AT+GMI
 */
int AtRequestManufacturerIdentification(const char *cmd, char *parameter, char *result, int timeout) {
    struct EventMessage msg;

    /* req  */
    msg.event = EVT_REQUEST_MANUFACTURERER_IDENTIFICATION_REQ;
    msg.len = 0;
    msg.fptr = RequestManufacturerIdentificationHandler;
    memcpy(msg.cmdName, cmd, strlen(cmd));
    msg.queueInfo.from = atTaskQueue;
    msg.queueInfo.to = productTaskQueue;
    msg.queueInfo.returnto = atTaskQueue;
    
    xQueueSend(productTaskQueue, &msg, portMAX_DELAY);

    // create timer
    if (timeout > 0) {
        timerIDGMI = xTimerCreate("GMITimer", (timeout / portTICK_PERIOD_MS), pdFALSE, 0, ATGMITimerHandler);
    }

    return 0;
}


/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/