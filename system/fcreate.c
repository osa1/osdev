#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fs.h>

#ifdef FS

int fcreate(char *filename, int mode)
{
    printf("fcreate: not implemented yet.\n");
    return SYSERR;
}

#endif /* FS */
