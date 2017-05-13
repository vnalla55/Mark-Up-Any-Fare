#pragma once

#include "Common/KeyedFactory.h"

#include <memory>

namespace sfc
{

template <typename Key, typename Type>
class KeyedPool
{
protected:
  KeyedFactory<Key, Type>& _factory;

public:
  typedef Key key_type;
  typedef Type value_type;
  typedef typename std::shared_ptr<Type> pointer_type;

  KeyedPool(KeyedFactory<Key, Type>& factory) : _factory(factory) {}
  virtual ~KeyedPool() {}
  virtual int numActive() { return 0; }
  virtual int numIdle() { return 0; }
  virtual pointer_type get(Key key) { return pointer_type(); }
  virtual void put(Key key, Type* object) {}
  virtual void invalidate(Key key, Type* object) {}
};

} // namespace sfc

