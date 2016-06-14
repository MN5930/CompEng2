// Multi taking Operating system with messaging for embbeded programming on the microprocessor
// Writen by M.Nazari

#include "kernel.h"
#include "kernel_hwdep.h"
#include "lists.h"
#include <string.h>
#include <limits.h>

uint tick_counter_sys;
int MODE;
TCB * Running = NULL;

static void ONE_MSG_REMOVE(mailbox *mBox, msg *tmp);
static void MSG_REMOVE(mailbox *mBox);
static void MSG_ADD(mailbox *mBox, msg *tmp);
void timer0_isr (void);
static void idle();

////////////////////////////// KERNEL INTIATION
int init_kernel(){
	
	if (List_initaion()==FAIL)
		return FAIL;
	MODE=INIT;
	tick_counter_sys=0;
	create_task(&idle,0xFFFFFFFF); 
	return OK;
	
}


////////////////////////////////CREATE TASK
exception create_task( void (* body)(), uint deadline ){


	volatile int first_time = TRUE;
	int isr_status;
	TCB * TCB_TASK;
	listobj * temp_obj;
	isr_status = set_isr(ISR_OFF);
	TCB_TASK =(TCB *)malloc(sizeof(TCB));
	if(TCB_TASK == NULL){
                set_isr(isr_status);
		return FAIL;
        }
	temp_obj = (listobj *)malloc(sizeof(listobj));
	if(temp_obj == NULL){
           isr_status = set_isr(ISR_OFF);
           free(TCB_TASK);
           set_isr(isr_status);
           return FAIL;
               
        }
	set_isr(isr_status);            ////

	TCB_TASK->DeadLine = deadline;
	TCB_TASK->PC = body ;
	TCB_TASK->SPSR=0;
	TCB_TASK->SP = &(TCB_TASK->StackSeg [STACK_SIZE-1]); 

	temp_obj->pTask = TCB_TASK;
	temp_obj->pMessage = NULL;
	temp_obj->pNext = NULL;
	temp_obj->pPrevious = NULL ;

       if(MODE == INIT)
	{
		insert_object_ready(temp_obj);
                Running= list_ready->pHead->pNext->pTask;
		return OK;
	}else{
		isr_off();
		SaveContext();
		if(first_time)
		{
			first_time = FALSE;
			insert_object_ready(temp_obj);
			Running= list_ready->pHead->pNext->pTask;
			LoadContext();
		}
	
	
}
return OK;
}




//////////////// RUN ///
void run(){
	timer0_start();
	MODE=RUNNING;
	Running = list_ready->pHead->pNext->pTask;
	LoadContext();
}

 /////////////// TERMINATE 
void terminate(){

	listobj * temp;
	int status;
	temp = extract_object_ready();
	status = set_isr(ISR_OFF);
	free(temp->pTask);
	free(temp);            
	set_isr(status);
	Running = list_ready->pHead->pNext->pTask;   
	LoadContext();
	

}




/////////////////////////////////////COMMUNICATION/////////
//// MAILBOX
mailbox*	create_mailbox( uint nMessages, uint nDataSize ){

	mailbox * inbox;
	int isr_status;
	isr_status = set_isr(ISR_OFF);
	inbox = (mailbox *)malloc(sizeof(mailbox));
	set_isr(isr_status);
	if(inbox == NULL)
	{
		return NULL;
	}
	isr_status = set_isr(ISR_OFF);
	inbox->pHead = (msg *)malloc(sizeof(msg));
	set_isr(isr_status);
	if(inbox->pHead == NULL)
	{
                isr_status = set_isr(ISR_OFF);
		free(inbox);
                set_isr(isr_status);
		return NULL;
	}
	isr_status = set_isr(ISR_OFF);
	inbox->pTail = (msg *)malloc(sizeof(msg));
	set_isr(isr_status);
	if(inbox->pTail == NULL)
	{
          
                isr_status = set_isr(ISR_OFF);
		free(inbox);
		free(inbox->pHead);
                set_isr(isr_status);
		return NULL;
	}
	
	
	inbox->pHead->pNext = inbox->pTail;
	inbox->pTail->pPrevious = inbox->pHead;

	inbox->nDataSize = nDataSize;
	inbox->nMaxMessages = nMessages;
	inbox->nMessages = 0;
	inbox->nBlockedMsg = 0;

	
	return inbox;

}






