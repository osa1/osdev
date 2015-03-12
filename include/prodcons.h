#include <future.h>
#include <stddef.h>
#include <stdio.h>
#include <thread.h>

extern int n;

extern semaphore consumed, produced;

void producer(int count);
void consumer(int count);
syscall future_prod(future *fut, semaphore print_sem, semaphore running);
syscall future_cons(future *fut, semaphore print_sem, semaphore running);
