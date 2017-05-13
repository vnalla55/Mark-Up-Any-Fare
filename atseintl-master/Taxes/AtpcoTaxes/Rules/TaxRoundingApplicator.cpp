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
#include "DataModel/Common/Types.h"
#include "TaxRoundingApplicator.h"
#include "Common/RoundingUtil.h"
#include "ServiceInterfaces/ActivationInfoService.h"
#include "ServiceInterfaces/CurrencyService.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/Services.h"
#include "ServiceInterfaces/TaxRoundingInfoService.h"

namespace tse
{
class Trx;
ATPCO_FALLBACK_DECL(ATPCO_TAX_useIATARulesRoundingFix)
ATPCO_FALLBACK_DECL(markupAnyFareOptimization)
}

namespace tax
{
const type::MoneyAmount TaxRoundingApplicator::_values[11] = {
  type::MoneyAmount(1, 10000), type::MoneyAmount(1, 1000), type::MoneyAmount(1, 100),
  type::MoneyAmount(1, 10),    type::MoneyAmount(1),       type::MoneyAmount(10),
  type::MoneyAmount(100),      type::MoneyAmount(1000),    type::MoneyAmount(10000),
  type::MoneyAmount(100000),   type::MoneyAmount(1000000)
};

type::MoneyAmount
TaxRoundingApplicator::unitValue(type::TaxRoundingUnit unit)
{
  if (unit == type::TaxRoundingUnit::Blank)
    return -1;
  if (unit == type::TaxRoundingUnit::NoRounding)
    return 0;

  static const std::string unitString("43210THSEUM");
  const type::Index scaleIdx = unitString.find(static_cast<unsigned char>(unit));

  return _values[scaleIdx];
}

bool
TaxRoundingApplicator::isRoundingBlank(const type::MoneyAmount& unit,
                                       const type::TaxRoundingDir dir) const
{
  return (unit == unitValue(type::TaxRoundingUnit::Blank)) || (dir == type::TaxRoundingDir::Blank);
}

bool
TaxRoundingApplicator::isAtpcoDefaultRoundingEnabled() const
{
  return _services.activationInfoService().isAtpcoDefaultRoundingActive();
}

void
TaxRoundingApplicator::doAtpcoDefaultRounding(PaymentDetail& paymentDetail) const
{
  type::MoneyAmount unit = unitValue(type::TaxRoundingUnit::TwoDigits);
  type::TaxRoundingDir dir = type::TaxRoundingDir::Nearest;

  bool doTruncation = false;
  if (!_services.fallbackService().isSet(tse::fallback::ATPCO_TAX_useIATARulesRoundingFix))
    doTruncation = true;

  paymentDetail.taxEquivalentAmount() =
      doRound(paymentDetail.taxEquivalentAmount(), unit, dir, doTruncation);

  if (!_services.fallbackService().isSet(tse::fallback::markupAnyFareOptimization))
  {
    paymentDetail.taxEquivalentWithMarkupAmount() =
      doRound(paymentDetail.taxEquivalentWithMarkupAmount(), unit, dir, doTruncation);
  }

  roundOC(paymentDetail, unit, dir);
  paymentDetail.taxOnChangeFeeAmount() = doRound(paymentDetail.taxOnChangeFeeAmount(), unit, dir, doTruncation);
}

bool
TaxRoundingApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (isAtpcoDefaultRoundingEnabled())
  {
    doAtpcoDefaultRounding(paymentDetail);
    return true;
  }

  type::MoneyAmount unit = unitValue(_taxRoundingRule->taxRoundingUnit());
  if (_taxRoundingRule->taxRoundingPrecision() == type::TaxRoundingPrecision::ToFives)
    unit *= 5;
  type::TaxRoundingDir dir = _taxRoundingRule->taxRoundingDir();

  if (paymentDetail.isTaxAndEquivalentCurrencyEqual() && paymentDetail.isFlatTax() &&
      isRoundingBlank(unit, dir))
  {
    return true;
  }

  type::CurDecimals decimals =
      std::min(4u, _services.currencyService().getCurrencyDecimals(paymentDetail.taxEquivalentCurrency()));
  type::MoneyAmount currencyUnit = _values[4u - decimals];

