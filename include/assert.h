/**
 * @file assert.h
 * Custom implementation of assert macro.
 * Warning: NOT STANDARD COMPLIANT. Just a useful implementation.
 */

/* Implemented by osa */

#include <stdio.h>

#define assert(EX) (void)((EX) || (__assert (#EX, __FILE__, __LINE__),0))

void __assert(char *assertion, char *file, int line)
{
    printf("assertion failed: %s (%s, line %d)\n", assertion, file, line);
    kill(gettid());
}
