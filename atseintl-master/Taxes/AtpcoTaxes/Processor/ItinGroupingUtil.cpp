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

#include "DataModel/Common/CodeOps.h"
#include "DataModel/Services/CarrierFlight.h"
#include "DataModel/Services/CarrierFlightSegment.h"
#include "DomainDataObjects/FarePath.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Request.h"
#include "Processor/ItinGroupingUtil.h"
#include "ServiceInterfaces/CarrierApplicationService.h"
#include "ServiceInterfaces/CarrierFlightService.h"
#include "ServiceInterfaces/LocService.h"
#include "ServiceInterfaces/PassengerTypesService.h"
#include "ServiceInterfaces/RulesRecordsService.h"
#include "ServiceInterfaces/SectorDetailService.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/BasePathUtils.h"
#include "Rules/TaxData.h"
#include "Rules/BusinessRulesContainer.h"
#include "Rules/SectorDetailMatcher.h"

#include <boost/spirit/include/karma.hpp>
#include <algorithm>

namespace tax
{

void
ItinGroupingUtil::DateSegmenter::buildDateSegmentKey(const Itin& itin, std::string& key)
{
  key.append("|");
  key.append(_nation.rawArray(), _nation.length());
  if (!_journeyDateLimits.empty())
  {
    type::Date originDate(itin.travelOriginDate());
    int i = 0;
    for (std::vector<type::Date>::iterator it = _journeyDateLimits.begin();
         it != _journeyDateLimits.end();
         ++it, ++i)
    {
      if (*it > originDate)
        break;
    }
    key.append("-JRN");
    char buf[8] = "";
    char* p = buf;
    boost::spirit::karma::generate(p, boost::spirit::int_, i);
    key.append(buf);
  }

  std::vector<type::Date>::const_iterator dtIt = _departureDateLimits.begin();

  int i = 0;
  for (type::Index fuId = 0; fuId < itin.flightUsages().size(); ++fuId)
  {
    type::Index depId = 2 * fuId;
    type::Index arrId = depId + 1;

    if (itin.geoPath()->geos()[depId].getNation() != _nation &&
        itin.geoPath()->geos()[arrId].getNation() != _nation)
      continue;

    type::Date departureDate(itin.flightUsages()[fuId].departureDate());
    for (; dtIt != _departureDateLimits.end(); ++dtIt, ++i)
    {
      if (*dtIt > departureDate)
        break;
    }
    key += '|';
    char buf[8] = "";
    char* p = buf;
    boost::spirit::karma::generate(p, boost::spirit::int_, i);
    key.append(buf);
  }
}

void
ItinGroupingUtil::DateSegmenter::fillDateLimits(type::TaxPointTag taxPointTag)
{
  RulesRecordsService::ByNationConstValue tn =
      _rulesRecordsService->getTaxRulesContainers(_nation, taxPointTag, _ticketingDate);
  for (const TaxData& taxData: *tn)
  {
    const std::vector<std::shared_ptr<BusinessRulesContainer>> containers =
        taxData.getDateFilteredCopy(type::ProcessingGroup::Itinerary, _ticketingDate);

    for (const std::shared_ptr<BusinessRulesContainer>& rulesContainer : containers)
    {
      if (rulesContainer->getValidatorsGroups()._itinGroup._travelDatesJourneyRule)
      {
        const TravelDatesJourneyRule& rule =
            *rulesContainer->getValidatorsGroups()._itinGroup._travelDatesJourneyRule;

        if (!rule.firstTravelDate().is_blank_date())
          _journeyDateLimits.push_back(rule.firstTravelDate());
        if (!rule.lastTravelDate().is_blank_date())
          _journeyDateLimits.push_back(rule.lastTravelDate());
      }
      if (rulesContainer->getValidatorsGroups()._itinGroup._travelDatesTaxPointRule)
      {
        const TravelDatesTaxPointRule& rule =
            *rulesContainer->getValidatorsGroups()._itinGroup._travelDatesTaxPointRule;

        if (!rule.firstTravelDate().is_blank_date())
          _departureDateLimits.push_back(rule.firstTravelDate());
        if (!rule.lastTravelDate().is_blank_date())
          _departureDateLimits.push_back(rule.lastTravelDate());
      }
    }
  }
}

void
ItinGroupingUtil::DateSegmenter::initDateLimits()
{
  fillDateLimits(type::TaxPointTag::Departure);
  fillDateLimits(type::TaxPointTag::Arrival);
  fillDateLimits(type::TaxPointTag::Sale);
  fillDateLimits(type::TaxPointTag::Delivery);

  auto greaterThanTicketingDate = [&](const type::Date& date)
    {
      return date > _ticketingDate.date();
    };

  std::sort(_journeyDateLimits.begin(), _journeyDateLimits.end());
  _journeyDateLimits.erase(std::unique(_journeyDateLimits.begin(), _journeyDateLimits.end()),
                           _journeyDateLimits.end());
  _journeyDateLimits.erase(_journeyDateLimits.begin(),
                           std::find_if(_journeyDateLimits.begin(),
                                        _journeyDateLimits.end(),
                                        greaterThanTicketingDate));

  std::sort(_departureDateLimits.begin(), _departureDateLimits.end());
  _departureDateLimits.erase(std::unique(_departureDateLimits.begin(), _departureDateLimits.end()),
                             _departureDateLimits.end());
  _departureDateLimits.erase(_departureDateLimits.begin(),
                             std::find_if(_departureDateLimits.begin(),
                                          _departureDateLimits.end(),
                                          greaterThanTicketingDate));
}

void
ItinGroupingUtil::FlightSegmenter::buildFltSegmentKey(const Itin& itin, std::string& key)
{
  key.append("|FLTRNG-");
  key.append(_nation.rawArray(), _nation.length());

  // Departures
  for (type::Index geoId = 0; geoId < itin.geoPath()->geos().size(); geoId += 2)
  {
    const Geo& geo = itin.geoPath()->geos()[geoId];

    if (geo.getNation() != _nation)
      continue;

    if (geoId > 0)
    {
      const Flight* flightBefore = itin.flightUsages()[geoId / 2 - 1].flight();
      buildKeyPart(_flightRangesBefore, flightBefore, type::TaxPointTag::Departure, key);
    }

    const Flight* flightAfter = itin.flightUsages()[geoId / 2].flight();
    buildKeyPart(_flightRangesAfter, flightAfter, type::TaxPointTag::Departure, key);
  }

  // Arrivals
  key.append("_a");
  for (type::Index geoId = itin.geoPath()->geos().size() - 1;
       geoId < itin.geoPath()->geos().size();
       geoId -= 2)
  {
    const Geo& geo = itin.geoPath()->geos()[geoId];

    if (geo.getNation() != _nation)
      continue;

    FlightRangesByCarrierAndTag::const_iterator rangesByCarrierTag;

    if (geoId < itin.geoPath()->geos().size() - 1)
    {
      const Flight* flightBefore = itin.flightUsages()[geoId / 2 + 1].flight();
      buildKeyPart(_flightRangesBefore, flightBefore, type::TaxPointTag::Arrival, key);
    }

    const Flight* flightAfter = itin.flightUsages()[geoId / 2].flight();
    buildKeyPart(_flightRangesAfter, flightAfter, type::TaxPointTag::Arrival, key);
  }
}


void
ItinGroupingUtil::FlightSegmenter::buildKeyPart(FlightRangesByCarrierAndTag& flightRanges,
                                                const Flight* flight,
                                                type::TaxPointTag taxPointTag,
                                                std::string& key)
{
  const type::CarrierCode& marketingCarrier = flight->marketingCarrier();
  FlightRangesByCarrierAndTag::const_iterator rangesByCarrierTag =
      flightRanges.find(std::make_pair(marketingCarrier,
                                       taxPointTag));

  key.append(marketingCarrier.rawArray(), marketingCarrier.length());
  if (rangesByCarrierTag != flightRanges.end())
  {
    const std::vector<FlightRange>& carrierTagFlightRanges = rangesByCarrierTag->second;

    for (type::Index i = 0; i < carrierTagFlightRanges.size(); ++i)
    {
      const FlightRange& range = carrierTagFlightRanges[i];

      if (flight->marketingCarrierFlightNumber() >= range.first &&
          flight->marketingCarrierFlightNumber() <= range.second)
      {
        char buf[8] = "";
        char* p = buf;
        boost::spirit::karma::generate(p, boost::spirit::int_, i);
        key += '_';
        key.append(buf);
        break;
      }
    }
  }
}

void
ItinGroupingUtil::FlightSegmenter::fillFlightRanges(type::TaxPointTag taxPointTag)
{
  RulesRecordsService::ByNationConstValue tn =
      _rulesRecordsService->getTaxRulesContainers(_nation, taxPointTag, _ticketingDate);
  for (const TaxData& taxData: *tn)
  {
    const std::vector<std::shared_ptr<BusinessRulesContainer>> containers =
        taxData.getDateFilteredCopy(type::ProcessingGroup::Itinerary, _ticketingDate);

    for (const std::shared_ptr<BusinessRulesContainer>& rulesContainer : containers)
    {
      if (rulesContainer->getValidatorsGroups()._itinGroup._carrierFlightRule)
      {
        const CarrierFlightRule& rule =
            *rulesContainer->getValidatorsGroups()._itinGroup._carrierFlightRule;

        if (rule.carrierFlightItemBefore() != 0)
        {
          std::shared_ptr<const CarrierFlight> carrierFlightBefore =
              _carrierFlightService->getCarrierFlight(rule.vendor(),
                                                      rule.carrierFlightItemBefore());

          for (const CarrierFlightSegment& cfs: carrierFlightBefore->segments)
          {
            if (cfs.flightFrom != -1)
              _flightRangesBefore[std::make_pair(cfs.marketingCarrier, taxPointTag)].push_back(
                  std::make_pair(cfs.flightFrom, cfs.flightTo));
          }
        }

        if (rule.carrierFlightItemAfter() != 0)
        {
          std::shared_ptr<const CarrierFlight> carrierFlightAfter =
              _carrierFlightService->getCarrierFlight(rule.vendor(), rule.carrierFlightItemAfter());

          for (const CarrierFlightSegment& cfs: carrierFlightAfter->segments)
          {
            if (cfs.flightFrom != -1)
              _flightRangesAfter[std::make_pair(cfs.marketingCarrier, taxPointTag)].push_back(
                  std::make_pair(cfs.flightFrom, cfs.flightTo));
          }
        }
      }
    }
  }
}

void
ItinGroupingUtil::FlightSegmenter::initFlightRanges()
{
  fillFlightRanges(type::TaxPointTag::Departure);
  fillFlightRanges(type::TaxPointTag::Arrival);

  for (FlightRangesByCarrierAndTag::iterator it = _flightRangesBefore.begin();
       it != _flightRangesBefore.end();
       ++it)
  {
    std::vector<FlightRange>& fltRange = it->second;
    std::sort(fltRange.begin(), fltRange.end());
    fltRange.erase(std::unique(fltRange.begin(), fltRange.end()), fltRange.end());
  }
  for (FlightRangesByCarrierAndTag::iterator it = _flightRangesAfter.begin();
       it != _flightRangesAfter.end();
       ++it)
  {
    std::vector<FlightRange>& fltRange = it->second;
    std::sort(fltRange.begin(), fltRange.end());
    fltRange.erase(std::unique(fltRange.begin(), fltRange.end()), fltRange.end());
  }
}

bool
ItinGroupingUtil::AYPrevalidator::checkLoc1Stopover(const ValidatorsGroups& validators)
{
  if (validators._taxPointBeginGroup._taxPointLoc1StopoverTagRule)
  {
    const TaxPointLoc1StopoverTagRule& rule =
        *validators._taxPointBeginGroup._taxPointLoc1StopoverTagRule;

    if (rule.getStopoverTag() == type::StopoverTag::Connection)
      return false;
  }
  return true;
}

bool
ItinGroupingUtil::AYPrevalidator::checkLoc1Zone(const ValidatorsGroups& validators)
{
  if (validators._fillerGroup._taxPointLoc1Rule)
  {
    const Geo& origin = _itin.geoPath()->geos()[2 * _fuId];
    const TaxPointLoc1Rule& rule = *validators._fillerGroup._taxPointLoc1Rule;

    if (!_services.locService().isInLoc(origin.locCode(), rule.locZone(), rule.vendor()))
      return false;
  }
  return true;
}

bool
ItinGroupingUtil::AYPrevalidator::checkPointOfSaleZone(const ValidatorsGroups& validators)
{
  if (validators._saleGroup._pointOfSaleRule)
  {
    const PointOfSaleRule& rule = *validators._saleGroup._pointOfSaleRule;

    if (!_services.locService().isInLoc(_itin.pointOfSale()->loc(),
                                        rule.getLocZone(), rule.getVendor()))
      return false;
  }
  return true;
}

bool
ItinGroupingUtil::AYPrevalidator::checkValidatingCarrier(const ValidatorsGroups& validators)
{
  if (validators._itinGroup._validatingCarrierRule)
  {
    const ValidatingCarrierRule& rule =
        *validators._itinGroup._validatingCarrierRule;

    std::shared_ptr<const CarrierApplication> carrierApplication =
        _services.carrierApplicationService().getCarrierApplication(rule.vendor(),
                                                                    rule.carrierAppl());

    bool match = false;
    for (const CarrierApplicationEntry& entry: carrierApplication->entries)
    {
      if (entry.applind == type::CarrierApplicationIndicator::Positive &&
          (entry.carrier == "$$" || entry.carrier == _itin.farePath()->validatingCarrier()))
      {
        match = true;
        if (entry.carrier != "$$")
          break;
      }

      if (entry.applind == type::CarrierApplicationIndicator::Negative &&
          entry.carrier == _itin.farePath()->validatingCarrier())
      {
        match = false;
        break;
      }
    }

    if (!match)
      return false;
  }
  return true;
}

bool
ItinGroupingUtil::AYPrevalidator::checkCarrierFlight(const ValidatorsGroups& validators)
{
  if (validators._itinGroup._carrierFlightRule)
  {
    const Flight& flight = *_itin.flightUsages()[_fuId].flight();
    const CarrierFlightRule& rule = *validators._itinGroup._carrierFlightRule;

    if (rule.carrierFlightItemAfter() != 0)
    {
      std::shared_ptr<const CarrierFlight> carrierFlightAfter =
          _services.carrierFlightService().getCarrierFlight(rule.vendor(),
                                                            rule.carrierFlightItemAfter());

      bool match = false;
      for (const CarrierFlightSegment& cfs: carrierFlightAfter->segments)
      {
        if (flight.marketingCarrier() == cfs.marketingCarrier &&
            (cfs.flightFrom <= flight.marketingCarrierFlightNumber() &&
             cfs.flightTo >= flight.marketingCarrierFlightNumber()))
        {
          match = true;
          break;
        }
      }

      if (!match)
        return false;
    }
  }
  return true;
}

bool
ItinGroupingUtil::AYPrevalidator::checkPassenger(const ValidatorsGroups& validators)
{
  if (validators._itinGroup._passengerTypeCodeRule)
  {
    const PassengerTypeCodeRule& rule = *validators._itinGroup._passengerTypeCodeRule;

    std::shared_ptr<const PassengerTypeCodeItems> ptc =
        _services.passengerTypesService().getPassengerTypeCode(rule.vendor(), rule.itemNo());

    bool match = false;
    for (const PassengerTypeCodeItem& item: *ptc)
    {
      if (item.passengerType.empty() ||
          (item.matchIndicator == type::PtcMatchIndicator::Input &&
           item.passengerType == _itin.passenger()->_code) ||
          (item.matchIndicator == type::PtcMatchIndicator::Output &&
           item.passengerType == _itin.farePath()->outputPtc()))
      {
        match = true;
        break;
      }
    }

    if (!match)
      return false;
  }
  return true;
}

bool
ItinGroupingUtil::AYPrevalidator::checkSectorDetail(const ValidatorsGroups& validators)
{
  if (validators._miscGroup._sectorDetailRule)
  {
    const SectorDetailRule& rule = *validators._miscGroup._sectorDetailRule;

    std::shared_ptr<SectorDetail const> sectorDetail =
        _services.sectorDetailService().getSectorDetail(rule.vendor(), rule.itemNo());

    SectorDetailMatcher matcher(_itin.flightUsages(),
                                _request.flights(),
                                _request.fares(),
                                _itin.farePath()->fareUsages(),
                                *_itin.geoPathMapping());

    bool match = false;
    for (const SectorDetailEntry& entry: sectorDetail->entries)
    {
      if (matcher.matchSectorDetails(entry, _fuId))
      {
        match = (entry.applTag == type::SectorDetailAppl::Positive);
        break;
      }
    }

    if (!match)
      return false;
  }

  return true;
}

bool
ItinGroupingUtil::AYPrevalidator::doesAYapply()
{
  const Geo& origin = _itin.geoPath()->geos()[2 * _fuId];
  RulesRecordsService::ByNationConstValue tn =
      _services.rulesRecordsService().getTaxRulesContainers(origin.getNation(),
                                                            type::TaxPointTag::Departure,
                                                            _ticketingDate);

  for (const TaxData& taxData: *tn)
  {
    if (taxData.getTaxName().taxCode() != "AY")
      continue;

    bool AYapplies = false;

    const std::vector<std::shared_ptr<BusinessRulesContainer>> containers =
        taxData.getDateFilteredCopy(type::ProcessingGroup::Itinerary, _ticketingDate);

    for (const std::shared_ptr<BusinessRulesContainer>& rulesContainer : containers)
    {
      if (!checkLoc1Stopover(rulesContainer->getValidatorsGroups()))
        continue;

      if (!checkLoc1Zone(rulesContainer->getValidatorsGroups()))
        continue;

      if (!checkPointOfSaleZone(rulesContainer->getValidatorsGroups()))
        continue;

      if (!checkValidatingCarrier(rulesContainer->getValidatorsGroups()))
        continue;

      if (!checkCarrierFlight(rulesContainer->getValidatorsGroups()))
        continue;

      if (!checkPassenger(rulesContainer->getValidatorsGroups()))
        continue;

      if (!checkSectorDetail(rulesContainer->getValidatorsGroups()))
        continue;

      if (rulesContainer->getValidatorsGroups()._miscGroup._ticketMinMaxValueRule &&
          tax::BasePathUtils::baseFareAmount(*_itin.farePath()) != 0)
        continue;

      AYapplies = !rulesContainer->getValidatorsGroups()._miscGroup._exemptTagRule ? true : false;
      break;
    }
    return AYapplies;
  }
  return false;
}

bool
ItinGroupingUtil::doesAYapply(const Request& request,
                              const Itin& itin,
                              type::Index fuId,
                              const type::Timestamp& ticketingDate)
{
  return AYPrevalidator(request, itin, fuId, ticketingDate, _services).doesAYapply();
}

void
ItinGroupingUtil::getNationTransitTimes(const type::Nation nation,
                                        const type::Timestamp& ticketingDate,
                                        std::set<int32_t>& transitTimes)
{
  getNationTransitTimesForTag(nation, ticketingDate, type::TaxPointTag::Departure, transitTimes);
  getNationTransitTimesForTag(nation, ticketingDate, type::TaxPointTag::Arrival, transitTimes);
}

void
ItinGroupingUtil::getNationTransitTimesForTag(const type::Nation nation,
                                              const type::Timestamp& ticketingDate,
                                              type::TaxPointTag taxPointTag,
                                              std::set<int32_t>& transitTimes)
{
  RulesRecordsService::ByNationConstValue tn =
      _services.rulesRecordsService().getTaxRulesContainers(nation, taxPointTag, ticketingDate);

  for (const TaxData& taxData: *tn)
  {
    const std::vector<std::shared_ptr<BusinessRulesContainer>> containers =
        taxData.getDateFilteredCopy(type::ProcessingGroup::Itinerary, ticketingDate);

    for (const std::shared_ptr<BusinessRulesContainer>& rulesContainer : containers)
    {
      if (rulesContainer->getValidatorsGroups()._fillerGroup._fillTimeStopoversRule)
      {
        const FillTimeStopoversRule& rule =
            *rulesContainer->getValidatorsGroups()._fillerGroup._fillTimeStopoversRule;
        int32_t transitTime = toInt(rule.number());

        switch (rule.unit())
        {
        case type::StopoverTimeUnit::Minutes:
          transitTimes.insert(transitTime);
          break;
        case type::StopoverTimeUnit::Hours:
        case type::StopoverTimeUnit::HoursSameDay: // same as hours - good enough for grouping
          transitTimes.insert(transitTime * 60);
          break;
        case type::StopoverTimeUnit::Days:
          transitTimes.insert(transitTime * 60 * 24);
          break;
        case type::StopoverTimeUnit::Months: // good enough for grouping
          transitTimes.insert(transitTime * 60 * 24 * 30);
          break;
        case type::StopoverTimeUnit::Blank:
          if (rule.number() == "A") // adding just "corner-case" times from "A" logic
          {
            transitTimes.insert(6 * 60);
            transitTimes.insert(10 * 60);
            transitTimes.insert(17 * 60);
          }
          if (rule.number() == "D")
          {
            transitTimes.insert(4 * 60);
            transitTimes.insert(6 * 60);
          }
          break;
        }
      }
    }
  }
}

} // namespace tax
