// ---------------------------------------------------------------------------
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
// ---------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/Common/TaxOnTaxCommon.h"
#include "Taxes/Common/TaxOnTaxFilter.h"
#include "Taxes/Common/TaxSplitDetails.h"

#include <boost/core/noncopyable.hpp>

namespace tse

{
class PricingTrx;
class TaxCodeReg;
class TaxResponse;
class TaxItem;
class CalculationDetails;

class TaxOnTax : private boost::noncopyable
{

public:
  TaxOnTax(CalculationDetails& calculationDetails, TaxSplitDetails& taxSplitDetails);
  TaxOnTax(TaxItem& taxItem);

  static bool useTaxOnTax(const TaxCodeReg& taxCodeReg);
  static bool useTaxOnTax(TaxTypeCode taxType,
                          const std::vector<std::string>& taxOnTaxCode,
                          Indicator taxOnTaxExcl);

  void calculateTaxOnTax(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         MoneyAmount& taxAmount,
                         MoneyAmount& taxableFare,
                         TaxCodeReg& taxCodeReg,
                         std::vector<TaxItem*>& taxOnTaxItems,
                         const MoneyAmount& specialPercentage);

  void calculateTaxOnTax(Trx& trx,
                         TaxResponse& taxResponse,
                         MoneyAmount& taxAmount,
                         MoneyAmount& taxableFare,
                         TaxItem& taxItem,
                         bool taxOnlyTaxesFromCurrentSegment = false,
                         const bool isShoppingTaxRequest = false);

  MoneyAmount calculateTaxFromTaxItem(const TaxResponse& taxResponse,
                                      const std::vector<std::string>& taxOnTaxCode,
                                      const NationCode& nation,
                                      std::vector<TaxItem*>& taxOnTaxItems,
                                      bool taxOnlyTaxesFromCurrentSegment,
                                      const bool isShoppingTaxRequest = false) const;

  void
  setSkipTaxOnTaxIfNoFare(bool skipTaxOnTax)
  {
    _skipTaxOnTaxIfNoFare = skipTaxOnTax;
  }

  void
  setRequireTaxOnTaxSeqMatch(bool requireTaxOnTaxSeqMatch)
  {
    _requireTaxOnTaxSeqMatch = requireTaxOnTaxSeqMatch;
  }

  void
  setRequireTaxOnDomTaxMatch(bool requireTaxOnDomTaxMatch)
  {
    _requireTaxOnDomTaxMatch = requireTaxOnDomTaxMatch;
  }

  void
  setIndexRange(std::pair<uint16_t, uint16_t> indexRange)
  {
    _indexRange = indexRange;
  }

  TaxOnTaxFilter& filter() {return _filter;}

private:
  bool _skipTaxOnTaxIfNoFare;
  bool _requireTaxOnTaxSeqMatch;
  bool _requireTaxOnDomTaxMatch;
  std::pair<uint16_t, uint16_t> _indexRange;
  CalculationDetails& _calculationDetails;
  TaxSplitDetails& _taxSplitDetails;
  TaxOnTaxFilter _filter;
};
}
