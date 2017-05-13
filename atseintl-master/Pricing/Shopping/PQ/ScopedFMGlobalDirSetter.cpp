// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
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
#include "Pricing/Shopping/PQ/ScopedFMGlobalDirSetter.h"

#include "Common/Assert.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "DataModel/FareMarket.h"
#include "DataModel/ShoppingTrx.h"

namespace tse
{
ScopedFMGlobalDirSetter::ScopedFMGlobalDirSetter(ShoppingTrx* trx,
                                                 FareMarket* fareMarket,
                                                 const GlobalDirection* globalDir)
  : _trx(trx), _globalDirection(GlobalDirection::NO_DIR), _fareMarket(fareMarket)
{
  TSE_ASSERT(_trx);
  TSE_ASSERT(_fareMarket);
  set(globalDir);
}

ScopedFMGlobalDirSetter::~ScopedFMGlobalDirSetter()
{
  _fareMarket->setGlobalDirection(_globalDirection);
}

void
ScopedFMGlobalDirSetter::set(const GlobalDirection* globalDir)
{
  _globalDirection = _fareMarket->getGlobalDirection();
  if (globalDir)
    _fareMarket->setGlobalDirection(*globalDir);
  else
    GlobalDirectionFinderV2Adapter::process(*_trx, *_fareMarket);
}
}
