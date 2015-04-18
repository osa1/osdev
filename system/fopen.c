#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fs.h>

#ifdef FS

int fopen(char *filename, int flags)
{
    if (!fs_initialized())
    {
        printf("fopen: file system is not initialized. use mkfs().\n");
        return SYSERR;
    }

    // printf("fsd.nblocks: %d\n", fsd.nblocks);
    // printf("fsd.blocksz: %d\n", fsd.blocksz);
    // printf("fsd.ninodes: %d\n", fsd.ninodes);
    // printf("fsd.inodes_used: %d\n", fsd.inodes_used);
    printf("fopen: not implemented yet.\n");
    return SYSERR;
}

#endif /* FS */
