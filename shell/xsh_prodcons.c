#include <prodcons.h>
#include <queue.h>
#include <stdlib.h>

int n;

semaphore consumed, produced;

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
    if (validate_arg(args[1]) != 0) {
      printf("ERROR: Can't parse argument: %s. It should be a positive integer.\n",
             args[1]);
      return 1;
    }
    count = atoi(args[1]);
  }

  consumed = semcreate(0);
  produced = semcreate(1);

  tid_typ producer_th = create(producer, 1024, 20, "producer", 3, count);
  tid_typ consumer_th = create(consumer, 1024, 20, "consumer", 3, count);

  resume(consumer_th);
  resume(producer_th);

  return 0;
}

int validate_arg(char *arg)
{
  int i = 0;
  while (arg[i] != '\0') {
    if (!(arg[i] >= '0' && arg[i] <= '9')) {
      return -1;
    }
    i++;
  }
  return 0;
}
