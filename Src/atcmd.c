/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "stm32f7xx_hal.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_sdram.h"

#include "cmsis_os.h" 

#include "../Inc/atcmd.h"
#include "../Inc/atTask.h"
#include "../Inc/event.h"


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

const command_t ATCommands[] = {
    COMMAND_ENTRY("",, NULL, 0),
    COMMAND_ENTRY("Z", ResetToDefaultConfiguration, NULL, 3000),
    COMMAND_ENTRY("E", CommandEcho, NULL, 0),
    COMMAND_ENTRY("+GSN", RequestProductSerialNumberIdentification, NULL, 3000),
    COMMAND_ENTRY("&F", SetToFactoryDefinedConfiguration, NULL, 3000),
    COMMAND_ENTRY("+ASTO", StoreTelephoneNumber, NULL, 3000),
    COMMAND_ENTRY("+GMM", RequestModelIdentification, NULL, 2000),
    COMMAND_ENTRY("S3", CommandLineTerminationCharacter, NULL, 0),
    COMMAND_ENTRY("S4", ResponseFormattingCharacter, NULL, 0),
    COMMAND_ENTRY("+GMI", RequestManufacturerIdentification, NULL, 2000),
    COMMAND_ENTRY("+GMR", RequestRevisionIdentification, NULL, 2000),
    COMMAND_ENTRY("Q", ResultCodeSuppression, NULL, 0),
    COMMAND_ENTRY("+TEMP", ReadTemperature, NULL, 2000),
    COMMAND_ENTRY("",, NULL, 0),
};

int isValidSyntaxFormat(char *cmd, int *repeat);
int isValidCommandFormat(char *cmd, int *type, int *param_type);
int seperateCommandNParam(char *cmd, int type, int param_type, char *cmdName, char *param);
int getCommandIndex(char *cmdName, int type, int *index);
void sendEcho(char *data);
void sendRsp(char *data);
void rxHandler(char *rxBuf, int len);
void atParser(void);
int getLineEnding(char *ending);

/****************************************************************************/
/** **/
/** EXPORTED VARIABLES **/
/** **/
/****************************************************************************/
ATCType **atcInfo = NULL;

/****************************************************************************/
/** **/
/** GLOBAL VARIABLES **/
/** **/
/****************************************************************************/
extern const command_t ATCommands[];
extern QueueHandle_t uartTaskQueue; 

/****************************************************************************/
/** **/
/** LOCAL VARIABLES **/
/** **/
/****************************************************************************/
char extended_characters[] = {'!', '%', '-', '.', '/', ':', '_', '\0'};


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
/*
 *  return lineending & length
 */
int getLineEnding(char *ending) {
    int len=0;
    
    ending[len++] = (*atcInfo)->s3;
    ending[len++] = (*atcInfo)->s4;
    ending[len] = '\0';
    
    return len;
}


/*
    All of command should start with "AT" and end with lineending (\r\n).
 But only  "A/\r\n" is out of rules and we need to handle it. 
 * return value
 *    1: valid syntax
 *    0: invalid syntax 
 */
int isValidSyntaxFormat(char *cmd, int *repeat) {
    int len;
    int index;
    char *temp;
    char lineEnding[3];
    temp = cmd;

    len = strlen(cmd);
    (void) getLineEnding(lineEnding);
    
    // to upper
    for (index = 0; index < 2; index++) {
        temp[index] = toupper(temp[index]);
    }

    // check line endings
    if (strncmp((char *) &temp[len - 2], lineEnding, 2) != 0) {
        *repeat = 0;
        return 0;
    }
    temp[len - 2] = '\0';

    // "A/" 
    if (len == 2 && strncmp(cmd, "A/", 2) == 0) {
        *repeat = 1;
        return 1;
    }

    // "AT"
    if (strncmp(cmd, "AT", 2) == 0) {
        *repeat = 0;
        return 1;
    } else {
        *repeat = 0;
        return 0;
    }
}

