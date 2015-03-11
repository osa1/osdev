#include <future.h>

syscall set_exclusive (future*, int*);
syscall set_shared    (future*, int*);
syscall set_queue     (future*, int*);

syscall future_set(future *f, int *i)
{
    wait(f->s);
    switch (f->flag)
    {
        case FUTURE_EXCLUSIVE:
            return set_exclusive(f, i);
        case FUTURE_SHARED:
            return set_shared(f, i);
        case FUTURE_QUEUE:
            return set_queue(f, i);
        default:
            signal(f->s);
            return SYSERR;
    }
}

syscall set_exclusive(future *f, int *i)
{
    /* We only use `get_queue` in exclusive mode. Setters are never blocked,
     * they either get a SYSERR(when they try to set an already set future) or
     * OK(when they set an empty future). */
    if (f->state == FUTURE_VALID)
    {
        signal(f->s);
        return SYSERR;
    }

    f->value = i;

    /* signal the getter, if there is one */
    if (f->state == FUTURE_WAITING)
    {
        ready(dequeue(f->get_queue), FALSE);
    }

    f->state = FUTURE_VALID;
    signal(f->s);
    yield();
    return OK;
}

syscall set_shared(future *f, int *i)
{
    signal(f->s);
    return SYSERR;
}

syscall set_queue(future *f, int *i)
{
    signal(f->s);
    return SYSERR;
}
