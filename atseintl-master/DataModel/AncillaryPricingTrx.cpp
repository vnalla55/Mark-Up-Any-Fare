//-------------------------------------------------------------------
//
//  File:        AncillaryPricingTrx.cpp
//
//  Description: Ancillary Pricing (AE* or WP-AE) Transaction object
//
//  Copyright Sabre 2010
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
#include "DataModel/AncillaryPricingTrx.h"

#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/Billing.h"

namespace tse
{

AncillaryPricingTrx::AncillaryPricingTrx()
{
  _baggagePolicy->setupPolicy(BaggagePolicy::ALL_TRAVELS, true);
  setTrxType(PRICING_TRX);
}

void
AncillaryPricingTrx::checkCurrencyAndSaleLoc()
{
  if (!getRequest()->salePointOverride().empty())
  {
    const Loc* loc = dataHandle().getLoc(getRequest()->salePointOverride(), ticketingDate());
    if (!loc)
      throw ErrorResponseException(ErrorResponseException::INVALID_CITY_AIRPORT_CODE,
                                   "INVALID POINT OF SALE");
  }

  if (getOptions()->currencyOverride().empty() &&
      !getRequest()->ticketingAgent()->currencyCodeAgent().empty())
  {
    getOptions()->currencyOverride() = getRequest()->ticketingAgent()->currencyCodeAgent();
    if (getRequest()->salePointOverride().empty())
      getOptions()->mOverride() = 'T';
  }

  if (!getOptions()->isMOverride() && !getRequest()->salePointOverride().empty())
  {
    CurrencyUtil::getSaleLocOverrideCurrency(getRequest()->salePointOverride(),
                                             getOptions()->currencyOverride(),
                                             getRequest()->ticketingDT());
  }
}

bool
AncillaryPricingTrx::isAB240AncillaryRequest()
{
  return (activationFlags().isAB240() && getOptions()->isOcOrBaggageDataRequested(RequestedOcFeeGroup::AncillaryData));
}

bool
AncillaryPricingTrx::isAddPadisData(bool isPadisAllowed, const ServiceGroup& serviceGroup)
{
  return (isPadisAllowed && _billing && (_billing->requestPath() != ACS_PO_ATSE_PATH) &&
        ("SA" == serviceGroup));
}

bool
AncillaryPricingTrx::isSecondCallForMonetaryDiscount()
{
  // TODO: [MonetaryDiscount] Write UTs for this function, once it's implementation is determined
  if (!activationFlags().isMonetaryDiscount())
    return false;

  for (const auto& itin : _itin)
    if (itin->getAncillaryPriceModifiers().size())
      return true;

  return false;
}

} // tse namespace
