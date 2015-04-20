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

#define NUM_FD 16
filetable oft[NUM_FD];
int next_open_fd = 0;

// number of blocks that fsystem struct uses
int fsystem_blocks;

// number of blocks freemask uses
int freemask_blocks;

// number of blocks inode_bitfield uses
int inode_bitfield_blocks;

// first inode block number
int inodes_start;

// first file block number
int blocks_start;

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
        printf("mkfs memget failed. (allocating freemaskbytes)\n");
        return SYSERR;
    }
    memset(fsd.freemask, 0, fsd.freemaskbytes);

    fsd.inodes_used = 0;
    i = fsd.ninodes;
    while ( (i % 8) != 0 ) {i++;}
    if ((fsd.inode_bitfield = memget(i / 8)) == (void*)SYSERR) {
        printf("mkfs memget failed. (allocating inode bitfield)\n");
        return SYSERR;
    }
    memset(fsd.inode_bitfield, 0, i / 8);

    fsystem_blocks = offset_block_num(sizeof(fsystem));
    freemask_blocks = offset_block_num(fsd.freemaskbytes);
    inode_bitfield_blocks = offset_block_num(i / 8);

    printf("Writing initial blocks to the drive.\n"
           "fsystem:\t%d blocks\nfreemask:\t%d blocks\ninode bitfield:\t%d blocks\n",
           fsystem_blocks, freemask_blocks, inode_bitfield_blocks);

    // Write fsystem to the drive
    for (i = 0; i < fsystem_blocks; i++)
    {
        int len = dev0_blocksize;
        if (i == fsystem_blocks - 1)
            len = sizeof(fsystem) % dev0_blocksize;

        if (bwrite(0, 0 + i, 0, ((void*)&fsd) + i * dev0_blocksize, len) == SYSERR)
        {
            printf("mkfs: Can't write fsystem to block %d.\n", i);
            return SYSERR;
        }
    }

    // Write freemask to the drive
    for (i = 0; i < freemask_blocks; i++)
    {
        int len = dev0_blocksize;
        if (i == freemask_blocks - 1)
            len = sizeof(fsystem) % dev0_blocksize;

        if (bwrite(0, fsystem_blocks + i,
                    0, ((void*)&fsd.freemask) + i * dev0_blocksize, len) == SYSERR)
        {
            printf("mkfs: Can't write freemask to block %d.\n",
                    fsystem_blocks + i);
            return SYSERR;
        }
    }

    // Write inode_bitfield to the drive
    for (i = 0; i < inode_bitfield_blocks; i++)
    {
        int len = dev0_blocksize;
        if (i == inode_bitfield_blocks - 1)
            len = sizeof(fsystem) % dev0_blocksize;

        if (bwrite(0, fsystem_blocks + freemask_blocks + i,
                    0, ((void*)&fsd.inode_bitfield) + i * dev0_blocksize, len) == SYSERR)
        {
            printf("mkfs: Can't write inode_bitfield to block %d.\n",
                    fsystem_blocks + freemask_blocks + i);
            return SYSERR;
        }
    }

    inodes_start = fsystem_blocks + freemask_blocks + inode_bitfield_blocks;
    blocks_start = inodes_start + (dev0_blocksize / sizeof(inode));

    printf("inodes_start:\t%d\n"
           "blocks_start:\t%d\n"
           "total blocks:\t%d\n", inodes_start, blocks_start, dev0_numblocks);

    return OK;
}

bool checkbit(char *bitfield, int idx)
{
    int byte, bit;
    byte = idx / 8;
    bit  = idx % 8;

    return ((bitfield[byte] & 0x80 /* 0b10000000 */ >> bit) != 0);
}

void setbit(char *bitfield, int idx)
{
    int byte, bit;
    byte = idx / 8;
    bit  = idx % 8;

    bitfield[byte] |= (0x80 >> bit);
}

int allocate_inode()
{
    char *bitfield = fsd.inode_bitfield;
    int i;
    for (i = 0; i < fsd.ninodes; i++)
        if (!checkbit(bitfield, i))
            return i;

    // Sanity check
    if (fsd.ninodes != fsd.inodes_used)
        printf("BUG: allocate_inode can't allocate, but inodes_used != ninodes\n");

    return SYSERR;
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
    // int inode_first_block = fsystem_blocks + freemask_blocks + inode_bitfield_blocks;

    return SYSERR; // TODO
}

int put_inode_by_num(int dev, int inode_number, inode *in)
{
    return SYSERR; // TODO
}

/* specify the block number to be set in the mask */
int setmaskbit(int b)
{
    setbit(fsd.freemask, b);
    return OK;
}

/* specify the block number to be read in the mask */
int getmaskbit(int b)
{
    return checkbit(fsd.freemask, b);
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
