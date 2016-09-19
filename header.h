#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ext2_fs.h"

#define BASE_OFFSET             0x400
#define BLOCK_OFFSET(block)     (BASE_OFFSET + (block-1) * block_size)


void inode_table(int fd, struct ext2_inode *inode, struct ext2_group_desc *group); 

void read_inode(int fd, int inode_no, struct ext2_group_desc *group, struct ext2_inode *inode);
