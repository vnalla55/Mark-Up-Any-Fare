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

namespace tse
{
class BaggageCharge;
class BaggageRevalidator;
class PricingTrx;

class BaggageTravelFeeCalculator : public AllowanceStepProcessor, public ChargeStepProcessor
{
public:
  static constexpr MoneyAmount INVALID_FEE_AMT = -1.0;

  BaggageTravelFeeCalculator(const PrecalcBaggage::BagTravelData& btData, const Ts2ss& ts2ss);
  const BaggageTravel& baggageTravel() const { return _bt; }

  MoneyAmount calcFee(FarePath& fp);

  // AllowanceStepProcessor overrides
  void dotTableCheckFailed(const BagValidationOpt& opt) override;
  AllowanceStepResult matchAllowance(const BagValidationOpt& opt) override;

  // ChargeStepProcessor overrides
  void matchCharges(const BagValidationOpt& opt) override;

private:
  void selectChargesForExcessPieces(const BagValidationOpt& opt,
                                    const uint32_t freePieces,
                                    const std::vector<BaggageCharge*>& charges) const;

  const PricingTrx& _trx;
  const PrecalcBaggage::BagTravelData& _btData;
  const uint32_t _requestedPieces;
  BaggageTravel& _bt;
  BaggageRevalidator* _revalidator = nullptr;
};
}
