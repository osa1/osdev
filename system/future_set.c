#include <future.h>

syscall future_set(future *f, int *i)
{
    irqmask im;
    int current_state;
    wait(f->s);
    im = disable();
    current_state = f->state;
    if (current_state == FUTURE_EMPTY || current_state == FUTURE_WAITING)
    {
        f->state = FUTURE_VALID;
        f->value = i;
        /* we're done updating the state, release the state lock */
        signal(f->s);
        if (current_state == FUTURE_WAITING)
        {
            /* there were blocked threads. signal all the threads. new threads
             * will check `state`, and read the value instead of adding
             * themselves to the `get_queue` */
            signaln(f->get_queue, NTHREAD);
        }
        restore(im);
        return OK;
    }
    else
    {
        /* future is already filled, can't squash the current value */
        signal(f->s);
        restore(im);
        return SYSERR;
    }
}
