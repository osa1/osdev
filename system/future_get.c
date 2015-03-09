#include <future.h>

syscall future_get(future *f, int *i)
{
    irqmask im;
    /* we're going to read future state, lock it */
    wait(f->s);
    im = disable();

    if (f->state == FUTURE_EMPTY)
    {
        /* update the state, add yourself to `get_queue` */
        f->state = FUTURE_WAITING;
        signal(f->s);
        restore(im);
        wait(f->get_queue);

        wait(f->s);
        im = disable();
        /* at this point we know the value is filled, and state should be set
         * to FUTURE_VALID. still run a sanity check. */
        if (f->state != FUTURE_VALID)
        {
            /* probably a bug in future_set or future_get or a race condition */
            signal(f->s);
            restore(im);
            return SYSERR;
        }

        /* update the state and read the value */
        f->state = FUTURE_EMPTY;
        *i = *(f->value);
        signal(f->s);
        restore(im);

        return OK;

    }
    else if (f->state == FUTURE_WAITING)
    {
        signal(f->s);
        restore(im);
        return SYSERR;

    }
    else if (f->state == FUTURE_VALID)
    {
        /* update the state and read the value */
        f->state = FUTURE_EMPTY;
        *i = *(f->value);
        signal(f->s);
        restore(im);

        return OK;
    }
    else
    {
        signal(f->s);
        restore(im);
        /* impossible or unhandled case */
        return SYSERR;
    }
}
