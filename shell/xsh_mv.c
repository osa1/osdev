#include <fs.h>
#include <stdio.h>
#include <string.h>

extern fsystem fsd;

/**
 * @ingroup shell
 *
 * A shell command that works like UNIX mv.
 */

// FIXME: New names are not applied. e.g. file is copied correctly, but not renamed.
shellcmd xsh_mv(int nargs, char *args[])
{
    if (nargs != 3)
    {
        printf("Wrong number of arguments provided.\n");
        printf("USAGE: mv <source> <destination>\n");
        return 1;
    }

    char *source = args[1];
    char *dest   = args[2];

    char *source_file_name = get_filename(source);
    char *dest_file_name   = get_filename(dest);

    directory source_parent;
    if (get_parent_directory(get_root_dir(), source, &source_parent) == SYSERR)
    {
        printf("mv: Can't read source directory.\n");
        return 1;
    }

    directory dest_parent;
    if (get_parent_directory(get_root_dir(), dest, &dest_parent) == SYSERR)
    {
        printf("mv: Can't read destination directory.\n");
        return 1;
    }

    // Make sure dest file doesn't exist, we don't overwrite files
    inode dest_file_inode;
    if (get_file_inode(&dest_parent, dest_file_name, &dest_file_inode, 0 /* match all */)
            != SYSERR)
    {
        printf("Destination file already exists. mv doesn't overwrite files.\n");
        return 1;
    }

    if (source_parent.inode_num == dest_parent.inode_num)
    {
        // Renaming a file, doesn't involve moves between directories
        int i;
        for (i = 0; i < source_parent.numentries; i++)
            if (strcmp(source_parent.entry[i].name, source_file_name) == 0)
            {
                strcpy(source_parent.entry[i].name, dest_file_name);
                break;
            }

        if (source_parent.inode_num == -1)
        {
            memcpy(&fsd.root_dir, &source_parent, sizeof(directory));
        }
        else
        {
            inode source_inode;
            get_inode_by_num(0, source_parent.inode_num, &source_inode);

            if (write_directory(&source_parent, source_inode.blocks) == SYSERR)
            {
                printf("mv: Can't write updated directory.\n");
                return 1;
            }
        }

        return 0;
    }
    else
    {
        // Make sure there's room in destination
        if (dest_parent.numentries == DIRECTORY_SIZE)
        {
            printf("mv: Can't move file: Destination directory if full.\n");
            return 1;
        }

        // Start updating local copies and updating
        int i, j;

        // Update source directory
        // FIXME: Move this loop to a helper function, named `remove_dir_entry`
        dirent file_entry;
        file_entry.inode_num = -1; // marker
        for (i = 0; i < source_parent.numentries; i++)
        {
            file_entry = source_parent.entry[i];
            if (strcmp(file_entry.name, source_file_name) == 0)
            {
                // remove the entry
                for (j = i + 1; j < source_parent.numentries; j++)
                    source_parent.entry[j-1] = source_parent.entry[j];
                source_parent.numentries--;
                break;
            }
        }

        if (file_entry.inode_num == -1)
        {
            printf("mv: Can't file source file in the directory.\n");
            return 1;
        }

        // Update destination directory
        dest_parent.entry[dest_parent.numentries++] = file_entry;

        // Write updated directories to the device
        if (source_parent.inode_num == -1)
        {
            memcpy(&fsd.root_dir, &source_parent, sizeof(directory));
        }
        else
        {
            inode source_inode;
            get_inode_by_num(0, source_parent.inode_num, &source_inode);

            if (write_directory(&source_parent, source_inode.blocks) == SYSERR)
            {
                printf("mv: Can't write updated source directory.\n");
                return 1;
            }
        }

        if (dest_parent.inode_num == -1)
        {
            memcpy(&fsd.root_dir, &dest_parent, sizeof(directory));
        }
        else
        {
            inode dest_inode;
            get_inode_by_num(0, dest_parent.inode_num, &dest_inode);

            if (write_directory(&dest_parent, dest_inode.blocks) == SYSERR)
            {
                printf("mv: Can't write updated destination directory.\n");
                return 1;
            }
        }

        return 0;
    }
}
