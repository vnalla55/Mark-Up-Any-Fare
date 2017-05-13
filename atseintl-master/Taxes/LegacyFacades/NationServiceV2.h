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

#include "Taxes/AtpcoTaxes/DataModel/Common/Codes.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/NationService.h"

namespace tse
{

class NationServiceV2 : public tax::NationService
{
public:
  NationServiceV2() = default;
  ~NationServiceV2() = default;

  std::string getMessage(const tax::type::Nation& nationCode,
                         const tax::type::Timestamp& ticketingDate) const override;
};

} // namespace tse
