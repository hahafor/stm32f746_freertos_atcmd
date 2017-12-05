/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   contact.h
 * Author: seongjinoh
 *
 * Created on July 30, 2017, 9:00 AM
 */

#ifndef CONTACT_H
#define CONTACT_H

#ifdef __cplusplus
extern "C" {
#endif

#define INIT_CAPACITY       100
#define MAX_BUF_SIZE        32
    
    #define  CONTACT_FILE_NAME      "directory.txt"
    
    typedef struct {
        char        *name;
        char        *number;
        char        *email;
        char        *group;
    } Person;


#ifdef __cplusplus
}
#endif

#endif /* CONTACT_H */

