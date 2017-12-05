/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


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

#define MODEL_NAME                  "STM32F746-FreeRTOS"

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
int factoryResetHandler(struct EventMessage msg);
static int doFactoryReset(int mode);
int getResetMode(void);
int setResetMode(int mode);
int sendResetDone(void);

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
/****************************************************************************/
/** **/
/** LOCAL FUNCTIONS **/
/** **/
/****************************************************************************/
static int resetMode = 0;
static int sentConfirmed = 0;

struct EventMessage rsp;

int setResetMode(int mode) {
    resetMode = mode;
    
    return 0;
}

int getResetMode(void) {
    return resetMode;
}

int sendResetDone(void) {
    if (sentConfirmed) {
        rsp.event = EVT_RESET_TO_DEFAULT_CONFIGURATION_RSP;
        rsp.state = CMD_DONE;
     
        xQueueSend(rsp.queueInfo.to, &rsp, portMAX_DELAY);
        sentConfirmed=0;
    }
    
    return 0;
}

static int doFactoryReset(int mode) {
    // do factory reset or configuration
    
    
    // send "DONE"
    (void) sendResetDone();
    
    // after a few seconds
    
    // reboot...
    
    return 0;
}

int factoryResetHandler(struct EventMessage msg) {
    //struct EventMessage rsp;
    
    setResetMode(msg.parameters.paramEvent1);
    
    rsp.event = EVT_RESET_TO_DEFAULT_CONFIGURATION_CNF;
    rsp.len = 0;
    memcpy(rsp.cmdName, msg.cmdName, strlen(msg.cmdName));
    rsp.state = CMD_CNF;
    rsp.fptr = msg.fptr;
    rsp.queueInfo.from = msg.queueInfo.to;
    rsp.queueInfo.to = msg.queueInfo.returnto;
    rsp.queueInfo.returnto = NULL;
     
    xQueueSend(rsp.queueInfo.to, &rsp, portMAX_DELAY);
    sentConfirmed = 1;
    
    doFactoryReset(getResetMode());
    
    return 0;
}


/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/