///////////// Number of massages 

int no_messages( mailbox* mBox ){

		return mBox->nMessages + mBox->nBlockedMsg;
}


//////////// remove_mailbox////////////
exception remove_mailbox(mailbox* mBox){
	int isr_status;
        isr_status = set_isr(ISR_OFF);
	if(!(no_messages(mBox))) 
	{
		
		free(mBox->pHead);
		free(mBox->pTail);
		free(mBox);
		set_isr(isr_status);
		return OK;
	}else
	{ 
                set_isr(isr_status);
		return NOT_EMPTY;
	}

}


///////SEND WAIT and BLOCK
exception send_wait( mailbox* mBox, void* pData ){

		
	listobj * temp;
	volatile int first_time = TRUE;
	SaveContext();
        isr_off();
	if (first_time){
		first_time = FALSE;
		if(no_messages(mBox) != 0 && mBox->pHead->pNext->Status == RECEIVER) {

			mBox->pHead->pNext->pBlock->pMessage = NULL;                     
			memcpy(mBox->pHead->pNext->pData, pData, mBox->nDataSize);
			temp = mBox->pHead->pNext->pBlock;
			extract_object_wait(temp);
			insert_object_ready(temp);
            MSG_REMOVE( mBox);
            Running = list_ready->pHead->pNext->pTask;   
			
		}
		else{

			msg *m_temp;
			m_temp = (msg *)malloc(sizeof(msg));
			if (m_temp == NULL)
				return FAIL;
			m_temp->pData = pData;
			m_temp->Status = SENDER;
			m_temp->pBlock = list_ready->pHead->pNext;
			list_ready->pHead->pNext->pMessage = m_temp;
			MSG_ADD(mBox,m_temp);
			mBox->nBlockedMsg++;  // BLOCK MSG
			insert_object_wait (extract_object_ready());
			Running = list_ready->pHead->pNext->pTask;


		}
		LoadContext();
	}else{
	
		if(tick_counter_sys >= Running->DeadLine){
			msg * tmp = list_ready->pHead->pNext->pMessage;                  
            isr_off();
            ONE_MSG_REMOVE(mBox,tmp);			   
			list_ready->pHead->pNext->pMessage = NULL;
			isr_on();
			return DEADLINE_REACHED;
		
		
		}else{

			return OK;
		}
	}
	return OK;
}


//////////////// RECIVE WAIT

exception receive_wait( mailbox* mBox, void* pData ){
        int isr_status;
	isr_off();	
	msg * msg_tmp;
	volatile int first_time = TRUE;
	SaveContext();
	if (first_time){
		first_time = FALSE;
		if(no_messages(mBox) !=0 && mBox->pHead->pNext->Status == SENDER){
			memcpy(pData, mBox->pHead->pNext->pData, mBox->nDataSize);
			
			
			if(mBox->nBlockedMsg != 0) {
                mBox->pHead->pNext->pBlock->pMessage= NULL;
				extract_object_wait(mBox->pHead->pNext->pBlock);
				insert_object_ready(mBox->pHead->pNext->pBlock);
				Running=list_ready->pHead->pNext->pTask;
				MSG_REMOVE(mBox);                         

			}else{
			
				MSG_REMOVE(mBox);                        
				
			
			}
		}else{
                        isr_status = set_isr(ISR_OFF);
			msg_tmp = (msg *)malloc(sizeof(msg));
                        set_isr(isr_status);
			if(msg_tmp==NULL)
				return FAIL;
			msg_tmp->Status=RECEIVER;
			msg_tmp->pBlock= list_ready->pHead->pNext;
			msg_tmp->pData = pData;
			MSG_ADD(mBox,msg_tmp);
			mBox->nBlockedMsg++;
			list_ready->pHead->pNext->pMessage = msg_tmp;
			insert_object_wait(	extract_object_ready());
			Running = list_ready->pHead->pNext->pTask;

           
		}
		LoadContext();

	}else{
		if(tick_counter_sys >= Running->DeadLine){
			isr_status = set_isr(ISR_OFF);
			msg_tmp = mBox->pHead->pNext->pBlock->pMessage;
			MSG_REMOVE(mBox);                         
			list_ready->pHead->pNext->pMessage=NULL;
			set_isr(isr_status);
			return DEADLINE_REACHED;
		}else{

			return OK;
		}

	}
        return OK;
}



