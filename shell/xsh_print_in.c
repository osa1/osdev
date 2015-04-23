#include <fs.h>
#include <stdio.h>
#include <string.h>

extern fsystem fsd;

/**
 * @ingroup shell
 *
 * Print inode bitfield, for debugging purposes.
 */
shellcmd xsh_print_in(int nargs, char *args[])
{
    int bs = fsd.ninodes / 8;
    if (fsd.ninodes % 8 != 0) bs++;
    printfreemask(fsd.inode_bitfield, bs);
    return OK;
}
