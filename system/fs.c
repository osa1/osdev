#include <bufpool.h>
#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef FS
#include <fs.h>

fsystem fsd;

int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

filedesc oft[NUM_FD];

// number of blocks that fsystem struct uses
int fsystem_blocks;

// number of blocks freemask uses
int freemask_blocks;

// number of blocks inode_bitfield uses
int inode_bitfield_blocks;

// first inode block number
int inodes_start;

// first file block number
int blocks_start;

int fileblock_to_diskblock(int dev, int fd, int fileblock);

/** NOTES
 *
 * Allocation/deallocation of inodes and blocks:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * We don't use free lists, we only keep bitfields. This means allocations need
 * linear search.
 *
 * Layout of the file system itself:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * TODO: fill this part
 *
 */

// FIXME: Bug: We're not setting bit fields for first couple of reserved
// blocks.
int mkfs(int dev, int num_inodes)
{
    int i;

    if (dev == 0) {
        fsd.nblocks = dev0_numblocks;
        fsd.blocksz = dev0_blocksize;
    } else {
        printf("Unsupported device\n");
        return SYSERR;
    }

    if (num_inodes < 1) {
        fsd.ninodes = DEFAULT_NUM_INODES;
    } else {
        fsd.ninodes = num_inodes;
    }

    i = fsd.nblocks;
    while ( (i % 8) != 0 ) {i++;}

    fsd.freemaskbytes = i / 8;
    if ((fsd.freemask = memget(fsd.freemaskbytes)) == (void *)SYSERR) {
        printf("mkfs memget failed. (allocating freemaskbytes)\n");
        return SYSERR;
    }
    memset(fsd.freemask, 0, fsd.freemaskbytes);

    i = fsd.ninodes;
    while ( (i % 8) != 0 ) {i++;}
    if ((fsd.inode_bitfield = memget(i / 8)) == (void*)SYSERR) {
        printf("mkfs memget failed. (allocating inode bitfield)\n");
        return SYSERR;
    }
    memset(fsd.inode_bitfield, 0, i / 8);

    // set special inode_num for root directory
    fsd.root_dir.inode_num = -1;

    fsystem_blocks = offset_block_num(sizeof(fsystem));
    freemask_blocks = offset_block_num(fsd.freemaskbytes);
    inode_bitfield_blocks = offset_block_num(i / 8);

    printf("Writing initial blocks to the drive.\n"
           "fsystem:\t%d blocks\nfreemask:\t%d blocks\ninode bitfield:\t%d blocks\n",
           fsystem_blocks, freemask_blocks, inode_bitfield_blocks);

    // Write fsystem to the drive
    for (i = 0; i < fsystem_blocks; i++)
    {
        int len = dev0_blocksize;
        if (i == fsystem_blocks - 1)
            len = sizeof(fsystem) % dev0_blocksize;

        if (bwrite(0, 0 + i, 0, ((void*)&fsd) + i * dev0_blocksize, len) == SYSERR)
        {
            printf("mkfs: Can't write fsystem to block %d.\n", i);
            return SYSERR;
        }

        // mark block as used
        setbit(fsd.freemask, 0 + i);
    }

    // Write freemask to the drive
    for (i = 0; i < freemask_blocks; i++)
    {
        int len = dev0_blocksize;
        if (i == freemask_blocks - 1)
            len = sizeof(fsystem) % dev0_blocksize;

        if (bwrite(0, fsystem_blocks + i,
                    0, ((void*)&fsd.freemask) + i * dev0_blocksize, len) == SYSERR)
        {
            printf("mkfs: Can't write freemask to block %d.\n",
                    fsystem_blocks + i);
            return SYSERR;
        }

        // mark block as used
        setbit(fsd.freemask, fsystem_blocks + i);
    }

    // Write inode_bitfield to the drive
    for (i = 0; i < inode_bitfield_blocks; i++)
    {
        int len = dev0_blocksize;
        if (i == inode_bitfield_blocks - 1)
            len = sizeof(fsystem) % dev0_blocksize;

        if (bwrite(0, fsystem_blocks + freemask_blocks + i,
                    0, ((void*)&fsd.inode_bitfield) + i * dev0_blocksize, len) == SYSERR)
        {
            printf("mkfs: Can't write inode_bitfield to block %d.\n",
                    fsystem_blocks + freemask_blocks + i);
            return SYSERR;
        }

        // mark block as used
        setbit(fsd.freemask, fsystem_blocks + freemask_blocks + i);
    }

    inodes_start = fsystem_blocks + freemask_blocks + inode_bitfield_blocks;
    blocks_start = inodes_start + (dev0_blocksize / sizeof(inode));

    printf("inodes_start:\t%d\n"
           "blocks_start:\t%d\n"
           "total blocks:\t%d\n", inodes_start, blocks_start, dev0_numblocks);

    // Reset fd table
    memset(&oft, 0, sizeof(filedesc) * NUM_FD);

    return OK;
}

