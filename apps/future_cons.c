#include <prodcons.h>

syscall future_cons(future *fut, semaphore print_sem, semaphore running)
{
    int i;
    printflock(print_sem, "trying to get the future.\n");
    if (future_get(fut, &i) != OK)
    {
        printflock(print_sem, "future_get failed\n");
        signal(running);
        return -1;
    }
    printflock(print_sem, "it produced %d\n", i);
    signal(running);
    return OK;
}
