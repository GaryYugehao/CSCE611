/*
 File: ContFramePool.C
 
 Author:
 Date  : 
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

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
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

//struct node{ContFramePool current_pool(unsigned long base, unsigned long n,
//				 unsigned long info_no, unsigned long n_info);
//				 node *next;};
node *pool_list_head;





ContFramePool::ContFramePool(unsigned long _base_frame_no, unsigned long _n_frames, unsigned long _info_frame_no, unsigned long _n_info_frames)
{
    // TODO: IMPLEMENTATION NEEEDED!

//	assert(nframes <= FRAME_SIZE * 4);
	
	base_frame_no 	= _base_frame_no;
	nframes 	= _n_frames;
	nfreeframes 	= _n_frames;
	info_frame_no   = _info_frame_no;
	ninfoframes	= _n_info_frames;


	//directly give address to the 1st position of bitmap
	if(info_frame_no == 0) {
		bitmap = (unsigned char*) (base_frame_no * FRAME_SIZE);	
//		isHead = (unsigned char*) (base_frame_no * FRAME_SIZE);
	}else{
		bitmap = (unsigned char*) (info_frame_no * FRAME_SIZE);
	}

	//initially, all frames are unallocated, not the head of sequence
	for(int i = 0; i * 4 < nframes; i++){
		bitmap[i] = 0xFF;
	}
	
	//When the info frame number is the base frame, then we mark
	//that frame as used
	if(info_frame_no == 0){
		bitmap[0] = 0x7F;
		nfreeframes--;
	}

	node* newNode;

	(*newNode).currentPool = this;	
	node* headNext = (*pool_list_head).next;
	if(headNext == NULL ){
		(*pool_list_head).next =  newNode;
	}else{
		(*newNode).next = (*pool_list_head).next;
		(*pool_list_head).next = newNode;
	}		
	
	Console::puts("Frame pool is initialized");
}

unsigned int ContFramePool::check_state(unsigned long _target_frame_no, unsigned int _input_mask){
	int frame_diff = _target_frame_no - base_frame_no;
	unsigned int bitmap_index = frame_diff / 4;	
	unsigned char mask1 = _input_mask >> (frame_diff % 4);
	if((bitmap[bitmap_index] & mask1) == 0){
		return 1; //Used
	}else{
		return 0; //Free
	}
}
void ContFramePool::set_state(unsigned long _target_frame_no, unsigned int _input_mask){
	unsigned int index = (_target_frame_no - base_frame_no) / 4;
	unsigned char mask = _input_mask >> (_target_frame_no - base_frame_no) % 4;
	bitmap[index] ^= mask;
}
unsigned long ContFramePool::get_frames(unsigned int _n_frames){

	assert(nfreeframes >= _n_frames);
    int cur = 0;
    int distance;
    unsigned int visited=0;
    unsigned int cur_head = base_frame_no;
    while(visited<_n_frames){
	unsigned int frame_pos = cur+base_frame_no;
	unsigned int cur_state = check_state(frame_pos, 0x80);
        while((cur<nframes) && (cur_state != 0)){ //Not free
            cur ++;
	    frame_pos ++;
	    cur_state = check_state(frame_pos, 0x80);
        }
        distance = cur;
        while((cur<nframes) && (cur_state == 0)){
            cur ++;
	    frame_pos ++;
	    cur_state = check_state(frame_pos, 0x80);
            if(cur-distance==_n_frames){
                break;
            }
        }
        visited = cur-distance;
        if(cur>=nframes){
            return 0;
        }
    }
    cur_head += distance;
    mark_inaccessible(cur_head, _n_frames);
    unsigned char temp_mask2 = 0x08>>((cur_head-base_frame_no)%4);
    bitmap[cur / 4] ^= temp_mask2;
    return cur_head;
}


void ContFramePool::mark_inaccessible(unsigned long _base_frame_no, unsigned long _n_frames)
{
	int i;
	for(i = _base_frame_no; i < _base_frame_no + _n_frames; i++){
		assert ((i >= base_frame_no) && (i < base_frame_no + nframes));
		set_state(i, 0x80);

	}
	nfreeframes -= _n_frames;


}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    // TODO: IMPLEMENTATION NEEEDED!
    //assert(false);
	node *currentNode = (*pool_list_head).next;
	ContFramePool cur_pool = *((*currentNode).currentPool);
	while(!(_first_frame_no >= cur_pool.base_frame_no && 
		 _first_frame_no < cur_pool.base_frame_no+cur_pool.nframes)){
		currentNode = (*currentNode).next;
		cur_pool = *((*currentNode).currentPool);
	}


	unsigned int cur_state = cur_pool.check_state(_first_frame_no, 0x08);

	if(cur_state == 1){
		cur_pool.set_state(_first_frame_no, 0x08);
	}
	
	int i;
	for(i = _first_frame_no; i < cur_pool.base_frame_no + cur_pool.nframes; i++){
		int frame_diff = i - cur_pool.base_frame_no;
		unsigned int bitmap_index = frame_diff / 4;
		//when reach the head of sequence
		unsigned int head_state = cur_pool.check_state(i, 0x08);
		if(head_state==1) break;
		//when there are free frames
		unsigned int free_state = cur_pool.check_state(i, 0x80);
		//Free->0
		if(free_state==1) break;
		cur_pool.bitmap[bitmap_index] |= 0x80 >> (frame_diff%4);
	}
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    // TODO: IMPLEMENTATION NEEEDED!
//    assert(false);
	//16384 = 16 k = 4KB * 4
	return (_n_frames / 16384 + (_n_frames % 16384 > 0 ? 1 : 0));
	

}
