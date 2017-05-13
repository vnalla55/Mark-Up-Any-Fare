//-------------------------------------------------------------------
//  Copyright Sabre 2008
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

#include "DataModel/RexPricingOptions.h"

#include "DataModel/RexBaseTrx.h"

namespace tse
{

CurrencyCode&
RexPricingOptions::currencyOverride()
{
  return (((_trx != nullptr) && _trx->isAnalyzingExcItin()) ? _excCurrencyOverride : _currencyOverride);
}

const CurrencyCode&
RexPricingOptions::currencyOverride() const
{
  return (((_trx != nullptr) && _trx->isAnalyzingExcItin()) ? _excCurrencyOverride : _currencyOverride);
}

NationCode&
RexPricingOptions::nationality()
{
  if ((_trx != nullptr) && _trx->isAnalyzingExcItin())
    return _excNationality.empty() ? _nationality : _excNationality;
  else
    return _nationality;
}

const NationCode&
RexPricingOptions::nationality() const
{
  if ((_trx != nullptr) && _trx->isAnalyzingExcItin())
    return _excNationality.empty() ? _nationality : _excNationality;
  else
    return _nationality;
}

LocCode&
RexPricingOptions::residency()
{
  if ((_trx != nullptr) && _trx->isAnalyzingExcItin())
    return _excResidency.empty() ? _residency : _excResidency;
  else
    return _residency;
}

const LocCode&
RexPricingOptions::residency() const
{
  if ((_trx != nullptr) && _trx->isAnalyzingExcItin())
    return _excResidency.empty() ? _residency : _excResidency;
  else
    return _residency;
}

NationCode&
RexPricingOptions::employment()
{
  if ((_trx != nullptr) && _trx->isAnalyzingExcItin())
    return _excEmployment.empty() ? _employment : _excEmployment;
  else
    return _employment;
}

const NationCode&
RexPricingOptions::employment() const
{
  if ((_trx != nullptr) && _trx->isAnalyzingExcItin())
    return _excEmployment.empty() ? _employment : _excEmployment;
  else
    return _employment;
}

void
RexPricingOptions::setSpanishLargeFamilyDiscountLevel(const SLFUtil::DiscountLevel level)
{
  if (_trx && _trx->isAnalyzingExcItin())
    _excSpanishLargeFamilyDiscountLevel = level;
  else
    _spanishLargeFamilyDiscountLevel = level;
}

SLFUtil::DiscountLevel
RexPricingOptions::getSpanishLargeFamilyDiscountLevel() const
{
  return _trx && _trx->isAnalyzingExcItin() ? _excSpanishLargeFamilyDiscountLevel
                                            : _spanishLargeFamilyDiscountLevel;
}

bool
RexPricingOptions::isRtw() const
{
  if (!_trx)
    return false;

  return _trx->trxPhase() == RexBaseTrx::REPRICE_EXCITIN_PHASE ? _trx->isExcRtw() : _rtw;
}

char&
RexPricingOptions::fareFamilyType()
{
  return _trx && _trx->isAnalyzingExcItin() ? _excFareFamilyType : _fareFamilyType;
}

const char&
RexPricingOptions::fareFamilyType() const
{
  return _trx && _trx->isAnalyzingExcItin() ? _excFareFamilyType : _fareFamilyType;
}


} // tse

