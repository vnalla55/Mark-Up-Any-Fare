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

#include "Common/Timestamp.h"
#include "DataModel/Common/Types.h"

#include <memory>
#include <vector>

namespace tax
{
class TaxReissue;

class TaxReissueService
{
public:
  virtual ~TaxReissueService() = default;

  virtual std::vector<std::shared_ptr<const TaxReissue>>
  getTaxReissues(const type::TaxCode& taxCode, const type::Timestamp& ticketingDate) const = 0;

protected:
  TaxReissueService() = default;
};

} // namespace tax
