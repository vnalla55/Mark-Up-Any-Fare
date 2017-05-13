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

#include <vector>

namespace tse
{
class PricingTrx;
class TaxCodeReg;
class AbstractGetTaxNation;
class AbstractGetTaxCodeReg;
class TaxOnChangeFeeConfig;

class TaxOnChangeFeeDriver
{
  std::vector<TaxCodeReg*> _taxCodeReg;

public:
  TaxOnChangeFeeDriver(PricingTrx& trx, AbstractGetTaxNation& getTaxNation,
      AbstractGetTaxCodeReg& getTaxCodeReg, TaxOnChangeFeeConfig& taxOnChangeFeeConfig);

  const std::vector<TaxCodeReg*>& getTaxCodeReg() const;
};

} // end of tse namespace
