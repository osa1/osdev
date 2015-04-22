#include <fs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef FS

extern fsystem fsd;

int mkdir(char *path)
{
    if (!fs_initialized())
    {
        printf("mkdir: file system is not initialized. use mkfs().\n");
        return SYSERR;
    }

    directory parent_dir;
    if (get_parent_directory(get_root_dir(), path, &parent_dir) == SYSERR)
    {
        printf("mkdir: Can't find parent directory of %s.\n", path);
        return SYSERR;
    }

    if (parent_dir.numentries == DIRECTORY_SIZE)
    {
        printf("mkdir: Can't create file, directory is full.\n");
        return SYSERR;
    }

    char *filename = strrchr(path, '/');
    if (filename == NULL)
        filename = path;
    else
        filename++; // skip '/'

    inode dir_inode;
    if (get_file_inode(&parent_dir, filename, &dir_inode, INODE_TYPE_DIR) != SYSERR)
    {
        printf("mkdir: Directory already exists.\n");
        return SYSERR;
    }

    // Allocate INode for the directory
    int inode_idx = allocate_inode();
    if (inode_idx == SYSERR)
    {
        printf("mkdir: Can't allocate inode for directory.\n");
        return SYSERR;
    }

    // TODO: Don't forget to write updated dir entry to it's blocks.
    dirent *entry = &parent_dir.entry[parent_dir.numentries++];
    memcpy(entry->name, filename, FILENAMELEN + 1); // FIXME: This may end up with segfault
    entry->inode_num = inode_idx;

    // Update parent directory
    if (parent_dir.inode_num == -1)
    {
        printf("mkdir: Updating root directory.\n");
        memcpy(&fsd.root_dir, &parent_dir, sizeof(directory));
    }
    else
    {
        printf("mkdir: Updating non-root directory.\n");
        return SYSERR;
    }

    // FIXME: Maybe remove this (and same code in fcreate)
    if (get_inode_by_num(0, inode_idx, &dir_inode) == SYSERR)
        return SYSERR;

    dir_inode.type = INODE_TYPE_DIR;
    dir_inode.inode_idx = inode_idx;
    dir_inode.size = 0;
    memset(dir_inode.blocks, 0, INODEBLOCKS * sizeof(int));

    int dir_blocks = offset_block_num(sizeof(directory));
    int i;
    for (i = 0; i < dir_blocks; i++)
    {
        if ((dir_inode.blocks[i] = allocate_block()) == SYSERR)
        {
            printf("mkdir: Block allocation failed.\n");
            return SYSERR;
        }
    }

    // Write updated inode
    printf("mkdir: Updating inode with index: %d\n", inode_idx);
    if (put_inode_by_num(0, inode_idx, &dir_inode) == SYSERR)
        return SYSERR;

    // write directory to allocated blocks
    directory new_dir;
    new_dir.numentries = 0;
    new_dir.inode_num = inode_idx;
    if (write_directory(&new_dir, dir_inode.blocks) == SYSERR)
    {
        printf("mkdir: Can't write directory to blocks allocated to inode.\n");
        return SYSERR;
    }

    // Update parent directory entry

    return OK;
}

#endif /* FS */
