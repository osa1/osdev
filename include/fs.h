/**
 * Some TODOs:
 *
 * - Lots of error checking are copied. Maybe create some functions or macros
 *   to reduce boilerplate and duplication.
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
    // TODO: Disabling these until figuring out how they're useful
    int cursor;
    // dirent *de; // TODO: why is this a pointer?
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
                   // TODO: Remove this and keep track of inode indexes in other places.
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
int put_inode_by_num(int dev, int inode_number, inode *in);
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
int find_closed_fd(void);
int allocate_block(void);
bool checkbit(char *bitfield, int bit_idx);
void setbit(char *bitfield, int bit_idx);
int get_parent_directory(directory *cur_dir, char *path, directory *output);
int get_file_inode(directory *cur_dir, char *filename, inode *output, int type);

/* Block Store functions */
int mkbsdev(int dev, int blocksize, int numblocks);
int bread(int bsdev, int block, int offset, void *buf, int len);
int bwrite(int bsdev, int block, int offset, void * buf, int len);
int offset_block_num(int offset);

/* Utilities */
char *get_filename(char *path);

/* debugging functions */
void printfreemask(void);
// TODO: No implementation is provided, but I don't think this is required for
//       the assignment
// void print_fsd(void);

void print_dirent(dirent*);
void print_directory(directory*);
void print_inode(inode*);

#endif /* FS_H */
