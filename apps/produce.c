#include <prodcons.h>
#include <thread.h>

void producer(int count)
{
  while (n <= count) {
    printf("producing %d\n", ++n);
  }
}
