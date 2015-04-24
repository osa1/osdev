#include <assert.h>
#include <fs.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define SIZE 1200

extern fsystem fsd;

void testbitmask(void);
void test_dir(void);

/**
 * @ingroup shell
 *
 * Shell command fstest.
 * @param nargs  number of arguments in args array
 * @param args   array of arguments
 * @return OK for success, SYSERR for syntax error
 */
shellcmd xsh_fstest(int nargs, char *args[])
{
    int rval;
    int fd, i, j;
    char *buf1, *buf2;

    /* Output help, if '--help' argument was supplied */
    if (nargs == 2 && strncmp(args[1], "--help", 7) == 0)
    {
        printf("Usage: %s\n\n", args[0]);
        printf("Description:\n");
        printf("\tFilesystem Test\n");
        printf("Options:\n");
        printf("\t--help\tdisplay this help and exit\n");
        return OK;
    }

    /* Check for correct number of arguments */
    if (nargs > 1)
    {
        fprintf(stderr, "%s: too many arguments\n", args[0]);
        fprintf(stderr, "Try '%s --help' for more information\n",
                args[0]);
        return SYSERR;
    }
    if (nargs < 1)
    {
        fprintf(stderr, "%s: too few arguments\n", args[0]);
        fprintf(stderr, "Try '%s --help' for more information\n",
                args[0]);
        return SYSERR;
    }

#ifdef FS
    printf("filesystem initialized: %d\n", fs_initialized());

    /* device "0" and default blocksize (=0) and count */
    mkbsdev(0, MDEV_BLOCK_SIZE, MDEV_NUM_BLOCKS);
    mkfs(0, DEFAULT_NUM_INODES);
    printf("filesystem initialized after mkfs: %d\n", fs_initialized());
    printf("get_block_size() -> %d\n", get_block_size());
    printf("get_directory_blocks() -> %d\n", get_directory_blocks());
    print_directory(get_root_dir());
    testbitmask();

    buf1 = memget(SIZE * sizeof(char));
    buf2 = memget(SIZE * sizeof(char));

    // Create test file
    // NB: Fixing the useless argument here.
    fd = fcreate("Test_File");
    if (fd == SYSERR)
    {
        printf("fcreate failed. Aborting.\n");
        return SYSERR;
    }

    // Fill buffer with random stuff
    for(i=0; i<SIZE; i++)
    {
        j = i % (127-33);
        j = j + 33;
        buf1[i] = (char) j;
    }

    rval = fwrite(fd, buf1, SIZE);
    if (rval == SYSERR || rval != SIZE)
    {
        printf("File write failed.\n");
        goto clean_up;
    }

    // Now my file offset is pointing at EOF file, I need to bring it back to
    // start of file Assuming here implementation of fseek is like
    // "original_offset = original_offset + input_offset_from_fseek"
    fseek(fd, -rval);

    //read the file
    rval = fread(fd, buf2, rval);

    if (rval == SYSERR)
    {
        printf("File read failed\n");
        goto clean_up;
    }

    buf2[rval] = '\0';

    printf("Content of file %s\n", buf2);

    rval = fclose(fd);
    assert(rval == OK);

    // Now we should be able to fopen the file, and read same contents.
    fd = fopen("Test_File", O_RDONLY);
    assert(fd != SYSERR);
    rval = fread(fd, buf2, SIZE);
    assert(fd != SYSERR);
    buf2[rval] = '\0';
    printf("Content of file %s\n",buf2);

    // We shouldn't be able to open a file that is already open
    printf("This should fail:\n");
    assert(fopen("Test_File", O_RDONLY) == SYSERR);

    rval = fclose(fd);
    assert(rval != SYSERR);


clean_up:
    memfree(buf1, SIZE);
    memfree(buf2, SIZE);

    test_dir();

#else
    printf("No filesystem support\n");
#endif

    return OK;
}

void test_dir(void)
{
    printf("---0\n");
    print_directory(get_root_dir());
    printf("----\n");
    assert(mkdir("d1") != SYSERR);
    // Now we should be see my_dir in root directory.
    printf("---1\n");
    print_directory(get_root_dir());
    assert(mkdir("d2") != SYSERR);
    printf("---2\n");
    print_directory(get_root_dir());
    assert(mkdir("d3") != SYSERR);
    assert(mkdir("d4") != SYSERR);
    assert(mkdir("d5") != SYSERR);
    assert(mkdir("d6") != SYSERR);
    assert(mkdir("d7") != SYSERR);
    print_directory(get_root_dir());
    printf("----\n");

    directory dir;
    get_parent_directory(get_root_dir(), "/d1/", &dir);
    print_directory(get_root_dir());

    int fd = fcreate("d1/d11.file");
    assert(fd != SYSERR);
    assert(fclose(fd) != SYSERR);

    assert(mkdir("/d1/d11") != SYSERR);
    assert(mkdir("/d1/d12") != SYSERR);
    assert(mkdir("/d2/d21") != SYSERR);
    assert(mkdir("/d2/d22") != SYSERR);
    printf("---3\n");
    print_directory(get_root_dir());
    printf("----\n");
}

void testbitmask(void)
{
    setmaskbit(31); setmaskbit(95); setmaskbit(159); setmaskbit(223);
    setmaskbit(287); setmaskbit(351); setmaskbit(415); setmaskbit(479);
    setmaskbit(90); setmaskbit(154); setmaskbit(218); setmaskbit(282);
    setmaskbit(346); setmaskbit(347); setmaskbit(348); setmaskbit(349);
    setmaskbit(350); setmaskbit(100); setmaskbit(164); setmaskbit(228);
    setmaskbit(292); setmaskbit(356); setmaskbit(355); setmaskbit(354);
    setmaskbit(353); setmaskbit(352);

    print_bitfield(fsd.freemask, fsd.freemaskbytes);

    clearmaskbit(31); clearmaskbit(95); clearmaskbit(159); clearmaskbit(223);
    clearmaskbit(287); clearmaskbit(351); clearmaskbit(415); clearmaskbit(479);
    clearmaskbit(90); clearmaskbit(154); clearmaskbit(218); clearmaskbit(282);
    clearmaskbit(346); clearmaskbit(347); clearmaskbit(348); clearmaskbit(349);
    clearmaskbit(350); clearmaskbit(100); clearmaskbit(164); clearmaskbit(228);
    clearmaskbit(292); clearmaskbit(356); clearmaskbit(355); clearmaskbit(354);
    clearmaskbit(353); clearmaskbit(352);

    print_bitfield(fsd.freemask, fsd.freemaskbytes);
}
