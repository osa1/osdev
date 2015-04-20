#include <fs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef FS

extern fsystem fsd;
extern filedesc oft[NUM_FD];
extern int dev0_blocksize;

int fwrite(int fd, void *buf, int nbytes)
{
    if (fd < 0 || fd >= NUM_FD)
    {
        printf("fwrite: File descriptor out of range: %d.\n", fd);
        return SYSERR;
    }

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
    int df = 0;
    while (nbytes != 0)
    {
        printf("writing %d bytes.\n", nbytes);
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
            desc->in.blocks[cursor_block] = block_idx;
            desc->in.size += dev0_blocksize;
        }

        // sanity check: inode block should be allocated
        if (!checkbit(fsd.freemask, block_idx))
        {
            printf("fwrite: File has unallocated block.\n");
            return SYSERR;
        }

        if (bwrite(0, block_idx, cursor_block_offset, buf,
                    MIN(dev0_blocksize - cursor_block_offset, nbytes)) == SYSERR) {
            printf("fwrite: bwrite failed.\n");
            return SYSERR;
        }

        df = MIN(dev0_blocksize - cursor_block_offset, nbytes);
        ret += df;

        // drop written parts
        nbytes -= df;
        buf += df;
        cursor_block++; // continue writing to next block
        cursor_block_offset = 0; // we start writing to next block from the beginning.
    }

    desc->cursor += ret;
    return ret;
}

#endif /* FS */