bool checkbit(char *bitfield, int idx)
{
    int byte, bit;
    byte = idx / 8;
    bit  = idx % 8;

    return ((bitfield[byte] & 0x80 /* 0b10000000 */ >> bit) != 0);
}

void setbit(char *bitfield, int idx)
{
    int byte, bit;
    byte = idx / 8;
    bit  = idx % 8;

    bitfield[byte] |= (0x80 >> bit);
}

int allocate_inode()
{
    char *bitfield = fsd.inode_bitfield;
    int i;
    for (i = 0; i < fsd.ninodes; i++)
        if (!checkbit(bitfield, i))
        {
            setbit(bitfield, i);
            return inodes_start + i;
        }

    return SYSERR;
}

int allocate_block(void)
{
    char *bitfield = fsd.freemask;
    int i;
    for (i = blocks_start; i < fsd.nblocks; i++)
    {
        if (!checkbit(bitfield, i))
        {
            setbit(bitfield, i);
            return i;
        }
    }

    return SYSERR;
}

int fileblock_to_diskblock(int dev, int fd, int fileblock)
{
    if (fileblock >= INODEBLOCKS - 2) {
        printf("No indirect block support\n");
        return SYSERR;
    }

    return oft[fd].in.blocks[fileblock];
}

/*
 * NOTE inodes:
 * ~~~~~~~~~~~~
 *
 * - We store inodes packaged, one block may contain multiple inodes, and some
 *   part of an inode may be in next block.
 *
 * - Since inodes are places in contiguous space, we can safely use bwrite with
 *   offset + size bigger than a block size.
 *
 */

/* read in an inode and fill in the pointer */
int get_inode_by_num(int dev, int inode_number, inode *in)
{
    // make sure inode_number is in range
    if (inode_number >= fsd.ninodes)
    {
        printf("get_inode_by_number: inode %d is not in range.\n", inode_number);
        return SYSERR;
    }

    int inode_block_num    = inodes_start + (inode_number * sizeof(inode)) / dev0_blocksize;
    int inode_block_offset = (inode_number * sizeof(inode)) % dev0_blocksize;
    // printf("get_inode_by_num: inode_block_num: %d, inode_block_offset: %d\n",
    //         inode_block_num, inode_block_offset);
    return bread(dev, inode_block_num, inode_block_offset, (void*)in, sizeof(inode));
}

int put_inode_by_num(int dev, int inode_number, inode *in)
{
    // make sure inode_number is in range
    if (inode_number >= fsd.ninodes)
    {
        printf("put_inode_by_number: inode %d is not in range.\n", inode_number);
        return SYSERR;
    }

    int inode_block_num    = inodes_start + (inode_number * sizeof(inode)) / dev0_blocksize;
    int inode_block_offset = (inode_number * sizeof(inode)) % dev0_blocksize;
    // printf("put_inode_by_num: inode_block_num: %d, inode_block_offset: %d\n",
    //         inode_block_num, inode_block_offset);
    return bwrite(dev, inode_block_num, inode_block_offset, (void*)in, sizeof(inode));
}

/* specify the block number to be set in the mask */
int setmaskbit(int b)
{
    setbit(fsd.freemask, b);
    return OK;
}

/* specify the block number to be read in the mask */
int getmaskbit(int b)
{
    return checkbit(fsd.freemask, b);
}

