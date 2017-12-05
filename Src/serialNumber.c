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
#define SERIAL_NUMBER_FILE          "serial.txt"
#define INITIAL_SERIAL_NUMBER       "A1234567890"

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
void initSerialNumber(void);
int readSerialNumber(struct EventMessage req);
int writeSerialNumber(char *serialNumber);

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
void initSerialNumber(void) {
    if (access(SERIAL_NUMBER_FILE, F_OK) == 0) {
        //file exist
    } else {
        // file not exist
        writeSerialNumber((char *) INITIAL_SERIAL_NUMBER);
    }
}

int readSerialNumber(struct EventMessage req) {
    struct EventMessage rsp;
    FILE *fp = fopen(SERIAL_NUMBER_FILE, "r");
    if (fp == NULL) {
        // file open error!
        return -1;
    }
    char buf[MAX_GSN_SIZE]= {0};
    
    fgets(buf, MAX_GSN_SIZE, (FILE*) fp);
    fclose(fp);
    
    rsp.event = EVT_PRODUCT_SERIAL_NUMBER_INDENTIFICATION_RSP;
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

int writeSerialNumber(char *serialNumber) {
    FILE *fp = fopen(SERIAL_NUMBER_FILE, "w");
    if (fp == NULL) {
        // file open error!
        return -1;
    }
    fputs(serialNumber, fp);
    fclose(fp);

    return 0;
}

/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/
