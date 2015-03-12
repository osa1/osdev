#include <prodcons.h>

syscall future_cons(future *fut, semaphore print_sem, semaphore running)
{
    int i;
    printflock(print_sem, "consumer %d trying to get the future.\n", thrcurrent);
    if (future_get(fut, &i) != OK)
    {
        printflock(print_sem, "future_get failed\n");
        signal(running);
        return -1;
    }
    printflock(print_sem, "consumer %d consumed %d\n", thrcurrent, i);
    signal(running);
    return OK;
}
