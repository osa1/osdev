#include <fs.h>
#include <stdio.h>
#include <string.h>

/**
 * @ingroup shell
 *
 * A shell command that saves superblock to the device.
 */
shellcmd xsh_savefs(int nargs, char *args[])
{
    return savefs();
}
