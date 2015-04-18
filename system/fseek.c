#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fs.h>

#ifdef FS

int fseek(int fd, int offset)
{
    printf("fseek: not implemented yet.\n");
    return SYSERR;
}

#endif /* FS */
