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

#include "Taxes/AtpcoTaxes/ServiceInterfaces/TaxReissueService.h"

namespace tse
{

class TaxReissueServiceV2 : public tax::TaxReissueService
{
public:
  TaxReissueServiceV2() : tax::TaxReissueService() {}
  virtual ~TaxReissueServiceV2() = default;

  std::vector<std::shared_ptr<const tax::TaxReissue>>
  getTaxReissues(const tax::type::TaxCode& taxCode,
                   const tax::type::Timestamp& ticketingDate) const final override;
};

} // namespace tse
