#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fs.h>

#ifdef FS

extern fsystem fsd;
extern filedesc oft[NUM_FD];
extern int dev0_blocksize;

#define MIN(a, b) (a < b ? a : b)

int fwrite(int fd, void *buf, int nbytes)
{
    filedesc *desc = &oft[fd];

    if (desc->state == O_CLOSED)
    {
        printf("fwrite: Can't write to closed file.\n");
        return SYSERR;
    }

    if (desc->state == O_RDONLY)
    {
        printf("fwrite: Can't write to a read-only file.\n");
        return SYSERR;
    }

    // Which inode block should we start writing to?
    int cursor_block = desc->cursor / dev0_blocksize;
    // What is the offset in block?
    int cursor_block_offset = desc->cursor % dev0_blocksize;

    int end_offset = desc->cursor + nbytes;
    int cursor_last_block = end_offset / dev0_blocksize;
    if (end_offset % dev0_blocksize != 0) cursor_last_block++;

    if (cursor_last_block >= INODEBLOCKS)
    {
        printf("fwrite: Write operation exceeds maximum file size.\n");
        return SYSERR;
    }

    int ret = 0;
    while (nbytes != 0)
    {
        int block_idx = desc->in.blocks[cursor_block];

        if (block_idx <= 0)
        {
            // Need to allocate a block
            block_idx = allocate_block();
            if (block_idx == SYSERR)
            {
                printf("fwrite: Block allocation failed.\n");
                return SYSERR;
            }
        }

        // sanity check: inode block should be allocated
        if (!checkbit(fsd.freemask, block_idx))
        {
            printf("fwrite: File has unallocated block.\n");
            return SYSERR;
        }

        if (bwrite(0, block_idx, cursor_block_offset, buf,
                    MIN(dev0_blocksize - cursor_block_offset, nbytes))
                == SYSERR) {
            printf("fwrite: bwrite failed.\n");
            return SYSERR;
        }
        ret += MIN(dev0_blocksize - cursor_block_offset, nbytes);

        // drop written parts
        nbytes -= MIN(dev0_blocksize - cursor_block_offset, nbytes);
        buf += dev0_blocksize - cursor_block_offset;
        cursor_block++; // continue writing to next block
        cursor_block_offset = 0; // we start writing to next block from the beginning.
    }

    return ret;
}

#endif /* FS */
