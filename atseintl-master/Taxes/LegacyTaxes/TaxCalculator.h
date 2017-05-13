// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
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

#include "Common/Thread/TseThreadingConst.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Taxes/LegacyTaxes/TaxMap.h"

namespace tse
{
class Itin;
class PricingTrx;
class TaxItinerary;

class TaxCalculator
{
  PricingTrx& _trx;
  uint16_t _remainingItin;
  TseRunnableExecutor _pooledExecutor;
  TseRunnableExecutor _synchronousExecutor;
  TaxMap::TaxFactoryMap& _taxFactoryMap;
  bool _isAnyItinValid = false;

  TaxCalculator(const TaxCalculator&) = delete;
  TaxCalculator& operator=(const TaxCalculator&) = delete;

  TaxItinerary* createTaxItinerary(Itin& itin);
  TseRunnableExecutor& getTseExecutor();

public:
  TaxCalculator(const TseThreadingConst::TaskId taskId,
                PricingTrx& trx,
                TaxMap::TaxFactoryMap& taxFactoryMap);
  void calculateTaxes(Itin& itin);
  bool isAnyItinValid() const;
  void wait();
};

} // end of tse namespace
