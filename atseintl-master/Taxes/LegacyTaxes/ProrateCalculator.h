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

#ifndef LEGACYTAXES_PRORATE_CALCULATOR_H
#define LEGACYTAXES_PRORATE_CALCULATOR_H

#include <boost/function.hpp>
#include <vector>

#include "Common/TseEnums.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class DataHandle;
class DateTime;
class Loc;
class PricingTrx;
class TravelSeg;

class ProrateCalculator
{
public:
  typedef boost::function<uint32_t(const Loc& origin,
                                   const Loc& destination,
                                   const GlobalDirection globalDirection,
                                   const DateTime& dateTime,
                                   DataHandle& dataHandle)> GetMileage;
  ProrateCalculator(const PricingTrx& trx,
                    const DateTime& travelDate,
                    const std::vector<TravelSeg*>& travelSegs,
                    const GetMileage& getMileage);
  ProrateCalculator(const PricingTrx& trx,
                    const DateTime& travelDate,
                    const std::vector<TravelSeg*>& travelSegs);

  MoneyAmount getProratedAmount(size_t from, size_t to, MoneyAmount amount) const;
private:
  const PricingTrx& _trx;
  const DateTime& _travelDate;
  std::vector<TravelSeg*> _travelSegs;
  GetMileage _getMileage;

  uint32_t getMileage(TravelSeg& travelSeg) const;
};
}
#endif
