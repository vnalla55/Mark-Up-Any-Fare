//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#pragma once

#include "Common/Assert.h"

namespace tse
{
class ShoppingTrx;
class V2IbfManager;

// A structure containing IBF data that
// has transaction-lifetime
class IbfData
{
public:
  IbfData(V2IbfManager* manager)
  {
    TSE_ASSERT(manager != nullptr);
    _v2IbfManager = manager;
  }

  V2IbfManager& getV2IbfManager()
  {
    TSE_ASSERT(_v2IbfManager != nullptr);
    return *_v2IbfManager;
  }

  static IbfData* createInitialIbfDataV2(ShoppingTrx& trx);

private:
  V2IbfManager* _v2IbfManager;
};

} // namespace tse
