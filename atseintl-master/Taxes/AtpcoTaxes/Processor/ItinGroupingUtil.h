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
#pragma once

#include "Common/Timestamp.h"
#include "DataModel/Common/Types.h"
#include "ServiceInterfaces/Services.h"

#include <boost/noncopyable.hpp>
#include <map>
#include <set>

namespace tax
{
class CarrierFlightService;
class Flight;
class Itin;
class Request;
class RulesRecordsService;
class ValidatorsGroups;

class ItinGroupingUtil
{
public:
  class DateSegmenter : public boost::noncopyable
  {
    friend class ItinGroupingUtilTest;
  public:
    DateSegmenter(const type::Nation& nation,
                  const type::Timestamp& ticketingDate,
                  Services& services)
      : _nation(nation),
        _ticketingDate(ticketingDate),
        _rulesRecordsService(&services.rulesRecordsService())
    {
      initDateLimits();
    }

    void buildDateSegmentKey(const Itin& itin, std::string& key);

  private:
    DateSegmenter(type::Nation& nation, type::Timestamp& ticketingDate) // for unit test
      : _nation(nation), _ticketingDate(ticketingDate), _rulesRecordsService(0)
    {
    }

    void initDateLimits();
    void fillDateLimits(type::TaxPointTag taxPointTag);

    const type::Nation& _nation;
    const type::Timestamp& _ticketingDate;
    const RulesRecordsService* _rulesRecordsService;

    std::vector<type::Date> _departureDateLimits;
    std::vector<type::Date> _journeyDateLimits;
  };

  class FlightSegmenter : public boost::noncopyable
  {
    friend class ItinGroupingUtilTest;
  public:
    FlightSegmenter(const type::Nation& nation,
                    const type::Timestamp& ticketingDate,
                    Services& services)
      : _nation(nation),
        _ticketingDate(ticketingDate),
        _rulesRecordsService(&services.rulesRecordsService()),
        _carrierFlightService(&services.carrierFlightService())
    {
      initFlightRanges();
    }

    void buildFltSegmentKey(const Itin& itin, std::string& key);

  private:
    FlightSegmenter(const type::Nation& nation, const type::Timestamp& ticketingDate) // for unit test
      : _nation(nation), _ticketingDate(ticketingDate), _rulesRecordsService(0),
        _carrierFlightService(0)
    {
    }

    typedef std::pair<type::FlightNumber, type::FlightNumber> FlightRange;
    typedef std::pair<type::CarrierCode, type::TaxPointTag> FlightRangeKey;
    typedef std::map<FlightRangeKey, std::vector<FlightRange> > FlightRangesByCarrierAndTag;

    void initFlightRanges();
    void fillFlightRanges(type::TaxPointTag taxPointTag);
    void buildKeyPart(FlightRangesByCarrierAndTag& flightRanges,
                      const Flight* flight,
                      type::TaxPointTag taxPointTag,
                      std::string& key);

    const type::Nation& _nation;
    const type::Timestamp& _ticketingDate;
    const RulesRecordsService* _rulesRecordsService;
    const CarrierFlightService* _carrierFlightService;

    FlightRangesByCarrierAndTag _flightRangesBefore;
    FlightRangesByCarrierAndTag _flightRangesAfter;
  };

  class AYPrevalidator : public boost::noncopyable
  {
  public:
    AYPrevalidator(const Request& request,
                   const Itin& itin,
                   type::Index fuId,
                   const type::Timestamp& ticketingDate,
                   Services& services)
      : _request(request), _itin(itin), _fuId(fuId), _ticketingDate(ticketingDate),
        _services(services) {}
    bool doesAYapply();

  private:
    bool checkLoc1Stopover(const ValidatorsGroups& validators);
    bool checkLoc1Zone(const ValidatorsGroups& validators);
    bool checkPointOfSaleZone(const ValidatorsGroups& validators);
    bool checkValidatingCarrier(const ValidatorsGroups& validators);
    bool checkCarrierFlight(const ValidatorsGroups& validators);
    bool checkPassenger(const ValidatorsGroups& validators);
    bool checkSectorDetail(const ValidatorsGroups& validators);

    const Request& _request;
    const Itin& _itin;
    type::Index _fuId;
    const type::Timestamp& _ticketingDate;
    Services& _services;
  };

  ItinGroupingUtil(Services& services)
    : _services(services)
  {
  }

  bool doesAYapply(const Request& request,
                   const Itin& itin,
                   type::Index fuId,
                   const type::Timestamp& ticketingDate);

  void getNationTransitTimes(const type::Nation nation,
                             const type::Timestamp& ticketingDate,
                             std::set<int32_t>& transitTimes);

private:
  void getNationTransitTimesForTag(const type::Nation nation,
                                   const type::Timestamp& ticketingDate,
                                   type::TaxPointTag taxPointTag,
                                   std::set<int32_t>& transitTimes);

  Services& _services;
};

} // namespace tax
