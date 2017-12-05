/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   event.h
 * Author: seongjinoh
 *
 * Created on July 26, 2017, 6:31 AM
 */

#ifndef EVENT_H
#define EVENT_H

#ifdef __cplusplus
extern "C" {
#endif
    
    #include <FreeRTOS.h>
    #include "queue.h"

    enum EventType {
        EVT_NONE,
        EVT_UART_TX_AT_CMD_REQ,
        EVT_ATC_HANDLE_AT_CMD_REQ,
        EVT_ATC_PARSING_REQ,
        EVT_ATC_HANDLE_REQ,
        EVT_CONTACT_READ_PHONEBOOK_REQ,
        EVT_CONTACT_READ_PHONEBOOK_RSP,
        EVT_CONTACT_WRITE_PHONEBOOK_REQ,
        EVT_CONTACT_WRITE_PHONEBOOK_RSP,
        EVT_CONTACT_VALID_RANGE_REQ,
        EVT_CONTACT_VALID_RANGE_RSP,
        EVT_RESET_TO_DEFAULT_CONFIGURATION_REQ,
        EVT_RESET_TO_DEFAULT_CONFIGURATION_CNF,
        EVT_RESET_TO_DEFAULT_CONFIGURATION_RSP,
        EVT_RESET_TO_FACTORY_DEFINED_CONFIGURATION_REQ,
        EVT_RESET_TO_FACTORY_DEFINED_CONFIGURATION_RSP,
        EVT_PRODUCT_SERIAL_NUMBER_INDENTIFICATION_REQ,
        EVT_PRODUCT_SERIAL_NUMBER_INDENTIFICATION_RSP,
        EVT_REQUEST_MODEL_IDENTIFICATION_REQ,
        EVT_REQUEST_MODEL_IDENTIFICATION_RSP,
        EVT_REQUEST_MANUFACTURERER_IDENTIFICATION_REQ,
        EVT_REQUEST_NANUFACTURERER_IDENTIFICATION_RSP,
        EVT_REQUEST_COMMAND_LINE_TERMINATION_CHARACTER_REQ,
        EVT_REQUEST_COMMAND_LINE_TERMINATION_CHARACTER_RSP,
        EVT_REQUEST_DCE_RESPONSE_FORMAT_REQ,
        EVT_REQUEST_DCE_RESPONSE_FORMAT_RSP,
        EVT_REQUEST_REVISION_IDENTIFICATION_REQ,
        EVT_REQUEST_REVISION_IDENTIFICATION_RSP,
        EVT_REQUEST_RESULT_CODE_SUPRESSION_REQ,
        EVT_REQUEST_RESULT_CODE_SUPRESSION_RSP,
        EVT_REQUEST_READ_TEMPERATURE_REQ,
        EVT_REQUEST_READ_TEMPERATURE_RSP,
        EVT_MAX,
    };

    typedef struct {
        QueueHandle_t from;
        QueueHandle_t to;
        QueueHandle_t returnto;
    } QueueInfo;
        
    struct EventMessage {
        QueueInfo           queueInfo;
        enum EventType      event;
        char                cmdName[13];
        int                 len;
        int                 state;
        void *(*fptr)(int cmd,void *arg);
        union {
            int             paramEvent1;
            int             paramEvent2;
            int             paramEvent3;
            char            paramString[128];
        } parameters;
    };


#define SAFEFREE(PTR)       free(PTR); PTR = NULL

#ifdef __cplusplus
}
#endif

#endif /* EVENT_H */

