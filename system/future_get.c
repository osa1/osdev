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
        *i = *(f->value);
        f->state = isempty(f->get_queue) ? FUTURE_WAITING : FUTURE_EMPTY;
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

syscall get_shared(future *f, int *i)
{
    signal(f->s);
    return SYSERR;
}

syscall get_queue(future *f, int *i)
{
    signal(f->s);
    return SYSERR;
}
