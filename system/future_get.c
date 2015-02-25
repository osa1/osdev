#include <future.h>

syscall future_get(future *f, int *i)
{
  /* we're going to read future state, lock it */
  wait(f->s);

  if (f->state == FUTURE_EMPTY) {
    /* update the state, release the lock and wait for value to be filled */
    f->state = FUTURE_WAITING;
    f->tid = gettid();
    signal(f->s);
    wait(f->produced);

    /* at this point we know the value is filled, and state should be set to
     * FUTURE_VALID. still run a sanity check. */
    if (f->state != FUTURE_VALID) {
      /* probably a bug in future_set or future_get or a race condition */
      return SYSERR; /* TODO: is there a way to set an error message somewhere? */
      /* FUTURE_VALID is actually completely redundant, our semaphore does the
       * same job already */
    }

    /* update the state and read the value */
    /* alternatively, we could just recursively call future_get again. asusming
     * we're running some kind of embedded hardware with very limited memory, I
     * guess this version is better even though contains some minor amount of
     * duplication */
    wait(f->s);
    f->state = FUTURE_EMPTY;
    *i = *(f->value);
    signal(f->s);

    return OK;

  } else if (f->state == FUTURE_WAITING) {
    signal(f->s);
    return SYSERR;

  } else if (f->state == FUTURE_VALID) {
    /* update the state and read the value */
    f->state = FUTURE_EMPTY;
    *i = *(f->value);
    signal(f->s);

    return OK;

  } else {
    signal(f->s);
    /* impossible or unhandled case */
    return SYSERR;
  }
}