/* 
 * return value
 *   1 : valid
 *   0 : invalid
 */
int isValidCommandFormat(char *cmd, int *type, int *param_type) {
    char *temp, *token;
    temp = cmd;

    // to upper
    temp[2] = toupper(temp[2]);

    // S Parameter
    if (temp[2] == 'S') {
        *type = ATC_BASIC_SPARAM;
        
        token = strchr((char *) &cmd[3], '=');
        if (token != NULL) {
            // TEST command ?
            token = strchr((char *) &cmd[3], '?');
            if (token != NULL) {
                *param_type = ATC_PARAM_TEST;
            } else {
                *param_type = ATC_PARAM_SET;
            }            
        } else {
            *param_type = ATC_PARAM_READ;
        }
        return 1;
    }
    
    // Basic Syntax command
    if ((temp[2] == '&') || (temp[2] >= 'A' && temp[2] <= 'Z')) {
        // Basic Syntax Format
        *type = ATC_BASIC_CMD;
        *param_type = ATC_PARAM_NONE;
        return 1;
    }
    
    // Extended Syntax command
    if (temp[2] == '+') {
        // Extended Syntax Format
        *type = ATC_EXTENDED_CMD;
        
        token = strchr((char *) &cmd[3], '=');
        if (token != NULL) {
            // TEST command ?
            token = strchr((char *) &cmd[3], '?');
            if (token != NULL) {
                *param_type = ATC_PARAM_TEST;
            } else {
                *param_type = ATC_PARAM_SET;
            }            
        } else {
            *param_type = ATC_PARAM_READ;
        }
        return 1;
    } 
        
    *type = ATC_SYNTAX_NONE;
    *param_type = ATC_PARAM_NONE;
    return 0;
}

/*
 * return value:
 *   0: no error
 *  -1: error
 */
int seperateCommandNParam(char *cmd, int type, int param_type, char *cmdName, char *param) {
    int len;
    char *temp, *token;

    len = (int)strlen(cmd);
    if (type == ATC_BASIC_CMD) {
        temp = cmd;
        // &
        if (cmd[2] == '&') {
            // to upper
            //temp[3] = toupper(temp[3]);
            if (cmd[3] == 'C' || cmd[3] == 'D' || cmd[3] == 'F') {
                memcpy(cmdName, (char *) &cmd[2], 2);
                if (strlen(cmd) > 4) {
                    memcpy(param, (char *) &cmd[4], len - 4);
                } else {
                    *param = '\0';
                }
                return 0;
            } else {
                // error, not supported
                *cmdName = '\0';
                *param = '\0';
                
                return -1;
            }
        } else {
            memcpy(cmdName, (char *) &cmd[2], 1);
            if (cmd[3] != '\0') {
                if (len > 3) {
                    memcpy(param, (char *) &cmd[3], len - 3);
                    param[strlen(param)] = '\0';
                } else {
                    *param = '\0';
                }
                return 0;
            } else {
                *param = '\0';
                return -1;
            }
        }
    } else if (type == ATC_BASIC_SPARAM) {
        token = strchr((char *) &cmd[3], '=');
        if (token != NULL) {
            len= strlen(token);
            token = strchr((char *) &cmd[4], '?');
            if (token != NULL) {
                // parameter test
                memcpy(cmdName, (char *)&cmd[2], (strlen(cmd) - len - 2) );
                *param = '\0';
            } else {
                // param set
                memcpy(cmdName, (char *)&cmd[2], (strlen(cmd) - len - 2) );
                memcpy(param, (char *)&cmd[2+strlen(cmdName)+1], (strlen(cmd) - (strlen(cmdName) -2 -1)));
            }
            return 0;
        } else {
            // param read
            token = strchr((char *) &cmd[4], '?');
            if (token != NULL) {
                len = strlen(token);
                memcpy(cmdName, (char *)&cmd[2], (strlen(cmd) - len - 2) );
            } else {
                memcpy(cmdName, (char *)&cmd[2], (strlen(cmd) - 2) );
            }
            *param = '\0';
            return 0;
        }
    } else if (type == ATC_EXTENDED_CMD) {
        token = strchr((char *) &cmd[2], '+');
        if (token != NULL) {
            len= strlen(token);
            // read command ?
            token = strchr((char *) &cmd[4], '=');
            if (token == NULL) {
                token = strchr((char *) &cmd[4], '?');
                if (token == NULL) {
                    // Action
                    memcpy(cmdName, (char *)&cmd[2], strlen(cmd) -2);
                } else {
                    // Read
                    len = strlen(token);
                    memcpy(cmdName, (char *)&cmd[2], (strlen(cmd) - len - 2 ));
                }
                *param = '\0';
                return 0;
            } else {
                len = strlen(token);
                token = strchr((char *) &cmd[4], '?');
                if (token == NULL) {
                    // param set
                    memcpy(cmdName, (char *)&cmd[2], strlen(cmd) - len -2);
                    memcpy(param, (char *)&cmd[2+strlen(cmdName)+1], strlen(cmd) -1);
                } else {
                    // param test
                    memcpy(cmdName, (char *)&cmd[2], strlen(cmd) - len -2);
                    *param = '\0';
                }
                return 0;
            }
        }
    }
    
    // error!
    *cmdName = '\0';
    *param = '\0';

    return -1;
}

