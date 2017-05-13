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

#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/GetTicketingDate.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

namespace tse
{

class TaxCodeReg;

class TaxOnChangeFeeConfig
{
public:
  virtual bool isTaxOnChangeFee(TaxCodeReg* taxCodeReg) const = 0;
};

class UtcConfigAdapter : public TaxOnChangeFeeConfig
{
  PricingTrx& _trx;
public:
  explicit UtcConfigAdapter(PricingTrx& trx) : _trx(trx) {}

  bool isTaxOnChangeFee(TaxCodeReg* taxCodeReg) const override
  {
    return utc::isTaxOnChangeFee(_trx, *taxCodeReg, GetTicketingDate(_trx).get());
  }
};

} // end of tse namespace
