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

#define REVISION_INFO                  "1.0.0.0-p1"
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
int readModelName(struct EventMessage req);
int readRevisionInfo(struct EventMessage req);

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
int readRevisionInfo(struct EventMessage req) {
    struct EventMessage rsp;
    char buf[MAX_REVISION_INFO_SIZE]= {0};
    
    sprintf(buf, "%s",REVISION_INFO);
      
    rsp.event = EVT_REQUEST_REVISION_IDENTIFICATION_RSP;
    rsp.len = strlen(buf);
    memcpy(rsp.parameters.paramString,buf, rsp.len );
    memcpy(rsp.cmdName, req.cmdName, strlen(req.cmdName));
    rsp.state = CMD_DONE;
    rsp.fptr = req.fptr;
    rsp.queueInfo.from = req.queueInfo.to;
    rsp.queueInfo.to = req.queueInfo.returnto;
    rsp.queueInfo.returnto = NULL;
     
    xQueueSend(rsp.queueInfo.to, &rsp, portMAX_DELAY);
    return 0;
}


/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/