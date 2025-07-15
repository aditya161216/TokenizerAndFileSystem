// Directory manipulation functions.
//
// Feel free to use as inspiration.

// based on cs3650 starter code

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME_LENGTH 48

#include "blocks.h"
#include "inode.h"
#include "slist.h"

typedef struct dirent {
  char name[DIR_NAME_LENGTH]; // name of the directory.
  int inum; // I node number corresponding to the directory.
  char _reserved[12]; // the data blocks referred to by the directory ?
} dirent_t;

void directory_init();

int directory_lookup(inode_t *dd, const char *name); 

int tree_lookup(const char *path); // tree look up

int directory_put(inode_t *dd, const char *name, int inum);

// char* getNameAtIndex(slist_t *split, int index);

char *getFileName(const char *path);

int getParentInum(const char *path);

int directory_delete(inode_t *dd, const char *name, int inum);

slist_t *directory_list(const char *path);

void print_directory(inode_t *dd);

#endif
