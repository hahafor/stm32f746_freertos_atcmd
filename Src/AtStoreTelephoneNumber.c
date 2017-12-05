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

static void ATASTOTimerHandler(void *arg);
static int isStorableStrings(char *str);
void *StoreTelephoneNumber(int evt, void *arg);

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
extern QueueHandle_t contactTaskQueue;

/****************************************************************************/
/** **/
/** LOCAL VARIABLES **/
/** **/
/****************************************************************************/
static TimerHandle_t timerIDASTO = NULL;
static int timerExpired = 0;
static char storableStrings[] = "0123456789ABCD#*+,\"TPW@!;";

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
portTASK_FUNCTION_PROTO(vTaskAT, pvParameters);
portTASK_FUNCTION_PROTO(vTaskContact, pvParameters);

/****************************************************************************/
/** **/
/** LOCAL FUNCTIONS **/
/** **/
/****************************************************************************/

// Handler : parent thread
// Receiver : receive result and send request
// Timer : timer thread
// Timer Handler : timeout handler


static int isStorableStrings(char *str) {
    int i, supported = 0, len;
    len = strlen(storableStrings);
    char *ptr = str;
    
    while (*ptr) {
        supported = 0;
        for (i=0; i<len; i++) {
            if (*ptr == storableStrings[i]) {
                supported = 1;
                break;
            }
        }
        
        if ( supported ) {
            *ptr++;
        } else {
            return 0;
        }
    }
    
    return 1;
}

void *StoreTelephoneNumber(int evt, void *arg) {
    char *ptr;
    char lineEnding[3]={0};
    int done = 0;

   
    while (!done) {
        // stop timer
        xTimerStop(timerIDASTO, 0);

        // error handling
        if (evt == CMD_NG || evt == CMD_NA || evt == CMD_TIMER_EXPIRED) {            
            done=1;
            (void) getLineEnding(lineEnding);
            ptr = (*atcInfo)->atc->cmd_inf->result;
            
            if ((*atcInfo)->dce_response_format) {
                sprintf(ptr, "%s%s%s",lineEnding,AT_STRING_ERROR,lineEnding);
            } else {
                sprintf(ptr, "%s%s",AT_STRING_ERROR,lineEnding);
            }
            sendRsp(ptr);

            goto cleanup;
        } else if (evt == CMD_OK || evt == CMD_CNF  ) {

        } else if ( evt == CMD_DONE) {
            // resp "DONE"
            done=1;
            (void) getLineEnding(lineEnding);
            ptr = (*atcInfo)->atc->cmd_inf->result;
            
            if ((*atcInfo)->dce_response_format) {
                sprintf(ptr, "%s%s%s",lineEnding,AT_STRING_DONE,lineEnding);
            } else {
                sprintf(ptr, "%s%s",AT_STRING_DONE,lineEnding);
            }
            sendRsp(ptr);
            
            goto cleanup;
        } else if (evt == CMD_INPROGRESS) {
            // to be revised...
            // reload timer
            // send contact info
            (void) getLineEnding(lineEnding);
            ptr = (*atcInfo)->atc->cmd_inf->result;
            
            if ((*atcInfo)->dce_response_format) {
                sprintf(ptr, "%s%s%s",lineEnding,(char *)&arg,lineEnding);
            } else {
                sprintf(ptr, "%s%s",(char *)&arg,lineEnding);
            }
            sendRsp(ptr);
        }
    }
    
cleanup:
    done=0;
    timerExpired = 0;
    return 0;
 }

/*
 * Timer Handler
 */
static void ATASTOTimerHandler(void *arg) {
    // stop timer
    xTimerStop(timerIDASTO, 0);

    // call CBF
    timerCBF = StoreTelephoneNumber(CMD_TIMER_EXPIRED, NULL);
    timerCBF(arg);
}


/* +ASTO */
int AtStoreTelephoneNumber(const char *cmd, char *parameter, char *result, int timeout) {
    struct EventMessage msg;

    if ( (*atcInfo)->atc->cmd_inf->param_type == ATC_PARAM_READ ) {
        // Read command
        msg.event = EVT_CONTACT_READ_PHONEBOOK_REQ;
        msg.len = 0;
    } else if ((*atcInfo)->atc->cmd_inf->param_type == ATC_PARAM_TEST) {
        // Test command
        msg.event = EVT_CONTACT_VALID_RANGE_REQ;
        msg.len = 0;
    } else {
        if (isStorableStrings(parameter)) {
            // Req. store
            msg.event = EVT_CONTACT_WRITE_PHONEBOOK_REQ;
            msg.len = strlen(parameter);
            memcpy(msg.parameters.paramString, parameter, msg.len);
        } else {
            // Parameter error
            StoreTelephoneNumber(CMD_NG, NULL);
            return -1;
        }
    }

    msg.fptr = StoreTelephoneNumber;
    memcpy(msg.cmdName, cmd, strlen(cmd));
    msg.queueInfo.from = atTaskQueue;
    msg.queueInfo.to = contactTaskQueue;
    msg.queueInfo.returnto = atTaskQueue;
    
    xQueueSend(atTaskQueue, &msg, portMAX_DELAY);

    // create timer
    if (timeout > 0) {
        timerIDASTO = xTimerCreate("ASTOTimer", (timeout / portTICK_PERIOD_MS), pdFALSE, 0, ATASTOTimerHandler);
    }
    
    return 0;
}

/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/
