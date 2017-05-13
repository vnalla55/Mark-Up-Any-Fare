//-------------------------------------------------------------------
//
//  File:        CacheTrx.h
//
//  Description: Cache update transaction object
//
//  Copyright Sabre 2004
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "DataModel/CacheUpdateEvent.h"
#include "DataModel/Trx.h"

#include <vector>

namespace tse
{
class Service;

class CacheTrx : public Trx
{
public:
  bool process(Service& srv) override;

  void push_back(CacheUpdateAction requestType,
                 const ObjectKey& key);

  bool push_back(CacheUpdateAction requestType,
                 const ObjectKey& key,
                 CacheUpdateEventPtrSet& uniqueEvents);

  static void processDelayedUpdates();

  const CacheUpdatePtrVector& cacheEvents() const { return _cacheEvents; }

private:
  CacheUpdatePtrVector _cacheEvents;

}; // class CacheTrx

} // tse namespace

