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

#include "Taxes/AtpcoTaxes/DataModel/Services/PreviousTicketTaxInfo.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/PreviousTicketService.h"
#include "Taxes/Common/UtcConfig.h"

#include <memory>

namespace tse
{
class PricingTrx;

class PreviousTicketServiceV2 : public tax::PreviousTicketService
{
  PricingTrx& _trx;
  std::unique_ptr<UtcConfig> _config;
  std::set<tax::PreviousTicketTaxInfo> _previousTicketTaxInfo;

  PreviousTicketServiceV2(const PreviousTicketServiceV2&) = delete;
  PreviousTicketServiceV2& operator=(const PreviousTicketServiceV2&) = delete;

public:
  PreviousTicketServiceV2(PricingTrx& trx, UtcConfig* utcConfig);

  const std::set<tax::PreviousTicketTaxInfo>& getTaxesForPreviousTicket() const override;

  std::string getParamName(const std::string& sabreTaxCode) const;

  std::set<std::string> getParentTaxes(const std::string& sabreTaxCode) const override;
};
}
