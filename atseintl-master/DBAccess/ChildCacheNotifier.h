// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/ChildCache.h"


#include <set>

namespace tse {

template <class Key>
class ChildCacheNotifier
{
public:
  virtual ~ChildCacheNotifier()
  {
    for (ChildCache<Key>* child : _childCacheList)
      child->notifierDestroyed();
  }

  void addListener(ChildCache<Key>& child)
  {
    _childCacheList.insert(&child);
  }
  void removeListener(ChildCache<Key>& child)
  {
    _childCacheList.erase(&child);
  }
protected:
  unsigned int keyRemoved(const Key& key) const
  {
    unsigned int result = 0;
    for (ChildCache<Key>* child : _childCacheList)
    {
      ++result;
      child->keyRemoved(key);
    }
    return result;
  }
  void cacheCleared() const
  {
    for (ChildCache<Key>* child : _childCacheList)
      child->cacheCleared();
  }


private:
  std::set<ChildCache<Key>*> _childCacheList;
};

}

