// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyFacades/CopyTaxResponse.h"
#include "Taxes/LegacyFacades/ItinSelector.h"

namespace tse
{

CopyTaxResponse::CopyTaxResponse(PricingTrx& trx, const ItinSelector& itinSelector)
    : _trx(trx), _itinSelector(itinSelector)
{
}

bool
CopyTaxResponse::isCopyToExcItin() const
{
  return _itinSelector.isRefundTrx() && _itinSelector.isExcItin();
}

bool
CopyTaxResponse::isCopyToNewItin() const
{
  return _itinSelector.isNewItin() &&
      _trx.isValidatingCxrGsaApplicable() &&
      (_trx.getTrxType() == PricingTrx::PRICING_TRX ||
       _trx.getTrxType() == PricingTrx::MIP_TRX);
}

bool
CopyTaxResponse::checkItin(Itin* itin) const
{
  return itin->errResponseCode() == ErrorResponseException::NO_ERROR &&
      !itin->getTaxResponses().empty();
}

void
CopyTaxResponse::copyToExcItin()
{
  for(Itin* itin : _itinSelector.get())
  {
    if (checkItin(itin))
      _trx.addToExcItinTaxResponse(itin->getTaxResponses().begin(),
          itin->getTaxResponses().end());
  }
}

void
CopyTaxResponse::copyToNewItin()
{
  _trx.taxResponse().clear();
  for(Itin* itin : _itinSelector.get())
  {
    if (checkItin(itin))
      _trx.taxResponse().insert(_trx.taxResponse().end(),
          itin->getTaxResponses().begin(), itin->getTaxResponses().end());
  }
}

} // end of tse namespace
