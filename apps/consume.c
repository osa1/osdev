#include <prodcons.h>
#include <thread.h>

void consumer(int count)
{
  while (n <= count) {
    wait(produced);
    printf("consuming %d\n", n);
    signal(consumed);
  }
}
