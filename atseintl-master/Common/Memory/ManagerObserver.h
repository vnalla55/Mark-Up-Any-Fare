//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/Memory/GlobalManager.h"
#include "Common/TrxCounter.h"

namespace tse
{
namespace Memory
{
class ManagerObserver : public TrxCounter::Observer
{
public:
  ManagerObserver();
  virtual ~ManagerObserver();

  void trxCreated(BaseTrx& trx) override;
  void trxDeleted(BaseTrx& trx) override;

private:
  GlobalManager _memoryManager;
};
}
}
