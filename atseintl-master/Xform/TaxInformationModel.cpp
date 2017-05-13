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

#include "Common/Money.h"
#include "Common/TseConsts.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/Pfc/PfcItem.h"
#include "Xform/TaxInformationModel.h"

namespace tse
{
static constexpr int PERCENTAGE_TAX_PRECISION = 4;

PfcItem*
TaxInformationModel::findPfc(MoneyAmount amount) const
{
  auto result = std::find_if(_taxResponse->pfcItemVector().begin(),
                             _taxResponse->pfcItemVector().end(),
                             [&amount](PfcItem* pfc)
                             { return fabs(pfc->pfcAmount() - amount) <= EPSILON; });

  if (result == _taxResponse->pfcItemVector().end())
    return nullptr;
  else
    return *result;
}

MoneyAmount
TaxInformationModel::getTaxAmount() const
{
  if (_taxRecord.isTaxFeeExempt())
    return MoneyAmount(0.0);

  return _taxRecord.getTaxAmount();
}

LocCode
TaxInformationModel::getTaxPointLocSabreTaxes() const
{
  if (_taxRecord.isTaxFeeExempt())
    return LocCode("TE");

  if (_taxRecord.taxCode() != "XF")
    return _taxRecord.localBoard();

  if (auto pfc = findPfc(_taxRecord.getTaxAmount()))
    return pfc->pfcAirportCode();
  else if (!_taxResponse->pfcItemVector().empty())
    return _taxResponse->pfcItemVector().front()->pfcAirportCode();

  return "";
}

uint16_t
TaxInformationModel::getPublishedCurrencyPrecision() const
{
  if (_taxRecord.taxType() == Tax::PERCENTAGE)
  {
    return PERCENTAGE_TAX_PRECISION;
  }
  else
  {
    return Money(_taxRecord.publishedCurrencyCode()).noDec(_trx.ticketingDate());
  }
}

} // end of tse namespace
