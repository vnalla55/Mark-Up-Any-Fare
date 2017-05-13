// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "DataModel/Common/Types.h"
#include "DomainDataObjects/TaxAndRounding.h"

namespace tax
{
class TaxRoundingInfoService;

class TaxInclIndProcessor
{
public:
  TaxInclIndProcessor(const type::MoneyAmount& amount)
    : _originalAmount(amount),
      _totalTaxPercentage(0, 1) {}

  void
  includeTax(const TaxAndRounding& taxAndRounding);

  type::MoneyAmount
  getAmount(const TaxRoundingInfoService& taxRoundingInfoService);

  type::MoneyAmount
  getTax(TaxAndRounding& taxAndRounding,
         const TaxRoundingInfoService& taxRoundingInfoService);

private:
  type::MoneyAmount
  sumFlatTaxes(const TaxRoundingInfoService& taxRoundingInfoService);

  type::MoneyAmount
  sumPercentageTaxes(const type::MoneyAmount& amount,
                     const TaxRoundingInfoService& taxRoundingInfoService);

  type::MoneyAmount _originalAmount;
  type::MoneyAmount _totalTaxPercentage;
  std::vector<TaxAndRounding> _taxVector;
};

} // end of tax namespace
