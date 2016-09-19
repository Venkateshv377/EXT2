#include "header.h"

unsigned int block_size;
int local_inode_index;
unsigned int dir_count, file_count;
unsigned int group_count, descr_list_size;
struct ext2_super_block super;
struct ext2_inode inode;
unsigned static int inode_temp;

int main(int argc, char *argv[])
{
	struct ext2_group_desc group;
	int fd, block = 2, inode_bitmap;
	if (argc > 2)
	{
		perror("Error:\n");
		printf("Usage: <.exe>  <image_name> \n");
	}

	if ((fd = open(argv[1], O_RDONLY, S_IRWXU)) < 0) {
		perror(argv[1]);
		exit(1);
	}

	/* read super-block */

	lseek(fd, BASE_OFFSET, SEEK_SET); 
	read(fd, &super, sizeof(super));

	if (super.s_magic != EXT2_SUPER_MAGIC) {
		fprintf(stderr, "Not a Ext2 filesystem\n");
		exit(1);
	}

	block_size = 1024 << super.s_log_block_size;
/*	printf("Reading super-block from device: %s\n", argv[1]);
	printf("Inodes count            : %x\n", super.s_inodes_count);
	printf("Blocks count            : %x\n", super.s_blocks_count);
	printf("Reserved blocks count   : %x\n", super.s_r_blocks_count);
	printf("Free blocks count       : %x\n", super.s_free_blocks_count);
	printf("Free inodes count       : %x\n", super.s_free_inodes_count);
	printf("First data block        : %x\n", super.s_first_data_block);
	printf("Block size              : %x\n", block_size);
	printf("Blocks per group        : %x\n", super.s_blocks_per_group);
	printf("Inodes per group        : %x\n", super.s_inodes_per_group);
	printf("Creator OS              : %x\n", super.s_creator_os);
	printf("First non-reserved inode: %x\n", super.s_first_ino);
	printf("Size of inode structure : %x\n", super.s_inode_size);*/
	// calculate number of block groups on the disk 
	group_count = 1 + (super.s_blocks_count-1) / super.s_blocks_per_group;

	// calculate size of the group descriptor list in bytes 
	descr_list_size = group_count * sizeof(struct ext2_group_desc);
/*	printf("sizeof struct ext2_group_desc: %d\n", sizeof(struct ext2_group_desc));
	printf("size of the group descriptor list:	%d\n", descr_list_size);
	printf("group count:	%x\n", group_count);*/

	lseek(fd, BASE_OFFSET + block_size, SEEK_SET);
	read(fd, &group, sizeof(group));

/*	printf("Inodes table block : %x\n", group.bg_inode_table);
	printf("Blocks bitmap block: %x\n", group.bg_block_bitmap);
	printf("Inodes bitmap block: %x\n", group.bg_inode_bitmap);
	printf("Free blocks count  : %x\n", group.bg_free_blocks_count);
	printf("Free inodes count  : %x\n", group.bg_free_inodes_count);
	printf("Directories count  : %x\n", group.bg_used_dirs_count);*/
	inode_bitmap = group.bg_inode_table * 0x400;

	int inode_no = 2;
	lseek(fd, BLOCK_OFFSET(group.bg_inode_table)+(inode_no-1)*sizeof(struct ext2_inode), SEEK_SET);
	read(fd, &inode, sizeof(struct ext2_inode));
	inode_table(fd, &inode);
	printf("\nNumber of file:		%d", file_count);
	printf("\nNumber of directories:	%x\n", dir_count);
}

void inode_table(int fd, struct ext2_inode *inode)
{
	int inode_bitmap;
	static struct ext2_inode temp_inode;
	static int inode_number;
	struct ext2_dir_entry_2 *entry;
	int i = 0;
	unsigned char *name;
	unsigned int size = 0;

	if ((name = calloc(block_size, sizeof(char))) == NULL)
		perror("Error:\n");

	lseek(fd, inode->i_block[i] * 0x400, SEEK_SET);
	read(fd, name, block_size);
	entry = (struct ext2_dir_entry_2 *)name;

	while((size < inode->i_size) && entry->inode) 
	{ 
		if (entry->file_type == 0x02 && (strcmp(entry->name, ".") && strcmp(entry->name, "..")))
		{
			printf("\033[22;36m%s\033[0m", entry->name);
			dir_count++;

			if (entry->inode > inode_temp)
			{
				group_count = ((entry->inode) / super.s_inodes_per_group);
				local_inode_index = (entry->inode) % super.s_inodes_per_group;
				lseek(fd,1024 + block_size + (group_count * 32) + 8,SEEK_SET);
				unsigned int addr;
				read(fd,&addr,4);
				lseek(fd,(addr * block_size) + ((local_inode_index -1 )* 0x80),SEEK_SET);
				read(fd, &temp_inode, sizeof(struct ext2_inode));
				inode_temp = entry->inode;
				inode_table(fd, &temp_inode);
			}
		}
		else if (entry->file_type == 1)
		{
			printf("\033[22;32m%s\033[0m", entry->name);
			file_count++;
		}
		entry = (void*) entry + entry->rec_len; 
		size += entry->rec_len;
	}
}
