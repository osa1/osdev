#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fs.h>

#ifdef FS

extern filedesc oft[NUM_FD];

int fseek(int fd, int offset)
{
    filedesc desc = oft[fd];

    int new_cursor = desc.cursor + offset;
    desc.cursor += offset;

    // TODO: Are there any other conditions that we might want to check?
    if (new_cursor < 0)
    {
        printf("fseek: Can't move cursor to %d.\n", new_cursor);
        return SYSERR;
    }

    return OK;
}

#endif /* FS */
