/* This file implements the memory "block store" for use by the in-memory
 * filesystem */

#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <bufpool.h>

extern int dev0_numblocks;
extern int dev0_blocksize;
extern char *dev0_blocks;

#ifdef FS
#include <fs.h>

int mkbsdev(int dev, int blocksize, int numblocks)
{
    if (dev != 0)
    {
        printf("Unsupported device: %d\n", dev);
        return SYSERR;
    }

    dev0_blocksize = blocksize == 0 ? MDEV_BLOCK_SIZE : blocksize;
    dev0_numblocks = numblocks == 0 ? MDEV_NUM_BLOCKS : numblocks;

    if ( (dev0_blocks = memget(dev0_numblocks * dev0_blocksize)) == (void *)SYSERR )
    {
        printf("mkbsdev memget failed\n");
        return SYSERR;
    }

    return OK;
}

// TODO: NB: This is broken, you can read multiple blocks at once by keeping
//           len big. The problem is, you can't know what's in next block.
int bread(int dev, int block, int offset, void *buf, int len)
{
    char *bbase;

    if (dev != 0)
    {
        printf("Unsupported device\n");
        return SYSERR;
    }

    if (offset >= dev0_blocksize)
    {
        printf("Bad offset\n");
        return SYSERR;
    }

    bbase = &dev0_blocks[block * dev0_blocksize];

    memcpy(buf, bbase + offset, len);

    return OK;
}

/* WARNING: This doesn't check the len, we may end up overriding other blocks
 * I'm wondering if this is the intended behavior. */
int bwrite(int dev, int block, int offset, void *buf, int len)
{
    char *bbase;

    if (dev != 0)
    {
        printf("Unsupported device\n");
        return SYSERR;
    }
    if (offset >= dev0_blocksize)
    {
        printf("Bad offset\n");
        return SYSERR;
    }

    bbase = &dev0_blocks[block * dev0_blocksize];
    memcpy((bbase+offset), buf, len);
    return OK;
}

#endif /* FS */
