// based on cs3650 starter code

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "inode.h"
#include "storage.h"
#include "directory.h"
#include "slist.h"

#define FUSE_USE_VERSION 26
#include <fuse.h>

// implementation for: man 2 access
// Checks if a file exists.
int nufs_access(const char *path, int mask) {
  int rv = 0;

  printf("entered access\n");

  int nodeNum = tree_lookup(path);

  // check if the given path exists
  if (nodeNum < 0) {
    // if it doesn't return a no such entity error.
    rv = -ENOENT;
  }

  // if the path does exist, we are assuming that the user of the file system has full access priveleges.

  printf("access(%s, %04o) -> %d\n", path, mask, rv);
  return rv;
}

// Gets an object's attributes (type, permissions, size, etc).
// Implementation for: man 2 stat
// This is a crucial function.
int nufs_getattr(const char *path, struct stat *st) {
  int rv = 0;

  // find the inode number of the last file / directory in the path.
  int nodeNum = tree_lookup(path);

  // if the path does not exist
  if (nodeNum < 0) {
    // return a no such entity error
    rv = -ENOENT;
  } else {
    // get the inode corresponding to the last file / directory in the given path
    inode_t *node = get_inode(nodeNum);

    // set the stat struct's fields based on the inode
    st->st_mode = node->mode;
    st->st_size = node->size;
    st->st_uid = getuid(); // this is always the user id because the user has admin priveleges.
  }

  printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, rv, st->st_mode,
         st->st_size);
  return rv;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi) {

  printf("entered read dir dir: %s", path);
  struct stat st;
  int rv = 0;

  // fill the slist with all the files and directories in the given directory path
  slist_t *listOfContents = directory_list(path); 

  // while the list of contents has a next element
  while (listOfContents->next) {
    // use the filler function in built into fuse to fill the buffer
    filler(buf, listOfContents->data, &st, 0);
    listOfContents = listOfContents->next;
  }

  // fill the buffer with the name with the final data in the list of contents
  filler(buf, listOfContents->data, &st, 0);

  printf("readdir(%s) -> %d\n", path, rv);
  return 0;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
// Note, for this assignment, you can alternatively implement the create
// function.
int nufs_mknod(const char *path, mode_t mode, dev_t rdev) {
  int rv = storage_mknod(path,mode);
  printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int nufs_mkdir(const char *path, mode_t mode) {

  // get the parent inum and inode
  int parent = getParentInum(path);
  inode_t *parentN = get_inode(parent);

  // call upon nufs mknod
  int rv = nufs_mknod(path, mode | 040000, 0);
  // get the inum & inode of the file
  int inum = tree_lookup(path);
  inode_t *node = get_inode(inum);

  // put the self & parent references into the corresponding directory
  directory_put(node, ".", inum);
  directory_put(node, "..", getParentInum(path));

  printf("mkdir(%s) -> %d\n", path, rv);
  printf("\n\n\n");
  return rv;
}

int nufs_unlink(const char *path) {

  // call upon the helper used to 'unlink' a file
  int rv = storage_unlink(path);
  printf("unlink(%s) -> %d\n", path, rv);
  return rv;
}

int nufs_link(const char *from, const char *to) {
  int rv = -1;
  printf("link(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

int nufs_rmdir(const char *path) {
  // call upon the helper used to remove a directory
  int rv = nufs_unlink(path);
  printf("rmdir(%s) -> %d\n", path, rv);
  return rv;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int nufs_rename(const char *from, const char *to) {

// get the parent of the both, remove the file from the 

  // get both the file names
  char *fileNew = getFileName(from);
  char *fileOld = getFileName(to);

  // get the parent inums of both of them
  int inumParentOld = getParentInum(from);
  int inumParentNew = getParentInum(to);

  // get the inum of the original location and put it in the correct location
  int thisInode = tree_lookup(from);
  directory_put(get_inode(inumParentNew),fileNew,thisInode);

  // delete the original entry
  directory_delete(get_inode(inumParentOld),fileOld,thisInode);

  int rv = -1;
  printf("rename(%s => %s) -> %d\n", from, to, rv);
  return 0;
}

int nufs_chmod(const char *path, mode_t mode) {
  int rv = -1;
  printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

int nufs_truncate(const char *path, off_t size) {
  int rv = -1;
  printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
  return rv;
}

// This is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// You can just check whether the file is accessible.
int nufs_open(const char *path, struct fuse_file_info *fi) {
  int rv = 0;
  printf("open(%s) -> %d\n", path, rv);
  return rv;
}

// Actually read data
int nufs_read(const char *path, char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi) {

  int rv = storage_read(path, buf, size, offset);

  printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}


// Actually write data
int nufs_write(const char *path, const char *buf, size_t size, off_t offset,
               struct fuse_file_info *fi) {

  int rv = storage_write(path, buf, size, offset);
  printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

// Update the timestamps on a file or directory.
int nufs_utimens(const char *path, const struct timespec ts[2]) {
  int rv = -1;
  printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n", path, ts[0].tv_sec,
         ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
  return 0;
}

// Extended operations
int nufs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi,
               unsigned int flags, void *data) {
  int rv = -1;
  printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
  return rv;
}

void nufs_init_ops(struct fuse_operations *ops) {
  memset(ops, 0, sizeof(struct fuse_operations));
  ops->access = nufs_access;
  ops->getattr = nufs_getattr;
  ops->readdir = nufs_readdir;
  ops->mknod = nufs_mknod;
  // ops->create   = nufs_create; // alternative to mknod
  ops->mkdir = nufs_mkdir;
  ops->link = nufs_link;
  ops->unlink = nufs_unlink;
  ops->rmdir = nufs_rmdir;
  ops->rename = nufs_rename;
  ops->chmod = nufs_chmod;
  ops->truncate = nufs_truncate;
  ops->open = nufs_open;
  ops->read = nufs_read;
  ops->write = nufs_write;
  ops->utimens = nufs_utimens;
  ops->ioctl = nufs_ioctl;
};

struct fuse_operations nufs_ops;

int main(int argc, char *argv[]) {
  assert(argc > 2 && argc < 6);
  printf("mount %s as data file\n", argv[--argc]);
  storage_init(argv[argc]);
  nufs_init_ops(&nufs_ops);
  return fuse_main(argc, argv, &nufs_ops, NULL);
}