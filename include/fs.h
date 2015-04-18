#ifndef FS_H
#define FS_H

#define FILENAMELEN 32
#define INODEBLOCKS 12
#define INODEDIRECTBLOCKS (INODEBLOCKS - 2)
#define DIRECTORY_SIZE 16

#define MDEV_BLOCK_SIZE 512
#define MDEV_NUM_BLOCKS 512
#define DEFAULT_NUM_INODES (MDEV_NUM_BLOCKS / 4)

#define INODE_TYPE_FILE 1
#define INODE_TYPE_DIR 2

typedef struct
{
    int id;
    short int type;
    short int nlink;
    int device;
    int size;
    int blocks[INODEBLOCKS];
} inode;

#define FSTATE_CLOSED 0
#define FSTATE_OPEN 1

/* Modes of file*/
#define O_CREAT 11

/* Flags of file*/
#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2

typedef struct
{
    int inode_num;
    char name[FILENAMELEN + 1]; // NB: adding one here for null-termination
} dirent;

// NB: I think this is for open files. Make sure it is.
typedef struct
{
    int state;
    int fileptr;
    dirent *de;
    inode in;
} filetable;

typedef struct
{
    int numentries;
    dirent entry[DIRECTORY_SIZE];
} directory;

typedef struct
{
    int nblocks;
    int blocksz;
    int ninodes;
    int inodes_used;
    int freemaskbytes;
    char *freemask;
    directory root_dir;
} fsystem;

/* file and directory functions */
int fopen(char *filename, int flags);
int fclose(int fd);
int fcreate(char *filename, int mode);
int fseek(int fd, int offset);
int fread(int fd, void *buf, int nbytes);
int fwrite(int fd, void *buf, int nbytes);

/* filesystem functions */
int mkfs(int dev, int num_inodes);
int mount(int dev);

/* filesystem internal functions */
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

/* Block Store functions */
int mkbsdev(int dev, int blocksize, int numblocks);
int bread(int bsdev, int block, int offset, void *buf, int len);
int bwrite(int bsdev, int block, int offset, void * buf, int len);

/* debugging functions */
void printfreemask(void);
// TODO: No implementation is provided, but I don't think this is required for
//       the assignment
// void print_fsd(void);

void print_dirent(dirent*);
void print_directory(directory*);

#endif /* FS_H */
