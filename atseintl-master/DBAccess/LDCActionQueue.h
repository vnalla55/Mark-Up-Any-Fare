//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/Thread/TSELockGuards.h"
#include "Common/Thread/TSEReadWriteLock.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/HashKey.h"
#include "DBAccess/LDCOperationCounts.h"

#include <fstream>
#include <queue>
#include <sstream>
#include <string>
#include <utility>

namespace tse
{
namespace ldc
{
template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J>
inline bool
keyInitialized(const HashKey<A, B, C, D, E, F, G, H, I, J>& key)
{
  return key.initialized;
}

template <size_t n>
inline bool
keyInitialized(const Code<n>& key)
{
  return (!key.empty());
}

inline bool
keyInitialized(const std::string& key)
{
  return (!key.empty());
}

inline bool
keyInitialized(int key)
{
  return true;
}

inline bool
keyInitialized(char key)
{
  return true;
}

inline bool
keyInitialized(bool key)
{
  return true;
}
}

template <typename Key>
class LDCOperation final
{
public:
  enum Type
  {
    WRITE = 0,
    REMOVE,
    CLEAR,
    Type_END // Keep this last!
  };

  LDCOperation(const std::string& tableName,
               Type opType,
               Key key,
               bool whileLoading,
               bool distCacheOp,
               bool ldcOp)
    : _tableName(tableName),
      _opType(opType),
      _key(key),
      _whileLoading(whileLoading),
      _time(time(nullptr)),
      _distCacheOp(distCacheOp),
      _ldcOp(ldcOp)
  {
  }

  LDCOperation() = default;
  LDCOperation& operator=(LDCOperation&&) = default;

  Type getType() const { return _opType; }

  const Key& getKey() const { return _key; }

  time_t getTime() const { return _time; }

  bool whileLoading() const { return _whileLoading; }

  bool distCacheOp() const { return _distCacheOp; }

  bool ldcOp() const { return _ldcOp; }

  const char* getTypeAsString() const
  {
    switch (_opType)
    {
    case WRITE:
      return "WRITE ";
      break;
    case REMOVE:
      return "REMOVE";
      break;
    case CLEAR:
      return "CLEAR ";
      break;
    default:
      return "ERROR!";
      break;
    }
  }

  std::string flatten() const
  {
    KeyStream flatKey(0);
    flatKey << _key;
    std::stringstream ss;
    ss << _tableName << " action=[" << std::setw(6) << std::setfill(' ') << getTypeAsString() << "]"
       << " time=[" << DateTime(_time) << "]"
       << " loading=[" << (_whileLoading ? "true " : "false") << "]"
       << " key=[" << flatKey << "]";
    return ss.str();
  }

private:
  std::string _tableName;
  Type _opType = Type::WRITE;
  Key _key;
  bool _whileLoading = false;
  time_t _time = 0;
  bool _distCacheOp = true;
  bool _ldcOp = true;
};

template <typename Key>
class LDCActionQueue final
{
public:
  LDCActionQueue(const std::string& name) : _tableName(name) {}

  LDCActionQueue(const LDCActionQueue&) = delete;
  LDCActionQueue& operator=(const LDCActionQueue&) = delete;

  ~LDCActionQueue()
  {
    if (_queueLog.is_open())
    {
      _queueLog.close();
    }
  }

  std::ofstream& logfile()
  {
    if (!_queueLog.is_open())
    {
      std::ostringstream filename;
      filename << DISKCACHE.getQueueLogDir() << "/ldcq." << _tableName << ".log";
      _queueLog.open(filename.str().c_str());
    }

    return _queueLog;
  }

  void initialize(const std::string& name, DiskCache::CacheTypeOptions* cto)
  {
    _cto = cto;

    if (name != _tableName)
    {
      _tableName = name;

      if (_queueLog.is_open())
      {
        _queueLog.close();
      }
    }
  }

  size_t size() const
  {
    TSEReadGuard<> l(const_cast<TSEReadWriteLock&>(_queueMutex));
    return _theQueue.size();
  }

  void clear()
  {
    TSEWriteGuard<> l(_queueMutex);
    _theQueue = OPQUEUE();
  }

  bool ldcEnabled() const { return ((_cto != nullptr) && _cto->enabled); }

  bool getNext(LDCOperation<Key>& target)
  {
    bool retval(false);
    TSEReadGuard<> l(_queueMutex);
    if (_theQueue.size() > 0)
    {
      target = std::move(_theQueue.front());
      _theQueue.pop();
      ++_popCounter;

      if (UNLIKELY(DISKCACHE.logQueueActivity()))
      {
        logfile() << "[POP." << _popCounter << "]  " << target.flatten() << std::endl;
#if 0
            KeyStream stream(0);
            stream << target.getKey() ;
            std::string flatKey( stream ) ;
            std::size_t pos = flatKey.find_first_of( '*' ) ;
            if( pos != std::string::npos )
            {
              logfile() << std::endl << "   ERROR!  POP." << _popCounter << " has a messed up key: [" << flatKey << "] !!!" << std::endl << std::endl ;
            }
#endif
      }

      retval = true;
    }
    return retval;
  }

  void queueWriteReq(const Key& key, bool isLoading, bool distCacheOp)
  {
    if (LIKELY((distCacheOp || ldcEnabled()) && ldc::keyInitialized(key)))
    {
      TSEWriteGuard<> l(_queueMutex);
      _theQueue.emplace(
          _tableName, LDCOperation<Key>::WRITE, key, isLoading, distCacheOp, ldcEnabled());
      ++_pushCounter;
      if (UNLIKELY(DISKCACHE.logQueueActivity()))
      {
        logfile() << "[PUSH." << _pushCounter << "] " << _theQueue.back().flatten() << std::endl;
      }
    }
  }

  void queueRemoveReq(const Key& key, bool isLoading, bool ldcOp)
  {
    if (ldcEnabled() && ldc::keyInitialized(key))
    {
      TSEWriteGuard<> l(_queueMutex);
      _theQueue.emplace(_tableName, LDCOperation<Key>::REMOVE, key, isLoading, false, ldcOp);

      ++_pushCounter;
      if (UNLIKELY(DISKCACHE.logQueueActivity()))
      {
        logfile() << "[PUSH." << _pushCounter << "] " << _theQueue.back().flatten() << std::endl;
      }
    }
  }

  void queueClearReq(bool isLoading)
  {
    if (ldcEnabled())
    {
      TSEWriteGuard<> l(_queueMutex);
      _theQueue.emplace(
          _tableName, LDCOperation<Key>::CLEAR, _dummy, isLoading, false, ldcEnabled());
      if (DISKCACHE.logQueueActivity())
      {
        logfile() << "[PUSH] " << _theQueue.back().flatten() << std::endl;
      }
    }
  }

private:
  using OPQUEUE = std::queue<LDCOperation<Key>>;

  Key _dummy;
  std::string _tableName;
  DiskCache::CacheTypeOptions* _cto = nullptr;
  TSEReadWriteLock _queueMutex;
  OPQUEUE _theQueue;
  std::ofstream _queueLog;
  long long _pushCounter = 0;
  long long _popCounter = 0;
};
} // namespace tse;