/* specify the block number to be unset in the mask */
int clearmaskbit(int b)
{
    int mbyte, mbit, invb;
    mbyte = b / 8;
    mbit = b % 8;

    invb = ~(0x80 >> mbit);
    invb &= 0xFF;

    fsd.freemask[mbyte] &= invb;
    return OK;
}

/* This is maybe a little overcomplicated since the lowest-numbered block is
 * indicated in the high-order bit.  Shift the byte by j positions to make the
 * match in bit7 (the 8th bit) and then shift that value 7 times to the
 * low-order bit to print.  Yes, it could be the other way...  */
void printfreemask(void)
{
    int i,j;

    for (i=0; i < fsd.freemaskbytes; i++)
    {
        for (j=0; j < 8; j++)
        {
            printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
        }
        if ( (i % 8) == 7 )
        {
            printf("\n");
        }
    }
    printf("\n");
}

void print_dirent(dirent *ent)
{
    inode in;
    if (get_inode_by_num(0, ent->inode_num, &in) == SYSERR)
    {
        printf("print_dirent: Can't get inode %d\n", ent->inode_num);
        return;
    }

    bool is_directory = in.type == INODE_TYPE_DIR;

    printf("inode_num: %d\n", ent->inode_num);
    printf("file name: %s%s\n", ent->name, is_directory ? " (directory)" : "");
}

void print_directory(directory *dir)
{
    printf("Directory with %d entries:\n", dir->numentries);
    int i;
    for (i = 0; i < dir->numentries; i++)
    {
        print_dirent(&dir->entry[i]);
    }
}

void print_inode(inode *in)
{
    printf("-- inode --\n"
           "- type:      %d\n"
           "- inode_idx: %d\n"
           "- size:      %d\n", in->type, in->inode_idx, in->size);
}

int fs_initialized(void)
{
    return fsd.nblocks != 0;
}

directory *get_root_dir(void)
{
    return &(fsd.root_dir);
}

int get_directory_blocks(void)
{
    if (!fs_initialized())
    {
        printf("get_directory_blocks: file system is not initialized.\n");
        return 0;
    }

    return offset_block_num(sizeof(directory));
}

// TODO: Just replace this with fsd.blocksz, and check initialization in major
// functions.
int get_block_size(void)
{
    if (!fs_initialized())
    {
        printf("get_block_size: file system is not initialized.\n");
        return 0;
    }

    return fsd.blocksz;
}

int load_directory(int *blocks, directory *output)
{
    int directory_blocks = get_directory_blocks();
    int i;
    for (i = 0; i < directory_blocks; i++)
    {
        int read_len;
        if (i == directory_blocks - 1)
            read_len = sizeof(directory) % fsd.blocksz;
        else
            read_len = fsd.blocksz;

        if (bread(0, blocks[i], 0, ((void*)output) + (i * fsd.blocksz), read_len) == SYSERR)
            return SYSERR;
    }
    return OK;
}

int write_directory(directory *dir, int *blocks)
{
    int directory_blocks = get_directory_blocks();
    int i;
    for (i = 0; i < directory_blocks; i++)
    {
        int read_len;
        if (i == directory_blocks - 1)
            read_len = sizeof(directory) % fsd.blocksz;
        else
            read_len = fsd.blocksz;

        if (bwrite(0, blocks[i], 0, ((void*)dir) + (i * fsd.blocksz), read_len) == SYSERR)
            return SYSERR;
    }

    return OK;
}

int find_closed_fd(void)
{
    int i;
    for (i = 0; i < NUM_FD; i++)
        if (oft[i].state == O_CLOSED)
            return i;
    return SYSERR;
}

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
            if (get_inode_by_num(0, entry.inode_num, output) == SYSERR) {
                printf("get_file_inode: Can't get inode by num: %d\n", entry.inode_num);
                return SYSERR;
            }

            if (output->type == type)
                return OK;
        }
    }

    return SYSERR;
}

/**
 * Utilities
 */

/**
 * Drop slashes in the path.
 */
char *get_filename(char *path)
{
    char *ret = strrchr(path, '/');
    if (ret == NULL)
        return path;
    else
        return ret + 1; // drop '/'
}

#endif /* FS */
