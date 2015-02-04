#include <stddef.h>
#include <stdio.h>
#include <thread.h>

extern int n;

void producer(int count);
void consumer(int count);
