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
#include <vector>

#include "Common/OCUtil.h"
#include "DomainDataObjects/OptionalService.h"

namespace tax
{

bool
OCUtil::isOCSegmentRelated(const type::OptionalServiceTag& type)
{
  return (type == tax::type::OptionalServiceTag::FlightRelated ||
      type == tax::type::OptionalServiceTag::PrePaid ||
      type == tax::type::OptionalServiceTag::BaggageCharge);
}

bool
OCUtil::isOCValidForGroup(const type::OptionalServiceTag& type,
    const type::ProcessingGroup& processingGroup)
{
  if(type == type::OptionalServiceTag::Blank)
  {
    return true;
  }

  if (processingGroup == type::ProcessingGroup::OC)
  {
    return (type == type::OptionalServiceTag::FlightRelated) ||
        (type == type::OptionalServiceTag::PrePaid) ||
        (type == type::OptionalServiceTag::TicketRelated) ||
        (type == type::OptionalServiceTag::Merchandise) ||
        (type == type::OptionalServiceTag::FareRelated);
  }
  else if (processingGroup == type::ProcessingGroup::Baggage)
  {
    return (type == type::OptionalServiceTag::BaggageCharge);
  }

  return false;
}

std::string
OCUtil::getOCTypeString(const type::OptionalServiceTag& type)
{
  switch (type)
  {
  case type::OptionalServiceTag::PrePaid:
    return "PREPAID";

  case type::OptionalServiceTag::FlightRelated:
    return "FLIGHTRELATED";

  case type::OptionalServiceTag::TicketRelated:
    return "TICKETRELATED";

  case type::OptionalServiceTag::Merchandise:
    return "TICKETRELATED";

  case type::OptionalServiceTag::FareRelated:
    return "FARERELATED";

  case type::OptionalServiceTag::BaggageCharge:
    return "BAGGAGECHARGE";

  default:
    return "BLANK";
  }
}

} // end of tax namespace
