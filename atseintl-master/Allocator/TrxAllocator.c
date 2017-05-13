#include <assert.h>
#include <inttypes.h>
#include <malloc.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

// macros to let us do branch prediction.
#define LIKELY(x) __builtin_expect((long)(x), (long)1)
#define UNLIKELY(x) __builtin_expect((long)(x), (long)0)

#define SUPER_BLOCK_SIZE ((size_t)(4096))

static pthread_mutex_t heap_mutex = PTHREAD_MUTEX_INITIALIZER;

// the start of super blocks that are not yet initialized. Will always
// be somewhere between start_of_heap and end_of_heap.
static uint8_t* uninit_super_blocks = 0;

// the boundaries of the heap we have for doing our allocations
static void* start_of_heap = 0;
static void* end_of_heap = 0;

static int use_trx_allocator = 0;

#define MAX_TIME_STRING_SIZE 128
#define LOG_LEVEL_FATAL 50000
#define LOG_LEVEL_ERROR 40000
#define LOG_LEVEL_WARN 30000
#define LOG_LEVEL_INFO 20000
#define LOG_LEVEL_DEBUG 10000
static unsigned int log_level = LOG_LEVEL_ERROR;
void
set_custom_trx_allocator_log_level(unsigned int level)
{
  log_level = level;
}

static unsigned int gc_sleep_time = 300;
void
set_custom_trx_allocator_gc_sleep_time(unsigned int seconds)
{
  gc_sleep_time = seconds;
}

#define ADDR_ON_HEAP(addr) ((void*)(addr) >= start_of_heap&&(void*)(addr) < end_of_heap)

// a free item within a super block. This object will reside in
// the memory that is actually used by an object once the memory
// is allocated.
struct FreeListItem
{
  struct FreeListItem* next;
};

typedef struct FreeListItem FreeListItem;

// super blocks always cycle through these states:
typedef enum
{ SUPER_BLOCK_TRX_OWNS, // a transaction currently owns this super block
  SUPER_BLOCK_FULL, // sb has lots of objects allocated, but is
  // now owned by a transaction. When the number
  // of objects allocated falls beneath a threshold
  // it can be moved to the sb free list
  SUPER_BLOCK_MAIN_FREE_LIST, // sb is on the free list and may be
  // acquired by a transaction.
} SUPER_BLOCK_MODE;

struct SuperBlock
{
  pthread_t owner;
  int owner_nalloc;
  FreeListItem* owner_free_list;

  pthread_mutex_t mutex;
  int obj_size; // object sizes that may be allocated by this sb
  uint8_t* uninit; // pointer to start of uninitialized objects
  FreeListItem* free_list; // pointer to free list of items of obj_size
  int nalloc; // number of items allocated
  SUPER_BLOCK_MODE mode;

  // this field is considered to be synchronized by the
  // sb_free_list_mutex, since it keeps track of where the super block
  // is in the super block free list. It is NOT synchronized
  // by the above 'mutex' member of this struct.
  struct SuperBlock* next;

  // a flag which is marked to tell us that during a sweep of the
  // garbage collector, this super block was owned. When it is recycled,
  // the flag will be cleared. If the garbage collector runs again
  // before this flag is cleared, it is assumed that the block has
  // permanent allocations and the block needs to be marked for its
  // permanent allocations and recycled.
  int needs_gc;

  // the number of allocations sitting in the block that are considered
  // permanent. Blocks are designed to have allocations from
  // transactions that are released very quickly. If a block has
  // allocations that survive two gc cycles, they are considered
  // permanent allocations.
  int npermanent_alloc;
};

typedef struct SuperBlock SuperBlock;

#define ADDR_TO_SUPERBLOCK(addr)                                                                   \
  ((SuperBlock*)(((size_t)(addr)) - (((size_t)(addr)) % SUPER_BLOCK_SIZE)))
#define SUPER_BLOCK_MAX_ALLOCS(sz) ((SUPER_BLOCK_SIZE - sizeof(SuperBlock)) / sz)

