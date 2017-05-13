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
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/GetTaxCodeReg.h"

#include <vector>

namespace tse
{
class TaxCodeReg;

class AbstractGetTaxCodeReg
{
public:
  virtual const std::vector<TaxCodeReg*>* get(const TaxCode& taxCode) const = 0;
};

class GetTaxCodeRegAdapter : public AbstractGetTaxCodeReg
{
  PricingTrx& _trx;
public:
  GetTaxCodeRegAdapter(PricingTrx& trx) : _trx(trx)
  {
  }

  const std::vector<TaxCodeReg*>* get(const TaxCode& taxCode) const override
  {
    return Cat33GetTaxCodeReg(_trx, taxCode).taxCodeReg();
  }
};

} // end of tse namespace
