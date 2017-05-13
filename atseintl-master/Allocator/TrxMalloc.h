#ifndef TRX_MALLOC_H
#define TRX_MALLOC_H

#include <stddef.h>

extern "C" {

void
enable_custom_trx_allocator();
void
set_custom_trx_allocator_memory_limit(size_t megabytes);
void
set_custom_trx_allocator_gc_sleep_time(unsigned int seconds);
void
set_custom_trx_allocator_log_level(unsigned int level);

void
enter_malloc_context();
void
exit_malloc_context();
void*
disable_malloc_context();
void
enable_malloc_context(void* ptr);

int
trx_malloc_get_stats(char* buf, size_t buflen);
}

struct MallocContext
{
  MallocContext(bool value = true) : active(value)
  {
    if (active)
    {
      enter_malloc_context();
    }
  }

  ~MallocContext()
  {
    if (active)
    {
      exit_malloc_context();
    }
  }

  bool active;
};

struct MallocContextDisabler
{
  explicit MallocContextDisabler(bool activateNow = true) : context(nullptr), active(false)
  {
    if (activateNow)
      activate();
  }

  ~MallocContextDisabler()
  {
    if (active)
      enable_malloc_context(context);
  }

  void activate()
  {
    if (active)
      return;

    active = true;
    context = disable_malloc_context();
  }

  void* context;
  bool active;
};

#endif
