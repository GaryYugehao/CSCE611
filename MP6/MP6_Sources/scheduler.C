/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

FIFOQ::FIFOQ(FIFOQ& n){
    thread = n.thread;
    next = n.next;
  }

FIFOQ::FIFOQ(Thread* t){
    thread = t;
    next = NULL;
  }

FIFOQ::FIFOQ(){
    thread = NULL;
    next = NULL;
  }
void FIFOQ::setNext(FIFOQ* input_thread){
    next = input_thread;
  }
Thread* FIFOQ::getThread(){
	if(thread == NULL){
		return NULL;
	}
	return thread;
	}

FIFOQ* FIFOQ::getNext(){
	return next;	
}
void FIFOQ::addTail(Thread* input_thread){
	//No thread in current ready queue
	if(thread == NULL && next == NULL){
		thread = input_thread;
	//We already arrived the ready quque tail
	}else if(thread != NULL && next == NULL){
		next = new FIFOQ(input_thread);
	//We haven't arrived the ready queue tail	
	}else{
		next->addTail(input_thread);
	}
    
 }
Thread* FIFOQ::popHead(){
	Thread* res = NULL;
	//There is only one thread in the ready queue
	if(thread != NULL && next == NULL){
		res = thread;
		thread = NULL;
	//more than one thread in the ready queue
	}else if(thread !=NULL && next != NULL){
		//store the head thread in the ready
		res = thread;
		thread = next->thread;
		//set the head thread as the second thread
		FIFOQ* nextFIFOQ = next;
		//set the head thread's next pointer to the third one
		next = nextFIFOQ->next;
		//remove the second thread
		delete nextFIFOQ;
	}
	return res;

}

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {

  FIFOQ ready_queue;
  unsigned int len_queue = 0;
  Console::puts("Constructed Scheduler.\n");
  this->disks = NULL;
}


//
void Scheduler::yield() {

/*
  
  	if (len_queue > 0){
  		Thread* popedThread = ready_queue.popHead();
  		Thread::dispatch_to(popedThread);
		len_queue--;
  	}


  	Console::puts("Successfully yielded. \n");
*/

	if(disks == NULL || not disks->is_ready() || disks->len_block == 0){
		if (len_queue > 0){
	  		Thread* popedThread = ready_queue.popHead();
	  		Thread::dispatch_to(popedThread);
			len_queue--;
  		}else{
			Console::puts("Ready queue is empty too. \n");
		}
	}else{
		Thread* popedThread = disks->block_queue_popHead();
		Thread::dispatch_to(popedThread);
	}
}


//
void Scheduler::resume(Thread * _thread) {



  
	ready_queue.addTail(_thread);
	len_queue++;

	Console::puts("Successfully resumed. \n");

}

//
void Scheduler::add(Thread * _thread) {
  
  	ready_queue.addTail(_thread);
	len_queue++;

	Console::puts("Successfully added. \n");

}


//
void Scheduler::terminate(Thread * _thread) {
	
	for(int i=0; i < len_queue;i++){
		Thread* cur = ready_queue.popHead();
		if(_thread->ThreadId() == cur->ThreadId()){
			len_queue--;
		}else{
			ready_queue.addTail(cur);	
		}	
	}

	Console::puts("Successfully terminated. \n");

}

void Scheduler::register_disk(BlockingDisk* disk){
	disks = disk;
}

