#include "inode.h"
#include "blocks.h"
#include "bitmap.h"
#include <assert.h>

int TOTAL_NODES = 4096/sizeof(inode_t);

// debugging the inode -> print all the fields
void print_inode(inode_t *node){
  // print all the fields in one line.
    printf("refs: %d, mode: %d, size: %d, block: %d\n", node->refs, node->mode, node->size, node->block);
}


// given the i number, return the relevant i node.
inode_t *get_inode(int inum) {

    assert (inum < 256); // making sure the inumber is less than the total blocks

    // offset the address by 64, because that is the total size of the data bitmap and the inode bitmap
    // do not want to over write data.
    void *node_table = blocks_get_block(0) + 64;

    // move to the inode by getting to the node table and incrementing the void star address by
    // the inumber * size of an inode
    inode_t *node = (inode_t*) (node_table + inum * sizeof(inode_t));

    // return a pointer to the requested inode
    return node;
}


// allocate an inode and set the fields
int alloc_inode(int mode, int size) {
  // method is almost exactly the same as the blocks alloc

  // get the inode bitmap 
  void *bbm = get_inode_bitmap();

  // for loop looping through all the nodes
  for (int ii = 0; ii < TOTAL_NODES; ii++) {
    // find the first free bitmap numbers
    if (bitmap_get(bbm, ii) == 0) {
      // set the found free bitmap to allocated
      bitmap_put(bbm, ii, 1);
      // set the node here, so we don't forget about it later
      setNode(ii,mode,size, 1);
      // there is a 1:1 correspondence between the bit numbers to the inumber
      // return the bit number found
      return ii;
    }
  }

  // return -1 if there are no more free inodes to be allocated 
  return -1;

}


// set the fields of an inode
void setNode(int inum, int mode, int size, int refs) {  

    inode_t *thisNode = get_inode(inum); // get the inode from the i number
    thisNode->size = size; // set the size
    thisNode->mode = mode; // the mode is the mode
    thisNode->refs = refs; // the refs are 1

    int newBlockNum = alloc_block(); // new allocated block
    thisNode->block = newBlockNum;
}

