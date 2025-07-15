#include "directory.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include "slist.h"


int TOTAL_DIRENTS = 4096 / sizeof(dirent_t);


// set up the root directory
void directory_init() {
    // set the root directory to size 0, and set the file type to directory
    int rootDirNum = alloc_inode(040755, 0);
    // get the inode pointer to the root directory
    inode_t *rootDir = get_inode(rootDirNum);
    // put this in the information about the root directory in the root directory's entries
    directory_put(rootDir, ".", rootDirNum);
}

// put the information about a file / directory in the given directory dd's entries
int directory_put(inode_t *dd, const char *name, int inum) {
    // get the block number of this directory
    int bnum = dd->block;
    // get the directory entries by casting the block's pointer
    dirent_t *dirents = (dirent_t*) blocks_get_block(bnum);

    // iterate over the dirents (contigous memory)
    for (int i = 0; i < TOTAL_DIRENTS; i++) {
        // find the next free spot to place a dirent
        if (dirents[i].inum == 0 && strcmp(dirents[i].name, ".") != 0) {
            // set the dirent to inum
            dirents[i].inum = inum;
            // string copy because its a const char * and not just a char *
            strcpy(dirents[i].name, name);
            // increase the size of the inode referring to this directory, because we have just added another dirent
            dd->size += sizeof(dirent_t);
            // return 0 on success
            return 0;
        }
    }

    // there is no more space in this directory for more files / directories
    return -1;
}


// the file name will be the last element in the split list
char *getFileName(const char *path) { 
    // use the explode method given in slist to split the path on the / delimiter,
    // obtaining an in-order method to slowly move up the path
    slist_t *split = s_explode(path, '/');

    // iterate over a non-empty list, getting the last element
    while (split->next) {
        split = split->next;
    }

    // assert that split exists (is not null).
    assert(split);

    // return the data of the last element
    return split->data;
}

// get the inode number of the parent directory in the path (second last)
int getParentInum(const char *path) {

    if(strcmp(path,"/") == 0){
        return 0;
    }

    // use the explode method given in slist to split the path on the / delimiter,
    // obtaining an in-order method to slowly move up the path
    slist_t *split = s_explode(path, '/');

    // we are currently in the root directory
    int currentInum = 0;

    // move onto the next element as the first one is the root directory.
    split = split->next;


    // while the next and next of that exist, and the current inode number is a relevant inode
    while (split->next && currentInum >= 0) {
        // set the current inode number to the inode of the next element in the split list
        currentInum = directory_lookup(get_inode(currentInum), split->data);
        // move on to the next element in the split path
        split = split->next;
    }


    // return the valid inode number
    return currentInum;
}


// given an inode find the name of the file and return its inum
int directory_lookup(inode_t *dd, const char *name) {

    void* currentData = blocks_get_block(dd->block); // the data block

    dirent_t *dirents = (dirent_t*) currentData; // we are now at the dirents

    // iterate over the dirents
    for (int i = 0; i < TOTAL_DIRENTS; i++) {
        // check if the dirent name is the given file name
        if (strcmp(dirents[i].name, name) == 0) {
            // if it is, return the the inum of that dirent
            return dirents[i].inum;
        }
    }

    // file does not exist in given inode
    return -1;
}


// returns the inum of the last object (file or directory) of the path
int tree_lookup(const char *path) {
    // obtain the filename from the path
    char *obj_name = getFileName(path);

    // make sure that the path is not the root
    if (strcmp(path, "/") == 0) {
        // if it is return the root directory's inumber
        return 0;
    }
    else { // this is not the root directory

        // get the inode number of the file's parent
        int parentInum = getParentInum(path);

        // get the parent inode
        inode_t *parentInode = get_inode(parentInum);

        int inodeNum = directory_lookup(parentInode, obj_name);

        if (inodeNum < 0){ // if the path does not exist
        // throw a no such entity error
            return -ENOENT;
        }

        // return the relevant inode number
        return inodeNum;
    }
}

// put all of a given directory's entries in an slist and return a pointer to the slist
slist_t *directory_list(const char *path) {
    // get the inode of the path
    int pathInum = tree_lookup(path);

    // if the file / path does not exist
    if (pathInum < 0) {
        // return null, there are no entries
        return NULL;
    }

    // get the inode from the path number
    inode_t *currInode = get_inode(pathInum);
    // ontain its data
    void *data_block = blocks_get_block(currInode->block);
    // cast the data block to a dirent, so we can iterate over the dirents and access their fields.
    dirent_t *dirents = (dirent_t*) data_block; // we are now at the dirents
    // create the initial s list by putting the first entry in the dirents and conjoining it to null.
    slist_t *listOfEntries = s_cons(dirents[0].name,0);

    // iterate over the dirents
    for (int i = 1; i < TOTAL_DIRENTS; i++) {
        // if the dirent exists
        if (dirents[i].inum != 0) {
            // add it to our list of entries
            listOfEntries = s_cons(dirents[i].name, listOfEntries);
        }
    }
    // return the conjoined string list
    return listOfEntries;
}

int directory_delete(inode_t *dd, const char *name, int inum) {
    
    void* currentData = blocks_get_block(dd->block); // the data block

    dirent_t *dirents = (dirent_t*) currentData; // we are now at the dirents

    // iterate over the dirents
    for (int i = 0; i < TOTAL_DIRENTS; i++) {
        // check if the dirent name is the given file name
        if (dirents[i].inum == inum) {
            // pointer to the start of memory returned by memmove
            memmove(dirents + i, dirents + i + 1, 4096 - (i * sizeof(dirent_t)));
            // reset the size of the directory inode
            dd->size -= sizeof(dirent_t);
            return 0;
        }
        currentData += sizeof(dirent_t);
    }

    return -1;
}

void print_directory(inode_t *dd){
    void* data = blocks_get_block(dd->block);

    dirent_t *dirents = (dirent_t*) data; // we are now at the dirents

    // iterate over the dirents
    for (int i = 0; i < TOTAL_DIRENTS; i++) {
        // if the dirent exists
        if (i < 5) {
            // add it to our list of entries
            printf("name: %s, inum %d \n",dirents[i].name,dirents[i].inum);
        }
    }

    return;
}
