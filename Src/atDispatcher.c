/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>

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
void atcHandler(struct NODE *req);

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
extern int readModelName(char *modelName);

/****************************************************************************/
/** **/
/** LOCAL FUNCTIONS **/
/** **/
/****************************************************************************/

void atcHandler(struct NODE *req) {
    int rxCommand = req->element->cmd;

    // need pre processing?
    switch (rxCommand) {
        case EVT_CONTACT_READ_PHONEBOOK_RSP:
            break;
            
        case EVT_CONTACT_WRITE_PHONEBOOK_RSP:
            break;
            
        case EVT_CONTACT_VALID_RANGE_RSP:
            break;
            
        case EVT_PRODUCT_SERIAL_NUMBER_INDENTIFICATION_RSP:
            break;
            
        case EVT_REQUEST_MODEL_IDENTIFICATION_RSP:
            break;
            
        case EVT_RESET_TO_DEFAULT_CONFIGURATION_CNF:
            break;
            
        default:
            break;
    }

    // call handler
    (req->fptr)(req->element->cmd, req->element->message);
    
    // free
    if (req->element != NULL)
        free(req->element);
    if (req != NULL)
        free(req);

    taskYIELD();
}

/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/
