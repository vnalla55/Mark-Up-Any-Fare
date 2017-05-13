#pragma once

#include "DataModel/CacheUpdateAction.h"

#include <ctime>

namespace tse
{

class ObjectKey;

class DelayedUpdates
{
public:

  bool delayUpdate(const ObjectKey& key,
                   CacheUpdateAction action);

  void processDelayedUpdates();

  static std::time_t serverStartTime();
};

}// tse
