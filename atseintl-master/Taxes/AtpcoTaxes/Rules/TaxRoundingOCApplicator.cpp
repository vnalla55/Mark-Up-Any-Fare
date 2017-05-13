// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Rules/TaxRoundingOCApplicator.h"

#include "Common/RoundingUtil.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/CurrencyService.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/Services.h"
#include "ServiceInterfaces/TaxRoundingInfoService.h"

namespace tse
{
class Trx;
ATPCO_FALLBACK_DECL(ATPCO_TAX_useIATARulesRoundingFix)
}

namespace tax
{

bool
TaxRoundingOCApplicator::shouldComputeOcAmount(const OptionalService& optionalService) const
{
  return optionalService.taxInclInd() && optionalService.taxAmount() != 0;
}

bool
TaxRoundingOCApplicator::shouldComputeTaxesOnOc(const OptionalService& optionalService) const
{
  return !optionalService.isFailed() && !optionalService.taxInclInd();
}

void
TaxRoundingOCApplicator::computeOcAmount(OptionalService& optionalService,
                                         PaymentDetail& paymentDetail,
                                         RoundingInfo& roundingInfo) const
{
  if (!optionalService.amount())
    return;

  optionalService.taxAndRounding().setRoundingInfo(roundingInfo);

  for(const OptionalService& oc : _request.optionalServices())
  {
    if (optionalService.index() == oc.index())
    {
      OptionalService& mutableOc = const_cast<OptionalService&>(oc);
      mutableOc.includeTax(optionalService.taxAndRounding(), _services.taxRoundingInfoService());
    }
  }

  for(RawPayments::value_type& rawPayment : _rawPayments)
  {
    PaymentDetail* paymentDetailFound = &rawPayment.detail;

    if (paymentDetailFound->getTaxPointBegin().id() == paymentDetail.getTaxPointBegin().id())
    {
      for(OptionalService& oc : paymentDetailFound->optionalServiceItems())
      {
        if (optionalService.index() == oc.index())
          oc.includeTax(optionalService.taxAndRounding(), _services.taxRoundingInfoService());
      }
    }
  }
}

void
TaxRoundingOCApplicator::computeTaxesOnOc(OptionalService& ocItem,
                                          RoundingInfo& roundingInfo) const
{
  bool doTruncation = false;
  if (!_services.fallbackService().isSet(tse::fallback::ATPCO_TAX_useIATARulesRoundingFix))
    doTruncation = true;

  if (roundingInfo.standardRounding)
  {
    type::MoneyAmount ocAmount = ocItem.getTaxEquivalentAmount();
    _services.taxRoundingInfoService().doStandardRound(
        ocAmount, roundingInfo.unit, roundingInfo.dir, -1, true);
    ocItem.setTaxEquivalentAmount(ocAmount);
  }
  else
  {
    ocItem.setTaxEquivalentAmount(
        doRound(ocItem.getTaxEquivalentAmount(), roundingInfo.unit, roundingInfo.dir, doTruncation));
  }
}

RoundingInfo
TaxRoundingOCApplicator::computeRoundingUnitAndDir(PaymentDetail& paymentDetail) const
{
  RoundingInfo result;

  if (isAtpcoDefaultRoundingEnabled())
  {
    result.unit = unitValue(type::TaxRoundingUnit::TwoDigits);
    result.dir = type::TaxRoundingDir::Nearest;
  }

  result.unit = unitValue(_taxRoundingRule->taxRoundingUnit());
  result.dir = _taxRoundingRule->taxRoundingDir();

  if (paymentDetail.isTaxAndEquivalentCurrencyEqual() && paymentDetail.isFlatTax() &&
      isRoundingBlank(result.unit, result.dir))
  {
    result.unit = unitValue(type::TaxRoundingUnit::NoRounding);
    result.dir = type::TaxRoundingDir::NoRounding;
    return result;
  }

  type::CurrencyCode taxCurrency = _services.currencyService().getNationCurrency(paymentDetail.taxCurrency(),
      paymentDetail.taxName().nation());
  if ((taxCurrency != paymentDetail.taxEquivalentCurrency()) || isRoundingBlank(result.unit, result.dir))
  {
    const type::Nation& nation = paymentDetail.taxName().nation();
    _services.taxRoundingInfoService().getTrxRoundingInfo(nation, result.unit, result.dir);
    result.standardRounding = true;
  }

  if (result.dir == type::TaxRoundingDir::NoRounding)
  {
    type::CurDecimals decimals =
        std::min(4u, _services.currencyService().getCurrencyDecimals(paymentDetail.taxEquivalentCurrency()));

    result.dir = type::TaxRoundingDir::RoundDown;
    result.unit = _values[4u - decimals];
  }

  return result;
}

bool
TaxRoundingOCApplicator::apply(PaymentDetail& paymentDetail) const
{
  RoundingInfo roundingInfo = computeRoundingUnitAndDir(paymentDetail);

  for(OptionalService& ocItem : paymentDetail.optionalServiceItems())
  {
    if (shouldComputeOcAmount(ocItem))
      computeOcAmount(ocItem, paymentDetail, roundingInfo);
    else if (shouldComputeTaxesOnOc(ocItem))
      computeTaxesOnOc(ocItem, roundingInfo);
  }
  return true;
}

} // end of tax namespace