#define ALIGNMENT (sizeof(void*))

#define ROUND_UP(n)                                                                                \
  ((n % ALIGNMENT) == 0 ? ((n) == 0 ? ALIGNMENT : (n)) : (n) + (ALIGNMENT - ((n) % ALIGNMENT)))

#define MAX_MALLOC_SIZE ((size_t)1024)
#define SIZE_TO_BUCKET(sz) (sz / ALIGNMENT)
#define NUM_BUCKETS ((SIZE_TO_BUCKET(MAX_MALLOC_SIZE)) + 1)

#define SUPER_BLOCK_GROUP_SIZE (1024)

// A central place where we store linked lists of blank super blocks,
// ready to be used. Synchronized by heap_mutex. Each linked list will
// have SUPER_BLOCK_GROUP_SIZE super blocks in it.
//
// The size of the buffer is intended to be one which won't
// realistically be outgrown.
SuperBlock* global_super_block_groups[4096 * 4];

// The current number of items in global_super_block_groups.
int global_super_block_groups_size;

// A thread-sharded store of free super blocks. It contains lists of
// super blocks which are partly free and have an assigned size.
// lists[0] contains unsized super blocks. When lists[0] grows to
// SUPER_BLOCK_GROUP_SIZE, the list will be moved to spare_list and
// we will start over. If spare_list is already populated with a list,
// that list will be moved to global_super_block_groups.
//
// When we want to get a new super block, we look in lists for a list
// of the specific size we want. If there's none in our specific size,
// we look at lists[0]. If lists[0] is empty, we move spare_list to
// lists[0]. If spare_list is empty then we lock the global heap_mutex
// and move a group into lists[0]. If this fails we expand the heap
// and get a new super block.
//
// The idea is to store as little memory in each shard as possible, yet
// make it so we access the global storage as infrequently as possible.
struct SuperBlockFreeList
{
  pthread_mutex_t mutex;
  SuperBlock* lists[NUM_BUCKETS];
  int nblocks;
  SuperBlock* spare_list;
};

typedef struct SuperBlockFreeList SuperBlockFreeList;

static void
init_super_block_free_list(SuperBlockFreeList* free_list)
{
  memset(free_list, 0, sizeof(*free_list));
  pthread_mutex_init(&free_list->mutex, NULL);
}

// The number of free lists we have, with threads hashing to a free
// list. The higher this value, the more memory usage overhead there
// will be. The lower this value, the more possibility of contention.
#define NUM_FREE_LISTS (7)

static SuperBlockFreeList sb_free_lists[NUM_FREE_LISTS];

static __thread SuperBlockFreeList* thread_free_list = NULL;

// A pointer to a free list that is used for freeing super blocks
// in a balanced manner. Each time we access it we rotate it to
// the next free lit.
static __thread SuperBlockFreeList* balanced_free_list = NULL;

typedef struct
{
  void* (*malloc)(size_t);
  void (*free)(void*);
  void* (*calloc)(size_t, size_t);
  void* (*realloc)(void*, size_t);
  const char* name;
} AllocatorDispatch;

extern AllocatorDispatch allocator_dispatch;

static void
init_super_block(SuperBlock* sb, int obj_size)
{
  pthread_mutex_init(&sb->mutex, NULL);
  sb->owner = 0;
  sb->owner_nalloc = 0;
  sb->owner_free_list = 0;
  sb->obj_size = obj_size;
  sb->uninit = (uint8_t*)(sb + 1);
  sb->free_list = 0;
  sb->nalloc = 0;
  sb->next = 0;
  sb->mode = SUPER_BLOCK_MAIN_FREE_LIST;
  sb->needs_gc = 0;
  sb->npermanent_alloc = 0;
}

static void
clear_super_block_size(SuperBlock* sb)
{
  sb->obj_size = -1;
  sb->uninit = (uint8_t*)(sb + 1);
  sb->free_list = 0;
}

