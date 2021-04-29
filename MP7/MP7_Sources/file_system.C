/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"

unsigned int FileSystem::size;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
    //assert(false);
	disk = NULL;
	len_blk = 0;
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system form disk\n");
    //assert(false);
	disk = _disk;
	len_blk = (size/512);
	for (int i=0;i<len_list;i++){
		files_list[i].key = -1;
		files_list[i].id = -1;
	}
	current_file_idx = 0;
	files_list_idx = -1;

	memset(blk_list,0,512);
	Console::puts("Successfully mounting file system.\n");
	return true;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) {
    Console::puts("formatting disk\n");
    //assert(false);
	char temp_buffer[512];
	memset(temp_buffer, 0, 512);
	for(int i=0; i<512;i++){
		_disk->write(i, (unsigned char*)temp_buffer);
	}
	FileSystem::size = _size;
	Console::puts("Successfully formatting the disk.\n");
	return true;
	
}

File * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file\n");
    //assert(false);
	for(int i=0; i<len_list;i++){
		int cur_id = files_list[i].id;
		int cur_key = files_list[i].key;
		if(cur_id == _file_id){
			unsigned char temp_buffer[512];
			disk->read(cur_key, temp_buffer);
			File* res = (File*) new File((inode*)temp_buffer);
			Console::puts("Find the file.\n");
			return res;
		}
	}
	Console::puts("Didn't find the file.\n");
	return NULL;

}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file\n");
    //assert(false);
    if(LookupFile(_file_id)){
		Console::puts("File already existed.\n");
		return false;
	}
	int new_head = next_valid();
	unsigned char temp_buffer[512];
	inode* next_inode = (inode*)temp_buffer;
	next_inode->key = new_head;
	next_inode->len = 0;
	occupy(new_head);
	disk->write(new_head, temp_buffer);
	files_list_idx += 1;
	if(files_list_idx >= len_list){
		Console::puts("Reach the maximum, begin overwriting.\n");
		files_list_idx = files_list_idx%len_list;
	}
	files_list[files_list_idx].key = new_head;
	files_list[files_list_idx].id = _file_id;
	current_file_idx += 1;
	Console::puts("Successfully created the file.\n");
	return true;
	
	
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file\n");
    //assert(false);
    if(not LookupFile(_file_id)){
		Console::puts("File didn't exist.\n");
		return false;
	}
	File* target_file = LookupFile(_file_id);
	int rel_key = target_file->cur_inode->key;
	for(int i=0; i<target_file->cur_inode->len;i++){
		release(target_file->cur_inode->loc[i]);
	}
	release(rel_key);
	for(int i = 0; i<len_list;i++){
		int cur_id = files_list[i].id;
		if(cur_id == _file_id){
			files_list[i].id = -1;
			files_list[i].key = -1;
		}
	}
	current_file_idx -= 1;
	if(current_file_idx == 0){
		files_list_idx = -1;
	}
	Console::puts("Successfully deleted the file.\n");
	return true;
}

void FileSystem::occupy(unsigned int begin_idx){
	blk_list[begin_idx/8] |= (1<<(begin_idx%8));
}
void FileSystem::release(unsigned int begin_idx){
	blk_list[begin_idx/8] &= ~(1<<(begin_idx%8));
}
int FileSystem::next_valid(){
	for(int i = 0; i<(len_blk/8); i++){
		for(int j=0; j<8; j++){
			if((blk_list[i] & (1<<j)) == 0){
				return(8*i+j);
			}
		}
	}
	return -1;
}
