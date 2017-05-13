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
#include <cassert>

#include "Common/OCUtil.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/OptionalService.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TaxPointFinder.h"
#include "Rules/TaxPointLoc3AsNextStopoverApplicator.h"
#include "Rules/TaxPointLoc3AsNextStopoverRule.h"
#include "Rules/TaxPointUtils.h"
#include "ServiceInterfaces/LocService.h"

namespace tax
{

TaxPointLoc3AsNextStopoverApplicator::TaxPointLoc3AsNextStopoverApplicator(
    const TaxPointLoc3AsNextStopoverRule& rule,
    const GeoPath& geoPath,
    const LocService& locService)
  : BusinessRuleApplicator(&rule),
    _geoPath(geoPath),
    _locService(locService),
    _rule(rule)
{
}

TaxPointLoc3AsNextStopoverApplicator::~TaxPointLoc3AsNextStopoverApplicator() {}

bool
TaxPointLoc3AsNextStopoverApplicator::matchLocs(const Geo& geo3, PaymentDetail& paymentDetail) const
{
  int16_t dir = (paymentDetail.getTaxPointLoc1().id() < geo3.id()) ? 1 : -1;
  if (_locService.isInLoc(geo3.locCode(), _rule.locZone3(), _rule.vendor()))
  {
    for (type::Index i = paymentDetail.getTaxPointBegin().id() + dir;
         i != paymentDetail.getLoc3().id();
         i += 2 * dir)
    {
      const Geo& geo = _geoPath.geos()[i];

      if (paymentDetail.ticketedPointTag() == type::TicketedPointTag::MatchTicketedPointsOnly &&
          geo.unticketedTransfer() == type::UnticketedTransfer::Yes)
        continue;

      paymentDetail.setTaxPointLoc2(&geo);
      return true;
    }
  }
  return false;
}

void
TaxPointLoc3AsNextStopoverApplicator::matchLocsForOC(const Geo& geo3, PaymentDetail& paymentDetail)
    const
{
  bool matchedLoc3 = _locService.isInLoc(geo3.locCode(), _rule.locZone3(), _rule.vendor());
  for (OptionalService& optionalService : paymentDetail.optionalServiceItems())
  {
    if (OCUtil::isOCSegmentRelated(optionalService.type()) && !optionalService.isFailed())
    {
      if (!matchedLoc3 || optionalService.getTaxPointEnd().id() != geo3.id())
      {
        optionalService.setFailedRule(&_rule);
        continue;
      }

      optionalService.setTaxPointLoc3(geo3);

      bool matchedLoc2 = false;
      int16_t dir =
          (optionalService.getTaxPointBegin().id() < optionalService.getTaxPointEnd().id()) ? 1
                                                                                            : -1;

      for (type::Index i = optionalService.getTaxPointBegin().id() + dir;
           i != optionalService.getTaxPointEnd().id();
           i += 2 * dir)
      {
        const Geo& geo = _geoPath.geos()[i];

        if (paymentDetail.ticketedPointTag() == type::TicketedPointTag::MatchTicketedPointsOnly &&
            geo.unticketedTransfer() == type::UnticketedTransfer::Yes)
          continue;

        matchedLoc2 = true;
        optionalService.setTaxPointLoc2(geo);
        break;
      }

      if (!matchedLoc2)
      {
        optionalService.setFailedRule(&_rule);
      }
    }
  }
}

bool
TaxPointLoc3AsNextStopoverApplicator::apply(PaymentDetail& paymentDetail) const
{
  bool result = false;
  const bool matchTicketedOnly =
      paymentDetail.ticketedPointTag() == type::TicketedPointTag::MatchTicketedPointsOnly;
  result = applyOnYqYr(paymentDetail.getTaxPointBegin().id(),
                       paymentDetail.taxPointsProperties(),
                       matchTicketedOnly,
                       paymentDetail.getMutableYqYrDetails()) ||
           result;
  result = applyOnItinerary(paymentDetail) || result;
  return result;
}

bool
TaxPointLoc3AsNextStopoverApplicator::applyOnYqYr(const type::Index beginId,
                                                  const TaxPointsProperties& properties,
                                                  const bool matchTicketedOnly,
                                                  TaxableYqYrs& taxableYqYrs) const
{
  int32_t direction =
      (_geoPath.geos()[beginId].loc().tag() == type::TaxPointTag::Departure) ? 1 : -1;
  TaxPointFinder taxPointFinder(
      beginId, 0, properties, matchTicketedOnly, direction, _geoPath.geos());
  const std::vector<const Geo*>& loc3 =
      taxPointFinder.find<StopoverValidator>(taxableYqYrs._ranges);

  for (type::Index i = 0; i < loc3.size(); ++i)
  {
    if (taxableYqYrs.isFailedRule(i))
        continue;

    taxableYqYrs._data[i]._taxPointLoc3 = loc3[i];
    taxableYqYrs._data[i]._taxPointEnd = loc3[i];
    if (!loc3[i] || !_locService.isInLoc(loc3[i]->locCode(), _rule.locZone3(), _rule.vendor()))
    {
      taxableYqYrs.setFailedRule(i, *getBusinessRule());
    }
  }

  const std::vector<const Geo*>& loc2 =
      taxPointFinder.find<ConnectionValidator>(taxableYqYrs._ranges);
  assert(loc2.size() == taxableYqYrs._subject.size());

  for (type::Index i = 0; i < loc2.size(); ++i)
  {
    if (taxableYqYrs.isFailedRule(i))
      continue;

    if (loc2[i] != nullptr &&
        ((direction == 1 && loc2[i]->id() < loc3[i]->id()) ||
         (direction == -1 && loc2[i]->id() > loc3[i]->id())))
    {
      taxableYqYrs._data[i]._taxPointLoc2 = loc2[i];
    }
    else
    {
      taxableYqYrs.setFailedRule(i, *getBusinessRule());
    }
  }

  return !taxableYqYrs.areAllFailed();
}

bool
TaxPointLoc3AsNextStopoverApplicator::applyOnItinerary(PaymentDetail& paymentDetail) const
{
  bool result = false;
  type::Index i = paymentDetail.getTaxPointBegin().id();
  const Geo& geo1 = paymentDetail.getTaxPointLoc1();
  type::Index endId = 0;
  int step = 0;
  if (geo1.loc().tag() == type::TaxPointTag::Departure)
  {
    endId = _geoPath.geos().size() - 1;
    step = 1;
  }
  else if (geo1.loc().tag() == type::TaxPointTag::Arrival)
  {
    endId = 0;
    step = -1;
  }

  if (step != 0)
  {
    if (i == endId)
      return false;

    type::Index nextId = i;
    while (nextId != endId)
    {
      nextId += step;
      const Geo& nextGeo = _geoPath.geos()[nextId];
      if (geo1.loc().tag() == nextGeo.loc().tag())
        continue;

      paymentDetail.setLoc3(&nextGeo);

      if (paymentDetail.isStopover(nextId))
      {
        matchLocsForOC(nextGeo, paymentDetail);
        result = matchLocs(nextGeo, paymentDetail);
        if (result)
          paymentDetail.setTaxPointEnd(nextGeo);

        break;
      }
    }
  }

  if (!result)
  {
    paymentDetail.setFailedRule(&_rule);
    result = !paymentDetail.areAllOptionalServicesFailed();
  }
  return result;
}

} // namespace tax
