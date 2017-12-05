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


/****************************************************************************/
/** **/
/** DEFINITIONS AND MACROS **/
/** **/
/****************************************************************************/
#define RX_BUF_SIZE     1024
#define TX_BUF_SIZE     1024

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
portTASK_FUNCTION_PROTO(vTaskUart, pvParameters);

/****************************************************************************/
/** **/
/** EXPORTED VARIABLES **/
/** **/
/****************************************************************************/
QueueHandle_t uartTaskQueue = 0;

/****************************************************************************/
/** **/
/** GLOBAL VARIABLES **/
/** **/
/****************************************************************************/
extern QueueHandle_t atTaskQueue;

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
extern int testContact(struct EventMessage req);
extern int readContact(struct EventMessage req);
extern int writeContact(struct EventMessage req);
extern void initContact(void);
extern void load(char *fileName);
/****************************************************************************/
/** **/
/** LOCAL FUNCTIONS **/
/** **/
/****************************************************************************/

portTASK_FUNCTION_PROTO(vTaskUart, pvParameters) {
    struct EventMessage msg;

    initContact();
    
    for (;;) {
        xQueueReceive(uartTaskQueue, &msg, portMAX_DELAY);
        switch (msg.event) {
            case EVT_CONTACT_READ_PHONEBOOK_REQ:
                readContact(msg);
                break;

            case EVT_CONTACT_WRITE_PHONEBOOK_REQ:
                writeContact(msg);
                break;

            case EVT_CONTACT_VALID_RANGE_REQ:       
                testContact(msg);
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

