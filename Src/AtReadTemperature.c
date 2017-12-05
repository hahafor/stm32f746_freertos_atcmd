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

static void ATTEMPTimerHandler(void *arg);

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
extern QueueHandle_t sensorTaskQueue;

/****************************************************************************/
/** **/
/** LOCAL VARIABLES **/
/** **/
/****************************************************************************/
static TimerHandle_t timerIDTEMP = NULL;


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
void *ReadTemperatureHandler(int evt, void *arg);
int AtReadTemperature(const char *cmd, char *parameter, char *result, int timeout);

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
void *ReadTemperatureHandler(int evt, void *arg) {
    char *ptr;
    char lineEnding[3]={0};
    
    // stop timer
    xTimerStop(timerIDTEMP, 0);
    
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
static void ATTEMPTimerHandler(void *arg) {
    // stop timer
    xTimerStop(timerIDTEMP, 0);

    // call CBF
    timerCBF = ReadTemperatureHandler(CMD_TIMER_EXPIRED, NULL);
    timerCBF(arg);
}

/* 
 * AT Command Receiver
 *   command : AT+TEMP=<value1>,<value2>
 *   value1 : 0 - stop, 1 - read
 *   value2 : n time
 *   value3 : duration, at least 1000ms
 *     ex) AT+TEMP=1,1,0    : Read 1 time
 *     ex) AT+TEMP=1,2,1000 : Read 2 times every 1000ms
 *     ex) AT+TeMP=0,0,0    : Stop reading temperature
 *     ex) AT+TEMP=1,999,2000 : Read temperature every 2000ms
 */
int AtReadTemperature(const char *cmd, char *parameter, char *result, int timeout) {
    struct EventMessage msg;
    
    /* req  */
    msg.event = EVT_REQUEST_READ_TEMPERATURE_REQ;
    msg.len = 0;
    
    msg.fptr = ReadTemperatureHandler;
    memcpy(msg.cmdName, cmd, strlen(cmd));
    msg.queueInfo.from = atTaskQueue;
    msg.queueInfo.to = sensorTaskQueue;
    msg.queueInfo.returnto = atTaskQueue;
    
    xQueueSend(sensorTaskQueue, &msg, portMAX_DELAY);

    // create timer
    if (timeout > 0) {
        timerIDTEMP = xTimerCreate("TEMPTimer", (timeout / portTICK_PERIOD_MS), pdFALSE, 0, ATTEMPTimerHandler);
    }

    return 0;
}


/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/