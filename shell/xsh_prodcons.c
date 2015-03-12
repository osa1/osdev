#include <prodcons.h>
#include <queue.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int n;

semaphore consumed, produced;

bool search_dash_f(char *args[]);
int search_count_arg(char *args[], int def);
bool validate_arg(char *arg);

shellcmd xsh_prodcons(int nargs, char *args[])
{
    n = 0;
    int count = 2000;
    bool use_futures = FALSE;

    if (nargs > 3)
    {
        printf(
                "ERROR: Wrong number of arguments given. Expected 2, 1 or 0, found %d.\n"
                "USAGE: prodcons COUNT\n", nargs - 1);
        return 1;
    }

    use_futures = search_dash_f(args);
    count = search_count_arg(args, count);

    printf("use_futures: %d\n", use_futures);
    printf("count: %d\n", count);

    if (use_futures == FALSE)
    {
        /* run as before */
        consumed = semcreate(0);
        produced = semcreate(1);

        tid_typ producer_th = create(producer, 1024, 20, "producer", 1, count);
        tid_typ consumer_th = create(consumer, 1024, 20, "consumer", 1, count);

        resume(consumer_th);
        resume(producer_th);
    }
    else
    {
        /* run program that uses futures */
        future *f1, *f2, *f3;

        f1 = future_alloc(FUTURE_EXCLUSIVE);
        f2 = future_alloc(FUTURE_EXCLUSIVE);
        f3 = future_alloc(FUTURE_EXCLUSIVE);

        /* semaphore for printing */
        semaphore print_sem = semcreate(1);

        /* semaphores to wait on thrads */
        semaphore running = semcreate(0);

        future *f_exclusive, *f_shared, *f_queue;

        f_exclusive = future_alloc(FUTURE_EXCLUSIVE);
        f_shared = future_alloc(FUTURE_SHARED);
        f_queue = future_alloc(FUTURE_QUEUE);
        int i;

        printf("\ntesting FUTURE_EXCLUSIVE\n");
        resume(create(future_cons, 1024, 20, "fcons1", 3, f_exclusive, print_sem, running));
        resume(create(future_prod, 1024, 20, "fprod1", 3, f_exclusive, print_sem, running));
        wait(running); wait(running);

        printf("\ntesting FUTURE_SHARED\n");
        resume(create(future_cons, 1024, 20, "fcons2", 3, f_shared, print_sem, running));
        resume(create(future_cons, 1024, 20, "fcons3", 3, f_shared, print_sem, running));
        resume(create(future_cons, 1024, 20, "fcons4", 3, f_shared, print_sem, running));
        resume(create(future_cons, 1024, 20, "fcons5", 3, f_shared, print_sem, running));
        resume(create(future_prod, 1024, 20, "fprod2", 3, f_shared, print_sem, running));
        for (i = 0; i < 5; ++i) wait(running);

        printf("\ntesting FUTURE_QUEUE\n");
        resume(create(future_cons, 1024, 20, "fcons6", 3, f_queue, print_sem, running));
        resume(create(future_cons, 1024, 20, "fcons7", 3, f_queue, print_sem, running));
        resume(create(future_cons, 1024, 20, "fcons7", 3, f_queue, print_sem, running));
        resume(create(future_cons, 1024, 20, "fcons7", 3, f_queue, print_sem, running));
        resume(create(future_prod, 1024, 20, "fprod3", 3, f_queue, print_sem, running));
        resume(create(future_prod, 1024, 20, "fprod4", 3, f_queue, print_sem, running));
        resume(create(future_prod, 1024, 20, "fprod5", 3, f_queue, print_sem, running));
        resume(create(future_prod, 1024, 20, "fprod6", 3, f_queue, print_sem, running));
        for (i = 0; i < 8; ++i) wait(running);

        /* threads are done, free the semaphores and futures */
        semfree(print_sem);
        semfree(running);
        future_free(f1);
        future_free(f2);
        future_free(f3);
    }

    return 0;
}

bool search_dash_f(char *args[])
{
    char dash_f[] = "-f";
    int i = 0;
    while (args[i] != NULL)
    {
        char *arg = args[i];
        if (strcmp(arg, dash_f) == 0) return TRUE;
        i++;
    }
    return FALSE;
}

int search_count_arg(char *args[], int def)
{
    int i = 0;
    while (args[i] != NULL)
    {
        char *arg = args[i];
        if (validate_arg(arg) == TRUE)
        {
            return atoi(arg);
        }
        i++;
    }
    return def;
}

bool validate_arg(char *arg)
{
    int i = 0;
    while (arg[i] != '\0')
    {
        if (!(arg[i] >= '0' && arg[i] <= '9'))
        {
            return FALSE;
        }
        i++;
    }
    return TRUE;
}
