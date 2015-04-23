/**
 * Some TODOs and notes:
 *
 * - Lots of error checking are copied. Maybe create some functions or macros
 *   to reduce boilerplate and duplication.
 *
 * - Bad things may happen if a file is opened multiple times. We don't update
 *   inodes in the device before closing the file.
 *
 * - `write` shell command adds '\0' at the end, but doesn't de-allocate blocks
 *   or updates the inode size.
 *
 * - Superblock always kept in memory. It has allocated blocks in the device,
 *   but it's never updated. Implement a way to close the file system, and
 *   write superblock to disk when file system is closed.
 *
 * Implementation TODO:
 *
 * - Search directory for a given name: for time being exact match
 * - Search a file in a directory path: exact match will be sufficient
 * - Whether allocated space for a directory or file is consumed completely.
 *
 * Done:
 * - Init fs (fsinit)
 * - Create directories (mkdir)
 * - Rename directories (mv)
 * - List entries of directories (ls)
 * - File content display (cat)
 * - Remove directories (rm -rf)
 * - Meta data about directory (shown in ls)
 * - Other operations like deleting a file entry or clearing content of the file
 *   (rm, write empty string for clearing file)
 *
 */

#ifndef FS_H
#define FS_H

#include <stdlib.h>

#define FILENAMELEN 32
#define INODEBLOCKS 12
#define DIRECTORY_SIZE 16

#define MDEV_BLOCK_SIZE 512
#define MDEV_NUM_BLOCKS 512
#define DEFAULT_NUM_INODES (MDEV_NUM_BLOCKS / 4)

#define INODE_TYPE_FILE 1
#define INODE_TYPE_DIR 2

#define NUM_FD 16

typedef struct
{
    short int type; // directory or file
    int inode_idx; // this is required to be able to write inodes back to disk
                   // when file descriptors are closed
    int size; // file size
    int blocks[INODEBLOCKS]; // block indexes
} inode;

/* Flags for fopen */
// NB: O_CLOSED should be 0, because at initialization we fill the table with
// zeroes
#define O_CLOSED 0
#define O_RDONLY 1
#define O_WRONLY 2
#define O_RDWR   3

// NB: I think this is for open files. Make sure it is.
// Also, renaming this from `filetable`, since this is not a table in any way.
typedef struct
{
    int state;
    int cursor;
    inode in;
} filedesc;

typedef struct
{
    int inode_num;
    // NB: adding one here for null-termination, which makes it possible to use
    // cmpstr like functions
    char name[FILENAMELEN + 1];
} dirent;

typedef struct
{
    int numentries;
    int inode_num; // inode of directory entry. This is not strictly necessary,
                   // but I'm using this to make updating directory entries easier.
                   // NOTE: Root directory is a special case, and it has inode_num = -1.
    dirent entry[DIRECTORY_SIZE];
} directory;

typedef struct
{
    int nblocks;
    int blocksz;
    int ninodes;
    char *inode_bitfield;
    int freemaskbytes;
    char *freemask;
    directory root_dir;
} fsystem;

/* file and directory functions */
int fopen(char *filename, int flags);
int fclose(int fd);
int fcreate(char *filename);
int fseek(int fd, int offset);
int fread(int fd, void *buf, int nbytes);
int fwrite(int fd, void *buf, int nbytes);
int mkdir(char *path);

/* file system initialization */
int mkfs(int dev, int num_inodes);

/* file system utils and helpers */
int get_inode_by_num(int dev, int inode_number, inode *in);
int put_inode_by_num(int dev, inode *in);
int setmaskbit(int b);
int clearmaskbit(int b);
int getmaskbit(int b);
int fs_initialized(void);
directory *get_root_dir(void);
int get_directory_blocks(void);
int get_block_size(void);
int load_directory(int *blocks, directory *output);
int write_directory(directory *dir, int *blocks);
int allocate_inode(void);
int free_inode(inode *in);
int find_closed_fd(void);
int allocate_block(void);
int free_block(int block);
bool checkbit(char *bitfield, int bit_idx);
void setbit(char *bitfield, int bit_idx);
void clearbit(char *bitfield, int bit_idx);
int get_parent_directory(directory *cur_dir, char *path, directory *output);
int get_file_inode(directory *cur_dir, char *filename, inode *output, int type);
int allocate_blocks(int *blocks, int n);

/* Block Store functions */
int mkbsdev(int dev, int blocksize, int numblocks);
int bread(int bsdev, int block, int offset, void *buf, int len);
int bwrite(int bsdev, int block, int offset, void * buf, int len);
int offset_block_num(int offset);

/* Utilities */
char *get_filename(char *path);

/* debugging functions */
void print_bitfield(char *bytes, int len);
void print_dirent(dirent*);
void print_directory(directory*);
void print_inode(inode*);

#endif /* FS_H */
