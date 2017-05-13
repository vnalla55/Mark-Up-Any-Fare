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

#include "Taxes/LegacyFacades/TaxCodeTextServiceV2.h"

#include "DBAccess/DataHandle.h"
#include "DBAccess/TaxCodeTextInfo.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/Codes.h"
#include "Taxes/LegacyFacades/ConvertCode.h"

namespace tse
{

TaxCodeTextServiceV2::TaxCodeTextServiceV2(DataHandle& dataHandle) :
    TaxCodeTextService(),
    _dataHandle(dataHandle) {}

void TaxCodeTextServiceV2::getTaxCodeText(tax::type::Index itemNo,
                                          tax::type::Vendor vendor,
                                          std::vector<std::string>& out) const
{
  std::vector<const TaxCodeTextInfo*> taxCodeTextInfoVect;
  taxCodeTextInfoVect = _dataHandle.getTaxCodeText(toTseVendorCode(vendor), itemNo);

  for(const TaxCodeTextInfo* taxCodeTextInfo : taxCodeTextInfoVect)
  {
    if(!taxCodeTextInfo)
      continue;

    out.push_back(taxCodeTextInfo->taxCodeText());
  }
}

} /* namespace tse */
