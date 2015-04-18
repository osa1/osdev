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

int bread(int dev, int block, int offset, void *buf, int len)
{
    char *bbase;

    if (dev != 0)
    {
        printf("bread: Unsupported device.\n");
        return SYSERR;
    }

    // osa: Added this as a sanity check.
    if (offset + len > dev0_blocksize)
    {
        printf("bread: Can't read from next block.\n");
        return SYSERR;
    }

    if (offset >= dev0_blocksize)
    {
        printf("bread: Bad offset.\n");
        return SYSERR;
    }

    bbase = &dev0_blocks[block * dev0_blocksize];

    memcpy(buf, bbase + offset, len);

    return OK;
}

int bwrite(int dev, int block, int offset, void *buf, int len)
{
    char *bbase;

    if (dev != 0)
    {
        printf("bwrite: Unsupported device.\n");
        return SYSERR;
    }

    // osa: Added this as a sanity check.
    if (offset + len > dev0_blocksize)
    {
        printf("bwrite: Can't write to next block.\n");
        return SYSERR;
    }

    if (offset >= dev0_blocksize)
    {
        printf("bwrite: Bad offset.\n");
        return SYSERR;
    }

    bbase = &dev0_blocks[block * dev0_blocksize];
    memcpy(bbase + offset, buf, len);
    return OK;
}

/**
 * Return corresponding block index for given address.
 */
int offset_block_num(int offset)
{
    int i = offset / dev0_numblocks;
    if (offset % dev0_numblocks != 0) i++;
    return i;
}

#endif /* FS */
