# CompEng2 Course
 Real‐time kernel Operating System written in C
 
 Messaging method between tasks to stop or run a task in a microprocessor. 
 
 linked lists (Timer List, Waiting List and Ready List)
 
 Timer List:
  • The insertion of an element into the list, pass an integer with the function call. Insertion in the list is done so that     the element with the lowest value of nTCnt appears first.
  • Extraction is always done from the front, i.e. at the "head‐element".
Waiting List:
  • A list, where the list is sorted so that the element with the lowest value of TCB‐> Deadline is
    first.
  • Extraction made by the use of a pointer to the list element, struct l_obj * pBlock
Ready List
  • The element with the lowest value of TCB‐> Deadline is first placed first in the list. • Extraction is always done from    the front, i.e. at the "head‐element".
  
  Task administration
exception init_kernel()
This function initializes the kernel and its data structures and leaves the kernel in start-up mode. The init_kernel call must be made before any other call is made to the kernel.
Argument
none
Return parameter
Int: Description of the functionís status, i.e. FAIL/OK.

exception create_task(void(*task_body)(), uint deadline)
This function creates a task. If the call is made in start- up mode, i.e. the kernel is not running, only the necessary data structures will be created. However, if the call is made in running mode, it will lead to a rescheduling and possibly a context switch.
Argument
*task_body: A pointer to the C function holding the code of the task.
deadline: The kernel will try to schedule the task so it will meet this deadline.
Return parameter
Description of the functionís status, i.e. FAIL/OK.

void run()
This function starts the kernel and thus the system of created tasks. Since the call will start the kernel it will leave control to the task with tightest deadline. Therefore, it must be placed last in the application initialization code. After this call the system will be in running mode.
Argument
none
Return parameter
none

void terminate()
This call will terminate the running task. All data structures for the task will be removed. Thereafter, another task will be scheduled for execution.
Argument
none
Return parameter
none

Inter-Process Communication
Mailbox* create_mailbox(int nof_msg, int size_of_msg)
This call will create a Mailbox. The Mailbox is a FIFO communication structure used for asynchronous and synchronous communication between tasks.
Argument
nof_msg: Maximum number of Messages the Mailbox can hold.
Size_of msg: The size of one Message in the Mailbox.
Return parameter
Mailbox*: a pointer to the created mailbox or NULL.


exception remove_mailbox(Mailbox* mBox)
This call will remove the Mailbox if it is empty and return OK. Otherwise no action is taken and the call will return NOT_EMPTY.
Argument
Mailbox*: A pointer to the Mailbox to be removed.
Return parameter
OK: The mailbox was removed
NOT_EMPTY: The mailbox was not removed because it was not empty.

exception send_wait( Mailbox* mBox, void* Data)
This call will send a Message to the specified Mailbox. If there is a receiving task waiting for a Message on the specified Mailbox, send_wait will deliver it and the receiving task will be moved to the Readylist. Otherwise, if there is not a receiving task waiting for a Message on the specified Mailbox, the sending task will be blocked. In both cases (blocked or not blocked) a new task schedule is done and possibly a context switch. During the blocking period of the task its deadline might be reached. At that point in time the blocked task will be resumed with the exception: DEADLINE_REACHED. Note: send_wait and send_no_wait Messages shall not be mixed in the same Mailbox.
Argument
*mBox a pointer to the specified Mailbox.
*Data: a pointer to a memory area where the data of the communicated Message is residing.
Return parameter
exception: The exception return parameter can have two possible values:
• OK: Normal behavior, no exception occurred.
• DEADLINE_REACHED: This return parameter is given if the sending tasksí deadline is reached while it is blocked by the send_wait
call.

exception receive_wait( Mailbox* mBox, void* Data)
This call will attempt to receive a Message from the specified Mailbox. If there is a send_wait or a send_no_wait Message waiting for a receive_wait or a receive_no_wait Message on the specified Mailbox, receive_wait will collect it. If the Message was of send_wait type the sending task will be moved to the Readylist. Otherwise, if there is not a send Message (of either type) waiting on the specified Mailbox, the receiving task will be blocked. In both cases (blocked or not blocked) a new task schedule is done and possibly a context switch. During the blocking period of the task its deadline might be reached. At that point in time the blocked task will be resumed with the exception: DEADLINE_REACHED.
Argument
*mBox: a pointer to the specified Mailbox.
*Data: a pointer to a memory area where the data of the communicated Message is to be stored.
Return parameter
exception: The exception return parameter can have two possible values:
• OK: Normal function, no exception occurred.
• DEADLINE_REACHED: This return parameter is given if the receiving tasksí deadline is reached while it is blocked by the receive_wait
call.


exception send_no_wait( Mailbox* mBox, void* Data)
This call will send a Message to the specified Mailbox. The sending task will continue execution after the call. When the Mailbox is full, the oldest Message will be overwritten. The send_no_wait call will imply a new scheduling and possibly a context switch. Note: send_wait and send_no_wait Messages shall not be mixed in the same Mailbox.
Argument
*mBox: a pointer to the specified Mailbox.
*Data: a pointer to a memory area where the data of the communicated Message is residing.
Return parameter
Description of the functionís status, i.e. FAIL/OK.

exception receive_no_wait( Mailbox* mBox, void* Data)
This call will attempt to receive a Message from the specified Mailbox. The calling task will continue execution after the call. When there is no send Message available, or if the Mailbox is empty, the function will return FAIL. Otherwise, OK is returned. The call might imply a new scheduling and possibly a context switch.
Argument
*mBox: a pointer to the specified Mailbox. *Data: a pointer to the Message.
Return parameter
Integer indicating whether or not a Message was received (OK/FAIL).

Timing functions

exception wait(uint nTicks)
This call will block the calling task until the given number of ticks has expired.
Argument
nTicks: the duration given in number of ticks
Return parameter
exception: The exception return parameter can have two possible values:
• OK: Normal function, no exception occurred.
• DEADLINE_REACHED: This return parameter is given if the receiving tasksí deadline is reached while it is blocked by the receive_wait
call.


void set_ticks( uint nTicks)
This call will set the tick counter to the given value.
Argument
nTicks: the new value of the tick counter
Return parameter
none


uint ticks(void)
This call will return the current value of the tick counter
Argument
none
Return parameter
A 32 bit value of the tick counter

uint deadline(void)
This call will return the deadline of the specified task
Argument
none
Return parameter
the deadline of the given task

void set_deadline(uint deadline)
This call will set the deadline for the calling task. The task will be rescheduled and a context switch might occur.
Argument
deadline: the new deadline given in number of ticks.
Return parameter
none


void TimerInt(void)
This function is not available for the user to call.
It is called by an ISR (Interrupt Service Routine) invoked every tick. Note, context is automatically saved prior to call and automatically loaded on function exit.


void SaveContext (void) 
This function is hardware dependent. All relevant registers are saved to the TCB of the currently running task.


void LoadContext (void) 
This function is hardware dependent. All relevant registers are restored from the TCB of the currently running task to the CPU registers.


void timer0_isr (void) 
This function is not available for the user to call.
It is an ISR (Interrupt Service Routine) invoked every tick. Note. It calls the C-Function TimerInt().

