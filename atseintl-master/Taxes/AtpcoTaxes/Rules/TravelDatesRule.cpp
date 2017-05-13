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
#include <boost/lexical_cast.hpp>

#include "Rules/TravelDatesRule.h"

#include "Rules/TravelDatesApplicator.h"
#include "DomainDataObjects/Request.h"

namespace tax
{

TravelDatesJourneyRule::ApplicatorType
TravelDatesJourneyRule::createApplicator(type::Index const& itinIndex,
                                         const Request& request,
                                         Services& /*services*/,
                                         RawPayments& /*itinPayments*/) const
{
  type::Date startDate = request.getItinByIndex(itinIndex).travelOriginDate();
  if (startDate.is_blank_date())
    startDate = request.ticketingOptions().ticketingDate();
  return ApplicatorType(this, _firstTravelDate, _lastTravelDate, startDate);
}

std::string
TravelDatesJourneyRule::getDescription(Services&) const
{
  return std::string("JOURNEY START DATE MUST BE BETWEEN ") +
         boost::lexical_cast<std::string>(_firstTravelDate) + " AND " +
         boost::lexical_cast<std::string>(_lastTravelDate);
}

TravelDatesTaxPointRule::ApplicatorType
TravelDatesTaxPointRule::createApplicator(type::Index const& itinIndex,
                                          const Request& request,
                                          Services& /*services*/,
                                          RawPayments& /*itinPayments*/) const
{
  const Itin& itin = request.getItinByIndex(itinIndex);
  return ApplicatorType(
      this, request.ticketingOptions().ticketingDate(), _firstTravelDate, _lastTravelDate, itin);
}

std::string
TravelDatesTaxPointRule::getDescription(Services&) const
{
  return std::string("TRAVEL DATE MUST BE BETWEEN ") +
         boost::lexical_cast<std::string>(_firstTravelDate) + " AND " +
         boost::lexical_cast<std::string>(_lastTravelDate);
}
}