/////////////////////////////// SEND NO WAIT
exception send_no_wait( mailbox* mBox, void* pData ){
        
        int isr_status;
	volatile int first_time = TRUE;
	isr_off();
	SaveContext();
	if (first_time){
	
		first_time = FALSE;
		if(no_messages(mBox)!=0 && mBox->pHead->pNext->Status== RECEIVER){

			memcpy(mBox->pHead->pNext->pData,pData,mBox->nDataSize);
			                         
			mBox->pHead->pNext->pBlock->pMessage = NULL;
			extract_object_wait(mBox->pHead->pNext->pBlock);
			insert_object_ready(mBox->pHead->pNext->pBlock); 
			MSG_REMOVE(mBox);                              
			Running = list_ready->pHead->pNext->pTask;
			//LoadContext();

			}else {

				msg * msg_tmp;
                                if(mBox->nMaxMessages <= mBox->nMessages) {
				mBox->pHead->pNext->pBlock->pMessage = NULL;
				MSG_REMOVE(mBox);
			}
                                isr_status = set_isr(ISR_OFF);
				msg_tmp = (msg *)malloc(sizeof(msg));
                                set_isr(isr_status);
			if(msg_tmp==NULL){
				return FAIL;
			}
                        isr_status = set_isr(ISR_OFF);
			msg_tmp->pData =(char *)malloc(mBox->nDataSize);
                        set_isr(isr_status);
			if(msg_tmp->pData==NULL){
                                isr_status = set_isr(ISR_OFF);
				free(msg_tmp);
                                set_isr(isr_status);
				return FAIL;
                               
			}
			
			

			memcpy(msg_tmp->pData, pData, mBox->nDataSize);
			msg_tmp->pBlock = NULL;                        //not block msg
			msg_tmp->Status=SENDER;

			MSG_ADD(mBox,msg_tmp);
			(mBox->nMessages)++;

		}
                LoadContext();
	
	}
	return OK;
}



///////////////////////////////////////// RECIEVE NO WAIT
int  receive_no_wait( mailbox* mBox, void* pData ){
	listobj * tmp; 
	volatile int first_time= TRUE;
	isr_off();
	SaveContext();
	if(first_time){
	 
		first_time = FALSE;
		if(no_messages(mBox) != 0 && mBox->pHead->pNext->Status == SENDER ){
			memcpy(pData,mBox->pHead->pNext->pData,mBox->nDataSize);

			if(mBox->nBlockedMsg != 0) {
				tmp=mBox->pHead->pNext->pBlock;
				mBox->pHead->pNext->pBlock->pMessage= NULL; 
				MSG_REMOVE(mBox);
				extract_object_wait(tmp);
				insert_object_ready(tmp);
				Running = list_ready->pHead->pNext->pTask;
                               
			
		}else{
		
			MSG_REMOVE(mBox);
			}
		}else{
		return FAIL;
		}
		LoadContext();
	}
	return OK;
	
	}




/////WAIT             //////////////////////////?????

