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

#include "Common/FallbackUtil.h"
#include "Taxes/LegacyTaxes/Cat33TaxReissue.h"
#include <algorithm>

namespace tse
{
FIXEDFALLBACK_DECL(taxReissueEmptyCarierProcessing);

const std::string TaxReissueSelector::LEGACY_TAXES_TAX_TYPE = "000";

TaxReissue*
TaxReissueSelector::getTaxReissue(const TaxType& taxType, const CarrierCode& carrier,
    bool skipCat33Only/*=false*/) const
{
  auto carrierMatch = [](const TaxReissue* taxReissue, const CarrierCode& carrier)
  {
    if (!fallback::fixed::taxReissueEmptyCarierProcessing())
      if (carrier.empty())
        return true;

    const auto& carriers = taxReissue->validationCxr();
    return carriers.empty() ||
           std::find(carriers.begin(), carriers.end(), carrier) != carriers.end();
  };

  if (!skipCat33Only)
  {
    for (TaxReissue* taxReissue : _taxReissues)
    {
      if (taxReissue->cat33onlyInd() == 'Y' && taxReissue->taxType() == taxType &&
          carrierMatch(taxReissue, carrier))
      {
        return taxReissue;
      }
    }
  }

  for (TaxReissue* taxReissue : _taxReissues)
  {
    if (taxReissue->cat33onlyInd() != 'Y' && taxReissue->taxType() == taxType &&
        carrierMatch(taxReissue, carrier))
    {
      return taxReissue;
    }
  }

  return nullptr;
}

const std::string Cat33TaxReissue::DEFAULT_CARRIER = "YY";

Indicator
Cat33TaxReissue::getRefundableTaxTag() const
{
  if (_taxReissue && _taxReissue->refundInd() == 'N')
    return 'N';
  else
    return 'Y';
}

} // end of tse namespace
