#include <fs.h>
#include <stdio.h>
#include <string.h>

extern filedesc oft[NUM_FD];

/**
 * @ingroup shell
 *
 * A shell command that is similar to UNIX cat.
 */
shellcmd xsh_cat(int nargs, char *args[])
{
    if (nargs != 2)
    {
        printf("Wrong number of arguments.\n");
        printf("USAGE: cat <file path>\n");
    }

    char *filepath = args[1];

    int fd;
    if ((fd = fopen(filepath, O_RDONLY)) == SYSERR)
    {
        printf("cat: fopen failed.\n");
        return SYSERR;
    }

    int size = oft[fd].in.size;
    char *buf = (char*) memget(size);

    if (buf == NULL)
    {
        printf("Can't allocate enough memory to read the file. "
               "Maybe file is too big or run out of memory?\n");
        return SYSERR;
    }

    if (fread(fd, buf, size) == SYSERR)
    {
        printf("cat: Can't read the file.\n");
        return SYSERR;
    }

    printf("%s", buf);

    return fclose(fd);
}
