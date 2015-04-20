#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fs.h>

#ifdef FS

extern filedesc oft[NUM_FD];

/**
 * Search the file system, starting with given directory, for given file path's
 * parent. Input is always interpreted as relative path to the given directory.
 * (even if it starts with '/')
 *
 * (Doesn't check if file system is initialized)
 *
 * Returns negative in case of an error.
 */
int get_parent_directory(directory *cur_dir, char *path, directory *output)
{
    // Skip initial slash, as noted in the documentation.
    if (*path == '/') path++;

    char *dir_sep = strchr(path, '/');
    if (dir_sep != NULL)
    {
        // we have a directory to traverse, find it in cur_dir's entries
        *dir_sep = '\0';
        int i;
        for (i = 0; i < cur_dir->numentries; i++)
        {
            dirent entry = cur_dir->entry[i];
            if (strcmp(entry.name, path) == 0)
            {
                // Found a matching name. Read the INode, make sure the entry
                // is a directory, load directory struct, and continue
                // seraching from new directory.

                // again, making up a dev number here. this assignment sucks.
                inode entry_node;
                if (get_inode_by_num(0, entry.inode_num, &entry_node) == SYSERR)
                {
                    printf("get_parent_directory: Can't read entry inode.\n");
                    *dir_sep = '/';
                    return SYSERR;
                }

                if (entry_node.type != INODE_TYPE_DIR)
                {
                    // Not a directory node, skip.
                    continue;
                }

                // Found what we've been looking for, read directory struct
                *dir_sep = '/';

                if (load_directory(entry_node.blocks, cur_dir) == SYSERR)
                {
                    printf("get_parent_directory: Can't load entry's blocks.\n");
                    return SYSERR;
                }

                return get_parent_directory(cur_dir, dir_sep + 1, output);
            }
        }

        printf("Can't find directory %s in the tree.\n", path);
        *dir_sep = '/';
        return SYSERR;
    }
    else
    {
        *dir_sep = '/';
        *output = *cur_dir;
        return OK;
    }
}

/**
 * NB: This also works for reading directory inodes.
 */
int get_file_inode(directory *cur_dir, char *filename, inode *output, int type)
{
    // sanity check
    if (strchr(filename, '/') != NULL)
    {
        printf("get_file_inode: File name has '/' in it.\n");
        return SYSERR;
    }

    int i;
    for (i = 0; i < cur_dir->numentries; i++)
    {
        dirent entry = cur_dir->entry[i];
        if (strcmp(entry.name, filename) == 0)
        {
            if (get_inode_by_num(0, entry.inode_num, output) == SYSERR)
                return SYSERR;

            if (output->type == type)
                return OK;
        }
    }

    // printf("get_file_inode: File isn't in directory.\n");
    return SYSERR;
}

/*
 * I think corresponding POSIX function for this is `creat()`. This is from
 * it's man page:
 *
 * > creat() is equivalent to open() with flags equal to O_CREAT|O_WRONLY|O_TRUNC
 *
 * So we should use O_WRONLY int fd table entry.
 *
 */
int fcreate(char *path, fcreate_mode mode)
{
    if (!fs_initialized())
    {
        printf("fcreate: file system is not initialized. use mkfs().\n");
        return SYSERR;
    }

    if (mode != FCREATE_FILE && mode != FCREATE_DIR)
    {
        printf("fcreate: mode argument is invalid: %d\n", mode);
        printf("It should be %s or %s.\n", "FCREATE_FILE", "FCREATE_DIR");
        return SYSERR;
    }

    int fd_entry = find_closed_fd();
    if (fd_entry == SYSERR)
    {
        printf("fcreate: Can't open a new file, close some file descriptors first.\n");
        return SYSERR;
    }

    directory dir;
    if (get_parent_directory(get_root_dir(), path, &dir) == SYSERR)
    {
        printf("fcreate: Can't find parent directory.\n");
        return SYSERR;
    }

    char *filename = strrchr(path, '/');
    if (filename == NULL)
        filename = path;
    else
        filename++; // skip '/'

    inode *fd_inode = &oft[fd_entry].in;
    if (mode == FCREATE_FILE
            && get_file_inode(&dir, filename, fd_inode, INODE_TYPE_FILE) != SYSERR)
    {
        printf("fcreate: File already exists.\n");
        return SYSERR;
    }
    else if (get_file_inode(&dir, filename, fd_inode, INODE_TYPE_DIR) != SYSERR)
    {
        printf("fcreate: Directory already exists.\n");
        return SYSERR;
    }

    // Allocate INode for the file/directory
    int inode_idx = allocate_inode();
    if (inode_idx == SYSERR)
    {
        printf("fcreate: Can't allocate inode for file.\n");
        return SYSERR;
    }

    if (get_inode_by_num(0, inode_idx, fd_inode) == SYSERR)
        return SYSERR;

    printf("Updating inode\n");
    if (mode == FCREATE_FILE)
        fd_inode->type = INODE_TYPE_FILE;
    else
        fd_inode->type = INODE_TYPE_DIR;
    fd_inode->size = 0;
    memset(fd_inode->blocks, 0, INODEBLOCKS * sizeof(int));

    // Write updated inode
    if (put_inode_by_num(0, inode_idx, fd_inode) == SYSERR)
        return SYSERR;

    // Update the file descriptor
    oft[fd_entry].state = O_WRONLY;
    oft[fd_entry].cursor = 0;

    return fd_entry;
}

#endif /* FS */