  type::CurrencyCode taxCurrency = _services.currencyService().getNationCurrency(paymentDetail.taxCurrency(),
      paymentDetail.taxName().nation());

  bool differentCurrency = (taxCurrency != paymentDetail.taxEquivalentCurrency());
  if (differentCurrency || isRoundingBlank(unit, dir))
  {
    _services.taxRoundingInfoService().getTrxRoundingInfo(paymentDetail.taxName().nation(), unit, dir);

    if (differentCurrency && isRoundingBlank(unit, dir))
    {
      // Most likely ticket sold in PE/NI/SV and priced in USD - in this case do no rounding
      dir = type::TaxRoundingDir::NoRounding;
    }

    paymentDetail.calcDetails().taxBeforeRounding = paymentDetail.taxEquivalentAmount();
    paymentDetail.calcDetails().taxWithMarkupBeforeRounding = paymentDetail.taxEquivalentWithMarkupAmount();

    paymentDetail.calcDetails().roundingUnit = unit;
    paymentDetail.calcDetails().roundingDir = dir;
    paymentDetail.calcDetails().currencyUnit = currencyUnit;

    _services.taxRoundingInfoService().doStandardRound(
        paymentDetail.taxEquivalentAmount(), unit, dir, currencyUnit);
    _services.taxRoundingInfoService().doStandardRound(
        paymentDetail.taxEquivalentWithMarkupAmount(), unit, dir, currencyUnit);

    standardRoundOC(paymentDetail, unit, dir);
    _services.taxRoundingInfoService().doStandardRound(paymentDetail.taxOnChangeFeeAmount(), unit, dir, currencyUnit);
    return true;
  }

  if (dir == type::TaxRoundingDir::NoRounding)
  {
    // if NoRounding specified, truncate the amount at point deduced from currency
    unit = currencyUnit;
    dir = type::TaxRoundingDir::RoundDown;
  }

  paymentDetail.calcDetails().taxBeforeRounding = paymentDetail.taxEquivalentAmount();
  paymentDetail.calcDetails().taxWithMarkupBeforeRounding = paymentDetail.taxEquivalentWithMarkupAmount();

  paymentDetail.calcDetails().roundingUnit = unit;
  paymentDetail.calcDetails().roundingDir = dir;
  paymentDetail.calcDetails().currencyUnit = currencyUnit;

  bool doTruncation = false;
  if (!_services.fallbackService().isSet(tse::fallback::ATPCO_TAX_useIATARulesRoundingFix))
    doTruncation = true;

  paymentDetail.taxEquivalentAmount() =
      doRound(paymentDetail.taxEquivalentAmount(), unit, dir, doTruncation);
  paymentDetail.taxEquivalentWithMarkupAmount() =
      doRound(paymentDetail.taxEquivalentWithMarkupAmount(), unit, dir, doTruncation);

  roundOC(paymentDetail, unit, dir);
  paymentDetail.taxOnChangeFeeAmount() = doRound(paymentDetail.taxOnChangeFeeAmount(), unit, dir, doTruncation);

  return true; // Rounded as specified
}

void
TaxRoundingApplicator::roundOC(PaymentDetail& paymentDetail,
                               const type::MoneyAmount& unit,
                               const type::TaxRoundingDir& dir) const
{
  for (OptionalService& ocItem : paymentDetail.optionalServiceItems())
  {
    if (ocItem.isFailed() || ocItem.taxInclInd())
      continue;

    bool doTruncation = false;
    if (!_services.fallbackService().isSet(tse::fallback::ATPCO_TAX_useIATARulesRoundingFix))
      doTruncation = true;

    ocItem.setTaxEquivalentAmount(doRound(ocItem.getTaxEquivalentAmount(), unit, dir, doTruncation));
  }
}

void
TaxRoundingApplicator::standardRoundOC(PaymentDetail& paymentDetail,
                                       type::MoneyAmount& unit,
                                       type::TaxRoundingDir& dir) const
{
  for (OptionalService& ocItem : paymentDetail.optionalServiceItems())
  {
    if (ocItem.isFailed())
      continue;

    type::MoneyAmount ocAmount = ocItem.getTaxEquivalentAmount();
    _services.taxRoundingInfoService().doStandardRound(ocAmount, unit, dir, -1, true);
    ocItem.setTaxEquivalentAmount(ocAmount);
  }
}
}
