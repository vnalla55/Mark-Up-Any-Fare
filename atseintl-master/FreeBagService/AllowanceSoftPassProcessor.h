//-------------------------------------------------------------------
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

#pragma once

#include "FreeBagService/AllowanceStepProcessor.h"

namespace tse
{
namespace PrecalcBaggage
{
class BagTravelData;
}

class AllowanceSoftPassProcessor final : public AllowanceStepProcessor
{
public:
  AllowanceSoftPassProcessor(PrecalcBaggage::BagTravelData& btdata) : _bagTravelData(btdata) {}

  AllowanceStepResult matchAllowance(const BagValidationOpt& opt) override;
  void dotTableCheckFailed(const BagValidationOpt& opt) override;

private:
  PrecalcBaggage::BagTravelData& _bagTravelData;
};
}
