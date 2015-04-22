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

    if (oft[fd].in.type != INODE_TYPE_FILE && oft[fd].in.type != INODE_TYPE_DIR)
    {
        printf("fclose: BUG: file descriptor inode type is not %s or %s.\n",
                "INODE_TYPE_FILE", "INODE_TYPE_DIR");
        return SYSERR;
    }

    oft[fd].state = O_CLOSED;
    if (put_inode_by_num(0, &oft[fd].in) == SYSERR)
    {
        printf("fclose: Can't write inode back to disk.\n");
        return SYSERR;
    }

    return OK;
}

#endif /* FS */
