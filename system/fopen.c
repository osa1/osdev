#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fs.h>

#ifdef FS

extern fsystem fsd;

int fopen(char *path, int flags)
{
    if (!fs_initialized())
    {
        printf("fopen: file system is not initialized. use mkfs().\n");
        return SYSERR;
    }

    directory parent_dir;
    if (get_parent_directory(&fsd.root_dir, path, &parent_dir) == SYSERR)
    {
        printf("fopen: Can't get parent directory of %s\n.", path);
        return SYSERR;
    }

    printf("Parent directory:\n");
    print_directory(&parent_dir);

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

    printf("fopen: not implemented yet.\n");
    return SYSERR;
}

#endif /* FS */
