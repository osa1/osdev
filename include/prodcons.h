#include <stddef.h>
#include <stdio.h>
#include <thread.h>

extern int n;

extern semaphore consumed, produced;

void producer(int count);
void consumer(int count);
