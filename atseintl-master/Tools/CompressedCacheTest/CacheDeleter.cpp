//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#include "CacheDeleter.h"
#include "BaseTrx.h"

#include <mutex>

namespace tse
{

namespace
{
  boost::int64_t _trxIdSeed(0);
}

CacheDeleterBase::CacheDeleterBase()
{
  TrxCounter::registerDeleter(this);
}
CacheDeleterBase::~CacheDeleterBase()
{
  TrxCounter::unregisterDeleter(this);
}

void CacheDeleterBase::emptyTrash()
{
}

std::set<CacheDeleterBase*> TrxCounter::_observers;
std::set<boost::int64_t> TrxCounter::_activeTrxs;
std::mutex TrxCounter::_observersMutex;
std::mutex TrxCounter::_activeTrxMutex;
bool TrxCounter::_fallbackCacheDeleter(false);

void TrxCounter::registerDeleter(CacheDeleterBase* deleter)
{
  static bool dummy(false);
  std::unique_lock<std::mutex> lock(_observersMutex);
  _observers.insert(deleter);
}
void TrxCounter::unregisterDeleter(CacheDeleterBase* deleter)
{
  std::unique_lock<std::mutex> lock(_observersMutex);
  _observers.erase(deleter);
}
void TrxCounter::appendTrx(BaseTrx* trx)
{
  std::unique_lock<std::mutex> lock(_activeTrxMutex);
  trx->setBaseIntId(_trxIdSeed++);
  _activeTrxs.insert(trx->getBaseIntId());
}
void TrxCounter::removeTrx(const BaseTrx* trx)
{
  {
    std::unique_lock<std::mutex> lock(_activeTrxMutex);
    _activeTrxs.erase(trx->getBaseIntId());
    // notify all
    std::for_each(_observers.begin(),
                  _observers.end(),
                  boost::bind(&CacheDeleterBase::notify, _1, trx->getBaseIntId()));
  }
  if (!_fallbackCacheDeleter)
  {
    std::for_each(_observers.begin(),
                  _observers.end(),
                  boost::bind(&CacheDeleterBase::clearAccumulator, _1));
  }
}
const std::set<boost::int64_t>& TrxCounter::getActiveTrxs()
{
  return _activeTrxs;
}
}// tse
