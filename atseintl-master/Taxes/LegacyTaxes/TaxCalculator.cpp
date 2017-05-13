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

#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyTaxes/TaxCalculator.h"
#include "Taxes/LegacyTaxes/TaxItinerary.h"

namespace tse
{
TaxCalculator::TaxCalculator(const TseThreadingConst::TaskId taskId,
                             PricingTrx& trx,
                             TaxMap::TaxFactoryMap& taxFactoryMap)
  : _trx(trx),
    _pooledExecutor(taskId),
    _synchronousExecutor(TseThreadingConst::SYNCHRONOUS_TASK),
    _taxFactoryMap(taxFactoryMap)
{
  _remainingItin = ItinSelector(_trx).get().size();
}

TaxItinerary*
TaxCalculator::createTaxItinerary(Itin& itin)
{
  TaxItinerary* taxItinerary = nullptr;
  _trx.dataHandle().get(taxItinerary);
  TSE_ASSERT(taxItinerary);
  taxItinerary->initialize(_trx, itin, _taxFactoryMap);
  return taxItinerary;
}

TseRunnableExecutor&
TaxCalculator::getTseExecutor()
{
  return (_remainingItin > 1) ? _pooledExecutor : _synchronousExecutor;
}

void
TaxCalculator::calculateTaxes(Itin& itin)
{
  if (itin.errResponseCode() == ErrorResponseException::NO_ERROR)
  {
    _isAnyItinValid = true;
    getTseExecutor().execute(*createTaxItinerary(itin));
  }

  --_remainingItin;
}

bool
TaxCalculator::isAnyItinValid() const
{
  return _isAnyItinValid;
}

void
TaxCalculator::wait()
{
  _pooledExecutor.wait();
}

} // end of tse namespace
