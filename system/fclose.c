#include <fs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef FS

extern filedesc oft[NUM_FD];

int fclose(int fd)
{
    if (fd < 0 || fd >= NUM_FD)
    {
        printf("fclose: File descriptor out of range: %d.\n", fd);
        return SYSERR;
    }

    // TODO: Normally, we'd need to check caches and update the drive
    // accordingly, but since we don't use any caches in this implementation,
    // we can just remove the `oft` entry.
    //
    // Make sure we really don't need to implement a cache in this assignment.
    oft[fd].state = O_CLOSED;

    return OK;
}

#endif /* FS */
