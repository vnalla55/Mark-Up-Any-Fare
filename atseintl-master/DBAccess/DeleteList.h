#pragma once

#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Memory/CompositeManager.h"
#include "Common/Memory/Config.h"
#include "Common/StopWatch.h"
#include "Common/Thread/TSEFastMutex.h"
#include "Common/Thread/TSELockGuards.h"
#include "DBAccess/raw_ptr.h"
#include "Util/BranchPrediction.h"

#include <algorithm>
#include <cstdio>
#include <functional>
#include <memory>

#include <pthread.h>

namespace tse
{

class DataHandle;// tests to compile

/**
 *  This keeps track of pointers & smart pointers that are to be freed on exit
 *  This should only be allocated on the stack and never copied.
 */
class DeleteList
{
private:
  class DeleterBase
  {
  public:
    virtual void deallocate(void* ptr) const = 0;
    virtual const char* typeName() const = 0;
    virtual size_t typeSize() const = 0;
    virtual bool pooled() const = 0;

  protected:
    virtual ~DeleterBase() {}
  };

  template <typename T>
  class PointerDeleter : public DeleterBase
  {
  public:
    static const PointerDeleter<T>& instance() { return _instance; }

  private:
    PointerDeleter() {}

    PointerDeleter(const PointerDeleter<T>&);
    void operator=(const PointerDeleter<T>&);

    void deallocate(void* ptr) const override { delete static_cast<T*>(ptr); }

    const char* typeName() const override { return typeid(T).name(); }
    size_t typeSize() const override { return sizeof(T); } // lint !e1516
    bool pooled() const override { return false; }

    static const PointerDeleter<T> _instance;
  };

  template <typename T>
  class PooledPointerDeleter : public DeleterBase
  {
  public:
    static const PooledPointerDeleter<T>& instance() { return _instance; }

  private:
    PooledPointerDeleter() {}
    PooledPointerDeleter(const PooledPointerDeleter<T>&);
    void operator=(const PooledPointerDeleter<T>&);

    void deallocate(void* ptr) const override
    {
      if (LIKELY(ptr != nullptr))
      {
        // This is essentially the same code as sfc::ObjectPool::put()
        static_cast<T*>(ptr)->~T();
      }

      ::free(ptr);
    }

    const char* typeName() const override { return typeid(T).name(); }
    size_t typeSize() const override { return sizeof(T); } // lint !e1516
    bool pooled() const override { return true; }

    static const PooledPointerDeleter<T> _instance;
  };

public:
  class PointerManager
  {
    friend class DeleteList;

  public:
    PointerManager() : _ptr(nullptr), _deleter(nullptr) {}
    template <typename T>
    PointerManager(T* ptr, const DeleterBase& deleter)
      : _ptr(ptr), _deleter(&deleter)
    {
    }
    void deallocate() { _deleter->deallocate(_ptr); }
    const void* address() const { return _ptr; }
    bool empty() const { return _ptr == nullptr; }
    const char* typeName() const { return _deleter->typeName(); }
    size_t typeSize() const { return _deleter->typeSize(); }
    bool pooled() const { return _deleter->pooled(); }

  private:
    void* _ptr;
    const DeleterBase* _deleter;
  };

  // a simple container specially designed to store a delete list.
  // this container is especially optimized for storing just one element
  // in a buffer it contains itself, since many DeleteLists are typically
  // created to just store one item.
  //
  // the container is also designed to scale to very large sizes well.
  template <typename T>
  class Storage : public Memory::Manager
  {
  public:
    typedef T* iterator;
    typedef const T* const_iterator;

    iterator begin() { return _begin; }
    iterator end() { return _end; }
    const_iterator begin() const { return _begin; }
    const_iterator end() const { return _end; }

    Storage() : _begin(&_buf), _end(_begin), _endStorage(_begin + 1) {}

    virtual ~Storage() { deallocate(); }

    void push_back(const T& item, const size_t itemSize = 0)
    {
      const TSEGuard l(_mutex);

      if (itemSize)
        increaseTotalMemory(itemSize);

      if (_end == _endStorage)
      {
        reallocate(size() * 2);
      }
      *_end = item;
      ++_end;
    }