// a function which we can use to get detailed stats on the state
// of the heap
static void
detailed_stats_debug_only()
{
  int nhalf = 0;
  int sizes[10000];
  int n;
  for (n = 0; n != 10000; ++n)
  {
    sizes[n] = 0;
  }
  int ntotal = 0, nowned = 0, nsized = 0;
  int mode[3] = { 0, 0, 0 };
  int nalloc = 0, owner = 0;
  unsigned char* c;
  for (c = start_of_heap; c < uninit_super_blocks; c += SUPER_BLOCK_SIZE)
  {
    SuperBlock* sb = (SuperBlock*)c;
    ++ntotal;
    if (sb->owner)
    {
      ++nowned;
    }

    mode[sb->mode]++;

    if (sb->obj_size > 0)
    {
      ++nsized;
      nalloc += sb->nalloc;
      owner += sb->owner_nalloc;

      sizes[sb->obj_size]++;
      if (sb->nalloc < SUPER_BLOCK_MAX_ALLOCS(sb->obj_size) / 2)
      {
        ++nhalf;
      }
    }
  }

  // output number of allocations in each size.
  for (n = 0; n != 10000; ++n)
  {
    if (sizes[n])
    {
      fprintf(stderr, "%d: %d; ", n, sizes[n]);
    }
  }

  fprintf(stderr, "\n");
  fprintf(stderr,
          "total: %d; owned: %d; sized: %d; half: %d; mode owned: %d; full: %d; free: %d; nalloc: "
          "%d; owner: %d; global groups: %d\n",
          ntotal,
          nowned,
          nsized,
          nhalf,
          mode[0],
          mode[1],
          mode[2],
          nalloc,
          owner,
          global_super_block_groups_size);
}

static void
add_super_block_to_free_list(SuperBlock* sb)
{
  if (UNLIKELY(!thread_free_list))
  {
    thread_free_list = &sb_free_lists[pthread_self() % NUM_FREE_LISTS];
  }

  // the super block doesn't need to be garbage collected, since it's
  // being moved to the free list here.
  sb->needs_gc = 0;

  const int index = sb->obj_size == -1 ? 0 : sb->obj_size / ALIGNMENT;

  SuperBlockFreeList* free_list;
  if (LIKELY(index == 0))
  {
    free_list = thread_free_list;
  }
  else
  {
    // for a sized block, we want to make sure we recycle it back
    // in a balanced manner, since sized blocks have no other
    // mechanism to share into the global pool.
    if (!balanced_free_list)
    {
      balanced_free_list = thread_free_list;
    }

    free_list = balanced_free_list;

    // rotate the balanced free list pointer to the next free list.
    if (++balanced_free_list == sb_free_lists + NUM_FREE_LISTS)
    {
      balanced_free_list = sb_free_lists;
    }
  }

  SuperBlock* spare_list = 0;

  pthread_mutex_lock(&free_list->mutex);
  sb->next = free_list->lists[index];
  free_list->lists[index] = sb;

  if (UNLIKELY(index == 0 && ++free_list->nblocks == SUPER_BLOCK_GROUP_SIZE))
  {
    // the thread free list has now reached its maximum size. Move
    // it off to the spare list.
    spare_list = free_list->spare_list;
    free_list->spare_list = free_list->lists[index];
    free_list->lists[index] = 0;
    free_list->nblocks = 0;
  }
  pthread_mutex_unlock(&free_list->mutex);

  if (UNLIKELY(spare_list))
  {
    // we have a spare list which must be moved to the global
    // array of free lists.
    pthread_mutex_lock(&heap_mutex);
    assert(global_super_block_groups_size <
           sizeof(global_super_block_groups) / sizeof(*global_super_block_groups));

    global_super_block_groups[global_super_block_groups_size++] = spare_list;
    pthread_mutex_unlock(&heap_mutex);
  }
}

