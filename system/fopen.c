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

    // printf("Parent directory:\n");
    // print_directory(&parent_dir);

    // now that we have parent directory, drop slashes and search the file in
    // directory
    char *filename = strrchr(path, '/');
    if (filename == NULL)
        filename = path;
    else
        filename++; // skip '/'

    if (get_file_inode(&parent_dir, filename, &oft[fd].in, INODE_TYPE_FILE) == SYSERR)
    {
        printf("fopen: Can't get file inode: %s\n", filename);
        return SYSERR;
    }
    oft[fd].state = flags;
    oft[fd].cursor = 0;

    return fd;
}

#endif /* FS */
