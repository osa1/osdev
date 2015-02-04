#include <prodcons.h>
#include <queue.h>
#include <stdlib.h>

int n;

shellcmd xsh_prodcons(int nargs, char *args[])
{
  n = 0;
  int count = 2000;

  if (nargs > 2) {
      printf(
          "ERROR: Wrong number of arguments given. Expected 1 or 0, found %d.\n"
          "USAGE: prodcons COUNT\n", nargs);
      return 1;
  }

  if (nargs == 2) {
    count = atoi(args[1]);
  }

  tid_typ producer_th = create(producer, 1024, 20, "producer", 3, count);
  tid_typ consumer_th = create(consumer, 1024, 20, "consumer", 3, count);

  resume(consumer_th);
  resume(producer_th);

  return 0;
}
