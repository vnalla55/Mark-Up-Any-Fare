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

#pragma once

#include "Util/FlatFwd.h"

#include <boost/thread/mutex.hpp>

#include <stdint.h>

namespace tse
{
class BaseTrx;

namespace TrxCounter
{
class Observer
{
public:
  virtual ~Observer() = 0;

  // A callback called when a transaction is created.
  virtual void trxCreated(BaseTrx& trx);

  // A callback called just before transaction is deleted.
  virtual void trxDeleted(BaseTrx& trx);

  // A callback called after the previous one.
  // It is called without activeTrxMutex lock held.
  virtual void trxAfterDelete();
};

inline Observer::~Observer() {}

void registerObserver(Observer& observer);
void unregisterObserver(Observer& observer);
void appendTrx(BaseTrx& trx);
void removeTrx(BaseTrx& trx);
const FlatSet<int64_t>& getActiveTrxsIds();
const FlatSet<BaseTrx*>& getActiveTrxs();

// Lock this mutex to prevent more transactions from being created.
// WARNING: While this mutex is locked, you shouldn't (un)register any observers.
extern boost::mutex activeTrxMutex;
}
}