/*
 * return value:
 *   0: no error
 *  -1: error
 */
int getCommandIndex(char *cmdName, int type, int *index) {
    int i = START_INDEX;

    while (ATCommands[i].cmd != NULL && strlen(ATCommands[i].cmd) > 0) {
        if (strlen(ATCommands[i].cmd) > 0 && strncmp(ATCommands[i].cmd, cmdName, strlen(ATCommands[i].cmd)) == 0) {
            *index = i;
            return 0;
        }
        i++;
    }
    *index = 0;
    return -1;
}

void sendEcho(char *data) {
    doSendMsg(data);
}

void sendRsp(char *data) {
    doSendMsg(data);
}

void rxHandler(char *rxBuf, int len) {
    char *temp;
    NODE *message = NULL;
    ELEMENT *element = NULL;
    struct EventMessage msg;

    //
    if ((*atcInfo)->echo) {
        sendEcho(message->element->message);
    }

    // check length
    if (len < 4)
        return;

    temp = strtok(rxBuf, ";");
    while (temp != NULL) {
        element = (struct ELEMENT *) malloc(sizeof (struct ELEMENT));
        memset(element, 0, sizeof (struct ELEMENT));
        element->len = len;
        strcpy(element->message, rxBuf);

        message = createMessage(element);
        message->atcEvt = ATC_EVT_PARSER;
        addMessage(atcMessage, message);

        temp = strtok(NULL, ";");
    }

    msg.event = EVT_ATC_PARSING_REQ;
    msg.len = 0;

    xQueueSend(uartTaskQueue, &msg, portMAX_DELAY);
}

/*
 * 
 */
