#include <fs.h>
#include <stdio.h>
#include <string.h>

extern char *dev0_blocks;

/**
 * @ingroup shell
 *
 * A shell command that initializes file system.
 */
shellcmd xsh_initfs(int nargs, char *args[])
{
    if (nargs != 1)
    {
        printf("initfs doesn't get any arguments.\n");
        return 1;
    }

    bool initialized = FALSE;
    if (dev0_blocks == NULL)
    {
        mkbsdev(0, MDEV_BLOCK_SIZE, MDEV_NUM_BLOCKS);
        initialized = TRUE;
    }

    if (!fs_initialized())
    {
        mkfs(0, DEFAULT_NUM_INODES);
        initialized = TRUE;
    }

    if (!initialized)
    {
        printf("File system is already initialized.\n");
        return 1;
    }

    printf("File system initialized.\n");
    return 0;
}