void
formatTime(struct timeval* end_time, char* formattedTime)
{
  strcpy(formattedTime, "");
  struct timeval t;
  if (NULL == end_time)
  {
    gettimeofday(&t, NULL);
    end_time = &t;
  }
  struct tm tmstruct;
  struct tm* tmptr = localtime_r(&end_time->tv_sec, &tmstruct);
  if (tmptr != NULL)
  {
    static const char* format = "%Y-%m-%d %H:%M:%S";
    strftime(formattedTime, (size_t)MAX_TIME_STRING_SIZE, format, &tmstruct);
  }
}

// function which walks over the heap and collects and recycles blocks
// that have long-term allocations in them.
static void
garbage_collect()
{
  int ncollect = 0, nmark = 0;
  struct timeval start_time, end_time;
  gettimeofday(&start_time, NULL);

  pthread_mutex_lock(&heap_mutex);
  void* start = start_of_heap;
  void* end = uninit_super_blocks;
  pthread_mutex_unlock(&heap_mutex);

  unsigned char* c;
  for (c = start; c != end; c += SUPER_BLOCK_SIZE)
  {
    SuperBlock* sb = (SuperBlock*)c;
    if (sb->mode == SUPER_BLOCK_FULL)
    {
      int add_to_free_list = 0;
      pthread_mutex_lock(&sb->mutex);
      if (sb->mode == SUPER_BLOCK_FULL && sb->nalloc < SUPER_BLOCK_MAX_ALLOCS(sb->obj_size) / 2)
      {
        if (sb->needs_gc)
        {
          sb->mode = SUPER_BLOCK_MAIN_FREE_LIST;
          add_to_free_list = 1;

          // this many allocations in this block have made it
          // this long, so they are thought to be permanent.
          sb->npermanent_alloc = sb->nalloc;
          ++ncollect;
        }
        else
        {
          sb->needs_gc = 1;
          ++nmark;
        }
      }
      pthread_mutex_unlock(&sb->mutex);

      if (add_to_free_list)
      {
        add_super_block_to_free_list(sb);
      }
    }
  }

  if (LOG_LEVEL_INFO >= log_level)
  {
    gettimeofday(&end_time, NULL);
    const int usecs = (end_time.tv_sec - start_time.tv_sec) * 1000000 +
                      ((int)end_time.tv_usec - (int)start_time.tv_usec);
    char formattedTime[MAX_TIME_STRING_SIZE];
    formatTime(&end_time, formattedTime);
    fprintf(stderr,
            "GARBAGE COLLECTION (%s): (%dus) collected %d blocks; marked %d blocks\n",
            formattedTime,
            usecs,
            ncollect,
            nmark);
  }
}

static void*
gc_thread(void* ptr)
{
  while (1)
  {
    sleep(gc_sleep_time);
    garbage_collect();
    if (LOG_LEVEL_DEBUG >= log_level)
    {
      detailed_stats_debug_only();
    }
  }

  return 0;
}

static SuperBlock*
allocate_new_super_block_group()
{
  pthread_mutex_lock(&heap_mutex);
  if (UNLIKELY(uninit_super_blocks == end_of_heap))
  {
    pthread_mutex_unlock(&heap_mutex);
    return 0;
  }

  uint8_t* begin = uninit_super_blocks;
  size_t size = SUPER_BLOCK_SIZE * SUPER_BLOCK_GROUP_SIZE;
  if (UNLIKELY(size > (uint8_t*)end_of_heap - begin))
  {
    size = (uint8_t*)end_of_heap - begin;
  }

  uint8_t* end = begin + size;
  uninit_super_blocks = (void*)end;
  pthread_mutex_unlock(&heap_mutex);

  SuperBlock* result = 0;
  while (begin != end)
  {
    SuperBlock* sb = (SuperBlock*)begin;
    init_super_block(sb, -1);
    sb->next = result;
    result = sb;
    begin += SUPER_BLOCK_SIZE;
  }

  return result;
}

