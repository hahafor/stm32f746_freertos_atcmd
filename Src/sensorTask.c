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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f7xx_hal.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_sdram.h"

#include <FreeRTOS.h>

#include "cmsis_os.h" 

#include "../Inc/event.h"
#include "../Inc/atcmd.h"

/****************************************************************************/
/** **/
/** DEFINITIONS AND MACROS **/
/** **/
/****************************************************************************/
#define MCP3008_CS          1

#define MCP3008_CH0_LM35    0

#define MAX_ADC_VALUE       0xFF
#define MAX_ARRAY_SIZE      10

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
portTASK_FUNCTION_PROTO(vTaskSensor, pvParameters);

void *(*timerCBF) (void *);

int stopTempRptTimer(void);
int TempRptHandler(struct EventMessage req);
int startTempRptTimer(int newTimeOut);
int rptTemperatureInfo(void);


/****************************************************************************/
/** **/
/** EXPORTED VARIABLES **/
/** **/
/****************************************************************************/
QueueHandle_t sensorTaskQueue = 0;

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
static TimerHandle_t timerTempRpt = NULL;

static int cmd=0, nTimes=0, timeOut=0, count=0;
struct EventMessage rsp;

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
extern int getAvgTemperature(void);
extern int doMCP3008Adc(void);
extern int stopMCP3008(void);

/****************************************************************************/
/** **/
/** LOCAL FUNCTIONS **/
/** **/
/****************************************************************************/


int rptTemperatureInfo(void) {
    char buf[2048]= {0};
 
    sprintf(buf, "%s:%03d", rsp.cmdName, getAvgTemperature());
      
    rsp.len = strlen(buf);
    memcpy(rsp.parameters.paramString,buf, rsp.len );
    rsp.state = CMD_OK;
    xQueueSend(rsp.queueInfo.to, &rsp, portMAX_DELAY);
    
    return 0;
}


static void TempRptTimerHandler(void *arg) {
    count++;
    
    if (nTimes != 999 && nTimes == count) {
        stopTempRptTimer();
    }
    
    // report temperature
    (void) rptTemperatureInfo();
}

int startTempRptTimer(int newTimeOut) {
    if (timerTempRpt) {
        // stop & clear working timer
        stopTempRptTimer();
    }
    
    timerTempRpt = xTimerCreate("Temp Report Timer", (newTimeOut / portTICK_PERIOD_MS), 
           (nTimes > 1) ?  pdTRUE : pdFALSE , 0, TempRptTimerHandler);
    
    return 0;
}


int stopTempRptTimer(void) {
    xTimerStop(timerTempRpt, 0);
    cmd = 0, nTimes = 0, timeOut =0, count=0;
    return 0;
}


int TempRptHandler(struct EventMessage req) {
    if (req.parameters.paramEvent1) {
        
        cmd = req.parameters.paramEvent1;
        nTimes = req.parameters.paramEvent2;
        timeOut = req.parameters.paramEvent3;
         
        rsp.event = EVT_REQUEST_READ_TEMPERATURE_RSP;
        memcpy(rsp.cmdName, req.cmdName, strlen(req.cmdName));
        rsp.fptr = req.fptr;
        rsp.queueInfo.from = req.queueInfo.to;
        rsp.queueInfo.to = req.queueInfo.returnto;
        rsp.queueInfo.returnto = NULL;

        rptTemperatureInfo();
        
        startTempRptTimer(timeOut);
    
    } else {
        // stop
        stopTempRptTimer();
    }
    
    return 0;
}


portTASK_FUNCTION_PROTO(vTaskSensor, pvParameters) {
    struct EventMessage msg;

    // init & run MCP3008 ADC
    doMCP3008Adc();
    
    for (;;) {
        xQueueReceive(sensorTaskQueue, &msg, portMAX_DELAY);
        switch (msg.event) {
                
            case EVT_REQUEST_READ_TEMPERATURE_REQ:
                TempRptHandler(msg);
                break;
                
            default:
                break;
        }
    }
}

/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/