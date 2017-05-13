#include "DataModel/CacheUpdateEvent.h"

#include <boost/atomic.hpp>

namespace tse
{

namespace
{

const bool debug(false);
boost::atomic<int> _numberEvents(0);

}// namespace

CacheUpdateEvent::CacheUpdateEvent(const ObjectKey& key,
                                   CacheUpdateAction action,
                                   CacheControl* ctrl)
  : _key(key)
  , _action(action)
  , _ctrl(ctrl)
  , _time(std::time(nullptr))
{
  if (debug)
  {
    ++_numberEvents;
  }
}

CacheUpdateEvent::~CacheUpdateEvent()
{
  if (debug)
  {
    --_numberEvents;
    std::cerr << __FUNCTION__ << " !! _numberEvents=" << _numberEvents << std::endl;
  }
}

}// tse
