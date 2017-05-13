//-------------------------------------------------------------------
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

#pragma once

#include "Common/OcTypes.h"
#include "DataModel/PrecalcBaggageData.h"
#include "FreeBagService/AllowanceStepProcessor.h"
#include "FreeBagService/ChargeStepProcessor.h"

#include <boost/container/flat_map.hpp>

namespace tse
{
class BaggageRevalidator;
class FareMarket;
class PaxTypeFare;
class PricingTrx;

class BaggageTravelLowerBoundCalculator : public AllowanceStepProcessor, public ChargeStepProcessor
{
public:
  static constexpr MoneyAmount INVALID_FEE_AMT = std::numeric_limits<MoneyAmount>::max();

  BaggageTravelLowerBoundCalculator(const PrecalcBaggage::BagTravelData& btData,
                                    const FareMarket& fm);

  MoneyAmount calcLowerBound(PaxTypeFare& ptf);

  // AllowanceStepProcessor overrides
  void dotTableCheckFailed(const BagValidationOpt& opt) override;
  AllowanceStepResult matchAllowance(const BagValidationOpt& opt) override;

  // ChargeStepProcessor overrides
  void matchCharges(const BagValidationOpt& opt) override;

private:
  MoneyAmount
  calcLowerBoundForCxr(uint32_t freePieces, const PrecalcBaggage::ChargeRecords& cxrRecords);

  Ts2ss _ts2ss;
  const PricingTrx& _trx;
  const PrecalcBaggage::BagTravelData& _btData;
  const uint32_t _requestedPieces;
  BaggageTravel& _bt;
  BaggageRevalidator* _revalidator;

  // intermediate results
  boost::container::flat_map<CarrierCode, uint32_t> _maxFreePieces;
  MoneyAmount _lowerBound = INVALID_FEE_AMT;
};
}
