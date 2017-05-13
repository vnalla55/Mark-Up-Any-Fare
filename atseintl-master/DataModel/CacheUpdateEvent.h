#pragma once

#include "DataModel/CacheUpdateAction.h"
#include "DBAccess/ObjectKey.h"

#include <ctime>
#include <list>
#include <memory>
#include <set>
#include <vector>

namespace tse
{

class CacheControl;

struct CacheUpdateEvent
{
  CacheUpdateEvent(const ObjectKey& key,
                   CacheUpdateAction action,
                   CacheControl* ctrl = nullptr);

  ~CacheUpdateEvent();

  const ObjectKey _key;
  const CacheUpdateAction _action;
  CacheControl* _ctrl;
  const time_t _time;
};

typedef std::shared_ptr<CacheUpdateEvent> CacheUpdateEventPtr;

struct CacheUpdateEventPtrLess
{
  bool operator ()(CacheUpdateEventPtr first,
                   CacheUpdateEventPtr second)
  {
    if (first->_action < second->_action)
    {
      return true;
    }
    if (first->_action > second->_action)
    {
      return false;
    }
    return first->_key < second->_key;
  }
};

typedef std::vector<CacheUpdateEventPtr> CacheUpdatePtrVector;

typedef std::set<CacheUpdateEventPtr, CacheUpdateEventPtrLess> CacheUpdateEventPtrSet;

typedef std::list<CacheUpdateEventPtr> CacheUpdatePtrList;

}// tse
