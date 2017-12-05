/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   atmcd.h
 * Author: seongjinoh
 *
 * Created on July 26, 2017, 8:05 AM
 */

#ifndef ATMCD_H
#define ATMCD_H

#ifdef __cplusplus
extern "C" {
#endif



    /****************************************************************************/
    /** **/
    /** MODULES USED **/
    /** **/
    /****************************************************************************/

    /****************************************************************************/
    /** **/
    /** DEFINITIONS AND MACROS **/
    /** **/
    /****************************************************************************/
    /* strings */
    #define AT_STRING_OK                    (char *)"OK"    
    #define AT_STRING_NA                    (char *)"NA"
    #define AT_STRING_NG                    (char *)"NG"
    #define AT_STRING_ERROR                 (char *)"ERROR"
    #define AT_STRING_DONE                  (char *)"DONE"

    #define AT_STRING_CONNECT               (char *)"CONNECT"
    #define AT_STRING_RING                  (char *)"RING"
    #define AT_STRING_NO_CARRIER            (char *)"NO CARRIER"
    #define AT_STRING_NO_DIALTONE           (char *)"NO DIALTONE"
    #define AT_STRING_BUSY                  (char *)"BUSY"
    #define AT_STRING_NO_ANSWER             (char *)"NO ANSWER"    
    #define AT_STRING_CMEERROR              (char *)"+CME Error:"

    #define START_INDEX                     1

    #define MAX_ATC_SIZE                    10
    #define MAX_EXTENDED_CMD_SIZE           16

    #define MAX_GSN_SIZE                    2048
    #define MAX_MODEL_INFO_SIZE             2048
    #define MAX_MANUFACTURER_INFO_SIZE      2048
    #define MAX_REVISION_INFO_SIZE          2048


    // S3 : Command line termination character
    //    : default - Carriage Return  '\r'
    #define MAX_S3_RANGE                127

    // S4 : Command line termination character
    //    : default - Carriage Return  '\r'
    #define MAX_S4_RANGE                127

    /****************************************************************************/
    /** **/
    /** TYPEDEFS AND STRUCTURES **/
    /** **/
    /****************************************************************************/

/* Command Syntax */
    enum {
        ATC_SYNTAX_NONE = 0,
        ATC_BASIC_CMD,
        ATC_BASIC_SPARAM,
        ATC_EXTENDED_CMD,
        ATC_SYNTAX_MAX,
    };

    /* PARAM Type */
    enum {
        ATC_PARAM_NONE =0,
        ATC_PARAM_SET,
        ATC_PARAM_READ,
        ATC_PARAM_TEST,
        ATC_PARAM_MAX,
    };
    
/* Response Type */
    enum {
        RESP_CMD,
        RESP_RESULT,
    };

    // result
    enum {
        CMD_NONE = 0,
        CMD_OK,
        CMD_CNF,
        CMD_DONE,
        CMD_INFO,
        CMD_NA,
        CMD_NG,
        CMD_INPROGRESS,
        CMD_TIMER_EXPIRED,
        CMD_MAX,
    };
    /* extended command */
    // Action Execution command :
    //   +<name>
    //   +<name>[=<value>]
    //   +<name>[=<compound_value>]
    // Action Test Command
    //   +<name>=?
    // 
    // Parameter set command syntax 
    //     +<name>=[<value>]
    //     +<name>=[<compound_value>]
    // Parameter read command syntax 
    //      +<name>?
    // Parameter test command
    //      +<name>=?
    //

    /* function pointer prototype */
    typedef int (*genericDispatcher)(const char *cmd, char *parameter, char *result, int timeout);

    typedef struct command {
        const char          *cmd;
        genericDispatcher   dispatcher;
        const char          *_descrtiption;
        int                 timeout;
    } command_t;

    #define COMMAND_ENTRY(label, dispatcher, description, timeout) \
    {   (const char *)label, \
        At##dispatcher, \
        (const char *)description, \
        timeout }

    typedef struct COMMAND_INFO {
        char    rx_string[128];         // recieved command string
        char    cmd[128];               // current 
        int     len;                    // comand length except "/r/n"
        char    cmd_name[MAX_EXTENDED_CMD_SIZE + 1]; // command name
        int     index;                  // command index
        char    param[32];              // param 
        int     cmd_type;               // command type
        int     param_type;             // parameter type
        int     timeout;                // timeout
        int     rsp_required;           // transmit the result code
        int     rsp_type;               // response string
        char    result[64];             // result 
    } COMMAND_INFO;

    typedef struct ATC {
        COMMAND_INFO    *cmd_inf;
        int             repeat;
    } ATC;

    typedef struct ATCType {
        ATC         *atc;

        int         count;
        int         v_param;
        int         echo;
        char        s3;
        char        s4;
        int         dce_response_format;        // V[<value>]
        int         result_code_suppression;    // Q[<value>]
        
        char        serial_number[MAX_GSN_SIZE + 1];                // +GSN
        char        manufacturer[MAX_MANUFACTURER_INFO_SIZE+1];     // +GMI
        char        model[MAX_MODEL_INFO_SIZE+1];                   // +GMM
        char        revision[MAX_REVISION_INFO_SIZE+1];             // +GMR
    } ATCType;


    /****************************************************************************/
    /** **/
    /** EXPORTED VARIABLES **/
    /** **/
    /****************************************************************************/
    extern ATCType **atcInfo;


    /****************************************************************************/
    /** **/
    /** EXPORTED FUNCTIONS **/
    /** **/
    /****************************************************************************/
    extern void rxHandler(char *rxBuf, int len);
    extern void atParser(void);
    extern void getPrevCommandInfo(struct ATCType **atcType, COMMAND_INFO *cmd_info);
    extern void sendRsp(char *data);
    extern int getLineEnding(char *lineEnding);
    
    /* AT Command Receiver */
    extern int At(const char *cmd, char *parameter, char *result, int timeout);
    extern int AtResetToDefaultConfiguration(const char *cmd, char *parameter, char *result, int timeout);
    extern int AtCommandEcho(const char *cmd, char *parameter, char *result, int timeout);
    extern int AtRequestProductSerialNumberIdentification(const char *cmd, char *parameter, char *result, int timeout);
    extern int AtStoreTelephoneNumber(const char *cmd, char *parameter, char *result, int timeout);
    extern int AtSetToFactoryDefinedConfiguration(const char *cmd, char *parameter, char *result, int timeout);
    extern int AtRequestModelIdentification(const char *cmd, char *parameter, char *result, int timeout);
    extern int AtCommandLineTerminationCharacter(const char *cmd, char *parameter, char *result, int timeout);
    extern int AtResponseFormattingCharacter(const char *cmd, char *parameter, char *result, int timeout);
    extern int AtDCEResponseFormat(const char *cmd, char *parameter, char *result, int timeout);
    extern int AtRequestManufacturerIdentification(const char *cmd, char *parameter, char *result, int timeout);
    extern int AtRequestRevisionIdentification(const char *cmd, char *parameter, char *result, int timeout);
    extern int AtResultCodeSuppression(const char *cmd, char *parameter, char *result, int timeout);
    extern int AtReadTemperature(const char *cmd, char *parameter, char *result, int timeout);
    
    /* supported functions */


#ifdef __cplusplus
}
#endif

#endif /* ATMCD_H */
