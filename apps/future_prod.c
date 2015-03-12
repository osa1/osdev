#include <prodcons.h>

syscall future_prod(future *fut, semaphore print_sem, semaphore running)
{
    int *j = (int*) memget(sizeof(int));
    int i;

    for (i=0; i<1000; i++)
    {
        *j += i;
    }
    wait(print_sem);
    printf("setting the future: %d\n", (*j));
    signal(print_sem);
    int ret = future_set(fut, j);
    signal(running);
    return ret;
}