    T& operator[](size_t n) const
    {
      const TSEGuard l(_mutex);
      return _begin[n];
    }

    void clear()
    {
      const TSEGuard l(_mutex);
      deallocate();
      _begin = &_buf;
      _end = _begin;
      _endStorage = _begin + 1;

      _accurateTotalMemory = 0;
      notifyMemoryManager();
    }

    size_t size() const { return static_cast<size_t>(_end - _begin); }

    size_t capacity() const { return _endStorage - _begin; }

    size_t accurateTotalMemory() const { return _accurateTotalMemory; }

    void enableMemoryTracking() { _nextCheck = Memory::dhCheckPeriod; }

    void increaseTotalMemory(const size_t by)
    {
      _accurateTotalMemory += by;
      if (UNLIKELY(_accurateTotalMemory >= _nextCheck))
        notifyMemoryManager();
    }

    virtual void updateTotalMemory() override final { setTotalMemory(this->_accurateTotalMemory); }

  private:
    T _buf;
    T* _begin;
    T* _end;
    T* _endStorage;
    mutable TSEFastMutex _mutex;

    size_t _accurateTotalMemory = 0;
    size_t _nextCheck = std::numeric_limits<size_t>::max();

    void reallocate(size_t sz)
    {
      // IF the statistics are being collected, log the usage and allocation numbers
      if (UNLIKELY(DeleteList::_debugLoggingFlags & DeleteList::DEBUG_ALLOCATION))
      {
        tse::StopWatch sw; // used for collecting timing statistics
        // START the stopwatch so we can report time for list deletion
        sw.start();
        T* buffer = new T[sz];
        std::copy(_begin, _end, buffer);
        _end = buffer + size();
        _endStorage = buffer + sz;
        deallocate();
        _begin = buffer;
        sw.stop();
        _reallocCPU += sw.cpuTime();
        _reallocElapsed += sw.elapsedTime();
        LOG4CXX_DEBUG(_logger,
                      "reallocate() elapsed time: "
                          << sw.elapsedTime() << " seconds. CPUTime: " << sw.cpuTime()
                          << " New allocation size: " << sz << " Total CPU: " << _reallocCPU
                          << " Total Elapsed: " << _reallocElapsed);
      }
      else
      {
        T* buffer = new T[sz];
        std::copy(_begin, _end, buffer);
        _end = buffer + size();
        _endStorage = buffer + sz;
        deallocate();
        _begin = buffer;
      }
      if (UNLIKELY(DeleteList::_debugLoggingFlags & DeleteList::DEBUG_STATISTICS))
        _reallocCount++;
    }

    void deallocate()
    {
      if (_begin != &_buf)
      {
        delete[] _begin;
      }
    }

    void notifyMemoryManager()
    {
      if (UNLIKELY(this->_accurateTotalMemory == 0))
      {
        // deallocate();
        this->_nextCheck = Memory::dhCheckPeriod;
      }
      else if (UNLIKELY(this->_accurateTotalMemory >= this->_nextCheck + Memory::dhCheckPeriod))
      {
        // Big increase
        this->_nextCheck = this->_accurateTotalMemory + Memory::dhCheckPeriod;
      }
      else
      {
        this->_nextCheck += Memory::dhCheckPeriod;
      }

      updateTotalMemory();
    }
  };

private:
  static const size_t MaxContainers = 127;

  // if we only have one container of pointers, multiple threads will
  // contend over them, which will be very slow if there are lots
  // of threads performing lots of allocations and using the same DeleteList.
  //
  // we want to make an array of containers for DeleteLists that are
  // accessed from multiple threads, but we want DeleteLists to be cheap
  // to construct in functions that will only use them from a single thread.
  //
  // thus we provide space for just one container on the stack, since this
  // is fine for single threads. If we have multiple threads, we will
  // allocate containers on the heap
  const size_t _numContainers;

  typedef Storage<PointerManager> PtrList;

  PtrList _ptrsSingle;
  PtrList* _ptrs;

  typedef Storage<std::shared_ptr<void>> SmartPtrList;
  SmartPtrList _smartPtrsSingle;
  SmartPtrList* _smartPtrs;

  DeleteList(const DeleteList&);
  DeleteList& operator=(const DeleteList&);

