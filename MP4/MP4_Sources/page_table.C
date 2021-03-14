#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;

const unsigned int PageTable::vm_pool_cnt = 5;
VMPool* PageTable::vm_pool_list[vm_pool_cnt];


void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   //assert(false);
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;
   paging_enabled = 0;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
   //assert(false);
	page_directory = (unsigned long *)(kernel_mem_pool->get_frames(1) << 12);
	page_table = (unsigned long *)(kernel_mem_pool->get_frames(1) << 12);
	//the page table comes right after the page directory
	
	unsigned long address=0; // holds the physical address of where a page is
	unsigned int i;

	// map the first 4MB of memory
	for(i=0; i<1024; i++){
		page_table[i] = address | 3; // attribute set to: supervisor level, read/write, present(011 in binary)
		address = address + 4096; // 4096 = 4kb
	};
	// fill the first entry of the page directory
	page_directory[0] = (unsigned long )page_table; // attribute set to: supervisor level, read/write, present(011 in binary)
	page_directory[0] = page_directory[0] | 3;
	
	for(i=1; i<1024; i++){
		page_directory[i] = 0 | 2; // attribute set to: supervisor level, read/write, not present(010 in binary)
	};
	page_directory[1023] = (unsigned long) page_directory | 3;
	//Let the last page directory entry pointed to itself.

	unsigned int j;
	//empty the vm pool lists
	for (j=0; j<vm_pool_cnt; j++){
		vm_pool_list[j] = NULL;
	}
	Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   //assert(false);
	current_page_table = this;
	write_cr3((unsigned long)page_directory); // put that page directory address into CR3
   	Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   //assert(false);
	paging_enabled = 1;
	write_cr0( read_cr0() | 0x80000000); // set the paging bit in CR0 to 1
   	Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
  //assert(false);
	unsigned long *cur_page_dir = (unsigned long*) read_cr3();
	unsigned long *page_table;
	unsigned long addr = read_cr2();
	unsigned long dir_offset = addr>>22;
	unsigned long err_info = _r->err_code;
	if((err_info & 1) == 1){
		Console::puts("Protection Fault.\n");	
	}
	else{
		//Check if the logical address is legitimate
		Console::puts("Checking the logical address\n");
		
		VMPool** vm_list = current_page_table->vm_pool_list;

		bool logical_valid = false;
		for(int i=0;i<vm_pool_cnt;i++){
			if(vm_list[i] != NULL){
				logical_valid = true;
				Console::puts("Current list is valid\n");
			}
		}
		
		if(logical_valid == false){
			Console::puts("No valid address..\n");
		} 
		// If the current page directory's offset corresponding page table is "non present"
		if((cur_page_dir[dir_offset] & 1) != 1){
			Console::puts("not present\n");
			//get a frame from free frame pool and initialization process
			cur_page_dir[dir_offset] = (unsigned long)((kernel_mem_pool->get_frames(1)<<12) | 3);
			page_table = (unsigned long*)(cur_page_dir[dir_offset] & 0xFFFFF000);
			//empty all the entries in the new page table.
			for(int i = 0; i< 1024; i++){
				page_table[i] = 0|2;
			}		
		}
		// Add the page table into the page directory entry.
		unsigned long table_offset = ((addr>>12) & 0x3FF);
		page_table = (unsigned long*)(cur_page_dir[dir_offset] & 0xFFFFF000);
		page_table[table_offset] = (process_mem_pool->get_frames(1) << 12) | 3;
		Console::puts("handled page fault\n");
	}
 
}

//
void PageTable::register_pool(VMPool * _vm_pool)
{
    //for each pool in the list
    for(int i=0; i<vm_pool_cnt; ++i){
	//If we find out an empty one
        if (vm_pool_list[i] == NULL){
            vm_pool_list[i] = _vm_pool;
            Console::puts("Successfully register Virtual memory pool.\n");
            return;
        }
    }
    Console::puts("Failed to register virtual memory pool.\n");


    
}

//
void PageTable::free_page(unsigned long _page_no) 
{
	unsigned long dir_offset = _page_no >> 22;
	unsigned long table_offset = ((_page_no >> 12) & 0x3FF);
	unsigned long *page_table = (unsigned long *)((dir_offset<<12)|0xFFC00000);
	unsigned long frame_idx = page_table[table_offset];
	process_mem_pool->release_frames(frame_idx);
	Console::puts("Pages are freed\n");
}
