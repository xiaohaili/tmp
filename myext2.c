#include <stdio.h>
#include <stdlib.h>
#include <ext2fs/ext2_fs.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#define SUPERBLOCK_OFFSET 1024

int read_super(int df, struct ext2_super_block *super)
{
	lseek(df, SUPERBLOCK_OFFSET, SEEK_SET);
	read(df, super, sizeof(struct ext2_super_block));

	if (super->s_magic != 0xef53) {
		printf("Error: not ext2. magic=0x%0x\n", super->s_magic);
		return -1;
	}

	return 0;
}

void print_super(struct ext2_super_block *super)
{
	int i;

	printf("Superblock (");
	for (i = 0; i < 16; i++) {
		printf("%02x", super->s_uuid[i]);
	}
	printf(")\n");

	printf( "\tinodes count: %d\n"				\
		"\tblocks count: %d\n"				\
		"\tinode size: %d\n"				\
		"\tblock size: %d\n"				\
		"\tinodes per group: %d\n"			\
		"\tblocks per group: %d\n"			\
		"\tfirst data block: %d\n",			\
		super->s_inodes_count, super->s_blocks_count,
		super->s_inode_size, 1024 << super->s_log_block_size,
		super->s_inodes_per_group, super->s_blocks_per_group,
		super->s_first_data_block);
}

int read_gd_table(int df, struct ext2_super_block *super, struct ext2_group_desc **gd_table, int *gd_table_size)
{
	int i;
	int nr_groups;

	int gd_table_offset = (super->s_first_data_block + 1) *	\
				(1024 << super->s_log_block_size);

	printf("Read group desc table at %d.\n", gd_table_offset);

	nr_groups = super->s_blocks_count / super->s_blocks_per_group + 1;
	*gd_table_size = nr_groups;

	gd_table = malloc(nr_groups * sizeof(struct ext2_group_desc));

	lseek(df, gd_table_offset, SEEK_SET);
	read(df, gd_table, nr_groups * sizeof(struct ext2_group_desc));

/*
	for (i = 0; i < nr_groups * sizeof(struct ext2_group_desc); i++) {
		if (!(i % 16)) printf("\n");
		printf("%02x ", *((unsigned char * )gd_table + i));
	}
	printf("\n");
*/
}

void print_gd_table(struct ext2_group_desc **gd_table, int nr_groups)
{
	int i;
	struct ext2_group_desc *gd = (struct ext2_group_desc *)gd_table;

	printf("Total: %d group desc.\n", nr_groups);
	for (i = 0; i < nr_groups; i++) {
		printf("\tgroup %d: %d free blocks.\n",
				i, gd->bg_free_blocks_count);
		gd++;
	}
}

int main(int argc, char *argv[])
{
	char *diskfile;
	int df;

	unsigned int nr_groups;

	struct ext2_super_block super;
	struct ext2_group_desc **gd_table;

	if (argc < 2 ) exit(-1);

	diskfile = argv[1];

	printf("Open %s as a ext2 filesystem.\n", diskfile);

	if (!(df = open(diskfile, O_RDONLY))) {
		printf("Error: open %s\n", diskfile);
		exit(-1);
	}

	if (read_super(df, &super)) {
		printf("Error: read super block\n");
		exit(-1);
	}
	print_super(&super);

	read_gd_table(df, &super, gd_table, &nr_groups);
	print_gd_table(gd_table, nr_groups);
}
