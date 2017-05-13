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

#include "Taxes/LegacyFacades/NationServiceV2.h"

#include "DBAccess/DataHandle.h"
#include "DBAccess/TaxNation.h"
#include "Taxes/AtpcoTaxes/Common/Timestamp.h"
#include "Taxes/LegacyFacades/ConvertCode.h"

namespace tse
{

std::string NationServiceV2::getMessage(const tax::type::Nation& nationCode,
                                        const tax::type::Timestamp& ticketingDate) const
{
  const DateTime tseTicketingDate = toDateTime(ticketingDate);
  DataHandle dataHandle(tseTicketingDate);

  const TaxNation* taxNation = dataHandle.getTaxNation(toTseNationCode(nationCode),
                                                       tseTicketingDate);

  if (taxNation)
    return taxNation->message();

  return "";
}

}
