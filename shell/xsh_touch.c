#include <fs.h>
#include <stdio.h>
#include <string.h>

/**
 * @ingroup shell
 *
 * A simple version of touch command.
 */
shellcmd xsh_touch(int nargs, char *args[])
{
    if (nargs != 2)
    {
        printf("Wrong number of arguments.\n"
               "USAGE: touch <file path>\n");
        return 1;
    }

    return fcreate(args[1]);
}