static SuperBlock*
get_super_block(int size)
{
  pthread_mutex_lock(&thread_free_list->mutex);

  // first try to get a block which supplies objects of the size
  // we want.
  SuperBlock** free_list = &thread_free_list->lists[size / ALIGNMENT];
  if (LIKELY(!*free_list))
  {
    // try to get a clear block
    free_list = &thread_free_list->lists[0];

    if (UNLIKELY(!*free_list))
    {
      // try to copy the spare list of clear blocks over to
      // the main list.
      *free_list = thread_free_list->spare_list;
      thread_free_list->spare_list = 0;

      if (!*free_list)
      {
        // try to get a block from the global store.
        pthread_mutex_lock(&heap_mutex);
        if (global_super_block_groups_size)
        {
          *free_list = global_super_block_groups[--global_super_block_groups_size];
        }
        pthread_mutex_unlock(&heap_mutex);

        if (!*free_list)
        {
          // allocate a new group.
          *free_list = allocate_new_super_block_group();
          if (!*free_list)
          {
            // give up. :-(
            pthread_mutex_unlock(&thread_free_list->mutex);
            return 0;
          }
        }
      }

      // we acquired a new group, so we know we have a full
      // group worth of blocks in this free list.
      thread_free_list->nblocks = SUPER_BLOCK_GROUP_SIZE;
    }

    // we're going to be taking one of our cleared blocks, so
    // decrement the count.
    thread_free_list->nblocks--;
  }

  SuperBlock* result = *free_list;
  *free_list = (*free_list)->next;
  pthread_mutex_unlock(&thread_free_list->mutex);

  pthread_mutex_lock(&result->mutex);
  result->owner = pthread_self();
  result->owner_nalloc = 0;
  result->owner_free_list = 0;
  result->obj_size = size;
  result->nalloc++;
  result->mode = SUPER_BLOCK_TRX_OWNS;
  result->next = 0;
  pthread_mutex_unlock(&result->mutex);

  return result;
}

struct TrxAllocInfo
{
  SuperBlock* buckets[NUM_BUCKETS];
};

typedef struct TrxAllocInfo TrxAllocInfo;

static __thread TrxAllocInfo* trx_info = 0;

static TrxAllocInfo*
create_trx_alloc_info()
{
  return (TrxAllocInfo*)(*allocator_dispatch.calloc)((size_t)1, sizeof(TrxAllocInfo));
}

static void
merge_free_lists(SuperBlock* sb)
{
  if (sb->owner_free_list)
  {
    if (!sb->free_list)
    {
      sb->free_list = sb->owner_free_list;
      sb->owner_free_list = 0;
      return;
    }

    // We want to merge the owner_free_list into the free_list.
    // It is unlikely that the free_list will be very long at
    // this stage, since until the end of the transaction,
    // most frees are done by the thread that malloced. So, we
    // walk through the free list and find the end and then
    // add the owner_free_list on the end.
    FreeListItem* tail = sb->free_list;
    while (tail->next)
    {
      tail = tail->next;
    }

    tail->next = sb->owner_free_list;

    sb->owner_free_list = 0;
  }
}

static void
destroy_trx_alloc_info(TrxAllocInfo* info)
{
  int n;
  for (n = 0; n != NUM_BUCKETS; ++n)
  {
    SuperBlock* sb = info->buckets[n];
    if (sb)
    {
      pthread_mutex_lock(&sb->mutex);
      sb->owner = 0;
      sb->nalloc += sb->owner_nalloc;
      sb->owner_nalloc = 0;

      merge_free_lists(sb);

      int nalloc = --sb->nalloc;
      int add_to_free_list = 0;
      if (nalloc <= sb->npermanent_alloc)
      {
        sb->mode = SUPER_BLOCK_MAIN_FREE_LIST;
        if (nalloc == 0)
        {
          clear_super_block_size(sb);
        }

        add_to_free_list = 1;
      }
      else
      {
        sb->mode = SUPER_BLOCK_FULL;
      }

      pthread_mutex_unlock(&sb->mutex);

      if (add_to_free_list)
      {
        add_super_block_to_free_list(sb);
      }
    }
  }

  (*allocator_dispatch.free)(info);
}