exception	wait( uint nTicks ){

        int status;
	volatile int first_time_wait = TRUE;   
	isr_off();
	SaveContext();
	if(first_time_wait){
		first_time_wait = FALSE;
		list_ready->pHead->pNext->nTCnt = tick_counter_sys + nTicks;
		insert_object_timer(extract_object_ready());
		Running = list_ready->pHead->pNext->pTask;
		LoadContext();

	}
	else{
		if (Running->DeadLine >= tick_counter_sys){            

			status = DEADLINE_REACHED;

		}
		else{
			status= OK;
		}
	}
	return status;
}




/////SET TICKS
void            set_ticks( uint no_of_ticks ){

	tick_counter_sys= no_of_ticks;

}


//// REturn TICKS
uint            ticks( void ){

	return tick_counter_sys;

}


/////DEADLINE
uint		deadline( void ){

	return Running->DeadLine;
}



//////// SET DEADLINE
void            set_deadline( uint nNew ){
	
	volatile int first_time = TRUE;
	isr_off();
	SaveContext();
	if (first_time ){

		first_time = FALSE;
		Running->DeadLine=nNew;
		insert_object_ready(extract_object_ready());
		Running=list_ready->pHead->pNext->pTask;
		LoadContext();

	}


}



//////////TIMER INTIATION //// NOT FOR USERS  
void TimerInt(void){

	tick_counter_sys++;
	listobj * temp;
	listobj * temp2;

	temp = list_time->pHead->pNext; 
	while(temp != list_time->pTail ){
		
		if(temp->nTCnt <= tick_counter_sys){
			temp = temp->pNext;                                    ///**	
			insert_object_ready(extract_object_timer()); 
			Running = list_ready->pHead->pNext->pTask;
			
		}else {
			break;                                         
		}
	}


	    temp = list_wait->pHead->pNext; 
		while(temp != list_wait->pTail ){
			if(temp->pTask->DeadLine <= tick_counter_sys){
				listobj * temp2;
				temp2 = temp;
				temp = temp->pNext;                             ///**
				extract_object_wait(temp2);
				insert_object_ready(temp2);   
				
				Running = list_ready->pHead->pNext->pTask;

			}else {                                  
				break;                             
			}
	}

}


//////////////////////////// IDLE FUNCTION
static void idle()
{
	while(1){}
}



///////////////////////////////////////////////////  Extra FUCNTIONS  /////////////////////////////////////////////////////////////

static void MSG_ADD(mailbox *mBox, msg *tmp)
{
int isr_status;
isr_status = set_isr(ISR_OFF);
tmp->pPrevious = mBox->pTail->pPrevious;
tmp->pNext = mBox->pTail;
mBox->pTail->pPrevious->pNext = tmp;
mBox->pTail->pPrevious = tmp;
set_isr(isr_status);
}


///////// REMOVE A MSG FROM TOP OF A MAILBOX
static void MSG_REMOVE(mailbox *mBox){
  
int isr_status;
msg *tmp = mBox->pHead->pNext;
isr_status = set_isr(ISR_OFF);
if(mBox->nBlockedMsg != 0) {  //BOCK MSG
	mBox->pHead->pNext->pNext->pPrevious = mBox->pHead;
        mBox->pHead->pNext = mBox->pHead->pNext->pNext;
        free(tmp);
        (mBox->nBlockedMsg)--;

} else {
	if(mBox->nMessages != 0) {   // UNBLOCK MSG
		mBox->pHead->pNext->pNext->pPrevious = mBox->pHead;
                mBox->pHead->pNext = mBox->pHead->pNext->pNext;
                free(tmp->pData);
		free(tmp);
                (mBox->nMessages)--;
	}
}
 set_isr(isr_status);
}


static void ONE_MSG_REMOVE(mailbox *mBox, msg *tmp)
{
  
	int isr_status;
        isr_status = set_isr(ISR_OFF);
	if(mBox->nBlockedMsg != 0) {
        tmp->pPrevious->pNext = tmp->pNext;
        tmp->pNext->pPrevious = tmp->pPrevious;
        free(tmp);
        (mBox->nBlockedMsg)--; 
        set_isr(isr_status);
}

}
