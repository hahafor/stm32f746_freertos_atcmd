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
#include "cmsis_os.h" 

#include <semphr.h>

#include "../Inc/event.h"
#include "../Inc/contact.h"
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
void initContact(void);
static int readLine(FILE *fp, char str[], int n);
void load(char *fileName);
static void add(char *name, char *number, char *email, char *group);
static int composeName(char str[], int limit);
static void releasePerson(Person *p);

static void save(char *fileName);
static int search(char *name);
static void printPerson(Person *p);
static void delete(char *name);
static void status(void);

int testContact(struct EventMessage req);
int readContact(struct EventMessage req);
int writeContact(struct EventMessage req);
/****************************************************************************/
/** **/
/** EXPORTED VARIABLES **/
/** **/
/****************************************************************************/
Person **directory;

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
int n = 0;
int capacity;

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

void initContact(void) {
    directory = (Person **)malloc(INIT_CAPACITY * (sizeof(Person *)));
    capacity = INIT_CAPACITY;
}

static int readLine(FILE *fp, char str[], int n) {
    int ch, i=0;
    
    while((ch=fgetc(fp) != '\n') && ch != EOF) {
        if (i<n) {
            str[i++] = ch;
        }
    }
    str[n]='\0';
    
    return i;
}

static void reallocate(void) {
    capacity *= 2;
    Person **tmp =  (Person **)malloc(capacity * sizeof(Person *));
    for(int i=0;i<n;i++) {
        tmp[i] = directory[i];
    }
    free(directory);
    directory = tmp;
}

void load(char *fileName) {
    char buffer[MAX_BUF_SIZE];
    char *name=NULL, *number=NULL, *email=NULL, *group=NULL;
    char *token;
    
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL) {
        printf("Open failed\n");
        return;
    }
    
    while(1) {
        if (readLine(fp, buffer, MAX_BUF_SIZE) <= 0) {
            break;
        }
        
        name = strtok(buffer, ",");
        
        token = strtok(buffer,",");
        if (strcmp(token, " ") != 0)
            number = strdup(token);
        
        token = strtok(buffer,",");
        if (strcmp(token, " ") != 0)
            email = strdup(token);
        
        token = strtok(buffer,",");
        if (strcmp(token, " ") != 0)
            group = strdup(token);
        
        add(strdup(name),number,email,group);
    }
    fclose(fp);
}


static void add(char *name, char *number, char *email, char *group) {
    if (n >= capacity)
        reallocate();
    
    int i=n-1;
    while(i >= 0 && strcmp(directory[i]->name, name) > 0) {
        directory[i+1] = directory[i];
        i--;
    }
    
    directory[i+1]->name = strdup(name);
    directory[i+1]->number = strdup(number);
    directory[i+1]->email = strdup(email);
    directory[i+1]->group = strdup(group);
    
    n++;
}


static int composeName(char str[], int limit) {
    char *ptr;
    int length = 0;
    
    ptr = strtok(NULL, " ");
    if (ptr == NULL) {
        return 0;
    }
    
    strcpy(str, ptr);
    length += strlen(ptr);
    
    while((ptr = strtok(NULL, " ")) != NULL) {
        if (length + strlen(ptr) + 1 < limit) {
            str[length++] = ' ';
            str[length] = '\0';
            strcat(str, ptr);
            length += strlen(ptr);
        }
    }
    return length;
}

static void releasePerson(Person *p) {
    free(p->name);
    if (p->number != NULL) free(p->number);
    if (p->email != NULL) free(p->email);
    if (p->group != NULL) free(p->group);
    free(p);
}

static void save(char *fileName) {
    int i;
    FILE *fp = fopen(fileName, "w");
    if (fp == NULL) {
        printf("open failed\n");
        return;
    }
    
    for(i=0;i<n;i++) {
        fprintf(fp, "%s,", directory[i]->name);
        fprintf(fp, "%s,", directory[i]->number);
        fprintf(fp, "%s,", directory[i]->email);
        fprintf(fp, "%s,\n", directory[i]->group);
    }
    
    fclose(fp);
}


static int search(char *name) {
    int i;
    for(i=0; i<n; i++) {
        if (strcmp(name, directory[i]->name) == 0) {
            return i;
        }
    }
    return -1;
}


static void printPerson(Person *p) {
    printf("%s:\n", p->name);
    printf("    : Phone: %s\n", p->number);
    printf("    : Phone: %s\n", p->email);
    printf("    : Phone: %s\n", p->group);
}
                            

static void delete(char *name) {
    int i =search(name);
    if ( i == -1) {
        printf("No person named '%s' exists.\n", name);
        return;
    }
    
    Person *p = directory[i];
    for(int j=i; j<n-1;j++) {
        directory[j] = directory[j+1];
    }
    n--;
    releasePerson(p);
    printf("'%s' was deleted successfully.\n",name);
}