void
set_custom_trx_allocator_memory_limit(size_t megabytes)
{
  if (start_of_heap != 0)
  {
    return;
  }

  size_t bytes = megabytes * ((size_t)(1024 * 1024));
  uninit_super_blocks = (uint8_t*)(mmap(NULL,
                                        bytes + SUPER_BLOCK_SIZE,
                                        PROT_READ | PROT_WRITE,
                                        MAP_PRIVATE | MAP_ANONYMOUS,
                                        -1,
                                        (size_t)0));
  if (uninit_super_blocks == (void*)-1)
  {
    fprintf(stderr, "could not allocate heap for trx allocator\n");
    uninit_super_blocks = NULL;
  }
  while ((((size_t)uninit_super_blocks) % SUPER_BLOCK_SIZE) != 0)
  {
    uninit_super_blocks += 4096;
  }
  start_of_heap = uninit_super_blocks;
  end_of_heap = uninit_super_blocks + bytes;

  // initialize the free lists.
  int n;
  for (n = 0; n != NUM_FREE_LISTS; ++n)
  {
    init_super_block_free_list(&sb_free_lists[n]);
  }
}

void
exit_malloc_context();

void
enter_malloc_context()
{
  if (!use_trx_allocator || !start_of_heap)
  {
    return;
  }

  if (trx_info)
  {
    exit_malloc_context();
  }

  trx_info = create_trx_alloc_info();

  if (!thread_free_list)
  {
    thread_free_list = &sb_free_lists[pthread_self() % NUM_FREE_LISTS];
  }
}

void
exit_malloc_context()
{
  if (!use_trx_allocator || trx_info == 0)
  {
    return;
  }

  destroy_trx_alloc_info(trx_info);
  trx_info = 0;
}

void*
dlmalloc(size_t);
void
dlfree(void*);
void*
dlcalloc(size_t nemb, size_t size);
void*
dlrealloc(void* ptr, size_t size);

const static AllocatorDispatch allocator_dispatch_dl = { &dlmalloc, &dlfree, &dlcalloc, &dlrealloc,
                                                         "DL" };

void*
je_malloc(size_t);
void
je_free(void*);
void*
je_calloc(size_t nemb, size_t size);
void*
je_realloc(void* ptr, size_t size);

const static AllocatorDispatch allocator_dispatch_je = { &je_malloc, &je_free, &je_calloc,
                                                         &je_realloc, "JE" };

/* The allocator from Intel TBB library */

void*
scalable_malloc(size_t);
void
scalable_free(void*);
void*
scalable_calloc(size_t nemb, size_t size);
void*
scalable_realloc(void* ptr, size_t size);

const static AllocatorDispatch allocator_dispatch_tbb = { &scalable_malloc, &scalable_free, &scalable_calloc,
                                                         &scalable_realloc, "TBB" };

static void
init_allocator_dispatch(void)
{
  char* name = getenv("TRX_ALLOCATOR");

  if (name == NULL || *name == '\0' || !strcmp(name, "DL"))
  {
    allocator_dispatch = allocator_dispatch_dl;
    return;
  }

  if (!strcmp(name, "JE"))
  {
    allocator_dispatch = allocator_dispatch_je;
    return;
  }

  if (!strcmp(name, "TBB"))
  {
    allocator_dispatch = allocator_dispatch_tbb;
    return;
  }

  fprintf(stderr, "UNKNOWN TRX_ALLOCATOR ENVIRONMENT VARIABLE!\n");
  fflush(stderr);
  abort();
}

static void*
malloc_dispatch(size_t size)
{
  init_allocator_dispatch();
  return (*allocator_dispatch.malloc)(size);
}

