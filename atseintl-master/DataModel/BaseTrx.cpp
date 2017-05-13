//-------------------------------------------------------------------
//
//  File:        BaseTrx.cpp
//  Created:     October 2, 2009
//
//  Copyright Sabre 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "DataModel/BaseTrx.h"

#include "Common/TrxCounter.h"

#include <boost/atomic.hpp>

namespace tse
{
inline static int64_t
nextId()
{
  static boost::atomic<int64_t> _id(-1);
  return ++_id;
}

BaseTrx::BaseTrx(Memory::TrxManager* manager) : _memoryManager(manager), _baseIntId(nextId())
{
  TrxCounter::appendTrx(*this);
}

BaseTrx::~BaseTrx()
{
  TrxCounter::removeTrx(*this);
}
} // namespace tse
