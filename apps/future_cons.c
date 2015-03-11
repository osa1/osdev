#include <prodcons.h>

syscall future_cons(future *fut, semaphore print_sem, semaphore running)
{
    int i;
    wait(print_sem);
    printf("trying to get the future.\n");
    signal(print_sem);
    if (future_get(fut, &i) != OK)
    {
        wait(print_sem);
        printf("future_get failed\n");
        signal(print_sem);
        return -1;
    }
    wait(print_sem);
    printf("it produced %d\n", i);
    signal(print_sem);
    signal(running);
    return OK;
}
