#ifndef TestDataAccessObject_h
#define TestDataAccessObject_h

#include "DBAccess/DataAccessObject.h"

namespace tse
{
template <typename Key, typename Type>
class TestDataAccessObject : public DataAccessObject<Key, Type>
{
public:
  virtual Type* create(const Key& key) = 0;
  virtual CreateResult<Type> create(const Key& key, bool) = 0;
  virtual void destroy(Key key, Type* vect) = 0;
};
} // tse
#endif // TestDataAccessObject_h
