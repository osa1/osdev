#include <fs.h>
#include <stdio.h>
#include <string.h>

/**
 * @ingroup shell
 *
 * A simple version of ls command.
 */

shellcmd xsh_ls(int nargs, char *args[])
{
    char *path = "/";
    if (nargs == 2 && strcmp(args[1], "--help") == 0)
    {
        printf("Usage:\n");
        printf("Shows contents of given directory. "
               "Shows contents of root if no arguments are given.\n");
        return 1;
    }

    if (!fs_initialized())
    {
        printf("ls: file system is not initialized. Use mkfs().\n");
        return SYSERR;
    }

    if (nargs == 2) path = args[1];

    directory dir;
    if (get_parent_directory(get_root_dir(), path, &dir) == SYSERR)
    {
        printf("ls: Can't get parent directory of %s.\n", path);
        return SYSERR;
    }

    char *filename = get_filename(path);
    printf("filename: \"%s\"\n", filename);
    print_directory(&dir);
    printf("dir.inode_num = %d\n", dir.inode_num);

    if (*filename == '\0' && dir.inode_num == -1)
    {
        printf("printing parent directory\n");
        print_directory(&dir);
    }
    else
    {
        printf("not parent\n");
        inode in;
        if (get_file_inode(&dir, filename, &in, INODE_TYPE_DIR) == SYSERR)
        {
            printf("Can't get inode of file \"%s\".\n", filename);
            return SYSERR;
        }

        if (load_directory(in.blocks, &dir) == SYSERR)
        {
            printf("Can't load contents of directory.\n");
            return SYSERR;
        }

        print_directory(&dir);
    }

    return OK;
}
