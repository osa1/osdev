#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fs.h>

#ifdef FS

int fread(int fd, void *buf, int nbytes)
{
    printf("fread: not implemented yet.\n");
    return SYSERR;
}

#endif /* FS */
