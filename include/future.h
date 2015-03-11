#ifndef _FUTURE_H_
#define _FUTURE_H_

#include <stddef.h>
#include <thread.h>
#include <queue.h>

/* future states */
#define FUTURE_EMPTY	  0
#define FUTURE_WAITING 	  1
#define FUTURE_VALID 	  2

/* modes of operation for future */
#define FUTURE_EXCLUSIVE  0
#define FUTURE_SHARED     1
#define FUTURE_QUEUE      2

typedef struct futent
{
    int *value;
    int flag;
    int state;

    /* `s` is used to atomically compare-and-swap the state */
    semaphore s;

    qid_typ set_queue;
    qid_typ get_queue;
} future;

/* Interface for system call */
future *future_alloc(int future_flags);
syscall future_free(future*);
syscall future_get(future*, int*);
syscall future_set(future*, int*);

#endif /* _FUTURE_H_ */
