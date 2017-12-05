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
#define MAX_ARRAY_SIZE      12

#define ADC_POLL_DURATION   500

#define ADC_ERROR_VAL       (-9999)

#define MCP3008_CH_LM35         0
#define MCP3008_CH_LM35_MASK    ( 1 << MCP3008_CH_LM35 )

#define MCP3008_CH_MASK     ( MCP3008_CH_LM35_MASK )

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

int getAvgTemperature(void);
static int updateTemp(int adc);
static unsigned char readMCP3008Adc(unsigned char channel);
static int registerADCTimer(int newTimeOut);
int stopMCP3008(void);
int doMCP3008Adc(void);
int isSupportedCh(int maskBit);

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
static TimerHandle_t timerMCP3008 = NULL;


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

static int tempArray[MAX_ARRAY_SIZE]={0};

int getAvgTemperature(void) {
    int i, sum=0;
    int min=0xFF, max=0;
    
    for(i=0; i<MAX_ARRAY_SIZE; i++) {
        if ( min > tempArray[i]) {
            min = tempArray[i];
        }
        if (max < tempArray[i]) {
            max = tempArray[i];
        }
        sum += tempArray[i];
    }
    sum = sum - min - max;

    return (sum / (MAX_ARRAY_SIZE -2));
}


static int updateTemp(int adcValue) {
    static int index=0;
    
    if ( adcValue == ADC_ERROR_VAL)
        return -1;
    
    if ( adcValue < 0 || adcValue > MAX_ADC_VALUE )
        return -1;
    
    tempArray[index++] = adcValue;
    if (index >= MAX_ARRAY_SIZE)
        index = 0;
    
    return 0;
}

int isSupportedCh(int maskBit) {
    return ((MCP3008_CH_MASK >> maskBit) & 0x01);
}

static unsigned char readMCP3008Adc(unsigned char channel) {
    unsigned char buf[3];
    unsigned char adcValue = 0;
    
    // supported channel?
    if (!isSupportedCh(channel)) {
        return ADC_ERROR_VAL;
    }
    
    buf[0] = 0x01;
    buf[1] = ((0x01 << 3) | (channel & 0x07)) << 4;
    buf[2] = 0x0;
    
    // GPIO_OUT ( MCP3008_CS, LOW);      // LOW MCP' CS : Active
    // readSPIData(channel, buf, 3);
    
    buf[1] = buf[1] & 0x03;
    adcValue = (buf[1] << 8) | buf[2];
    
    // GPIO_OUT ( MCP3008_CS, HIGH);      // HIGH MCP'S CS
    
    return adcValue;
}


/*
 * Timer Handler
 */
static void ADCTimerHandler(void *arg) {
    // read temperature
    updateTemp( (int)readMCP3008Adc(MCP3008_CH0_LM35));
}


static int registerADCTimer(int newTimeOut) {    
    timerMCP3008 = xTimerCreate("MCP ADC Timer", (newTimeOut / portTICK_PERIOD_MS), pdTRUE, 0, ADCTimerHandler);
    return 0;
}


static int stopADCTimer(void) {
    xTimerStop(timerMCP3008, 0);
    
    return 0;
}


int doMCP3008Adc(void) {
    // If power source is LDO, turn on the LDO
   
    // after power is stable (you may need delay)
    
    // start ADC polling timer
    registerADCTimer(ADC_POLL_DURATION);
    
    return 0;
}

int stopMCP3008(void) {
    // stop timer
    stopADCTimer();
    
    // if power source is LDO, turn off the LDO
    
    return 0;
}
/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/