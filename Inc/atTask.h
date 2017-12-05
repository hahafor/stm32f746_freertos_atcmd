/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   atTask.h
 * Author: seongjinoh
 *
 * Created on July 26, 2017, 3:54 PM
 */

#ifndef ATTASK_H
#define ATTASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis_os.h" 

#define MAX_MSG_SIZE            128
    
    
    
    enum atcEventType {
        ATC_EVT_NONE = 0,
        ATC_EVT_SEND_ECHO,
        ATC_EVT_PARSER,
        ATC_EVT_MAX,
    };


    
typedef struct ELEMENT {
    int cmd;
    int value;
    int len;
    char message[ MAX_MSG_SIZE ];
    void (*fptr)(int cmd);
} ELEMENT;


    typedef struct NODE {
        struct NODE *prev;
        struct NODE *next;

        struct ELEMENT *element;
        int             atcEvt;
        void *(*fptr)(int cmd,void *arg);
    } NODE;

    typedef struct MESSAGE {
        NODE*           head;
        NODE*           tail;
        int             count;
    } MESSAGE;
    
    
    typedef struct _msg {
        long msg_type;
        int len;
        char msg_text[ MAX_MSG_SIZE ];
        int from;
        int to;
        int return_to;
    } msg_t;

    extern QueueHandle_t atTaskQueue;
    
    extern MESSAGE** atcMessage;
    extern void doSendMsg(char *buf);
    
    extern NODE* createMessage(ELEMENT *element);
    extern void addMessage(MESSAGE **message, NODE *node);
    extern void deleteMessage(MESSAGE **message, NODE *node);
    extern int getMessageCount(MESSAGE **message);

    
#ifdef __cplusplus
}
#endif

#endif /* ATTASK_H */

