/*
 File: vm_pool.C
 
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

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "page_table.H"
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    //assert(false);
	base_address = _base_address;
	size = _size;
	frame_pool = _frame_pool;
	page_table = _page_table;
	
	last_mem_region = 0;
	//take free frame to store the region list
	mem_region_list = (mem_region *)(frame_pool->get_frames(1)*PageTable::PAGE_SIZE);
	page_table->register_pool(this);
	
	Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
	//assert(false);
	unsigned long start_addr = 0;

	//Bubble sort the memory regions sequence based on their base address
	for (int i=0; i<last_mem_region; i++){
		for (int j=i+1; j<last_mem_region; j++){
			if (mem_region_list[j].base_addr < mem_region_list[i].base_addr){
				mem_region k;
				k = mem_region_list[i];
				mem_region_list[i] = mem_region_list[j];
				mem_region_list[j] = k;
			}
		}
	}


	// try to see if we could allocate the region
	if (last_mem_region < (PageTable::PAGE_SIZE / sizeof(mem_region))){
		if (last_mem_region == 0){
			start_addr = base_address;
		}else{
			//To see if we can allocate the first gap space for space allocation
			unsigned long gap_region = 0;
			gap_region = mem_region_list[0].base_addr - base_address;
			if (_size <= gap_region){
				//If we could
				start_addr = base_address;
			}else{
				//If we couldn't
				bool flag = false;
				//If more than 2 regions already been allocated
				if(last_mem_region > 1){
					for (int i=0; i<last_mem_region-1; i++){
						//updated the new gap region size
						gap_region = mem_region_list[i+1].base_addr - (mem_region_list[i].base_addr+mem_region_list[i].region_size);
						//If we could put it in & keep allocate untill the end
						if (_size <= gap_region){
							start_addr = mem_region_list[i].base_addr + mem_region_list[i].region_size;
							flag = true;
							break;
						}         
					}
				}
				//If we couldn't fina a position, put it at the end
				if(flag == false){
					start_addr = mem_region_list[last_mem_region-1].base_addr + mem_region_list[last_mem_region-1].region_size;
				}
			}
		}
		//varify if allocated start address would lead to a space overflow
		if ((start_addr + _size) <= base_address + size){
			mem_region_list[last_mem_region].base_addr = start_addr;
			mem_region_list[last_mem_region].region_size = _size;

			last_mem_region++;
			Console::puts("Successfully allocate space for memory.\n");
			return start_addr;

		}else if ((start_addr + _size) > base_address + size){
			Console::puts("Unsuccessfully allocate space for memory because of virtual pool size overflow.\n");
			return 0;
		}
		
	}else{
		Console::puts("Unsuccessfully allocate space for memory because of virtual pool size overflow(no enough regions).\n");
		return 0;
	}

}

void VMPool::release(unsigned long _start_address) {
	//To use this function, we must make sure the provided address is valid

	unsigned long match_pos;
	//For each regions before the last memory region
	for (int i=0; i<last_mem_region; i++){
		//Once we find it
		if (mem_region_list[i].base_addr = _start_address){
			//record the position and then release the corresponding page 
			match_pos = i;
			page_table->free_page(_start_address);
			//if no region left
			if (last_mem_region <= 1){
				mem_region_list[0].base_addr = 0;
				mem_region_list[0].region_size = 0;
			//if there are some regions left
			}else{
				//put the left regions into the new sequence
				for (int j=match_pos; j<last_mem_region-1; j++){
					mem_region_list[j] = mem_region_list[j+1];
				}
			}
			break;
		}
	}

	last_mem_region--;

	//after released the page, flushed the TLB
	page_table->load();


	Console::puts("Released region of memory.\n");
}


bool VMPool::is_legitimate(unsigned long _address) {
	// checking if the address is legitimate or not
	Console::puts("Checking given address is legitimate or not.\n");
	if (mem_region_list){
		//for each regions in the list
		for (int i=0; i<last_mem_region; i++){
			unsigned long head = mem_region_list[i].base_addr;
			unsigned long tail = mem_region_list[i].base_addr + mem_region_list[i].region_size;	
			//we find a region that included the given address
			if((head <= _address)&&(_address < tail)){
				Console::puts("Valid address.\n");
				return true;
			}
		}
	}
	Console::puts("Invalid address.\n");
	return false;
}

