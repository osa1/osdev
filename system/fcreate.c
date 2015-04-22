#include <fs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef FS

extern fsystem fsd;
extern filedesc oft[NUM_FD];

/*
 * I think corresponding POSIX function for this is `creat()`. This is from
 * it's man page:
 *
 * > creat() is equivalent to open() with flags equal to O_CREAT|O_WRONLY|O_TRUNC
 *
 * // So we should use O_WRONLY int fd table entry. <- NO
 * According to the provided test file, we should be able to read from the
 *
 * created file. So I'm using RW mode.
 *
 * This only creates files, for creating directories, see `mkdir()`.
 *
 */
int fcreate(char *path)
{
    if (!fs_initialized())
    {
        printf("fcreate: file system is not initialized. use mkfs().\n");
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
        printf("fcreate: Can't find parent directory of %s.\n", path);
        return SYSERR;
    }

    if (dir.numentries == DIRECTORY_SIZE)
    {
        printf("fcreate: Can't create file, directory is full.\n");
        return SYSERR;
    }

    char *filename = get_filename(path);

    inode fd_inode;
    if (get_file_inode(&dir, filename, &fd_inode, INODE_TYPE_FILE) != SYSERR)
    {
        printf("fcreate: File already exists.\n");
        return SYSERR;
    }

    // Allocate INode for the file
    int inode_idx = allocate_inode();
    printf("fcreate: allocated inode index: %d\n", inode_idx);
    if (inode_idx == SYSERR)
    {
        printf("fcreate: Can't allocate inode for file.\n");
        return SYSERR;
    }

    if (get_inode_by_num(0, inode_idx, &fd_inode) == SYSERR)
        return SYSERR;

    printf("Updating inode\n");
    fd_inode.type = INODE_TYPE_FILE;
    fd_inode.inode_idx = inode_idx;
    fd_inode.size = 0;
    memset(fd_inode.blocks, 0, INODEBLOCKS * sizeof(int));

    // Write updated inode
    if (put_inode_by_num(0, inode_idx, &fd_inode) == SYSERR)
        return SYSERR;

    // Update the file descriptor
    oft[fd_entry].state = O_RDWR;
    oft[fd_entry].cursor = 0;
    memcpy(&oft[fd_entry].in, &fd_inode, sizeof(inode));

    // Update directory
    if (dir.inode_num == -1)
    {
        printf("fcreate: Updating root directory.\n");
        fsd.root_dir.entry[fsd.root_dir.numentries].inode_num = inode_idx;
        // FIXME: Make sure filename is small enough
        strcpy(fsd.root_dir.entry[fsd.root_dir.numentries].name, filename);
        fsd.root_dir.numentries++;
    }
    else
    {
        dir.entry[dir.numentries].inode_num = inode_idx;
        // FIXME: Make sure filename is small enough
        strcpy(dir.entry[dir.numentries].name, filename);
        dir.numentries++;
        // FIXME: update inode blocks
    }

    return fd_entry;
}

#endif /* FS */
