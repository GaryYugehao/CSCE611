/* 
    File: file_system.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 10/04/05

    Description: Simple File System.
    

*/

#ifndef _FILE_SYSTEM_H_                   // include file only once
#define _FILE_SYSTEM_H_
#define len_list 10

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include "simple_disk.H"
#include "file.H"


/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

struct inode{
	unsigned int key;
	unsigned int loc[16];
	unsigned int len;

};

struct file_info{
	int key;
	int id;

};

/*--------------------------------------------------------------------------*/
/* FORWARD DECLARATIONS */ 
/*--------------------------------------------------------------------------*/

class File;

/*--------------------------------------------------------------------------*/
/* F i l e S y s t e m  */
/*--------------------------------------------------------------------------*/

class FileSystem {

friend class File; /* -- not sure if we need this; feel free to delete */

private:
     /* -- DEFINE YOUR FILE SYSTEM DATA STRUCTURES HERE. */
     
     SimpleDisk * disk;
     static unsigned int size;
	
     file_info files_list[len_list];
     /*the inode file list that store file info, length is defined.*/

     int tail_file_idx;
     /*where is the index of last written file in the inode file list.*/

     int total_file_cnt;
     /*where is the total number of written files in the inode file list.*/

     unsigned int len_blk;
     /*how many blocks we have.*/

     unsigned char blk_list[512];
     /*a list to store the information of a block, each block is 512 bytes so size is 512.*/
	
     
public:

    FileSystem();
    /* Just initializes local data structures. Does not connect to disk yet. */
    
    bool Mount(SimpleDisk * _disk);
    /* Associates this file system with a disk. Limit to at most one file system per disk.
     Returns true if operation successful (i.e. there is indeed a file system on the disk.) */
    
    static bool Format(SimpleDisk * _disk, unsigned int _size);
    /* Wipes any file system from the disk and installs an empty file system of given size. */
    
    File * LookupFile(int _file_id);
    /* Find file with given id in file system. If found, return the initialized
     file object. Otherwise, return null. */
    
    bool CreateFile(int _file_id);
    /* Create file with given id in the file system. If file exists already,
     abort and return false. Otherwise, return true. */
    
    bool DeleteFile(int _file_id);
    /* Delete file with given id in the file system; free any disk block occupied by the file. */
    
    void occupy(unsigned int begin_idx);
    /* Occupy the block list space with the given index in the block list. */

    void release(unsigned int begin_idx);
    /* Release the block list space with the given index in the block list. */

    int next_valid();
    /*Return the next valid index in the block list that could store files, if not, return -1.*/
   
};
#endif
