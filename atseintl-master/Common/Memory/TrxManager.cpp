//------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/Memory/TrxManager.h"

#include "Common/FallbackUtil.h"
#include "Common/Memory/Config.h"
#include "DataModel/Trx.h"

namespace tse
{
FALLBACK_DECL(unifyMemoryAborter)

namespace Memory
{
TrxManager::TrxManager(Trx* trx)
  : _trx(trx)
{
  setThreshold(trxThreshold);
}

TrxManager::~TrxManager()
{
  // Do it here, so that GlobalManager sees TrxManager instance.
  if (parent())
    parent()->unregisterManager(this);
}

void
TrxManager::setOutOfMemory()
{
  LocalManager::setOutOfMemory();
  if (_trx && !fallback::unifyMemoryAborter(_trx))
    _trx->setOutOfMemory();
}
}
}