  // this is how a thread decides which container it should use.
  size_t containerIndex() const { return pthread_self() % _numContainers; }

public:
  explicit DeleteList(size_t numContainers = 1, Memory::CompositeManager* parentManager = nullptr);

  /**
   *  Destructor should delete the pointers in the delete list.
   */
  ~DeleteList();

  struct Entry
  {
    explicit Entry(size_t size) : size(size), nheap(0), npool(0) {}
    size_t size;
    size_t nheap, npool;
  };

  template <typename M>
  void collectMetrics(M& data) const
  {
    for (size_t n = 0; n != _numContainers; ++n)
    {
      PtrList::const_iterator i1 = _ptrs[n].begin();
      PtrList::const_iterator i2 = _ptrs[n].end();

      for (; i1 != i2; ++i1)
      {
        if (i1->_ptr == nullptr)
        {
          continue;
        }
        const typename M::iterator itor =
            data.insert(std::pair<const char*, Entry>(i1->typeName(), Entry(i1->typeSize()))).first;
        if (i1->pooled())
        {
          itor->second.npool++;
        }
        else
        {
          itor->second.nheap++;
        }
      }
    }
  }

  /**
   *  don't make a copy of a RawPtr pointer.
   */
  template <typename T>
  void copy(RawPtr<T>& t)
  {
  }

  /**
   *  Make a copy of a smart pointer.
   */
  template <typename T>
  void copy(T& t)
  {
    const size_t index = containerIndex();
    _smartPtrs[index].push_back(std::static_pointer_cast<void>(t),
                                sizeof(typename T::element_type));
  }

  /**
   *  Adopt a pointer i.e. be responsible for cleaning up what it points to.
   */
  template <typename T>
  void adopt(T* t)
  {
    const size_t index = containerIndex();
    _ptrs[index].push_back(PointerManager(t, PointerDeleter<T>::instance()), sizeof(T));
  }

  // when we return the position of an adopted item, we need to record
  // both the container number and the index, and squeeze it all into
  // a size_t. We use the top 8 bits to record the container number, and
  // the rest to store the index.
  static const size_t ShiftBits = 8 * (sizeof(size_t) - 1);

  /**
   *  Adopt a pooled object pointer i.e. be responsible for returning it to the pool.
   */
  template <typename T>
  void adoptPooled(T* t)
  {
    const size_t index = containerIndex();
    _ptrs[index].push_back(PointerManager(t, PooledPointerDeleter<T>::instance()), sizeof(T));
  }

  /**
   *  Import all the objects managed by another DeleteList into this one.
   *  On exit, the other DeleteList will be cleared.
   */

  void import(DeleteList& another);

  void clear()
  {
    for (size_t n = 0; n != _numContainers; ++n)
    {
      deallocate(n);
    }
  }

public:
  // DEBUG FLAG VALUES

  enum DEBUGGING_FLAGS
  {
    DEBUG_NONE = 0,
    DEBUG_DESTRUCTION = 1,
    DEBUG_IMPORT = 2,
    DEBUG_ALLOCATION = 4,
    DEBUG_STATISTICS = 8,
  };

  static void setDebugFlags(unsigned int newValue) { _debugLoggingFlags = newValue; }
  static unsigned int getDebugFlags() { return _debugLoggingFlags; }

private:
  void deallocate();
  void deallocate(size_t n);

  void logContainerDetails(const std::string& logPrefix);
  void logContainerStats(const std::string& logPrefix);

public:
  static Logger _logger;
  static unsigned int _debugLoggingFlags;
  static size_t _reallocCount;
  static size_t _destroyCount;
  static size_t _importCount;
  static size_t _maxPointers;
  static size_t _maxSmartPointers;
  static size_t _singleCount;
  static size_t _smallCount;
  static size_t _mediumCount;
  static size_t _largeCount;
  static size_t _oneContainerCount;
  static size_t _oneContainerUsedCount;
  static double _reallocCPU;
  static double _reallocElapsed;
};

template <typename T>
const DeleteList::PooledPointerDeleter<T> DeleteList::PooledPointerDeleter<T>::_instance;

template <typename T>
const DeleteList::PointerDeleter<T> DeleteList::PointerDeleter<T>::_instance;

} // namespace tse

