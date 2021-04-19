/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "machine.H"

extern Scheduler * SYSTEM_SCHEDULER;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
	this->block_queue = new FIFOQ();
	len_block = 0;
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/


void BlockingDisk::wait_until_ready_block(){
	if( not SimpleDisk::is_ready()){
		Thread* cur = Thread::CurrentThread();
		this->block_queue_addTail(cur);
		SYSTEM_SCHEDULER->yield();
	}
}

void BlockingDisk::block_queue_addTail(Thread* _thread){
	this->block_queue->addTail(_thread);
	len_block++;
}

Thread* BlockingDisk::block_queue_popHead(){
	Thread* res = this->block_queue->popHead();
	len_block--;
	return res;
}

bool BlockingDisk::is_ready(){
	return SimpleDisk::is_ready();
}

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {

  //SimpleDisk::read(_block_no, _buf);
      issue_operation(READ, _block_no);

  wait_until_ready_block();

  /* read data from port */
  int i;
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = Machine::inportw(0x1F0);
    _buf[i*2]   = (unsigned char)tmpw;
    _buf[i*2+1] = (unsigned char)(tmpw >> 8);
  }
  Console::puts("Successfully read. \n");

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
      issue_operation(WRITE, _block_no);

  wait_until_ready_block();

  /* write data to port */
  int i; 
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
    Machine::outportw(0x1F0, tmpw);
  }
  //SimpleDisk::write(_block_no, _buf);
  Console::puts("Successfully write. \n");
}
