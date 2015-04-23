#include <fs.h>
#include <stdio.h>
#include <string.h>

extern fsystem fsd;

int rm_rec(directory *dir);

/**
 * @ingroup shell
 *
 * A shell command that works like UNIX rm.
 */
shellcmd xsh_rm(int nargs, char *args[])
{
    if (nargs == 1)
    {
        printf("Wring number of arguments provided.\n");
        printf("USAGE: rm [-rf] <file1> <file2> ... <fileN>\n");
        return 1;
    }

    bool rm_dirs = FALSE;

    // skip command name
    args++;
    nargs--;
    if (strcmp(args[0], "-rf") == 0)
    {
        rm_dirs = TRUE;
        // skip -rf
        args++;
        nargs--;
    }

    int i;
    for (i = 0; i < nargs; i++)
    {
        char *filepath = args[i];
        char *filename = get_filename(filepath);

        // printf("filepath: %s\nfilename: %s\n", filepath, filename);

        directory parent_dir;
        if (get_parent_directory(get_root_dir(), filepath, &parent_dir) == SYSERR)
        {
            printf("rm: Can't read source directory.\n");
            return 1;
        }

        dirent *file_entry;
        int j, k;
        for (j = parent_dir.numentries - 1; j >= 0; j--)
        {
            file_entry = &parent_dir.entry[j];
            if (strcmp(file_entry->name, filename) == 0)
            {
                inode in;
                if (get_inode_by_num(0, file_entry->inode_num, &in) == SYSERR)
                {
                    printf("rm: Can't read inode of file.\n");
                    return 1;
                }

                if (in.type == INODE_TYPE_DIR)
                {
                    if (!rm_dirs)
                    {
                        printf("rm: -rm is not provided, skipping directory: %s\n", filename);
                        continue;
                    }
                    directory contents;
                    load_directory(in.blocks, &contents);
                    rm_rec(&contents);
                }

                for (k = j; k < parent_dir.numentries; k++)
                    parent_dir.entry[k] = parent_dir.entry[k+1];
                parent_dir.numentries--;
                free_inode(&in);
            }
        }

        if (parent_dir.inode_num == -1)
            memcpy(&fsd.root_dir, &parent_dir, sizeof(directory));
        else
        {
            inode dir_inode;
            if (get_inode_by_num(0, parent_dir.inode_num, &dir_inode) == SYSERR)
            {
                printf("rm: Can't get inode of parent directory.\n");
                return 1;
            }

            if (write_directory(&parent_dir, dir_inode.blocks) == SYSERR)
            {
                printf("rm: Can't update parent directory.\n");
                return 1;
            }
        }
    }

    return 0;
}

/**
 * Recursively remove directory contents, freeing inodes and blocks. Doesn't
 * remove the directory itself.
 */
int rm_rec(directory *dir)
{
    int i;
    for (i = 0; i < dir->numentries; i++)
    {
        inode in;
        get_inode_by_num(0, dir->entry[i].inode_num, &in);

        if (in.type == INODE_TYPE_DIR)
        {
            // recursively remove contents
            directory subdir;
            load_directory(in.blocks, &subdir);
            rm_rec(&subdir);
        }

        free_inode(&in);
    }

    dir->numentries = 0;
    return OK;
}
