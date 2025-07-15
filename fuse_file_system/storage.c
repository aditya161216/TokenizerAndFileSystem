#include "storage.h"
#include "directory.h"
#include <assert.h>
#include "blocks.h"
#include "bitmap.h"
#include "inode.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "slist.h"
#include <sys/mman.h>


// setting the blocks to the storage, 
// calls blocks init, sets up the root directory, the inode table, and the bitmaps
// need to pass this method data.nufs
void storage_init(const char *path) {

    // the path is data.nufs, blocks init will set up all the blocks here.

    blocks_init(path); // mmaping the blocks and such is done here.

    void* inodeBitBlock = get_inode_bitmap(); // this will be the inode bitmap

    // if the file system is being initialised for the first time
    if (bitmap_get(inodeBitBlock, 0) == 0) {
        // set up the root directory and all the corresponding elements of the file system
        directory_init(); 
    }

    // if there already is some data on the file, we have to do nothing as the data will be read by the FUSE filesystem.
}

// set the stat in this method to the relevant fields of the file from the given path.
int storage_stat(const char *path, struct stat *st){
    int iNumber = tree_lookup(path); // obtain the inumber from the given path
    // if the file doesn't exist
    if (iNumber < 0) { 
        // throw a no such file error
        return -1;
    }

    // now that we know that the file exists, obtain the stats

    // get the inode 
    inode_t *inode = get_inode(iNumber);
    
    // set the stat structures fields to the relevant node data
    memset(st, 0, sizeof(struct stat));

    st->st_uid = getuid(); // this is always the case for the stat
    st->st_mode = inode->mode;
    st->st_size = inode->size;
    
    // return 0 on success
    return 0;
}

// make a file / directory at the given path
int storage_mknod(const char *path, int mode){

    char* fileName = getFileName(path); // the name that this new file / dir will be called

    // get the inode number of the parent directory of the file
    int Parentnum = getParentInum(path);

    printf("\n\n path : %s , inodeNum: %d \n\n", path, Parentnum);

    // return -1 if the parent dir could not be found
    if (Parentnum < 0){
        return -1;
    }

    // get the parent inode
    inode_t *parent = get_inode(Parentnum);

    // the mode of this new file / dir has been given.
    // on creation, the size is 0.
    int newInodeNum = alloc_inode(mode, 0);

    // now that we have allocated the inode, we must put it in the parent directory's data entry
    directory_put(parent,fileName,newInodeNum); 

    // printf("\n\n mode: %d \n\n", mode);
    return 0;
}


int storage_unlink(const char *path) {
    // get the inum of the path 
    // get the inode using the inum
    // go to the data block
    // memset the file to 0
    // set the size field to 0 along with refs to 0
    // then 'unlink' the reference to the data block
    //  - make it point to NULL
    // set the reference in the bitmap to 'free'
    
    // get the inum of the path
    int fileInum = tree_lookup(path);

    // if file is not found, throw an error
    if (fileInum < 0) {
        return -ENOENT;
    }

    // store the parent iNode for later
    int parentInum = getParentInum(path);

    inode_t *parentInode = get_inode(parentInum);

    // get the file inode
    inode_t *fileInode = get_inode(fileInum);

    void *data = blocks_get_block(fileInode->block);

    // reset the fields of the inode
    fileInode->size = 0;
    fileInode->refs = 0;

    // set the corresponding inode bitmap entry
    void *inode_bitmap = get_inode_bitmap();
    bitmap_put(inode_bitmap, fileInum, 0);

    // set the corresponding data bitmap entry
    void *data_bitmap = get_blocks_bitmap();
    bitmap_put(data_bitmap, fileInode->block, 0);

    // finally, delete the file name from the list of files its parent directory points to
    int dirDel = directory_delete(parentInode, getFileName(path), fileInum);

    if (dirDel < 0) {
        return -ENOENT;
    }

    // free the data block
    free_block(fileInode->block);
    fileInode->block = 0;

    printf("\n \n directory delete: path: %s status: %d \n \n",path,dirDel);
    
    return 0;
}


int storage_read(const char *path, char *buf, size_t size, off_t offset) {

    // get the inum
    int inum = tree_lookup(path);

    // throw an error if the file is not found
    if (inum < 0) {
        return -ENOENT;
    }

    // get the corresponding inode and data block
    inode_t *file_inode = get_inode(inum);
    void *file_data = blocks_get_block(file_inode->block);

    file_data += offset;

    // get the data from the file
    memcpy(buf, file_data, 4096);

    return 4096;
}

int storage_write(const char *path, const char *buf, size_t size, off_t offset) {

    // do a tree lookup on the path
    int inum = tree_lookup(path);

    // if not found then throw an error
    if (inum < 0) {
        return -ENOENT;
    }

    // find the corresponding inode
    inode_t *file_inode = get_inode(inum);

    // get the corresponding data block
    void *file_data = blocks_get_block(file_inode->block);

    file_data += offset;

    // set the data of the file
    memcpy(file_data, buf, size);

    // reset the size
    file_inode->size += size;

    return size;
}