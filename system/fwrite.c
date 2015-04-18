#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fs.h>

#ifdef FS

int fwrite(int fd, void *buf, int nbytes)
{
    printf("fwrite: not implemented yet.\n");
    return SYSERR;
}

#endif /* FS */
