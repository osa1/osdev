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

    if (desc->in.type != INODE_TYPE_FILE && desc->in.type != INODE_TYPE_DIR)
    {
        printf("fwrite: BUG: file descriptor inode type is not %s or %s.\n",
                "INODE_TYPE_FILE", "INODE_TYPE_DIR");
        return SYSERR;
    }

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
        int block_idx = desc->in.blocks[cursor_block];

        if (block_idx <= 0)
        {
            // Need to allocate a block
            block_idx = allocate_block();
            printf("allocated new block: %d\n", block_idx);
            if (block_idx == SYSERR)
            {
                printf("fwrite: Block allocation failed.\n");
                return SYSERR;
            }
            desc->in.blocks[cursor_block] = block_idx;
        }

        // sanity check: inode block should be allocated
        if (!checkbit(fsd.freemask, block_idx))
        {
            printf("fwrite: File has unallocated block, %d.\n", block_idx);
            return SYSERR;
        }

        if (bwrite(0, block_idx, cursor_block_offset, buf, // FIXME: This is overriding inode blocks
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
    if (desc->cursor > desc->in.size)
        desc->in.size = desc->cursor;

    return ret;
}

#endif /* FS */
