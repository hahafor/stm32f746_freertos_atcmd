/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f7xx_hal.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_sdram.h"
#include "cmsis_os.h" 

#include <semphr.h>


#include "../Inc/atTask.h"
#include "../Inc/event.h"
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
portTASK_FUNCTION_PROTO(vTaskAT, pvParameters);

static void initAtcMessage(MESSAGE **message);
static void initMessage(MESSAGE **message);
NODE* createMessage(ELEMENT *element);
void addMessage(struct MESSAGE **message, NODE *node);
void deleteMessage(struct MESSAGE **message, NODE *node);
int getMessageCount(MESSAGE **message);
static void initATC(ATCType** atcType);
void getPrevCommandInfo(struct ATCType **atcType, COMMAND_INFO *cmd_info);
static int doInit(void);
void doSendMsg(char *buf);
static int convertToHandlerFormat(struct EventMessage msg);

/****************************************************************************/
/** **/
/** EXPORTED VARIABLES **/
/** **/
/****************************************************************************/
MESSAGE** atcMessage = NULL;
QueueHandle_t atTaskQueue = 0;

/****************************************************************************/
/** **/
/** GLOBAL VARIABLES **/
/** **/
/****************************************************************************/
extern  SemaphoreHandle_t  atHandlerSignal;
extern QueueHandle_t uartTaskQueue; 

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
extern void atcHandler(struct NODE *req);

/****************************************************************************/
/** **/
/** LOCAL FUNCTIONS **/
/** **/
/****************************************************************************/


static void initAtcMessage(MESSAGE **message) {
    (*message) = (struct MESSAGE *) malloc(sizeof (struct MESSAGE));
    (*message)->head = NULL;
    (*message)->tail = NULL;
    (*message)->count = 0;
}


static void initMessage(MESSAGE **message) {
    (*message) = (struct MESSAGE *) malloc(sizeof (struct MESSAGE));
    (*message)->head = NULL;
    (*message)->tail = NULL;
    (*message)->count = 0;
}


NODE* createMessage(ELEMENT *element) {
    NODE* node = (struct NODE *) malloc(sizeof (struct NODE));
    node->prev = NULL;
    node->next = NULL;
    node->atcEvt = ATC_EVT_NONE;
    node->element = element;

    return node;
}

void addMessage(struct MESSAGE **message, NODE *node) {

    if (!(*message) || !node)
        return;

    if ((*message)->count == 0) {
        (*message)->head = (*message)->tail = node;
        node->next = node->prev = node;
    } else {
        node->prev = (*message)->tail;
        node->next = (*message)->head;

        (*message)->tail->next = node;
        (*message)->head->prev = node;
        (*message)->tail = node;
    }

    (*message)->count++;
}


void deleteMessage(struct MESSAGE **message, NODE *node) {
    if (!(*message) || !node)
        return;

    if ((*message)->count == 1) {
        (*message)->head = (*message)->tail = NULL;
        free(node);
    } else {
        if ((*message)->head == node) {
            (*message)->head = node->next;
        } else if ((*message)->tail == node) {
            (*message)->tail = node->prev;
        }
        node->prev->next = node->next;
        node->next->prev = node->prev;

        free(node);
    }
    (*message)->count--;
}


int getMessageCount(MESSAGE **message) {
    return (*message)->count;
}


static void initATC(ATCType** atcType) {
    (*atcType) = (struct ATCType *) malloc(sizeof (struct ATCType));
    memset((*atcType), 0, sizeof (ATCType));
    
    (*atcType)->count                   = 0;
    (*atcType)->echo                    = 0;
    (*atcType)->v_param                 = 0;
    (*atcType)->s3                      = '\r';
    (*atcType)->s4                      = '\n';
    (*atcType)->dce_response_format     = 1;
    (*atcType)->result_code_suppression = 0;
    
    memset((*atcType)->serial_number, 0, MAX_GSN_SIZE + 1);
    memset((*atcType)->manufacturer,0, MAX_MANUFACTURER_INFO_SIZE + 1);
    memset((*atcType)->model,0,MAX_MODEL_INFO_SIZE+1);
    memset((*atcType)->revision,0,MAX_REVISION_INFO_SIZE+1);
}


void getPrevCommandInfo(struct ATCType **atcType, COMMAND_INFO *cmd_info) {
    if (!(*atcType) || !(*atcType)->atc->cmd_inf) {
        cmd_info = NULL;
        return;
    }
    cmd_info = (*atcType)->atc->cmd_inf;
}


static int doInit(void) {
    initAtcMessage(atcMessage);

    initATC(atcInfo);

    return 0;
}

void doSendMsg(char *buf) {
    // send to UART task
    int len = 0;
    struct EventMessage msg;

    len = strlen(buf);
    msg.event = EVT_UART_TX_AT_CMD_REQ;
    strcpy((char *) &msg.parameters.paramString, buf);
    msg.parameters.paramString[len] = '\0';
    msg.len = len;

    xQueueSend(uartTaskQueue, &msg, portMAX_DELAY);
}

static int convertToHandlerFormat(struct EventMessage msg) {
    struct NODE *req = NULL;
    struct ELEMENT *element = NULL;
    struct EventMessage newMsg;
    
    /*  */
    element = (struct ELEMENT *) malloc(sizeof (struct ELEMENT));
    memset(element, 0, sizeof (ELEMENT));
    element->cmd = msg.event;
    element->len = strlen(msg.parameters.paramString);
    memcpy((void *) &req, (void *) &msg.parameters.paramString, element->len);

    req = createMessage(element); 

    newMsg.event = EVT_ATC_HANDLE_REQ;
    newMsg.len = sizeof (req);
    memcpy((void *) &newMsg.parameters.paramString, (void *) &req, sizeof (req));
    
    xQueueSend(atTaskQueue, &msg, portMAX_DELAY);
    
    return 0;
}


portTASK_FUNCTION_PROTO(vTaskAT, pvParameters) {
    struct EventMessage msg;
    struct NODE *req = NULL;

    doInit();
    
    for (;;) {
        xQueueReceive(atTaskQueue, &msg, portMAX_DELAY);
        switch (msg.event) {
            case EVT_ATC_HANDLE_AT_CMD_REQ:
                rxHandler(msg.parameters.paramString, msg.len);
                break;

                // for AT Task's internal dispatcher
            case EVT_ATC_PARSING_REQ:
                atParser();
                break;

            case EVT_ATC_HANDLE_REQ:
                req->fptr = msg.fptr;
                req->element->len = msg.len;
                req->element->cmd = msg.state;
                memcpy((void *) req->element->message, (void *) &msg.parameters.paramString, msg.len);
                atcHandler(req);
                break;

            default:
                convertToHandlerFormat(msg);
                break;
        }
    }
}

/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/
