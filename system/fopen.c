#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fs.h>

#ifdef FS

extern fsystem fsd;
extern filedesc oft[NUM_FD];

int fopen(char *path, int flags)
{
    if (!fs_initialized())
    {
        printf("fopen: file system is not initialized. use mkfs().\n");
        return SYSERR;
    }

    int fd;
    for (fd = 0; fd < NUM_FD; fd++)
        if (oft[fd].state == O_CLOSED)
            break;

    if (fd >= NUM_FD)
    {
        printf("fopen: Can't open file, reached file descriptor limit (%d). "
               "Close some files first.\n", NUM_FD);
        return SYSERR;
    }

    directory parent_dir;
    if (get_parent_directory(&fsd.root_dir, path, &parent_dir) == SYSERR)
    {
        printf("fopen: Can't get parent directory of %s\n.", path);
        return SYSERR;
    }

    // now that we have parent directory, drop slashes and search the file in
    // directory
    char *filename = strrchr(path, '/');
    if (filename == NULL)
        filename = path;
    else
        filename++; // skip '/'

    inode file_inode;
    if (get_file_inode(&parent_dir, filename, &file_inode, INODE_TYPE_FILE) == SYSERR)
    {
        printf("fopen: Can't get file inode: %s\n", filename);
        return SYSERR;
    }

    // Make sure the file is not already open
    int i;
    for (i = 0; i < NUM_FD; i++)
        if (oft[i].state != O_CLOSED && oft[i].in.inode_idx == file_inode.inode_idx)
        {
                printf("fopen: Can't open the file, it is already open.\n");
                return SYSERR;
        }

    oft[fd].in = file_inode;
    oft[fd].state = flags;
    oft[fd].cursor = 0;

    return fd;
}

#endif /* FS */