static void
free_dispatch(void* ptr)
{
  init_allocator_dispatch();
  (*allocator_dispatch.free)(ptr);
}

static void*
calloc_dispatch(size_t nemb, size_t size)
{
  init_allocator_dispatch();
  return (*allocator_dispatch.calloc)(nemb, size);
}

static void*
realloc_dispatch(void* ptr, size_t size)
{
  init_allocator_dispatch();
  return (*allocator_dispatch.realloc)(ptr, size);
}

AllocatorDispatch allocator_dispatch = { &malloc_dispatch, &free_dispatch, &calloc_dispatch,
                                         &realloc_dispatch, "NO ALLOCATOR" };

void*
malloc(size_t size)
{
  if (UNLIKELY(trx_info == 0 || size > MAX_MALLOC_SIZE))
  {
    return (*allocator_dispatch.malloc)(size);
  }

  size = ROUND_UP(size);

  int bucket = SIZE_TO_BUCKET(size);
  SuperBlock* sb = trx_info->buckets[bucket];
  if (UNLIKELY(sb == 0))
  {
    sb = get_super_block((int)size);
    if (sb == 0)
    {
      exit_malloc_context();
      if (LOG_LEVEL_INFO >= log_level)
      {
        fprintf(stderr, "TRX ALLOCATOR MEMORY EXHAUSTION!\n");
      }
      return (*allocator_dispatch.malloc)(size);
    }

    assert(sb->obj_size == (int)size);

    trx_info->buckets[bucket] = sb;
  }

  // the fast case: initialize from the uninit block, and we
  // can do so without locking.
  if (LIKELY(sb->uninit <= ((uint8_t*)sb) + SUPER_BLOCK_SIZE - size))
  {
    void* const res = (void*)sb->uninit;
    sb->uninit += size;
    sb->owner_nalloc++;
    return res;
  }

  // try to get from the owner's free list, which won't require locking.
  if (LIKELY(sb->owner_free_list))
  {
    void* const res = (void*)sb->owner_free_list;
    sb->owner_free_list = sb->owner_free_list->next;
    sb->owner_nalloc++;
    return res;
  }

  pthread_mutex_lock(&sb->mutex);
  void* res = 0;

  if (sb->owner_nalloc)
  {
    sb->nalloc += sb->owner_nalloc;
    sb->owner_nalloc = 0;
    if (sb->nalloc > SUPER_BLOCK_MAX_ALLOCS(size))
    {
      sb->nalloc--;
      sb->mode = SUPER_BLOCK_FULL;
      sb->owner = 0;
      merge_free_lists(sb);
      trx_info->buckets[bucket] = 0;
      pthread_mutex_unlock(&sb->mutex);
      return malloc(size);
    }
  }

  // move the free list to the owner free list so next time we can
  // get without locking
  res = sb->free_list;
  sb->owner_free_list = sb->free_list->next;
  sb->free_list = 0;

  // see if the super block is now full. If it is, then release it
  // from being owned by us. Otherwise just increment its reference
  // count.
  if (sb->nalloc == SUPER_BLOCK_MAX_ALLOCS(size))
  {
    // we should increment the reference count because we
    // allocated an item, but we decrement the reference count
    // because we are releasing the block, so it works out to
    // a no-op.
    sb->mode = SUPER_BLOCK_FULL;
    sb->owner = 0;
    merge_free_lists(sb);
    trx_info->buckets[bucket] = 0;
  }
  else
  {
    ++sb->nalloc;
  }

  pthread_mutex_unlock(&sb->mutex);

  return res;
}

