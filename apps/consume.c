#include <prodcons.h>
#include <thread.h>

void consumer(int count)
{
  while (n <= count) {
    printf("consuming %d\n", n);
  }
}
