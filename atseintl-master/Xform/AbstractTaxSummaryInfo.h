// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "FareCalc/CalcTotals.h"
#include <boost/core/noncopyable.hpp>

namespace tse
{
class CalcTotals;
class PricingTrx;
class TaxRecord;

class AbstractTaxSummaryInfo : private boost::noncopyable
{
public:
  virtual const TaxCode& getTaxCode() const = 0;
  virtual const MoneyAmount getTaxValue() const = 0;
  virtual const CurrencyNoDec getCurrencyNoDec() const = 0;
  virtual const CurrencyCode& getTaxCurrencyCode() const = 0;
  virtual const NationCode& getTaxCountryCode() const = 0;
  virtual bool getGoodAndServicesTax() const = 0;

  virtual ~AbstractTaxSummaryInfo() {}

  static std::shared_ptr<AbstractTaxSummaryInfo>
  create(CalcTotals& calcTotals, const TaxRecord& taxRecord, PricingTrx& pricingTrx);
};

} // end of tse namespace
