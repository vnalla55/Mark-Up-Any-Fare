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

#include "Common/CurrencyConverter.h"
#include "Common/FallbackUtil.h"
#include "Common/TseEnums.h"
#include "Common/TaxRound.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/AtpcoTaxes/Common/MoneyUtil.h"
#include "Taxes/AtpcoTaxes/Rules/MathUtils.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/TaxRoundingInfoServiceV2.h"

namespace tse
{
FALLBACK_DECL(roundTaxToZero);

namespace
{

void adjustDecimal(tse::RoundingFactor& factor, tse::CurrencyNoDec factorNoDec)
{
  while (factorNoDec-- > 0)
  {
    factor /= 10;
  }
}

}

TaxRoundingInfoServiceV2::TaxRoundingInfoServiceV2(PricingTrx& trx) : _trx(trx) {}

void
TaxRoundingInfoServiceV2::getFareRoundingInfo(const tax::type::CurrencyCode& currency,
                                              tax::type::MoneyAmount& unit,
                                              tax::type::TaxRoundingDir& dir) const
{
  // Need to define to call getNucInfo
  ExchRate nucFactor = 0.0;
  CarrierCode carrier;
  DateTime discontinueDate(pos_infin);
  DateTime effectiveDate(pos_infin);
  CurrencyNoDec nucFactorNoDec;

  // What we want
  RoundingFactor roundingFactor = 0.0;
  RoundingRule roundingRule;
  CurrencyNoDec nucRoundingFactorNoDec;

  CurrencyConverter converter;
  bool nucRC = converter.getNucInfo(carrier,
                                    currency.asString(),
                                    _trx.getRequest()->ticketingDT(),
                                    nucFactor,
                                    roundingFactor,
                                    roundingRule,
                                    nucRoundingFactorNoDec,
                                    nucFactorNoDec,
                                    discontinueDate,
                                    effectiveDate);

  if (LIKELY(roundingFactor))
  {
    if (LIKELY(nucRC))
    {
      adjustDecimal(roundingFactor, nucRoundingFactorNoDec);
    }
  }

  unit = tax::doubleToAmount(roundingFactor);
  dir = convertToTaxRoundingDir(roundingRule);
}

void
TaxRoundingInfoServiceV2::getNationRoundingInfo(const tax::type::Nation& nation,
                                          tax::type::MoneyAmount& unit,
                                          tax::type::TaxRoundingDir& dir) const
{
  TaxRound round;
  RoundingFactor v2Unit(tax::amountToDouble(unit));
  CurrencyNoDec v2Dec(0);
  RoundingRule v2Rule(convertToRoundingRule(dir));

  round.retrieveNationRoundingSpecifications(
      _trx, toTseNationCode(nation), v2Unit, v2Dec, v2Rule);

  unit = tax::doubleToAmount(v2Unit);
  dir = convertToTaxRoundingDir(v2Rule);
}

void
TaxRoundingInfoServiceV2::getTrxRoundingInfo(const tax::type::Nation& /*nation*/,
                                             tax::type::MoneyAmount& unit,
                                             tax::type::TaxRoundingDir& dir) const
{
  TaxRound round;
  RoundingFactor v2Unit(tax::amountToDouble(unit));
  CurrencyNoDec v2Dec(0);
  RoundingRule v2Rule(convertToRoundingRule(dir));

  round.retrieveNationRoundingSpecifications(_trx, v2Unit, v2Dec, v2Rule);

  unit = tax::doubleToAmount(v2Unit);
  dir = convertToTaxRoundingDir(v2Rule);
}

RoundingRule
TaxRoundingInfoServiceV2::convertToRoundingRule(tax::type::TaxRoundingDir dir) const
{
  if (dir == tax::type::TaxRoundingDir::Blank)
    return EMPTY;
  else if (dir == tax::type::TaxRoundingDir::RoundUp)
    return UP;
  else if (dir == tax::type::TaxRoundingDir::RoundDown)
    return DOWN;
  else if (dir == tax::type::TaxRoundingDir::Nearest)
    return NEAREST;
  else if (dir == tax::type::TaxRoundingDir::NoRounding)
    return NONE;

  return EMPTY;
}

tax::type::TaxRoundingDir
TaxRoundingInfoServiceV2::convertToTaxRoundingDir(const RoundingRule& rule) const
{
  switch (rule)
  {
  case UP:
    return tax::type::TaxRoundingDir::RoundUp;
  case DOWN:
    return tax::type::TaxRoundingDir::RoundDown;
  case NEAREST:
    return tax::type::TaxRoundingDir::Nearest;
  case NONE:
    return tax::type::TaxRoundingDir::NoRounding;
  default:
    return tax::type::TaxRoundingDir::Blank;
  }
}


void
TaxRoundingInfoServiceV2::doStandardRound(tax::type::MoneyAmount& amount,
                                          tax::type::MoneyAmount& unit,
                                          tax::type::TaxRoundingDir& dir,
                                          tax::type::MoneyAmount currencyUnit /* = -1*/,
                                          bool isOcFee /* = false */) const
{
  TaxRound round;
  MoneyAmount taxAmount = round.applyTaxRound(tax::amountToDouble(amount),
      getPaymentCurrency(), tax::amountToDouble(unit), convertToRoundingRule(dir));


  if (fallback::roundTaxToZero(&_trx))
  {
    // If non-zero, non-OC taxAmount is zero after rounding, ignore it and leave unrounded
    if (!isOcFee && !taxAmount)
      return;

    if (!taxAmount && currencyUnit != -1)
      taxAmount = round.applyTaxRound(tax::amountToDouble(amount), getPaymentCurrency(),
          tax::amountToDouble(currencyUnit), convertToRoundingRule(tax::type::TaxRoundingDir::RoundDown));
  }

  amount = tax::doubleToAmount(taxAmount);
}

CurrencyCode
TaxRoundingInfoServiceV2::getPaymentCurrency() const
{
  if (!_trx.getOptions()->currencyOverride().empty())
    return _trx.getOptions()->currencyOverride();

  return _trx.getRequest()->ticketingAgent()->currencyCodeAgent();
}

}
