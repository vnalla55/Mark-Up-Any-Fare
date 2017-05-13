//----------------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"

namespace tse
{

class AmVatTaxRatesOnCharges
{
public:
  // AmVatTaxRate class definition
  class AmVatTaxRate
  {
  public:
    const TaxCode& getTaxCode() const
    { return _taxCode; }

    void setTaxCode(const TaxCode& taxCode)
    { _taxCode = taxCode; }

    uint8_t getTaxRate() const
    { return _taxRate; }

    void setTaxRate(uint8_t taxRate)
    { _taxRate = taxRate; }

  private:
    TaxCode _taxCode;
    uint8_t _taxRate;
  };
  // end of AmVatTaxRate class definition

  AmVatTaxRatesOnCharges(const std::string& acmsData);

  const AmVatTaxRate* getAmVatTaxRate(const NationCode& nationCode) const;

  const std::map<NationCode, AmVatTaxRate>& getData() const
  { return _amVatTaxRates; }

private:
  std::map<NationCode, AmVatTaxRate> _amVatTaxRates;
};
}
