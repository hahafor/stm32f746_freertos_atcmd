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

#include <semphr.h>

#include "../Inc/event.h"
#include "../Inc/contact.h"

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
portTASK_FUNCTION_PROTO(vTaskProduct, pvParameters);

/****************************************************************************/
/** **/
/** EXPORTED VARIABLES **/
/** **/
/****************************************************************************/
QueueHandle_t productTaskQueue = 0;

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
extern int readSerialNumber(struct EventMessage req);
extern int readModelName(struct EventMessage req);
extern int factoryResetHandler(struct EventMessage msg);
extern int readManufacturerInfo(struct EventMessage req);
extern int readRevisionInfo(struct EventMessage req);

/****************************************************************************/
/** **/
/** LOCAL FUNCTIONS **/
/** **/
/****************************************************************************/

portTASK_FUNCTION_PROTO(vTaskProduct, pvParameters) {
    struct EventMessage msg;

    for (;;) {
        xQueueReceive(productTaskQueue, &msg, portMAX_DELAY);
        switch (msg.event) {
            case EVT_REQUEST_MODEL_IDENTIFICATION_REQ:
                readModelName(msg);
                break;
                
            case EVT_PRODUCT_SERIAL_NUMBER_INDENTIFICATION_REQ:
                readSerialNumber(msg);
                break;
            
            case EVT_RESET_TO_DEFAULT_CONFIGURATION_REQ:
                factoryResetHandler(msg);
                break;

            case EVT_RESET_TO_FACTORY_DEFINED_CONFIGURATION_REQ:
                factoryResetHandler(msg);
                break;
            
            case EVT_REQUEST_MANUFACTURERER_IDENTIFICATION_REQ:
                readManufacturerInfo(msg);
                break;
                
            case EVT_REQUEST_REVISION_IDENTIFICATION_REQ:
                readRevisionInfo(msg);
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