#include <future.h>

syscall future_set(future *f, int *i)
{
    wait(f->s);
    if (f->state == FUTURE_EMPTY || f->state == FUTURE_WAITING)
    {
        f->state = FUTURE_VALID;
        f->value = i;
        signal(f->produced);
        signal(f->s);
        return OK;
    }
    else
    {
        /* future is already filled, can't squash the current value */
        signal(f->s);
        return SYSERR;
    }
}
