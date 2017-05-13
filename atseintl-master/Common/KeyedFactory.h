#pragma once

#include "DBAccess/CreateResult.h"

namespace sfc
{

class CompressedData;

template <class Key, class Type>
class KeyedFactory
{
public:
  KeyedFactory() {}

  virtual ~KeyedFactory() {}

  virtual Type* create(Key key) { return nullptr; }

  virtual tse::CreateResult<Type> create(const Key& key, int)
  {
    tse::CreateResult<Type> result;
    return result;
  }

  virtual void destroy(Key key, Type* object) {}

  virtual Type* reCreate(const Key& key, Type* object) { return nullptr; }

  virtual void activate(Key key, Type* object) {}

  virtual bool validate(Key key, Type* object) { return true; }

  virtual void passivate(Key key, Type* object) {}

  virtual CompressedData* compress(const Type* data) const { return nullptr; }

  virtual Type* uncompress(const CompressedData& compressed) const { return nullptr; }
};

} // namespace sfc

