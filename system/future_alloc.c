#include <future.h>
#include <memory.h>

future *future_alloc(int future_flags)
{
    future *f = (future*) memget(sizeof(future));
    if (f == (void*)SYSERR)
    {
        return NULL;
    }
    f->value = NULL;
    f->flag = future_flags;
    f->state = FUTURE_EMPTY;
    f->tid = -1;

    /* initially we're allowing threads to read and write `state` field */
    f->s = semcreate(1);

    /* initially queues are empty */
    f->set_queue = queinit();
    f->get_queue = queinit();

    return f;
}
