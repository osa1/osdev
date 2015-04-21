#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fs.h>

#ifdef FS

extern filedesc oft[NUM_FD];

int fseek(int fd, int offset)
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

    int new_cursor = desc->cursor + offset;
    desc->cursor += offset;

    // TODO: Are there any other conditions that we might want to check?
    if (new_cursor < 0)
    {
        printf("fseek: Can't move cursor to %d.\n", new_cursor);
        return SYSERR;
    }

    return OK;
}

#endif /* FS */
