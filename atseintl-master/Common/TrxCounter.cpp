// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Common/TrxCounter.h"

#include "DataModel/BaseTrx.h"
#include "Util/BranchPrediction.h"
#include "Util/FlatSet.h"

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace tse
{
namespace TrxCounter
{
namespace
{
boost::shared_mutex _observersMutex;
FlatSet<Observer*> _observers;

FlatSet<int64_t> _trxIdSet;
FlatSet<BaseTrx*> _trxSet;
}

boost::mutex activeTrxMutex;

void
Observer::trxCreated(BaseTrx& /*trx*/)
{
}

void
Observer::trxDeleted(BaseTrx& /*trx*/)
{
}

void
Observer::trxAfterDelete()
{
}

void
registerObserver(Observer& observer)
{
  boost::unique_lock<boost::shared_mutex> lock(_observersMutex);
  _observers.insert(&observer);
}

void
unregisterObserver(Observer& observer)
{
  boost::unique_lock<boost::shared_mutex> lock(_observersMutex);
  _observers.erase(&observer);
}

void
appendTrx(BaseTrx& trx)
{
  boost::shared_lock<boost::shared_mutex> lock(_observersMutex);
  boost::lock_guard<boost::mutex> lockTrx(activeTrxMutex);

  _trxIdSet.insert(trx.getBaseIntId());
  _trxSet.insert(&trx);

  for (Observer* observer : _observers)
  {
    observer->trxCreated(trx);
  }
}

void
removeTrx(BaseTrx& trx)
{
  boost::shared_lock<boost::shared_mutex> lock(_observersMutex);

  {
    boost::lock_guard<boost::mutex> lockTrx(activeTrxMutex);

    FlatSet<BaseTrx*>::iterator it = _trxSet.find(&trx);

    if (UNLIKELY(it == _trxSet.end()))
      return;

    for (Observer* observer : _observers)
    {
      observer->trxDeleted(trx);
    }

    _trxIdSet.erase(trx.getBaseIntId());
    _trxSet.erase(it);
  }

  for (Observer* observer : _observers)
  {
    observer->trxAfterDelete();
  }
}

const FlatSet<int64_t>&
getActiveTrxsIds()
{
  return _trxIdSet;
}

const FlatSet<BaseTrx*>&
getActiveTrxs()
{
  return _trxSet;
}
}
}
