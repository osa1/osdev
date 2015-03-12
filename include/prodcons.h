#include <future.h>
#include <stddef.h>
#include <stdio.h>
#include <thread.h>

#define printflock(lock, ...)   \
    do {                        \
        wait(lock);             \
        printf(__VA_ARGS__);    \
        signal(lock);           \
    } while (0)

extern int n;

extern semaphore consumed, produced;

void producer(int count);
void consumer(int count);
syscall future_prod(future *fut, semaphore print_sem, semaphore running);
syscall future_cons(future *fut, semaphore print_sem, semaphore running);
