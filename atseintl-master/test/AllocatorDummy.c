#include <malloc.h>

void
set_custom_trx_allocator_log_level(unsigned int level)
{
}

void
set_custom_trx_allocator_gc_sleep_time(unsigned int seconds)
{
}

void
set_custom_trx_allocator_memory_limit(size_t megabytes)
{
}

void
enter_malloc_context()
{
}

void
exit_malloc_context()
{
}

int
trx_malloc_get_stats(char* buf, size_t buflen)
{
  return 0;
}

void
enable_custom_trx_allocator()
{
}

void*
disable_malloc_context()
{
  return 0;
}

void
enable_malloc_context(void* restore_context)
{
}
