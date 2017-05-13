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
#include "Common/TseStringTypes.h"

#include <boost/core/noncopyable.hpp>
#include <memory>
#include <utility>

namespace tse
{

class AbstractTaxSplitData : private boost::noncopyable
{
public:
  virtual const TaxCode& getTaxCode() const = 0;
  virtual const MoneyAmount& getTaxValue() const = 0;
  virtual const CurrencyNoDec getCurrencyNoDec() const = 0;
  virtual void setTaxValue(const MoneyAmount& taxValue) = 0;
  virtual void computeAndStoreTaxValue() = 0;
  virtual const CurrencyCode& getTaxCurrencyCode() const = 0;
  virtual const LocCode& getStationCode() const = 0;
  virtual const TaxDescription& getTaxDescription() const = 0;
  virtual std::pair<MoneyAmount, CurrencyNoDec> getAmountPublished() const = 0;
  virtual const CurrencyCode& getPublishedCurrency() const = 0;
  virtual const NationCode& getTaxCountryCode() const = 0;
  virtual bool getGoodAndServicesTax() const = 0;
  virtual CarrierCode getTaxAirlineCode() const = 0;
  virtual char getTaxType() const = 0;

  virtual ~AbstractTaxSplitData() {}
};

} // end of tse namespace