static void status(void) {
    int i;
    
    for(i=0;i<n;i++) {
        printPerson(directory[i]);
    }
    printf("Total: %d persons.\n",n);
}

static Person *getPerson(char *name) {
    int result = search(name);
    
    if (result != -1) {
        return directory[result];
    } else {
        //return NULL;
    }
}


//int testContact(struct EventMessage msg, QueueInfo queueInfo, char *cmdName) {
int testContact(struct EventMessage req) {
    int range=0;
    char buf[16]= {0};
    struct EventMessage rsp;
    
    if (range >= 1000 && range < 10000)
        range = 4;
    else if (range >= 100 && range < 1000)
        range = 3;
    else if (range >= 10 && range < 100)
        range = 2;
    else 
        range = 1;
    
    sprintf(buf, "(0-%d),(%d)",range,MAX_BUF_SIZE);
      
    rsp.event = EVT_CONTACT_VALID_RANGE_RSP;
    rsp.len = strlen(buf);
    memcpy(rsp.parameters.paramString,buf, rsp.len );
    memcpy(rsp.cmdName, req.cmdName, strlen(req.cmdName));
    rsp.state = CMD_DONE;
    rsp.fptr = req.fptr;
    rsp.queueInfo.from = req.queueInfo.to;
    rsp.queueInfo.to = req.queueInfo.returnto;
    rsp.queueInfo.returnto = NULL;
     
    xQueueSend(rsp.queueInfo.to, &rsp, portMAX_DELAY);
    return 0;
}


int readContact(struct EventMessage req) {
    struct EventMessage rsp;
    int i;
    char result[128]={0};
    
    if (n) {
        for(i=0;i<n;i++) {
            memset(result,0,128);

            sprintf(result, "%s:%d,%s,%s,%s,%s",req.cmdName,i,directory[i]->name, 
                    directory[i]->name,directory[i]->email,directory[i]->group);

            rsp.event = EVT_CONTACT_READ_PHONEBOOK_RSP;
            rsp.len = strlen(result);
            memcpy(rsp.parameters.paramString,result, rsp.len );
            memcpy(rsp.cmdName, req.cmdName, strlen(req.cmdName));
            if ( i < n-1)
                rsp.state = CMD_INPROGRESS;
            else
                rsp.state = CMD_DONE;
            rsp.fptr = req.fptr;
            rsp.queueInfo.from = req.queueInfo.to;
            rsp.queueInfo.to = req.queueInfo.returnto;
            rsp.queueInfo.returnto = NULL;

            xQueueSend(rsp.queueInfo.to, &rsp, portMAX_DELAY);
        }
    } else {
            memset(result,0,128);
            sprintf(result, "%s:%s",req.cmdName,AT_STRING_ERROR);

            rsp.event = EVT_CONTACT_READ_PHONEBOOK_RSP;
            rsp.len = strlen(result);
            memcpy(rsp.parameters.paramString,result, rsp.len );
            memcpy(rsp.cmdName, req.cmdName, strlen(req.cmdName));
            rsp.state = CMD_DONE;
            rsp.fptr = req.fptr;
            rsp.queueInfo.from = req.queueInfo.to;
            rsp.queueInfo.to = req.queueInfo.returnto;
            rsp.queueInfo.returnto = NULL;

            xQueueSend(rsp.queueInfo.to, &rsp, portMAX_DELAY);
    }
    return 0;
}



//static void writeContact(char *buffer) {
int writeContact(struct EventMessage req) {    
    char *name=NULL, *number=NULL, *email=NULL, *group=NULL;
    char *token=NULL;
    int *temp = NULL;
    struct EventMessage rsp;
    
    char *buffer = strdup(req.parameters.paramString);
    
    temp = strtok(buffer, ",");

    name = strtok(buffer, ",");

    token = strtok(buffer, ",");
    if (strcmp(token, " ") != 0)
        number = strdup(token);

    token = strtok(buffer, ",");
    if (strcmp(token, " ") != 0)
        email = strdup(token);

    token = strtok(buffer, ",");
    if (strcmp(token, " ") != 0)
        group = strdup(token);

    add(strdup(name), number, email, group);

    rsp.event = EVT_CONTACT_WRITE_PHONEBOOK_RSP;
    rsp.len = 0;
    memcpy(rsp.cmdName, req.cmdName, strlen(req.cmdName));
    rsp.fptr = req.fptr;
    rsp.queueInfo.from = req.queueInfo.to;
    rsp.queueInfo.to = req.queueInfo.returnto;
    rsp.queueInfo.returnto = NULL;

    xQueueSend(rsp.queueInfo.to, &rsp, portMAX_DELAY);
        
    return 0;
}

/****************************************************************************/
/** **/
/** EOF **/
/** **/
/****************************************************************************/
