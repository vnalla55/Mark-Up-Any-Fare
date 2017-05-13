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

#include "Common/LocationUtil.h"
#include "Common/OCUtil.h"
#include "ServiceInterfaces/LocService.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TaxPointLoc1InternationalDomesticApplicator.h"
#include "Rules/TaxPointLoc1InternationalDomesticRule.h"

namespace tax
{

namespace
{

const type::Nation USA{"US"};

bool
isNotFromUSOrIsUSAndGeoNotInBufferZone(type::Nation const& nation, const Geo& geo)
{
  return nation != USA || !geo.loc().inBufferZone();
}

} // namespace

TaxPointLoc1InternationalDomesticApplicator::TaxPointLoc1InternationalDomesticApplicator(
    const TaxPointLoc1InternationalDomesticRule& rule,
    const GeoPath& geoPath,
    const LocService& locService)
  : BusinessRuleApplicator(&rule), _geoPath(geoPath), _locService(locService), _intlDomLoc1IndRule(rule)
{
}

TaxPointLoc1InternationalDomesticApplicator::~TaxPointLoc1InternationalDomesticApplicator() {}

bool
TaxPointLoc1InternationalDomesticApplicator::apply(PaymentDetail& paymentDetail) const
{
  type::AdjacentIntlDomInd adjacentIntlDomInd = _intlDomLoc1IndRule.getAdjacentIntlDomInd();

  if (adjacentIntlDomInd == type::AdjacentIntlDomInd::Blank)
    return true;

  bool matchedLoc1 = false;

  if (adjacentIntlDomInd == type::AdjacentIntlDomInd::AdjacentStopoverDomestic)
    matchedLoc1 = applyAdjacentStopoverDomestic(paymentDetail);
  else if (adjacentIntlDomInd == type::AdjacentIntlDomInd::AdjacentStopoverInternational)
    matchedLoc1 = applyAdjacentStopoverInternational(paymentDetail);
  else if (adjacentIntlDomInd == type::AdjacentIntlDomInd::AdjacentDomestic)
    matchedLoc1 = applyAdjacent(paymentDetail, LocationUtil::isDomestic);
  else if (adjacentIntlDomInd == type::AdjacentIntlDomInd::AdjacentInternational)
    matchedLoc1 = applyAdjacent(paymentDetail, LocationUtil::isInternational);

  if (!matchedLoc1)
  {
    for(OptionalService & optionalService : paymentDetail.optionalServiceItems())
    {
      if (OCUtil::isOCSegmentRelated(optionalService.type()) && !optionalService.isFailed())
      {
        optionalService.setFailedRule(&_intlDomLoc1IndRule);
      }
    }

    TaxableYqYrs& taxableYqYrs = paymentDetail.getMutableYqYrDetails();
    for (type::Index i = 0; i < taxableYqYrs._subject.size(); ++i)
    {
      if (taxableYqYrs.isFailedRule(i))
        continue;

      taxableYqYrs.setFailedRule(i, _intlDomLoc1IndRule);
    }

    paymentDetail.setFailedRule(&_intlDomLoc1IndRule);
    return !paymentDetail.areAllOptionalServicesFailed() || !taxableYqYrs.areAllFailed();
  }

  return true;
}

bool
TaxPointLoc1InternationalDomesticApplicator::applyAdjacentStopoverDomestic(
    PaymentDetail& paymentDetail) const
{
  const int i = static_cast<int>(paymentDetail.getTaxPointBegin().id());
  const Geo& geo1 = _geoPath.geos()[i];
  const type::Nation& nation = geo1.loc().nation();

  if (geo1.loc().tag() == type::TaxPointTag::Departure)
  {
    if (i == 0)
      return false;

    for (int previousId = i - 2; previousId >= 0; previousId -= 2)
    {
      if (!paymentDetail.isStopover(previousId))
        continue;

      return LocationUtil::isDomestic(nation, _geoPath.geos()[previousId].getNation());
    }
  }
  else if (geo1.loc().tag() == type::TaxPointTag::Arrival)
  {
    int geoPathEndIndex = static_cast<int>(_geoPath.geos().size() - 1);
    if (i == geoPathEndIndex)
      return false;

    for (int nextId = i + 2; nextId <= geoPathEndIndex; nextId += 2)
    {
      if (!paymentDetail.isStopover(nextId))
        continue;

      return LocationUtil::isDomestic(nation, _geoPath.geos()[nextId].getNation());
    }
  }
  return false;
}

bool
TaxPointLoc1InternationalDomesticApplicator::applyAdjacentStopoverInternational(
    PaymentDetail& paymentDetail) const
{
  const int i = static_cast<int>(paymentDetail.getTaxPointBegin().id());
  const Geo& geo1 = _geoPath.geos()[i];
  const type::Nation& nation = geo1.loc().nation();

  if (geo1.loc().tag() == type::TaxPointTag::Departure)
  {
    if (i == 0)
      return false;

    for (int previousId = i - 2; previousId >= 0; previousId -= 2)
    {
      if (!paymentDetail.isStopover(previousId))
        continue;

      return LocationUtil::isInternational(nation, _geoPath.geos()[previousId].getNation()) &&
          isNotFromUSOrIsUSAndGeoNotInBufferZone(nation, _geoPath.geos()[previousId]);
    }
  }
  else if (geo1.loc().tag() == type::TaxPointTag::Arrival)
  {
    int geoPathEndIndex = static_cast<int>(_geoPath.geos().size() - 1);
    if (i == geoPathEndIndex)
      return false;

    for (int nextId = i + 2; nextId <= geoPathEndIndex; nextId += 2)
    {
      if (!paymentDetail.isStopover(nextId))
        continue;

      return LocationUtil::isInternational(nation, _geoPath.geos()[nextId].getNation()) &&
        isNotFromUSOrIsUSAndGeoNotInBufferZone(nation, _geoPath.geos()[nextId]);
    }
  }

  return false;
}

bool
TaxPointLoc1InternationalDomesticApplicator::applyAdjacent(PaymentDetail& paymentDetail,
                                                           FlightTypeChecker& flightTypeChecker)
    const
{
  const int i = static_cast<int>(paymentDetail.getTaxPointBegin().id());
  const Geo& geo1 = _geoPath.geos()[i];
  const type::Nation& nation = geo1.loc().nation();

  if (geo1.loc().tag() == type::TaxPointTag::Departure)
  {
    if (i == 0)
      return false;

    for (int previousId = i - 2; previousId >= 0; previousId -= 2)
    {
      if (skipUnticketedPoint(previousId))
        continue;

      return flightTypeChecker(nation, _geoPath.geos()[previousId].getNation());
    }
  }
  else if (geo1.loc().tag() == type::TaxPointTag::Arrival)
  {
    int geoPathEndIndex = static_cast<int>(_geoPath.geos().size() - 1);
    if (i == geoPathEndIndex)
      return false;

    for (int nextId = i + 2; nextId <= geoPathEndIndex; nextId += 2)
    {
      if (skipUnticketedPoint(nextId))
        continue;

      return flightTypeChecker(nation, _geoPath.geos()[nextId].getNation());
    }
  }
  return false;
}

bool
TaxPointLoc1InternationalDomesticApplicator::skipUnticketedPoint(const type::Index& id) const
{
  return (_intlDomLoc1IndRule.getTicketedPointTag() ==
              type::TicketedPointTag::MatchTicketedPointsOnly &&
         _geoPath.geos()[id].unticketedTransfer() == type::UnticketedTransfer::Yes);
}

} // namespace tax