void
free(void* ptr)
{
  if (ptr == 0)
  {
    return;
  }

  // if this item didn't come from us, it must have come from dlfree
  if (UNLIKELY(!ADDR_ON_HEAP(ptr)))
  {
    (*allocator_dispatch.free)(ptr);
    return;
  }

  // work out which super block we're allocated from
  SuperBlock* sb = ADDR_TO_SUPERBLOCK(ptr);

  if (LIKELY(sb->owner == pthread_self()))
  {
    sb->owner_nalloc--;
    if (sb->uninit - sb->obj_size == ptr)
    {
      sb->uninit = ptr;
    }
    else
    {
      FreeListItem* item = (FreeListItem*)ptr;
      item->next = sb->owner_free_list;
      sb->owner_free_list = item;
    }

    return;
  }

  // lock the super block and then perform operations to free it:
  // decrement the reference count, and add the freed memory
  // to the super block's free list.
  pthread_mutex_lock(&sb->mutex);
  int nalloc = --(sb->nalloc);

  {
    FreeListItem* item = (FreeListItem*)ptr;
    item->next = sb->free_list;
    sb->free_list = item;
  }

  if (UNLIKELY(nalloc < sb->npermanent_alloc))
  {
    sb->npermanent_alloc = nalloc;
  }

  // Determine if we want to move the super block to the free list.
  // We want to do so if it's currently in the 'full' state
  //(i.e. neither owned by a transaction, nor already in the free
  // list), and has less than half of its objects allocated.
  int move_to_free_list = 0;
  if (UNLIKELY(sb->mode == SUPER_BLOCK_FULL && nalloc <= sb->npermanent_alloc))
  {
    move_to_free_list = 1;
    sb->mode = SUPER_BLOCK_MAIN_FREE_LIST;
    assert(sb->owner == 0);
    // if the super block has no more objects allocated at all, then
    // we can clear it completely, meaning that it can be re-used
    // to take allocations for any size object.
    if (nalloc == 0)
    {
      clear_super_block_size(sb);
    }
  }

  pthread_mutex_unlock(&sb->mutex);

  // if we determined that it needs to be moved to the free list,
  // then do so here. We waited until we were outside the mutex,
  // because we never acquire the free list mutex after acquiring
  // a super block mutex (this could lead to deadlocks).
  if (UNLIKELY(move_to_free_list))
  {
    add_super_block_to_free_list(sb);
  }
}

void*
calloc(size_t nemb, size_t size)
{
  void* res = malloc(nemb * size);
  if (res)
  {
    memset(res, 0, nemb * size);
  }

  return res;
}

void*
realloc(void* ptr, size_t size)
{
  if (ptr == 0)
  {
    return malloc(size);
  }

  if (ADDR_ON_HEAP(ptr))
  {
    SuperBlock* sb = ADDR_TO_SUPERBLOCK(ptr);
    size_t old_size = (size_t)sb->obj_size;
    void* res = malloc(size);
    memcpy(res, ptr, old_size < size ? old_size : size);
    free(ptr);
    return res;
  }
  else
  {
    return (*allocator_dispatch.realloc)(ptr, size);
  }
}

int
trx_malloc_get_stats(char* buf, size_t buflen)
{
  const char* secondaryAllocator = allocator_dispatch.name;

  if (!start_of_heap)
  {
    return -1;
  }

  const int malloc_size = (int)((uninit_super_blocks - (uint8_t*)start_of_heap) / (1024 * 1024));
  const int free_list_size = 0; //(sb_free_list_size*SUPER_BLOCK_SIZE)/(1024*1024);

  int res = snprintf(buf,
                     buflen - 1,
                     "SECONDARY ALLOC: %s\nMALLOC SIZE: %d/%d\n",
                     secondaryAllocator,
                     free_list_size,
                     malloc_size);

  buf[buflen - 1] = 0;
  return res;
}

void
enable_custom_trx_allocator()
{
  use_trx_allocator = 1;

  // start the thread which performs garbage collection.
  pthread_t thread;
  pthread_create(&thread, NULL, gc_thread, NULL);
}

void*
disable_malloc_context()
{
  void* res = (void*)trx_info;
  trx_info = 0;
  return res;
}

void
enable_malloc_context(void* restore_context)
{
  trx_info = (TrxAllocInfo*)(restore_context);
}
