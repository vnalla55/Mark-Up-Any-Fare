#pragma once

#include "Common/KeyedFactory.h"
#include "Common/KeyedPool.h"
#include "Util/BranchPrediction.h"

#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

#include <map>
#include <queue>
#include <stdexcept>

#include <time.h>

namespace sfc
{

template <typename Key, typename Type>
class QueueKeyedPool : public sfc::KeyedPool<Key, Type>
{
private:
  using Map = std::map<Key, std::queue<Type*>>;
  Map _map;
  boost::mutex _mutex;
  boost::condition _cond;
  volatile int _maxSize; // TODO volatile is not for thread purpose. Remove it!
  volatile int _minSize;
  volatile int _numActive;
  volatile int _numExists;
  int _timeOut;
  int _idleCloseTimeout;
  unsigned int _decayRate;
  time_t _timeoutTime;

  std::queue<Type*>& getQueue(Key& key)
  {
    auto i = _map.find(key);

    if (i != _map.end())
    {
      return i->second;
    }
    else
    {
      _map.emplace(key, std::queue<Type*>());
      return _map[key];
    }
  }

  class Deleter
  {
  private:
    QueueKeyedPool* _queueKeyedPool;
    Key _key;

  public:
    Deleter(QueueKeyedPool* queueKeyedPool, Key key) : _queueKeyedPool(queueKeyedPool), _key(key) {}

    void operator()(Type* object) const { _queueKeyedPool->put(_key, object); }
  };

public:
  QueueKeyedPool(KeyedFactory<Key, Type>& factory,
                 int maxSize = 10,
                 int minSize = -1,
                 int idleCloseTimeout = -1,
                 int timeOut = -1,
                 unsigned int decayRate = 30)
    : KeyedPool<Key, Type>(factory),
      _maxSize(maxSize),
      _minSize(minSize),
      _numActive(0),
      _numExists(0),
      _timeOut(timeOut),
      _idleCloseTimeout(idleCloseTimeout),
      _decayRate(decayRate),
      _timeoutTime(0)
  {
    if (_minSize < 0 || _minSize > _maxSize)
      _minSize = _maxSize;

    if (_idleCloseTimeout < -1)
      _idleCloseTimeout = -1;

    if (_timeOut < -1)
      _timeOut = -1;
  }

  virtual ~QueueKeyedPool() = default;

  virtual int setMaxSize(int maxSize)
  {
    boost::mutex::scoped_lock lock(_mutex);
    _maxSize = maxSize;
    _cond.notify_all();
    return _maxSize;
  }

  virtual int setMinSize(int minSize)
  {
    boost::mutex::scoped_lock lock(_mutex);
    if (minSize < 0 || minSize > _maxSize)
      _minSize = _maxSize;
    else
      _minSize = minSize;
    _cond.notify_all();
    return _minSize;
  }

  virtual int setIdleCloseTimeout(int idleCloseTimeout)
  {
    boost::mutex::scoped_lock lock(_mutex);
    _idleCloseTimeout = idleCloseTimeout;
    _cond.notify_all();
    return _idleCloseTimeout;
  }

  int numActive() override
  {
    boost::mutex::scoped_lock lock(_mutex);
    return _numActive;
  }

  int numIdle() override
  {
    boost::mutex::scoped_lock lock(_mutex);
    return _numExists - _numActive;
  }

  std::shared_ptr<Type> get(Key key) override
  {
    boost::mutex::scoped_lock lock(_mutex);
    std::queue<Type*>& queue(getQueue(key));
    Type* object(nullptr);
    while (_numActive >= _maxSize)
    {
      if (_timeOut > 0)
      {
        if (!_cond.timed_wait(lock, boost::posix_time::seconds(_timeOut)))
        {
          throw std::runtime_error("Timed out trying to get pooled object");
        }
      }
      else
      {
        _cond.wait(lock);
      }
    }
    try
    {
      while (!queue.empty())
      {
        object = queue.front();
        queue.pop();
        if (LIKELY(this->_factory.validate(key, object)))
        {
          this->_factory.activate(key, object);
          break;
        }
        else
        {
          --_numExists;
          this->_factory.destroy(key, object);
          object = nullptr;
        }
      }
      if (!object)
      {
        while (_numExists >= _maxSize)
        {
          if (_timeOut > 0)
          {
            if (!_cond.timed_wait(lock, boost::posix_time::seconds(_timeOut)))
            {
              throw std::runtime_error("Timed out trying to create a new pooled object");
            }
          }
          else
          {
            _cond.wait(lock);
          }
        }
        object = this->_factory.create(key);
        ++_numExists;
        _timeoutTime = time(nullptr) + _idleCloseTimeout;
      }
    }
    catch (...)
    {
      _cond.notify_all();
      throw;
    }
    ++_numActive;
    if (_numActive == _numExists)
    {
      _timeoutTime = time(nullptr) + _idleCloseTimeout;
    }
    return std::shared_ptr<Type>(object, Deleter(this, key));
  }

  void put(Key key, Type* object) override
  {
    boost::mutex::scoped_lock lock(_mutex);
    try
    {
      std::queue<Type*>& queue(getQueue(key));
      if (LIKELY(_numActive <= _maxSize && this->_factory.validate(key, object)))
      {
        if (LIKELY(_idleCloseTimeout != -1))
        {
          if (_numExists > _minSize && _numActive < _numExists && _timeoutTime < time(nullptr))
          {
            --_numExists;
            this->_factory.destroy(key, object);
            _timeoutTime = time(nullptr) + _decayRate;
          }
          else
          {
            this->_factory.passivate(key, object);
            queue.push(object);
          }
        }
        else
        {
          this->_factory.passivate(key, object);
          queue.push(object);
        }
      }
      else
      {
        --_numExists;
        this->_factory.destroy(key, object);
      }
    }
    catch (...)
    {
      --_numActive;
      _cond.notify_all();
      throw;
    }
    --_numActive;
    _cond.notify_all();
  }

  void invalidate(Key key, Type* object) override {}
};

} // namespace sfc

