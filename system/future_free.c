#include <future.h>
#include <memory.h>

syscall future_free(future *f)
{
    semfree(f->s);
    semfree(f->set_queue);
    semfree(f->get_queue);
    return memfree(f, sizeof(future));
}
