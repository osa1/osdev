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
        printf("mkdir: Can't create file, parent directory is full.\n");
        return SYSERR;
    }

    char *filename = get_filename(path);

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

    // Update local copy of parent directory
    dirent *entry = &parent_dir.entry[parent_dir.numentries++];
    memcpy(entry->name, filename, FILENAMELEN + 1); // FIXME: This may end up with segfault
    entry->inode_num = inode_idx;

    // Write updates to the drive
    if (parent_dir.inode_num == -1)
    {
        memcpy(&fsd.root_dir, &parent_dir, sizeof(directory));
    }
    else
    {
        inode parent_dir_inode;
        if (get_inode_by_num(0, parent_dir.inode_num, &parent_dir_inode) == SYSERR)
        {
            // This is definitely a bug, we get same inode above
            printf("mkdir: Can't get parent directory inode.\n");
            return SYSERR;
        }

        if (write_directory(&parent_dir, parent_dir_inode.blocks) == SYSERR)
        {
            printf("mkdir: Can't update parent directory entry.\n");
            return SYSERR;
        }
    }

    // Initialize new inode for the new directory
    dir_inode.type = INODE_TYPE_DIR;
    dir_inode.inode_idx = inode_idx;
    dir_inode.size = 0;
    memset(dir_inode.blocks, 0, INODEBLOCKS * sizeof(int));

    // Allocate blocks for new directory entry
    if (allocate_blocks(dir_inode.blocks, get_directory_blocks()) == SYSERR)
    {
        printf("mkdir: Block allocation failed.\n");
        return SYSERR;
    }

    // Create directory entry, and write it to allocated blocks
    directory new_dir;
    new_dir.numentries = 0;
    new_dir.inode_num = inode_idx;
    if (write_directory(&new_dir, dir_inode.blocks) == SYSERR)
    {
        printf("mkdir: Can't write directory to blocks allocated to inode.\n");
        return SYSERR;
    }

    if (put_inode_by_num(0, &dir_inode) == SYSERR)
        return SYSERR;

    return OK;
}

#endif /* FS */
