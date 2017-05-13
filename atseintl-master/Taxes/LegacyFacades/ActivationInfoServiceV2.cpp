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

#include "Common/TrxUtil.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyFacades/ActivationInfoServiceV2.h"

using namespace tse;

ActivationInfoServiceV2::ActivationInfoServiceV2(PricingTrx& trx) : _trx(trx)
{
}

bool
ActivationInfoServiceV2::isAtpcoDefaultRoundingActive() const
{
  return tse::TrxUtil::isAtpcoTaxesDefaultRoundingEnabled(_trx);
}
