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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
class AbstractTaxInformationModel
{
public:
  virtual TaxCode getTaxCode() const = 0;
  virtual MoneyAmount getTaxAmount() const = 0;
  virtual CurrencyCode getTaxCurrencyCode() const = 0;
  virtual LocCode getTaxPointLocSabreTaxes() const = 0;
  virtual MoneyAmount getAmountPublished() const = 0;
  virtual uint16_t getPublishedCurrencyPrecision() const = 0;
  virtual CurrencyCode getPublishedCurrency() const = 0;
  virtual uint16_t getPaymentCurrencyPrecision() const = 0;
  virtual NationCode getTaxCountryCode() const = 0;
  virtual bool getGoodAndServicesTax() const = 0;
  virtual TaxDescription getTaxDescription() const = 0;

  virtual ~AbstractTaxInformationModel() {}
};

} // end of tse namespace
