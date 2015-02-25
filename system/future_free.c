#include <future.h>
#include <memory.h>

syscall future_free(future *f)
{
  semfree(f->s);
  semfree(f->produced);
  return memfree(f, sizeof(future));
}
