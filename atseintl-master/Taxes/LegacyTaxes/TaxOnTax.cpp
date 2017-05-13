// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"
#include "Common/TaxRound.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/Common/TaxUtility.h"

using namespace tse;

TaxOnTax::TaxOnTax(CalculationDetails& calculationDetails, TaxSplitDetails& taxSplitDetails)
    : _skipTaxOnTaxIfNoFare(false),
      _requireTaxOnTaxSeqMatch(false),
      _requireTaxOnDomTaxMatch(false),
      _calculationDetails(calculationDetails),
      _taxSplitDetails(taxSplitDetails)
{
}

TaxOnTax::TaxOnTax(TaxItem& taxItem)
: _skipTaxOnTaxIfNoFare(false),
  _requireTaxOnTaxSeqMatch(false),
  _requireTaxOnDomTaxMatch(false),
  _calculationDetails(taxItem.calculationDetails()),
  _taxSplitDetails(taxItem.taxSplitDetails())
{
}

bool
TaxOnTax::useTaxOnTax(const TaxCodeReg& taxCodeReg)
{
  return TaxOnTaxCommon::useTaxOnTax(taxCodeReg.taxType(), taxCodeReg.taxOnTaxCode(),
        taxCodeReg.taxOnTaxExcl());
}

bool
TaxOnTax::useTaxOnTax(TaxTypeCode taxType,
                      const std::vector<std::string>& taxOnTaxCode,
                      Indicator taxOnTaxExcl)
{
  return TaxOnTaxCommon::useTaxOnTax(taxType, taxOnTaxCode, taxOnTaxExcl);
}

void
TaxOnTax::calculateTaxOnTax(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            MoneyAmount& taxAmount,
                            MoneyAmount& taxableFare,
                            TaxCodeReg& taxCodeReg,
                            std::vector<TaxItem*>& taxOnTaxItems,
                            const MoneyAmount& specialPercentage)
{
  TaxOnTaxCommon taxOnTaxCommon(_skipTaxOnTaxIfNoFare, _requireTaxOnTaxSeqMatch,
      _requireTaxOnDomTaxMatch, _indexRange, _calculationDetails, _taxSplitDetails);

  if (_filter.enable())
    taxOnTaxCommon.filter() = _filter;

  MoneyAmount percentage = taxCodeReg.taxAmt();

  if (taxUtil::doUsTaxesApplyOnYQYR(trx, *(taxResponse.farePath())))
  {
    if (specialPercentage > EPSILON)
      percentage = specialPercentage;
  }

  bool taxOnlyTaxesFromCurrentSegment = Tax::shouldSplitPercentageTax(trx, taxCodeReg.taxCode());

  taxOnTaxCommon.calculateTaxOnTax(trx,
                                   taxResponse,
                                   taxAmount,
                                   taxableFare,
                                   taxCodeReg.nation(),
                                   taxCodeReg.taxOnTaxCode(),
                                   taxCodeReg.taxOnTaxExcl(),
                                   percentage,
                                   taxOnTaxItems,
                                   taxOnlyTaxesFromCurrentSegment,
                                   TrxUtil::isShoppingTaxRequest(&trx));
}

void
TaxOnTax::calculateTaxOnTax(Trx& trx,
                            TaxResponse& taxResponse,
                            MoneyAmount& taxAmount,
                            MoneyAmount& taxableFare,
                            TaxItem& taxItem,
                            bool taxOnlyTaxesFromCurrentSegment,
                            const bool isShoppingTaxRequest)
{
  TaxOnTaxCommon taxOnTaxCommon(_skipTaxOnTaxIfNoFare, _requireTaxOnTaxSeqMatch,
      _requireTaxOnDomTaxMatch, _indexRange, _calculationDetails, _taxSplitDetails);

  if (_filter.enable())
    taxOnTaxCommon.filter() = _filter;

  taxOnTaxCommon.calculateTaxOnTax(trx,
                                   taxResponse,
                                   taxAmount,
                                   taxableFare,
                                   taxItem.nation(),
                                   taxItem.taxOnTaxCode(),
                                   taxItem.taxOnTaxExcl(),
                                   taxItem.taxAmt(),
                                   taxItem.taxOnTaxItems(),
                                   taxOnlyTaxesFromCurrentSegment,
                                   isShoppingTaxRequest);
}

MoneyAmount
TaxOnTax::calculateTaxFromTaxItem(const TaxResponse& taxResponse,
                                  const std::vector<std::string>& taxOnTaxCode,
                                  const NationCode& nation,
                                  std::vector<TaxItem*>& taxOnTaxItems,
                                  bool taxOnlyTaxesFromCurrentSegment,
                                  const bool isShoppingTaxRequest) const
{
  TaxOnTaxCommon taxOnTaxCommon(_skipTaxOnTaxIfNoFare, _requireTaxOnTaxSeqMatch,
      _requireTaxOnDomTaxMatch, _indexRange, _calculationDetails, _taxSplitDetails);

  if (_filter.enable())
    taxOnTaxCommon.filter() = _filter;

  return taxOnTaxCommon.calculateTaxFromTaxItem(taxResponse,
                                                taxOnTaxCode,
                                                nation,
                                                taxOnTaxItems,
                                                taxOnlyTaxesFromCurrentSegment,
                                                isShoppingTaxRequest);
}

