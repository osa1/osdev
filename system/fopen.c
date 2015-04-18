#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fs.h>

#ifdef FS

int fopen(char *filename, int flags)
{
    printf("fopen: not implemented yet.\n");
    return SYSERR;
}

#endif /* FS */
