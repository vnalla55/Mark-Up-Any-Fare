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

#include "Processor/RequestAnalyzer.h"
#include "Processor/ItinGroupingUtil.h"
#include "Rules/RequestLogicError.h"
#include "Rules/TaxData.h"
#include "Rules/BusinessRulesContainer.h"
#include "DomainDataObjects/ErrorMessage.h"
#include "DomainDataObjects/Request.h"
#include "DomainDataObjects/Response.h"
#include "ServiceInterfaces/ConfigService.h"
#include "ServiceInterfaces/CurrencyService.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/LocService.h"
#include "ServiceInterfaces/Services.h"
#include "DomainDataObjects/FareUsage.h"

#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/karma.hpp>

#include <algorithm>

namespace tse
{
ATPCO_FALLBACK_DECL(markupAnyFareOptimization)
}

namespace tax
{
typedef CompactOptional<type::Index> OptionalIndex;

namespace
{

const char * const refIdError("Itin %1%: %2% (%3%) must be smaller than the number of %4% (%5%)");

template <class T>
void verifyOptionalRefId(type::Index itinId,
                         const OptionalIndex& refId,
                         const std::vector<T>& vec,
                         const char* refIdName,
                         const char* vecName,
                         const T*& target)
{
  if (refId.has_value())
  {
    if (UNLIKELY(refId.value() >= vec.size()))
    {
      throw RequestLogicError() << boost::format(refIdError)
        % itinId
        % refIdName
        % refId.value()
        % vecName
        % vec.size();
    }

    target = &vec[refId.value()];
  }
}

template <class T>
void verifyRefId(type::Index itinId,
                 type::Index refId,
                 const std::vector<T>& vec,
                 const char* refIdName,
                 const char* vecName,
                 const T*& target)
{
  if (UNLIKELY(refId >= vec.size()))
  {
    throw RequestLogicError() << boost::format(refIdError)
      % itinId
      % refIdName
      % refId
      % vecName
      % vec.size();
  }

  target = &vec[refId];
}

template <class T>
void verifyUsageRefId(type::Index pathId,
                      type::Index usageId,
                      type::Index refId,
                      const std::vector<T>& vec,
                      const char* pathName,
                      const char* usageName,
                      const char* vecName,
                      const T*& target)
{
  if (UNLIKELY(refId >= vec.size()))
  {
    throw RequestLogicError() <<
      boost::format("%1%[%2%].%3%[%4%].Index (%5%) must be smaller than the number of %6% (%7%)")
      % pathName
      % pathId
      % usageName
      % usageId
      % refId
      % vecName
      % vec.size();
  }

  target = &vec[refId];
}

void verifyItinRefIds(Itin& itin, Request& request)
{
  verifyOptionalRefId(itin.id(), itin.yqYrPathRefId(), request.yqYrPaths(),
                      "YqYrPathRefId", "YqYrPaths",
                      itin.yqYrPath());

  verifyOptionalRefId(itin.id(), itin.optionalServicePathRefId(), request.optionalServicePaths(),
                      "OptionalServicePathRefId", "OptionalServicePaths",
                      itin.optionalServicePath());

  verifyOptionalRefId(itin.id(), itin.yqYrPathGeoPathMappingRefId(), request.geoPathMappings(),
                      "YqYrPathGeoPathMappingRefId", "GeoPathMappings",
                      itin.yqYrPathGeoPathMapping());

  verifyOptionalRefId(itin.id(), itin.optionalServicePathGeoPathMappingRefId(), request.geoPathMappings(),
                      "OptionalServicePathGeoPathMappingRefId", "GeoPathMappings",
                      itin.optionalServicePathGeoPathMapping());

  if (!itin.farePathGeoPathMappingRefId().has_value())
  {
    throw RequestLogicError() << boost::format("Itin %1%: missing FarePathGeoPathMappingRefId")
      % itin.id();
  }

  verifyOptionalRefId(itin.id(), itin.farePathGeoPathMappingRefId(), request.geoPathMappings(),
                      "FarePathGeoPathMappingRefId", "GeoPathMappings",
                      itin.geoPathMapping());
}

template <typename TAG, int L, int H>
void append(std::string& str, Code<TAG, L, H> code)
{
  str.append(code.rawArray(), code.length());
}

} // anonymous namespace

RequestAnalyzer::RequestAnalyzer(Request& request_, Services& services_)
  : _services(services_), _request(request_), _useFlightRanges(false)
{
}

void
RequestAnalyzer::analyze()
{
  for (const PointOfSale& pointOfSale : _request.pointsOfSale())
  {
    Geo posTaxPoints;
    posTaxPoints.loc().code() = pointOfSale.loc();
    posTaxPoints.loc().nation() = _services.locService().getNation(pointOfSale.loc());
    posTaxPoints.loc().tag() = type::TaxPointTag::Sale;
    posTaxPoints.unticketedTransfer() = type::UnticketedTransfer::No;
    posTaxPoints.id() = 0; // pati todo question: do we want all POS to have TaxPointIndexBegin ==0
    _request.posTaxPoints().push_back(posTaxPoints);
  }

  verifyFarePaths();
  verifyYqYrPaths();
  verifyItins();
  verifyGeoPaths();

  if (!_services.fallbackService().isSet(tse::fallback::markupAnyFareOptimization))
  {
    if (!markupsInFarePathsExist())
    {
      // Internal TaxRq path. Markups are in Fares, and not FarePaths.
      sumMarkupsForFares();
    }
  }

  for(Itin & itin : _request.allItins())
  {
    itin.computeTimeline();
  }

  if (_request.ticketingOptions().ticketingDate() == type::Date::blank_date())
  {
    throw RequestLogicError()
        << "Required XML attribute missing or invalid: TaxRQ/ProcessingOptions/TicketingDate";
  }
  if (_request.ticketingOptions().ticketingTime().is_blank())
  {
    throw RequestLogicError()
        << "Required XML attribute missing or invalid: TaxRQ/ProcessingOptions/TicketingTime";
  }

  if (_request.ticketingOptions().paymentCurrency() == type::CurrencyCode())
  {
    throw RequestLogicError()
        << "Required XML attribute missing or invalid: TaxRQ/TicketingOptions/PaymentCurrency";
  }

  if (_services.configService().getHpsDomGroupingEnabled() ||
      _services.configService().getHpsIntlGroupingEnabled())
  {
    groupItins();
  }
  else
  {
    _request.itins().reserve(_request.allItins().size());
    for(Itin& itin : _request.allItins())
      _request.itins().push_back(&itin);
  }
}

void
RequestAnalyzer::verifyFarePaths()
{
  for (uint32_t i = 0; i < _request.farePaths().size(); i++)
  {
    FarePath& farePath = _request.farePaths()[i];
    for (uint32_t j = 0; j < farePath.fareUsages().size(); j++)
    {
      FareUsage& fareUsage = farePath.fareUsages()[j];

      verifyUsageRefId(i, j, fareUsage.index(), _request.fares(),
                       "FarePath", "FareUsage", "Fares",
                       fareUsage.fare());
    }
  }
}

void
RequestAnalyzer::verifyYqYrPaths()
{
  for (uint32_t i = 0; i < _request.yqYrPaths().size(); i++)
  {
    YqYrPath& yqYrPath = _request.yqYrPaths()[i];
    for (uint32_t j = 0; j < yqYrPath.yqYrUsages().size(); j++)
    {
      YqYrUsage& yqYrUsage = yqYrPath.yqYrUsages()[j];

      verifyUsageRefId(i, j, yqYrUsage.index(), _request.yqYrs(),
                       "YqYrPath", "YqYrUsage", "YqYrs",
                       yqYrUsage.yqYr());
    }
  }
}

void
RequestAnalyzer::verifyItins()
{
  for (uint32_t i = 0; i < _request.allItins().size(); i++)
  {
    Itin& itin = _request.allItins()[i];

    if (itin.id() != i)
    {
      throw RequestLogicError() << "Itin[" << i << "].Id doesn't match itin position";
    }

    verifyRefId(itin.id(), itin.pointOfSaleRefId(), _request.pointsOfSale(),
                "PointOfSaleRefId", "PointsOfSale",
                itin.pointOfSale());

    verifyRefId(itin.id(), itin.geoPathRefId(), _request.geoPaths(),
                "GeoPathRefId", "GeoPaths",
                itin.geoPath());

    verifyRefId(itin.id(), itin.farePathRefId(), _request.farePaths(),
                "FarePathRefId", "FarePaths",
                itin.farePath());

    verifyItinRefIds(itin, _request);
    assert (itin.geoPathMapping());
    assert (itin.farePath());
    assert (itin.farePathGeoPathMappingRefId().has_value());
    std::vector<Mapping> const& mappings = itin.geoPathMapping()->mappings();

    if (LIKELY(itin.geoPathRefId() < _request.geoPaths().size()))
    {
      std::vector<Geo> const& geos = _request.geoPaths()[itin.geoPathRefId()].geos();

      for (uint32_t j = 0; j < mappings.size(); j++)
        for (uint32_t k = 0; k < mappings[j].maps().size(); k++)
          if (UNLIKELY(mappings[j].maps()[k].index() >= geos.size()))
          {
            throw RequestLogicError()
                << "Itin " << itin.id() << ": GeoPathMapping[" << itin.farePathGeoPathMappingRefId().value()
                << "].Mapping[" << j << "].Map[" << k << "].Index ("
                << mappings[j].maps()[k].index() << ") must be smaller than the number of GeoPath["
                << itin.geoPathRefId() << "].Geos (" << geos.size() << ")";
          }
    }

    if (LIKELY(!_request.fares().empty()))
    {
      if (LIKELY(itin.farePathRefId() < _request.farePaths().size()))
      {
        std::vector<FareUsage> const& fareUsages =
          _request.farePaths()[itin.farePathRefId()].fareUsages();

        if (mappings.size() != fareUsages.size())
        {
          throw RequestLogicError() << "Itin " << itin.id() << ": Number of GeoPathMapping["
                                    << itin.farePathGeoPathMappingRefId().value() << "].Mappings ("
                                    << mappings.size() << ") must match the number of FarePath["
                                    << itin.farePathRefId() << "].FareUsages (" << fareUsages.size()
                                    << ")";
        }
      }
    }

    verifyRefId(itin.id(), itin.passengerRefId(), _request.passengers(),
                "PassengerRefId", "Passengers",
                itin.passenger());
  }

  for(Itin & itin : _request.allItins())
  {
    type::Index id = 0;
    for(FlightUsage & fu : itin.flightUsages())
    {
      fu.setId(id++);

      verifyRefId(itin.id(), fu.flightRefId(), _request.flights(),
                  "FlightUsage: FlightRefId", "Flights",
                  fu.flight());
    }

    if ((itin.geoPath()->geos().size() / 2) != itin.flightUsages().size())
    {
      throw RequestLogicError() << "Itin " << itin.id() << ": Number of FlightUsages ("
                                << itin.flightUsages().size() << ") must be half the number of "
                                << "GeoPath[" << itin.geoPathRefId() << "] geos ("
                                << itin.geoPath()->geos().size() << ")";
    }
  }
}

void
RequestAnalyzer::verifyGeoPaths()
{
  type::Index geoPathId = 0;
  for(GeoPath & geoPath : _request.geoPaths())
  {
    geoPath.id() = geoPathId++;
    type::Index geoId = 0;
    for(Geo & taxPoint : geoPath.geos())
    {
      const type::AirportCode& locCode = taxPoint.loc().code();
      taxPoint.id() = geoId++;
      taxPoint.loc().nation() = _services.locService().getNation(locCode);
      type::CityCode cityCode = _services.locService().getCityCode(locCode);
      if (cityCode.empty())
        cityCode.convertFrom(locCode);

      taxPoint.loc().cityCode() = cityCode;

      if (taxPoint.id() != 0)
      {
        taxPoint.setPrev(&geoPath.geos()[taxPoint.id() - 1]);
      }

      if (taxPoint.id() != geoPath.geos().size() - 1)
      {
        taxPoint.setNext(&geoPath.geos()[taxPoint.id() + 1]);
      }
      else
      {
        taxPoint.makeLast();
      }
    }
  }
}

void
RequestAnalyzer::sumMarkupsForFares()
{
  for (FarePath& farePath : _request.farePaths())
  {
    farePath.totalMarkupAmount() = 0;
    for (const FareUsage& fareUsage : farePath.fareUsages())
    {
      farePath.totalMarkupAmount() += fareUsage.fare()->markupAmount();
    }
  }
}

bool
RequestAnalyzer::markupsInFarePathsExist()
{
  for (FarePath& farePath : _request.farePaths())
  {
    if (farePath.totalMarkupAmount())
    {
      return true;
    }
  }

  return false;
}

void
RequestAnalyzer::groupItins()
{
  type::Timestamp ticketingDate(_request.ticketingOptions().ticketingDate(),
                                _request.ticketingOptions().ticketingTime());

  _useFlightRanges = _services.configService().getHpsUseFlightRanges();
  _travelDateDependantTaxNations = _services.configService().travelDateDependantTaxNations();
  _flightNoDependantTaxNations = _services.configService().flightNoDependantTaxNations();
  _sameDayDepartureTaxNations = _services.configService().sameDayDepartureTaxNations();

  type::Index itinClass = 0;
  for (Itin& itin : _request.allItins())
  {
    std::string key;
    if (_request.processing().applyUSCAGrouping())
      key = generateUSCAKey(itin, ticketingDate);
    else
      key = generateIntlKey(itin, ticketingDate);

    std::map<std::string, type::Index>::iterator it = _request.keyToItinClassMap().find(key);
    if (it == _request.keyToItinClassMap().end())
    {
      it = _request.keyToItinClassMap().insert(std::make_pair(key, itinClass)).first;
      _request.itins().push_back(&itin);
      ++itinClass;
    }
    _request.allItinsMap()[itin.id()] = it->second;
  }
}

std::string
RequestAnalyzer::generateUSCAKey(const Itin& itin, const type::Timestamp& ticketingDate)
{
  std::string key;
  key.reserve(256);
  const std::vector<Geo>& geos = itin.geoPath()->geos();
  assert(!geos.empty());
  type::AirportCode lastArrival(geos.front().locCode());
  append(key, lastArrival);

  for (type::Index geoId = 0; geoId < geos.size(); ++geoId)
  {
    const Geo& geo = geos[geoId];
    const Geo* nextGeo = geo.next();

    if (geo.isDeparture())
    {
      if (geo.locCode() != lastArrival)
      {
        key.append("-//-");
        append(key, geo.locCode());
      }
      type::Index flightId = geoId / 2;
      const FlightUsage& fu = itin.flightUsages()[flightId];
      key.append("-");
      const type::CarrierCode& marketingCarrier = fu.flight()->marketingCarrier();
      append(key, marketingCarrier);
      if (nextGeo && nextGeo->unticketedTransfer() == type::UnticketedTransfer::Yes)
      {
        key.append("-h");
        continue;
      }
      key.append("-");
    }
    else // assuming arrival
    {
      append(key, geo.locCode());
      lastArrival = geo.locCode();
    }
  }

  // stopover vs connection
  key.append("|");
  for (type::Index fuId = 0; fuId < itin.flightUsages().size() - 1; ++fuId)
  {
    const FlightUsage& currFlight = itin.flightUsages()[fuId];
    const FlightUsage& nextFlight = itin.flightUsages()[fuId + 1];

    // Ignoring open segments for now
    type::Timestamp currArrival(currFlight.arrivalDate(), currFlight.arrivalTime());
    type::Timestamp nextDeparture(nextFlight.departureDate(), nextFlight.departureTime());

    int32_t timeDiff = nextDeparture - currArrival;
    if (timeDiff <= 4 * 60)
      key += 'X';
    else if (timeDiff <= 12 * 60)
      key += 's';
    else
      key += 'S';
  }

  // AY prevalidate
  for (type::Index fuId = 0; fuId < itin.flightUsages().size(); ++fuId)
  {
    const Flight& flight = *itin.flightUsages()[fuId].flight();
    const type::AirportCode& orig = geos[2 * fuId].locCode();
    const type::AirportCode& dest = geos[2 * fuId + 1].locCode();
    std::string AYKey;
    AYKey.reserve(20);
    append(AYKey, itin.passenger()->_code);
    append(AYKey, itin.farePath()->validatingCarrier());
    append(AYKey, orig);
    append(AYKey, flight.marketingCarrier());
    append(AYKey, dest);
    char buf[8] = "";
    char* p = buf;
    boost::spirit::karma::generate(p, boost::spirit::int_, flight.marketingCarrierFlightNumber());
    AYKey.append(buf);

    ItinGroupingUtil itu(_services);
    AYApplicationCache::const_iterator itAY = _ayCache.find(AYKey);
    bool AYapplies = (itAY != _ayCache.end())
        ? itAY->second
        : _ayCache.emplace(AYKey, itu.doesAYapply(_request, itin, fuId, ticketingDate)).first->second;
    key += AYapplies ? '1' : '0';
  }

  buildFareAndYqYrKeyPart(itin, key);

  return key;
}

std::string
RequestAnalyzer::generateIntlKey(const Itin& itin, const type::Timestamp& ticketingDate)
{
  std::set<type::Nation> dateDepTaxNationsFound;
  std::set<type::Nation> fltNoDepTaxNationsFound;
  std::string key;
  key.reserve(256);
  const std::vector<Geo>& geos = itin.geoPath()->geos();
  assert(!geos.empty());
  type::AirportCode lastArrival(geos.front().locCode());
  append(key, lastArrival);

  for (type::Index geoId = 0; geoId < geos.size(); ++geoId)
  {
    const Geo& geo = geos[geoId];
    const Geo* nextGeo = geo.next();

    if (std::binary_search(_travelDateDependantTaxNations.begin(),
                           _travelDateDependantTaxNations.end(),
                           geo.getNation()))
      dateDepTaxNationsFound.insert(geo.getNation());

    if (_useFlightRanges)
    {
      if (std::binary_search(_flightNoDependantTaxNations.begin(),
                             _flightNoDependantTaxNations.end(),
                             geo.getNation()))
        fltNoDepTaxNationsFound.insert(geo.getNation());
    }

    if (geo.isDeparture())
    {
      if (geo.locCode() != lastArrival)
      {
        key.append("-//-");
        append(key, geo.locCode());
      }
      type::Index flightId = geoId / 2;
      const FlightUsage& fu = itin.flightUsages()[flightId];
      key.append("-");
      const type::CarrierCode& marketingCarrier = fu.flight()->marketingCarrier();
      append(key, marketingCarrier);
      if (nextGeo && nextGeo->unticketedTransfer() == type::UnticketedTransfer::Yes)
      {
        key.append("-h");
        continue;
      }
      if (!_useFlightRanges)
      {
        char fltNoBuf[8] = "";
        char *p = fltNoBuf;
        boost::spirit::karma::generate(p, boost::spirit::int_, fu.flight()->marketingCarrierFlightNumber());
        key.append(fltNoBuf);
      }
      key.append("-");
    }
    else // assuming arrival
    {
      append(key, geo.locCode());
      lastArrival = geo.locCode();
    }
  }

  if (!dateDepTaxNationsFound.empty())
    buildDateDependantKeyPart(itin, dateDepTaxNationsFound, ticketingDate, key);

  if (!fltNoDepTaxNationsFound.empty())
    buildFlightRangeKeyPart(itin, fltNoDepTaxNationsFound, ticketingDate, key);

  buildStopoverKeyPart(itin, ticketingDate, key);
  buildSameDayKeyPart(itin, key);
  buildFareAndYqYrKeyPart(itin, key);

  return key;
}

void
RequestAnalyzer::buildDateDependantKeyPart(const Itin& itin,
                                           std::set<type::Nation>& dateDepTaxNationsFound,
                                           const type::Timestamp& ticketingDate,
                                           std::string& key)
{
  for(type::Nation nation : dateDepTaxNationsFound)
  {
    DateSegmenterMap::iterator dsIt = _dateSegmenters.find(nation);
    ItinGroupingUtil::DateSegmenter* ds = nullptr;
    if (dsIt == _dateSegmenters.end())
    {
      ds = new ItinGroupingUtil::DateSegmenter(nation, ticketingDate, _services);
      _dateSegmenters.insert(nation, ds);
    }
    else
      ds = dsIt->second;

    ds->buildDateSegmentKey(itin, key);
  }
}

void
RequestAnalyzer::buildFlightRangeKeyPart(const Itin& itin,
                                         std::set<type::Nation>& fltNoDepTaxNationsFound,
                                         const type::Timestamp& ticketingDate,
                                         std::string& key)
{
  for(type::Nation nation: fltNoDepTaxNationsFound)
  {
    FlightSegmenterMap::iterator fsIt = _flightSegmenters.find(nation);
    ItinGroupingUtil::FlightSegmenter* fs = nullptr;
    if (fsIt == _flightSegmenters.end())
    {
      fs = new ItinGroupingUtil::FlightSegmenter(nation, ticketingDate, _services);
      _flightSegmenters.insert(nation, fs);
    }
    else
      fs = fsIt->second;

    fs->buildFltSegmentKey(itin, key);
  }
}

void
RequestAnalyzer::buildStopoverKeyPart(const Itin& itin,
                                      const type::Timestamp& ticketingDate,
                                      std::string& key)
{
  key.append("|");
  for (type::Index fuId = 0; fuId < itin.flightUsages().size() - 1; ++fuId)
  {
    const FlightUsage& currFlight = itin.flightUsages()[fuId];
    const FlightUsage& nextFlight = itin.flightUsages()[fuId + 1];

    // Ignoring open segments for now
    type::Timestamp currArrival(currFlight.arrivalDate(), currFlight.arrivalTime());
    type::Timestamp nextDeparture(nextFlight.departureDate(), nextFlight.departureTime());

    char c = getTransitTimeChar(itin.geoPath()->geos()[2 * fuId + 1].getNation(),
                                ticketingDate,
                                currArrival,
                                nextDeparture);
    key += c;
  }
}

void
RequestAnalyzer::buildSameDayKeyPart(const Itin& itin, std::string& key)
{
  key.append("|");
  for (type::Index fuId = 0; fuId < itin.flightUsages().size() - 1; ++fuId)
  {
    type::Date currArrival(itin.flightUsages()[fuId].arrivalDate());
    type::Date nextDeparture(itin.flightUsages()[fuId + 1].departureDate());

    bool sameDay = currArrival == nextDeparture;
    bool nationOnSameDateDepList =
        std::binary_search(_sameDayDepartureTaxNations.begin(),
                           _sameDayDepartureTaxNations.end(),
                           itin.geoPath()->geos()[2 * fuId + 1].getNation());

    key += (!sameDay && nationOnSameDateDepList) ? '1' : '0';
  }
}

void
RequestAnalyzer::buildFareAndYqYrKeyPart(const Itin& itin, std::string& key)
{
  key.append("|");
  append(key, itin.passenger()->_code);
  for (type::Index fuId = 0; fuId < itin.farePath()->fareUsages().size(); ++fuId)
  {
    const std::vector<Map>& maps = itin.geoPathMapping()->mappings()[fuId].maps();
    const FareUsage& fareUsage = itin.farePath()->fareUsages()[fuId];

    key.append("|");
    char idBuf[4] = "";
    char* p = idBuf;
    boost::spirit::karma::generate(p, boost::spirit::int_, fareUsage.index());
    key.append(idBuf);
    const type::AirportCode& frontGeoLoc = itin.geoPath()->geos()[maps.front().index()].locCode();
    const type::AirportCode& backGeoLoc = itin.geoPath()->geos()[maps.back().index()].locCode();
    append(key, frontGeoLoc);
    append(key, backGeoLoc);
  }
  if (itin.yqYrPath())
  {
    key.append("|");
    for (type::Index yqYrId = 0; yqYrId < itin.yqYrPath()->yqYrUsages().size(); ++yqYrId)
    {
      const YqYr* yqYr = itin.yqYrPath()->yqYrUsages()[yqYrId].yqYr();
      append(key, yqYr->code());
      key += yqYr->type();
      key.append(boost::lexical_cast<std::string>(yqYr->amount()));
    }
  }
}

char
RequestAnalyzer::getTransitTimeChar(const type::Nation& nation,
                                    const type::Timestamp& ticketingDate,
                                    type::Timestamp& arrivalDate,
                                    type::Timestamp& departureDate)
{
  std::set<int32_t>& transitTimes = getNationTransitTimes(nation, ticketingDate);

  int32_t timediff = departureDate - arrivalDate;
  // we don't excpect distance to be very big, but even if it is overflow will be OK
  return static_cast<char>('a' + std::distance(transitTimes.begin(),
                                               std::find_if(transitTimes.begin(), transitTimes.end(),
                                                            timediff <= boost::lambda::_1)));
}

std::set<int32_t>&
RequestAnalyzer::getNationTransitTimes(const type::Nation& nation,
                                       const type::Timestamp& ticketingDate)
{
  const TransitTimesMap::iterator it = _transitTimes.find(nation);
  if (it == _transitTimes.end())
  {
    TransitTimesMap::mapped_type& transitTimeSet = _transitTimes[nation];
    TransitTimesMap::iterator newIt =
        _transitTimes.insert(TransitTimesMap::value_type(nation, std::set<int32_t>())).first;

    // Fill non-typical values
    ItinGroupingUtil itu(_services);
    itu.getNationTransitTimes(nation, ticketingDate, transitTimeSet);
    if (nation == "US")
      transitTimeSet.insert(4 * 60);
    transitTimeSet.insert(12 * 60);
    transitTimeSet.insert(24 * 60);

    return transitTimeSet;
  }

  return it->second;
}

} // namespace tax
