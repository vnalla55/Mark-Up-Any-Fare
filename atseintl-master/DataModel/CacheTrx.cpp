#include "DataModel/CacheTrx.h"
#include "DataModel/DelayedUpdates.h"
#include <boost/algorithm/string.hpp>

namespace tse
{

namespace
{

DelayedUpdates delayedUpdates;

bool delayUpdate(const ObjectKey& key,
                 CacheUpdateAction action)
{
  return delayedUpdates.delayUpdate(key, action);
}

}// namespace

bool CacheTrx::process(Service& srv)
{
  return srv.process(*this);
}

void CacheTrx::push_back(CacheUpdateAction requestType,
                         const ObjectKey& key)
{
  if (!delayUpdate(key, requestType))
  {
    CacheUpdateEventPtr event(new CacheUpdateEvent(key, requestType));
    _cacheEvents.push_back(event);
  }
}

bool CacheTrx::push_back(CacheUpdateAction requestType,
                         const ObjectKey& key,
                         CacheUpdateEventPtrSet& uniqueEvents)
{
  if (LIKELY(delayUpdate(key, requestType)))
  {
    return true;
  }
  CacheUpdateEventPtr event(new CacheUpdateEvent(key, requestType));
  if (uniqueEvents.insert(event).second)
  {
    _cacheEvents.push_back(event);
    return true;
  }
  return false;
}

void CacheTrx::processDelayedUpdates()
{
  delayedUpdates.processDelayedUpdates();
}

} // tse
