#include <fs.h>
#include <stdio.h>
#include <string.h>

/**
 * @ingroup shell
 *
 * A simple version of mkdir command.
 */
shellcmd xsh_mkdir(int nargs, char *args[])
{
    if (nargs != 2)
    {
        printf("Wrong number of arguments.\n"
               "USAGE: mkdir <directory path>\n");
        return 1;
    }

    return mkdir(args[1]);
}
