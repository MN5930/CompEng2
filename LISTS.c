
#include "kernel.h"
#include <stdlib.h>
#include "lists.h"


list * list_time = NULL;
list * list_wait = NULL;
list * list_ready = NULL;


//TIME WAIT READY lists intiation return FAIL OR SUCCESS
exception List_initaion(){

	// time list initiation
	list_time = (list *)malloc ( sizeof (list));
		if (list_time == NULL)
			return FAIL;

	list_time->pHead = (listobj *)malloc(sizeof(listobj));
	if(list_time->pHead == NULL)
	{
		free(list_time);
		return FAIL;
	}
	list_time->pTail = (listobj *)malloc(sizeof(listobj));
	if(list_time->pTail == NULL)
	{
		free(list_time);
		free(list_time->pHead);
		return FAIL;
	}

	list_time->pHead->pPrevious = NULL;
	list_time->pHead->pNext = list_time->pTail;
	list_time->pTail->pPrevious = list_time->pHead;
	list_time->pTail->pNext = NULL;


	// wait list initiation
	list_wait = (list *)malloc ( sizeof (list));
		if (list_wait == NULL)
			return FAIL;
	list_wait->pHead = (listobj *)malloc (sizeof (listobj));
	if(list_wait->pHead == NULL)
	{
		free(list_wait);
		return FAIL;
	}
	list_wait->pTail = (listobj *)malloc(sizeof(listobj));
	if(list_wait->pTail == NULL)
	{
		free(list_wait);
		free(list_wait->pHead);
		return FAIL;
	}

	list_wait->pHead->pPrevious = NULL;
	list_wait->pHead->pNext = list_wait->pTail;
	list_wait->pTail->pPrevious = list_wait->pHead;
	list_wait->pTail->pNext = NULL;
	
	
	// ready list initiaion
	list_ready = (list *)malloc ( sizeof (list));
		if (list_ready == NULL)
			return FAIL;
	list_ready->pHead = (listobj *)malloc (sizeof (listobj));
	if(list_ready->pHead == NULL)
	{
		free(list_ready);
		return FAIL;
	}
	list_ready->pTail = (listobj *)malloc(sizeof(listobj));
	if(list_ready->pTail == NULL)
	{
		free(list_ready->pHead);
		free(list_ready);
		
		return FAIL;
	}

	list_ready->pHead->pPrevious = NULL;
	list_ready->pHead->pNext = list_ready->pTail;
	list_ready->pTail->pPrevious = list_ready->pHead;
	list_ready->pTail->pNext = NULL;
	return SUCCESS;



}

//Insert object into the timer list
void insert_object_timer(listobj * time){
 listobj * obj_tmp ;
 
 obj_tmp = list_time ->pHead ->pNext;
 if(list_time -> pTail == obj_tmp) {
	 list_time ->pHead ->pNext = time;
	 list_time ->pTail ->pPrevious  = time;
	 time -> pPrevious = list_time ->pHead;
	 time -> pNext = list_time ->pTail ;
	 return;
}

 while(time->nTCnt > obj_tmp ->nTCnt){
	 obj_tmp = obj_tmp ->pNext;  
	 if(obj_tmp == list_time->pTail)
		 break;
 }
 time->pNext = obj_tmp;
 time ->pPrevious = obj_tmp->pPrevious; 
 obj_tmp ->pPrevious->pNext = time;
 obj_tmp -> pPrevious = time;
 return;
 }





// extract an object from the front of the time list return NULL if Error
listobj * extract_object_timer(){
	listobj * objct_tmp;
	objct_tmp = list_time->pHead->pNext;
	if(objct_tmp == list_time->pTail)
		return NULL ;
	object_remove_list(objct_tmp);
	return objct_tmp;
}

//Insert an object into the wait list based on TCP deadline
void insert_object_wait(listobj * wait_temp){
  listobj * objc_temp;
 
  objc_temp = list_wait ->pHead ->pNext;
  if(list_wait -> pTail == objc_temp) {
	 list_wait ->pHead ->pNext = wait_temp;
	 list_wait ->pTail ->pPrevious  = wait_temp;
	 wait_temp -> pPrevious = list_wait ->pHead;
	 wait_temp -> pNext = list_wait ->pTail ;
	 return;
}
  while(wait_temp->pTask->DeadLine > objc_temp ->pTask->DeadLine){
	 objc_temp = objc_temp ->pNext;  
	 if(objc_temp == list_wait->pTail)
		 break;
 
 }
  wait_temp->pNext = objc_temp;
  wait_temp ->pPrevious = objc_temp->pPrevious; 
  objc_temp ->pPrevious->pNext = wait_temp;
  objc_temp -> pPrevious = wait_temp;
  return;
}


// Extract an object from the waiting list
void extract_object_wait(listobj * object_temp)   ////???
{
	if (list_wait->pHead->pNext == list_wait->pTail)
		return;
	object_remove_list(object_temp);
	return;
}


// insert object in the ready list 
void insert_object_ready(listobj * temp){

	listobj * ob_temp ;

 ob_temp = list_ready ->pHead ->pNext;
 if(list_ready -> pTail == ob_temp) {
	 list_ready ->pHead ->pNext = temp;
	 list_ready ->pTail ->pPrevious  = temp;
	 temp -> pPrevious = list_ready ->pHead;
	 temp -> pNext = list_ready ->pTail ;
	 return ;
}

 while(temp->pTask->DeadLine > ob_temp ->pTask->DeadLine){
	 ob_temp = ob_temp ->pNext;  
	 if(ob_temp == list_time->pTail)
		 break;
 }
 temp->pNext = ob_temp;
 temp ->pPrevious = ob_temp->pPrevious; 
 ob_temp ->pPrevious->pNext = temp;
 ob_temp -> pPrevious = temp;
 return ;
}


//extract object from front of ready list
listobj * extract_object_ready()
{
	listobj * _temp = NULL;
	if (list_ready->pHead->pNext == list_ready->pTail)
		return _temp ;
	_temp=list_ready->pHead->pNext;
	object_remove_list(_temp);
	return _temp;
}


// Remove an object from a list
///REMOVE LIST
void object_remove_list(listobj * temp){

	temp->pNext->pPrevious = temp ->pPrevious;
	temp->pPrevious->pNext = temp->pNext;
	temp->pPrevious = NULL;
	temp->pNext = NULL;
}


