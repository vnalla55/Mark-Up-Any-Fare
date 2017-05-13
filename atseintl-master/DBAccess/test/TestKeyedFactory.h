#ifndef TEST_KEYED_FACTORY_H
#define TEST_KEYED_FACTORY_H

#include "DBAccess/test/TestDataAccessObject.h"

namespace sfc
{
class CompressedData;

template <typename Key, typename Type>
class TestKeyedFactory : public KeyedFactory<Key, Type>
{
public:
  TestKeyedFactory(tse::TestDataAccessObject<Key, Type>& dao) : dao_(dao) {}

  virtual Type* create(Key key) { return dao_.create(key); }

  virtual tse::CreateResult<Type> create(const Key& key, int) { return dao_.create(key, 0); }

  virtual void destroy(Key key, Type* vect) { dao_.destroy(key, vect); }

  virtual CompressedData* compress(const Type* data) const { return dao_.compress(data); }

  virtual Type* uncompress(const CompressedData& compressed) const
  {
    return dao_.uncompress(compressed);
  }

private:
  tse::TestDataAccessObject<Key, Type>& dao_;
};
}

#endif // TEST_KEYED_FACTORY_H
