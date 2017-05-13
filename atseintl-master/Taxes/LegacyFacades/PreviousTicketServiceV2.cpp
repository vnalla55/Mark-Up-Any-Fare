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

#include "DataModel/PricingTrx.h"
#include "LegacyFacades/PreviousTicketServiceV2.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/PreviousTicketTaxInfo.h"

#include <boost/algorithm/string.hpp>
#include <tuple>

namespace tse
{
namespace
{
const std::string UTC_CONFIG_NAME{"ATPCAEXCH"};
const std::string UTC_CONFIG_PARAM_PREFIX{"PARENTTAXES"};
}

PreviousTicketServiceV2::PreviousTicketServiceV2(PricingTrx& trx, UtcConfig* utcConfig)
  : _trx(trx), _config(utcConfig)
{
  _config->readConfig(UTC_CONFIG_NAME);

  for (const tse::PreviousTicketTaxInfo& each : _trx.getParentTaxes())
  {
    _previousTicketTaxInfo.emplace(
        each.getSabreTaxCode(), each.getPercentage(), each.isCanadaExch());
  }
}

const std::set<tax::PreviousTicketTaxInfo>&
PreviousTicketServiceV2::getTaxesForPreviousTicket() const
{
  return _previousTicketTaxInfo;
}

std::string
PreviousTicketServiceV2::getParamName(const std::string& sabreTaxCode) const
{
  return UTC_CONFIG_PARAM_PREFIX + sabreTaxCode;
}

std::set<std::string>
PreviousTicketServiceV2::getParentTaxes(const std::string& sabreTaxCode) const
{
  return _config->get(getParamName(sabreTaxCode));
}
}
