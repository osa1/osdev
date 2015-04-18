#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fs.h>

#ifdef FS

int fclose(int fd)
{
    printf("fclose: not implemented yet.\n");
    return SYSERR;
}

#endif /* FS */
