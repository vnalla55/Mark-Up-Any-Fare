//---------------------------------------------------------------------------
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/ProrateCalculator.h"

#include "Common/DateTime.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/LocUtil.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "Taxes/Common/TaxMileageUtil.h"

namespace tse
{
ProrateCalculator::ProrateCalculator(const PricingTrx& trx,
                                     const DateTime& travelDate,
                                     const std::vector<TravelSeg*>& travelSegs)
  : _trx(trx),
    _travelDate(travelDate),
    _travelSegs(travelSegs),
    _getMileage(TaxMileageUtil::getDistance)
{
}

ProrateCalculator::ProrateCalculator(const PricingTrx& trx,
                                     const DateTime& travelDate,
                                     const std::vector<TravelSeg*>& travelSegs,
                                     const GetMileage& getMileage)
  : _trx(trx), _travelDate(travelDate), _travelSegs(travelSegs), _getMileage(getMileage)
{
}

MoneyAmount
ProrateCalculator::getProratedAmount(size_t from, size_t to, MoneyAmount amount) const
{
  uint32_t totalMiles = 0, rangeMiles = 0;
  size_t i = 0;
  for (; i < from; ++i)
    totalMiles += getMileage(*_travelSegs[i]);

  for (; i <= to; ++i)
  {
    uint32_t miles = getMileage(*_travelSegs[i]);
    totalMiles += miles;
    rangeMiles += miles;
  }

  for (; i < _travelSegs.size(); ++i)
    totalMiles += getMileage(*_travelSegs[i]);

  return amount * rangeMiles / totalMiles;
}

uint32_t
ProrateCalculator::getMileage(TravelSeg& travelSeg) const
{
  if (!travelSeg.isAir())
    return 0;

  DataHandle dh(_travelDate);
  GlobalDirection gd = GlobalDirection::XX;
  GlobalDirectionFinderV2Adapter::getGlobalDirection(&_trx, _travelDate, travelSeg, gd);

  return _getMileage(*travelSeg.origin(), *travelSeg.destination(), gd, _travelDate, dh);
}
}