void atParser(void) {
    int value = 0;
    char cmd[64] = {0,};
    struct NODE *message = NULL;
    struct ATC *atc = NULL;
    struct COMMAND_INFO *cmd_inf = NULL;
    struct EventMessage msg;

    if (getMessageCount(atcMessage) == 0) {
        return;
    }

    // clear previous one
    //deleteMessage(atcMessage, message);
    
    //
    memset(cmd, 0, 64);
    strcpy(cmd, message->element->message);
    cmd[message->element->len] = '\0';

    // check syntax format
    if (!isValidSyntaxFormat(cmd, &((*atcInfo)->atc->repeat))) {
        goto error;
    }

    // Repeat command ? (A/)
    if ((*atcInfo)->atc->repeat) {
        // copy previous command
        getPrevCommandInfo(atcInfo, cmd_inf);

        if (cmd_inf == NULL) {
            goto error;
        }
        memcpy(cmd, cmd_inf->cmd, cmd_inf->len);
    } else {
        atc = (struct ATC *) malloc(sizeof (struct ATC));
        memset(atc, 0, sizeof (struct ATC));

        cmd_inf = (struct COMMAND_INFO *) malloc(sizeof (struct COMMAND_INFO));
        memset(cmd_inf, 0, sizeof (struct COMMAND_INFO));

        atc->cmd_inf = cmd_inf;
        memcpy(cmd_inf->rx_string, message->element->message, message->element->len);
        cmd_inf->len = message->element->len;
    }

    // AT ?
    if (cmd_inf->len == 2 && (strncmp(cmd, "AT", 2) == 0)) {
        if ((*atcInfo)->atc->repeat) {
            (*atcInfo)->atc->repeat = 0;
        } else {
            cmd_inf->len = 2;
            memcpy(cmd_inf->cmd, cmd, cmd_inf->len);
            cmd_inf->cmd_type = ATC_BASIC_CMD;
            memcpy(cmd_inf->cmd_name, cmd, cmd_inf->len);
            cmd_inf->param[0] = '\0';
            cmd_inf->index = 1;
            cmd_inf->timeout = ATCommands[cmd_inf->index].timeout;
            memcpy(cmd_inf->result, "OK", 2);

            // free previous atc info
            if ((*atcInfo)->atc->cmd_inf)
                free((*atcInfo)->atc->cmd_inf);
            if ((*atcInfo)->atc)
                free((*atcInfo)->atc);

            // update as new one
            (*atcInfo)->atc = atc;
        }
        char lineEnding[3]={0};
        char *ptr = cmd_inf->result ;
    
        (void) getLineEnding(lineEnding);
        if ( (*atcInfo)->dce_response_format) {
            sprintf(ptr, "%s%s%s", lineEnding,AT_STRING_OK, lineEnding);
        } else {
            sprintf(ptr, "%s%s", AT_STRING_OK, lineEnding);
        }
        sendRsp(ptr);  
        
        goto cleanUp;
    }

    // check command format
    if (!isValidCommandFormat(cmd, &cmd_inf->cmd_type, &cmd_inf->param_type)) {
        goto error;
    }

    // seperate  cmd & param
    // int seperateCommandNParam(char *cmd, int type, char *cmdName, char *param) {   
    if (seperateCommandNParam(cmd, cmd_inf->cmd_type, cmd_inf->param_type, cmd_inf->cmd_name, cmd_inf->param) == -1) {
        goto error;
    }

    // index
    // int getCommandindex(char *cmdName, int type, int *index) {
    if (getCommandIndex(cmd_inf->cmd_name, cmd_inf->cmd_type, &cmd_inf->index) == -1) {
        goto error;
    }

    // update timeout
    cmd_inf->timeout = ATCommands[cmd_inf->index].timeout;

    /* call AT command receiver */
    value = ATCommands[cmd_inf->index].dispatcher(ATCommands[cmd_inf->index].cmd, cmd_inf->param, cmd_inf->result, cmd_inf->timeout);
    if (value != 0) {
        // do something
    }

    // free previous atc info
    if ((*atcInfo)->atc->cmd_inf)
        free((*atcInfo)->atc->cmd_inf);
    if ((*atcInfo)->atc)
        free((*atcInfo)->atc);

    // update as new one
    (*atcInfo)->atc = atc;

    goto cleanUp;

error:
    sendRsp(AT_STRING_ERROR);

    // free
    if (cmd_inf != NULL)
        free(cmd_inf);
    if (atc != NULL)
        free(atc);

cleanUp:
    // free
    if (message->element != NULL)
        free(message->element);
    if (message != NULL)
        free(message);

    if (getMessageCount(atcMessage) > 0) {
        msg.event = EVT_ATC_PARSING_REQ;
        msg.len = 0;
        xQueueSend(uartTaskQueue, &msg, portMAX_DELAY);
    }

    taskYIELD();
}

/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/
