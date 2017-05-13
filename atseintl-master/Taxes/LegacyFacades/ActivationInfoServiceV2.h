// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "Taxes/AtpcoTaxes/ServiceInterfaces/ActivationInfoService.h"

namespace tse
{

class PricingTrx;

class ActivationInfoServiceV2 : public tax::ActivationInfoService
{
public:
  ActivationInfoServiceV2(PricingTrx& trx);
  bool isAtpcoDefaultRoundingActive() const override;

private:
  PricingTrx& _trx;
};
}
