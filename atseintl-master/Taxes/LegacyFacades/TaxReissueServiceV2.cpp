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

#include "Taxes/LegacyFacades/TaxReissueServiceV2.h"

#include "DBAccess/DataHandle.h"
#include "DBAccess/TaxReissue.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/TaxReissue.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/DaoDataFormatConverter.h"

namespace tse
{

const tse::TaxType v2TaxType = "000";

std::vector<std::shared_ptr<const tax::TaxReissue>>
TaxReissueServiceV2::getTaxReissues(const tax::type::TaxCode& taxCode,
                                      const tax::type::Timestamp& ticketingDate) const
{
  DataHandle dataHandle(toDateTime(ticketingDate));
  const std::vector<TaxReissue*>& tseReissues =
      dataHandle.getTaxReissue(toTseTaxCode(taxCode), toDateTime(ticketingDate));

  std::vector<std::shared_ptr<const tax::TaxReissue>> ret;
  ret.reserve(tseReissues.size());
  for (const TaxReissue* tseReissue : tseReissues)
  {
    if (!tseReissue || tseReissue->taxType() == v2TaxType)
      continue;

    auto taxReissue = std::make_shared<tax::TaxReissue>();
    DaoDataFormatConverter::convert(*tseReissue, *taxReissue);
    ret.push_back(taxReissue);
  }

  ret.shrink_to_fit();
  return ret;
}

} // namespace tse
