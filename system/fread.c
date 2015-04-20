#include <fs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef FS

extern fsystem fsd;
extern filedesc oft[NUM_FD];
extern int dev0_blocksize;

int fread(int fd, void *buf, int nbytes)
{
    if (fd < 0 || fd >= NUM_FD)
    {
        printf("fread: File descriptor out of range: %d.\n", fd);
        return SYSERR;
    }

    filedesc *desc = &oft[fd];

    if (desc->state == O_CLOSED)
    {
        printf("fread: Can't read a closed file.\n");
        return SYSERR;
    }

    if (desc->state == O_WRONLY)
    {
        printf("fread: File is write only.\n");
        return SYSERR;
    }

    if (desc->cursor + nbytes >= desc->in.size)
    {
        printf("fread: Trying to read after end of file.\n"
               "(diff: %d)\n", desc->cursor + nbytes - desc->in.size);
        return SYSERR;
    }

    int cursor_block = desc->cursor / dev0_blocksize;
    int cursor_block_offset = desc->cursor % dev0_blocksize;

    int ret = 0;
    int df = 0;
    while (nbytes != 0)
    {
        printf("reading %d bytes.\n", nbytes);

        int block_idx = desc->in.blocks[cursor_block];

        if (block_idx <= 0)
        {
            printf("fread: This is probably a bug, block_idx <= 0, but region is in range.\n");
            return SYSERR;
        }

        if (bread(0, block_idx, cursor_block_offset, buf,
                    MIN(dev0_blocksize - cursor_block_offset, nbytes)) == SYSERR)
        {
            return SYSERR;
        }

        df = MIN(dev0_blocksize - cursor_block_offset, nbytes);
        ret += df;

        // Same stuff we do in fwrite
        nbytes -= df;
        buf += df;
        cursor_block++;
        cursor_block_offset = 0;
    }

    desc->cursor += ret;
    return ret;
}

#endif /* FS */
