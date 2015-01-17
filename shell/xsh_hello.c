/**
 * First assignment.
 */

#include <stdio.h>

shellcmd xsh_hello(int nargs, char *args[])
{
    if (nargs != 2) {
        printf(
            "ERROR: Wrong number of arguments given. Expected 2, found %d.\n"
            "USAGE: hello NAME\n", nargs);
        return 1;
    }
    char *name = args[1];
    printf("Hello %s,  Welcome to the oagacan's XINU!\n", name);
    return 0;
}
