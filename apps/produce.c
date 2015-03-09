#include <prodcons.h>
#include <thread.h>

void producer(int count)
{
    while (n <= count)
    {
        wait(consumed);
        printf("producing %d\n", ++n);
        signal(produced);
    }
}
