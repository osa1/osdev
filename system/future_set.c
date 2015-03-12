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

    *(f->value) = *i;
    f->tid = EMPTY; /* first come can read */

    /* signal the getter, if there is one */
    if (f->state == FUTURE_WAITING)
    {
        tid_typ t = dequeue(f->get_queue);
        f->tid = t;
        ready(t, FALSE);
    }

    f->state = FUTURE_VALID;
    signal(f->s);
    return OK;
}

syscall set_shared(future *f, int *i)
{
    /* FUTURE_SHARED is set-once. we don't need to keep the tid field updated
     * because anyone can read once the future is filled */
    if (f->state == FUTURE_VALID)
    {
        signal(f->s);
        return SYSERR;
    }

    *(f->value) = *i;

    /* signal all the getters */
    if (f->state == FUTURE_WAITING)
    {
        tid_typ t;
        while ((t = dequeue(f->get_queue)) != EMPTY)
        {
            ready(t, FALSE);
        }
    }

    f->state = FUTURE_VALID;
    signal(f->s);
    return OK;
}

syscall set_queue(future *f, int *i)
{
    if (f->state == FUTURE_VALID)
    {
        /* we have a value waiting to be read, add ourself to the setter queue */
        enqueue(thrcurrent, f->set_queue);
        thrtab[thrcurrent].state = THRWAIT;
        signal(f->s);
        yield();
        return future_set(f, i);
    }
    else if (f->state == FUTURE_WAITING)
    {
        *(f->value) = *i;
        f->state = FUTURE_VALID;
        ready(dequeue(f->get_queue), FALSE);
        signal(f->s);
        yield();
        return OK;
    }
    else /* FUTURE_EMPTY */
    {
        *(f->value) = *i;
        f->state = FUTURE_VALID;
        tid_typ t = dequeue(f->get_queue);
        f->tid = t;
        ready(t, FALSE);
        signal(f->s);
        return OK;
    }
}
