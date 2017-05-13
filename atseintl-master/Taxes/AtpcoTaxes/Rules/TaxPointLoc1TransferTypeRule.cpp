// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "Rules/TaxPointLoc1TransferTypeRule.h"
#include "Rules/TaxPointLoc1TransferTypeApplicator.h"

namespace tax
{

TaxPointLoc1TransferTypeRule::TaxPointLoc1TransferTypeRule(
    const type::TransferTypeTag transferTypeTag)
  : _transferTypeTag(transferTypeTag)
{
}

TaxPointLoc1TransferTypeRule::~TaxPointLoc1TransferTypeRule() {}

const type::EquipmentCode TaxPointLoc1TransferTypeRule::ChangeOfGauge = "CHG";

TaxPointLoc1TransferTypeRule::ApplicatorType
TaxPointLoc1TransferTypeRule::createApplicator(const type::Index& itinIndex,
                                               const Request& request,
                                               Services& /*services*/,
                                               RawPayments& /*itinPayments*/) const
{
  Itin const& itin = request.getItinByIndex(itinIndex);
  return ApplicatorType(*this, itin.flightUsages(), request.flights());
}

std::string
TaxPointLoc1TransferTypeRule::getDescription(Services&) const
{
  std::string origDestReason = "NOT APPLY IN JOURNEY ORIGIN OR JOURNEY DESTINATION POINT.\n";

  if (_transferTypeTag == type::TransferTypeTag::Interline)
    return origDestReason +
           std::string(
               " APPLY IF INBOUND FLIGHT HAS A DIFFERENT MARKETING CARRIER TO THE OUTBOUND FLIGHT");

  else if (_transferTypeTag == type::TransferTypeTag::OnlineWithChangeOfFlightNumber)
    return origDestReason + std::string(" APPLY IF INBOUND FLIGHT HAS THE SAME MARKETING CARRIER\n"
                                        " BUT DIFFERENT FLIGHT NUMBER TO THE OUTBOUND FLIGHT");

  else if (_transferTypeTag == type::TransferTypeTag::OnlineWithChangeOfGauge)
    return origDestReason + std::string(" APPLY IF INBOUND FLIGHT HAS THE SAME MARKETING CARRIER\n"
                                        " AND FLIGHT NUMBER AS THE OUTBOUND FLIGHT,\n"
                                        " BUT THE EQUIPMENT CODES ARE DIFFERENT OR ARE BOTH CHG");

  else if (_transferTypeTag == type::TransferTypeTag::OnlineWithNoChangeOfGauge)
    return origDestReason +
           std::string(" APPLY IF INBOUND FLIGHT HAS THE SAME MARKETING CARRIER\n"
                       " AND FLIGHT NUMBER AS THE OUTBOUND FLIGHT,\n"
                       " AND THE EQUIPMENT CODES ARE THE SAME BUT ARE NOT BOTH CHG");
  else
    return origDestReason;
}

type::TransferTypeTag
TaxPointLoc1TransferTypeRule::getTransferTypeTag() const
{
  return _transferTypeTag;
}
}
