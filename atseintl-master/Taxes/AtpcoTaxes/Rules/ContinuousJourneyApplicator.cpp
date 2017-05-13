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

#include "Rules/ContinuousJourneyApplicator.h"
#include "Rules/ContinuousJourneyRule.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TaxPointUtils.h"
#include "DomainDataObjects/GeoPath.h"

namespace tax
{

namespace
{
bool
is1stPointInCJ(std::vector<Trip> const& trips, type::Index id)
{
  for (const auto& trip : trips)
  {
    if (id == trip.first)
      return true;
  }
  return false;
}

bool
is2ndPointInCJ(std::vector<Trip> const& trips,
               type::Index id,
               const GeoPath& geoPath,
               bool matchTicketedOnly)
{
  for (const auto& trip : trips)
  {
    type::Index geoId = trip.first + 2;
    if (matchTicketedOnly)
    {
      while (geoPath.geos()[geoId].isUnticketed())
        geoId += 2;

      if (geoId > trip.second)
        return false;
    }

    if (id == geoId)
      return true;
  }
  return false;
}
}

ContinuousJourneyApplicator::ContinuousJourneyApplicator(ContinuousJourneyRule const& parent,
                                                         GeoPath const& geoPath)
  : BusinessRuleApplicator(&parent), _geoPath(geoPath), _continuousJourneyRule(parent)
{
}

ContinuousJourneyApplicator::~ContinuousJourneyApplicator() {}

bool
ContinuousJourneyApplicator::apply(PaymentDetail& paymentDetail) const
{
  const type::Index& geo1Id = paymentDetail.getTaxPointBegin().id();

  std::vector<Trip> journeys;
  SpecialTrips::findContinuousJourneys(paymentDetail, _geoPath, journeys);

  bool matchTicketedOnly =
      paymentDetail.ticketedPointTag() == type::TicketedPointTag::MatchTicketedPointsOnly;

  if (_continuousJourneyRule.getRtnToOrig() == type::RtnToOrig::ContinuousJourney1stPoint)
    return is1stPointInCJ(journeys, geo1Id);
  else if (_continuousJourneyRule.getRtnToOrig() == type::RtnToOrig::ContinuousJourney2ndPoint)
    return is2ndPointInCJ(journeys, geo1Id, _geoPath, matchTicketedOnly);
  else
  {
    paymentDetail.applicatorFailMessage() =
        "WRONG RETURN TO ORIGIN TAG FOR CONTINUOUS JOURNEY RULE";
    return false;
  }
}
}
