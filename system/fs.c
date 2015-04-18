#include <bufpool.h>
#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef FS
#include <fs.h>

static fsystem fsd;
int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

char block_cache[512];

#define SB_BLK 0
#define BM_BLK 1
#define RT_BLK 2

#define NUM_FD 16
filetable oft[NUM_FD];
int next_open_fd = 0;


#define INODES_PER_BLOCK (fsd.blocksz / sizeof(inode))
#define NUM_INODE_BLOCKS                            \
    (( (fsd.ninodes % INODES_PER_BLOCK) == 0) ?     \
            fsd.ninodes / INODES_PER_BLOCK :        \
            (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2

int fileblock_to_diskblock(int dev, int fd, int fileblock);

int mkfs(int dev, int num_inodes)
{
    int i;

    if (dev == 0) {
        fsd.nblocks = dev0_numblocks;
        fsd.blocksz = dev0_blocksize;
    } else {
        printf("Unsupported device\n");
        return SYSERR;
    }

    if (num_inodes < 1) {
        fsd.ninodes = DEFAULT_NUM_INODES;
    } else {
        fsd.ninodes = num_inodes;
    }

    i = fsd.nblocks;
    while ( (i % 8) != 0 ) {i++;}

    fsd.freemaskbytes = i / 8;

    if ((fsd.freemask = memget(fsd.freemaskbytes)) == (void *)SYSERR) {
        printf("mkfs memget failed.\n");
        return SYSERR;
    }

    /* zero the free mask */
    for(i=0;i<fsd.freemaskbytes;i++) {
        fsd.freemask[i] = '\0';
    }

    fsd.inodes_used = 0;

    /* write the fsystem block to SB_BLK, mark block used */
    setmaskbit(SB_BLK);
    bwrite(0, SB_BLK, 0, &fsd, sizeof(fsystem));

    /* write the free block bitmask in BM_BLK, mark block used */
    setmaskbit(BM_BLK);
    bwrite(0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

    return 1;
}

int fileblock_to_diskblock(int dev, int fd, int fileblock)
{
    if (fileblock >= INODEBLOCKS - 2) {
        printf("No indirect block support\n");
        return SYSERR;
    }

    return oft[fd].in.blocks[fileblock];
}

/* read in an inode and fill in the pointer */
int get_inode_by_num(int dev, int inode_number, inode *in)
{
    int bl, inn;
    int inode_off;

    if (dev != 0) {
        printf("Unsupported device\n");
        return SYSERR;
    }
    if (inode_number > fsd.ninodes) {
        printf("get_inode_by_num: inode %d out of range\n", inode_number);
        return SYSERR;
    }

    bl = inode_number / INODES_PER_BLOCK;
    inn = inode_number % INODES_PER_BLOCK;
    bl += FIRST_INODE_BLOCK;

    inode_off = inn * sizeof(inode);

    /*
       printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
       printf("inn*sizeof(inode): %d\n", inode_off);
       */

    bread(0, bl, 0, &block_cache[0], fsd.blocksz);
    memcpy(in, &block_cache[inode_off], sizeof(inode));

    return OK;
}

int put_inode_by_num(int dev, int inode_number, inode *in)
{
    int bl, inn;

    if (dev != 0) {
        printf("Unsupported device\n");
        return SYSERR;
    }
    if (inode_number > fsd.ninodes) {
        printf("put_inode_by_num: inode %d out of range\n", inode_number);
        return SYSERR;
    }

    bl = inode_number / INODES_PER_BLOCK;
    inn = inode_number % INODES_PER_BLOCK;
    bl += FIRST_INODE_BLOCK;

    /*
    printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
    */

    bread(0, bl, 0, block_cache, fsd.blocksz);
    memcpy(&block_cache[(inn*sizeof(inode))], in, sizeof(inode));
    bwrite(0, bl, 0, block_cache, fsd.blocksz);

    return OK;
}

/* specify the block number to be set in the mask */
int setmaskbit(int b)
{
    int mbyte, mbit;
    mbyte = b / 8;
    mbit = b % 8;

    fsd.freemask[mbyte] |= (0x80 >> mbit);
    return OK;
}

/* specify the block number to be read in the mask */
int getmaskbit(int b)
{
    int mbyte, mbit;
    mbyte = b / 8;
    mbit = b % 8;

    return ( ( (fsd.freemask[mbyte] << mbit) & 0x80 ) >> 7);
}

/* specify the block number to be unset in the mask */
int clearmaskbit(int b)
{
    int mbyte, mbit, invb;
    mbyte = b / 8;
    mbit = b % 8;

    invb = ~(0x80 >> mbit);
    invb &= 0xFF;

    fsd.freemask[mbyte] &= invb;
    return OK;
}

/* This is maybe a little overcomplicated since the lowest-numbered block is
 * indicated in the high-order bit.  Shift the byte by j positions to make the
 * match in bit7 (the 8th bit) and then shift that value 7 times to the
 * low-order bit to print.  Yes, it could be the other way...  */
void printfreemask(void)
{
    int i,j;

    for (i=0; i < fsd.freemaskbytes; i++)
    {
        for (j=0; j < 8; j++)
        {
            printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
        }
        if ( (i % 8) == 7 )
        {
            printf("\n");
        }
    }
    printf("\n");
}

void print_dirent(dirent *ent)
{
    printf("inode_num: %d\n", ent->inode_num);
    printf("dir name:  %s\n", ent->name);
}

void print_directory(directory *dir)
{
    printf("Directory with %d entries:\n", dir->numentries);
    int i;
    for (i = 0; i < dir->numentries; i++)
    {
        print_dirent(&dir->entry[i]);
    }
}

int fs_initialized(void)
{
    return fsd.nblocks != 0;
}

directory *get_root_dir(void)
{
    return &(fsd.root_dir);
}

int get_directory_blocks(void)
{
    if (!fs_initialized())
    {
        printf("get_directory_blocks: file system is not initialized.\n");
        return 0;
    }

    // ops... no ceil in stdlib
    // return ceil(sizeof(directory) / fsd.blocksz);
    int bs = sizeof(directory) / fsd.blocksz;
    if (sizeof(directory) % fsd.blocksz != 0) bs++;
    return bs;
}

int get_block_size(void)
{
    if (!fs_initialized())
    {
        printf("get_block_size: file system is not initialized.\n");
        return 0;
    }

    return fsd.blocksz;
}

int load_directory(int *blocks, directory *output)
{
    int directory_blocks = get_directory_blocks();
    int i;
    for (i = 0; i < directory_blocks; i++)
    {
        bread(0, blocks[i], 0, (void*)output + (i * get_block_size()), get_block_size());
    }
    return 0;
}

#endif /* FS */
