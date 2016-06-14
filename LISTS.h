#ifndef lists_H
#define lists_H

#include "kernel.h"

extern list * list_time;
extern list * list_wait ;
extern list * list_ready;

//// LIST INITIATION
exception List_initaion(void);

///// TIMER LIST
listobj * extract_object_timer(void);  //Head ELEMENT EXTRACT 
void insert_object_timer(listobj * );  // INSERT BASED ON LOWER nTCnt

///// WAIT LIST
void extract_object_wait(listobj * );    // EXTRACT WITH DEFINED POINTER
void insert_object_wait(listobj * );     // INSERT SORETED BASED ON DEADLINE


////READY LIST
void insert_object_ready(listobj * );  // LOWEST DEADLINE FIRST
listobj * extract_object_ready();     // HEAD ELEMENT EXTRACT

//// REMOVE A LIST 
void object_remove_list(listobj * ); //JUST REMOVE AN OBJECT


#endif