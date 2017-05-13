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

#include "DomainDataObjects/TaxInclIndProcessor.h"
#include "ServiceInterfaces/TaxRoundingInfoService.h"

namespace tax
{

void
TaxInclIndProcessor::includeTax(const TaxAndRounding& taxAndRounding)
{
  if (taxAndRounding.getIncludedTaxType() == PERCENTAGE)
    _totalTaxPercentage += taxAndRounding.getValue();

  _taxVector.push_back(taxAndRounding);
}

type::MoneyAmount
TaxInclIndProcessor::sumPercentageTaxes(const type::MoneyAmount& amount,
                                        const TaxRoundingInfoService& taxRoundingInfoService)
{
  type::MoneyAmount result(0, 1);
  for (TaxAndRounding& each : _taxVector)
  {
    if (each.getIncludedTaxType() != PERCENTAGE)
      continue;

    type::MoneyAmount taxAmount =  each.getRoundedPercentageTax(amount, _totalTaxPercentage);
    if (each.isStandardRounding())
      taxRoundingInfoService.doStandardRound(taxAmount, each.getUnit(), each.getDir(), -1, true);
    result += taxAmount;
  }
  return result;
}

type::MoneyAmount
TaxInclIndProcessor::sumFlatTaxes(const TaxRoundingInfoService& taxRoundingInfoService)
{
  type::MoneyAmount result(0, 1);
  for (TaxAndRounding& each : _taxVector)
  {
    if (each.getIncludedTaxType() != FLAT)
      continue;

    type::MoneyAmount taxAmount =  each.getRoundedFlatTax();
    if (each.isStandardRounding())
      taxRoundingInfoService.doStandardRound(taxAmount, each.getUnit(), each.getDir(), -1, true);
    result += taxAmount;
  }
  return result;
}

type::MoneyAmount
TaxInclIndProcessor::getAmount(const TaxRoundingInfoService& taxRoundingInfoService)
{
  const type::MoneyAmount amountWithoutFlatTaxes = _originalAmount -
      sumFlatTaxes(taxRoundingInfoService);
  return amountWithoutFlatTaxes - sumPercentageTaxes(amountWithoutFlatTaxes, taxRoundingInfoService);
}

type::MoneyAmount
TaxInclIndProcessor::getTax(TaxAndRounding& taxAndRounding,
                            const TaxRoundingInfoService& taxRoundingInfoService)
{
  type::MoneyAmount result(0, 1);

  if (taxAndRounding.getIncludedTaxType() == FLAT)
  {
    result = taxAndRounding.getRoundedFlatTax();
  }
  else
  {
    const type::MoneyAmount amountWithoutFlatTaxes = _originalAmount -
        sumFlatTaxes(taxRoundingInfoService);
    result = taxAndRounding.getRoundedPercentageTax(
        amountWithoutFlatTaxes, _totalTaxPercentage);
  }

  if (taxAndRounding.isStandardRounding())
    taxRoundingInfoService.doStandardRound(result, taxAndRounding.getRoundingInfo().unit,
        taxAndRounding.getRoundingInfo().dir, -1, true);

  return result;
}

}
