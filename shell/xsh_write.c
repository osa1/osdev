#include <fs.h>
#include <stdio.h>
#include <string.h>

/**
 * @ingroup shell
 *
 * A shell command that writes characters(first argument) to the file(second
 * argument).
 */
shellcmd xsh_write(int nargs, char *args[])
{
    if (nargs != 3)
    {
        printf("Wrong number of arguments.\n");
        printf("USAGE: write <string> <file path>\n");
    }

    char *str = args[1];
    char *filepath = args[2];

    int fd;
    if ((fd = fopen(filepath, O_WRONLY)) == SYSERR)
    {
        printf("write: fopen failed.\n");
        return SYSERR;
    }

    // drop initial quote, if exists
    if (str[0] == '"') str++;
    // drop trailing quote, if exists
    char *str_r = strrchr(str, '"');
    if (str_r != NULL) *str_r = '\0';
    int nbytes = strlen(str);

    if (fwrite(fd, str, nbytes + 1) == SYSERR)
    {
        printf("write: fwrite failed.\n");
        return SYSERR;
    }

    return fclose(fd);
}
