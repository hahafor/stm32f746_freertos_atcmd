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
#define MANUFACTURER_INFO                  "Seongjin Oh, pairoar@gmail.com"

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
int readManufacturerInfo(struct EventMessage req);

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
int readManufacturerInfo(struct EventMessage req) {
    struct EventMessage rsp;
    char buf[MAX_MANUFACTURER_INFO_SIZE]= {0};
    
    sprintf(buf, "%s",MANUFACTURER_INFO);
      
    rsp.event = EVT_REQUEST_NANUFACTURERER_IDENTIFICATION_RSP;
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