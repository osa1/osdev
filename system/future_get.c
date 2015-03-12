#include <future.h>

syscall get_exclusive (future*, int*);
syscall get_shared    (future*, int*);
syscall get_queue     (future*, int*);

syscall future_get(future *f, int *i)
{
    wait(f->s);
    switch (f->flag)
    {
        case FUTURE_EXCLUSIVE:
            return get_exclusive(f, i);
        case FUTURE_SHARED:
            return get_shared(f, i);
        case FUTURE_QUEUE:
            return get_queue(f, i);
        default:
            signal(f->s);
            return SYSERR;
    }
}

syscall get_exclusive(future *f, int *i)
{
    if (f->state == FUTURE_VALID)
    {
        if (f->tid == thrcurrent || f->tid == EMPTY)
        {
            *i = *(f->value);
            /* other threads should not read the value */
            f->tid = thrcurrent;
            signal(f->s);
            return OK;

        }
        else
        {
            /* value read by some other thread */
            signal(f->s);
            return SYSERR;
        }
    }
    else if (f->state == FUTURE_WAITING)
    {
        /* some other thread is in the queue */
        signal(f->s);
        return SYSERR;
    }
    else /* FUTURE_EMPTY */
    {
        f->state = FUTURE_WAITING;
        f->tid = thrcurrent;
        enqueue(thrcurrent, f->get_queue);
        thrtab[thrcurrent].state = THRWAIT;
        signal(f->s);
        yield();
        return future_get(f, i);
    }
}

syscall get_shared(future *f, int *i)
{
    if (f->state == FUTURE_VALID)
    {
        *i = *(f->value);
        signal(f->s);
        return OK;
    }
    else
    {
        f->state = FUTURE_WAITING;
        enqueue(thrcurrent, f->get_queue);
        thrtab[thrcurrent].state = THRWAIT;
        signal(f->s);
        yield();
        return future_get(f, i);
    }
}

syscall get_queue(future *f, int *i)
{
    if (f->state == FUTURE_VALID
            && (f->tid == thrcurrent || f->tid == EMPTY))
    {
        *i = *(f->value);
        f->state = isempty(f->get_queue) ? FUTURE_EMPTY : FUTURE_WAITING;
        f->tid = EMPTY;
        /* normally we would need a f->setter_tid field, but we don't care
         * about setters' FIFO property, so just signal one */
        ready(dequeue(f->set_queue), FALSE);
        signal(f->s);
        return OK;
    }
    else if (f->state == FUTURE_WAITING)
    {
        enqueue(thrcurrent, f->get_queue);
        thrtab[thrcurrent].state = THRWAIT;
        ready(dequeue(f->set_queue), FALSE);
        /* some other thread should be waiting, so don't change f->tid */
        signal(f->s);
        yield();
        return future_get(f, i);
    }
    else /* FUTURE_EMPTY or f->tid != thrcurrent */
    {
        enqueue(thrcurrent, f->get_queue);
        thrtab[thrcurrent].state = THRWAIT;
        ready(dequeue(f->set_queue), FALSE);
        if (f->state == FUTURE_EMPTY)
        {
            f->tid = thrcurrent;
            f->state = FUTURE_WAITING;
        }
        signal(f->s);
        yield();
        return future_get(f, i);
    }
}
