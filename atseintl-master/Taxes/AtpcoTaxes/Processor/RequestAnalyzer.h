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
#pragma once

#include "Common/Timestamp.h"
#include "DataModel/Common/Types.h"
#include "Processor/ItinGroupingUtil.h"

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/unordered_map.hpp>
#include <map>
#include <set>
#include <string>

namespace tax
{
class Request;
class Services;
class Itin;

class RequestAnalyzer : public boost::noncopyable
{
  typedef boost::ptr_map<type::Nation, ItinGroupingUtil::DateSegmenter> DateSegmenterMap;
  typedef boost::ptr_map<type::Nation, ItinGroupingUtil::FlightSegmenter> FlightSegmenterMap;
  typedef std::map<type::Nation, std::set<int32_t> > TransitTimesMap;
  typedef boost::unordered_map<std::string, bool> AYApplicationCache;
public:
  RequestAnalyzer(Request& request, Services& services);

  void analyze();
  Request& request() { return _request; }
  Services& services() { return _services; }

private:
  void verifyFarePaths();
  void verifyYqYrPaths();
  void verifyItins();
  void verifyGeoPaths();
  void sumMarkupsForFares();
  bool markupsInFarePathsExist();
  void groupItins();
  std::string generateIntlKey(const Itin& itin, const type::Timestamp& ticketingDate);
  std::string generateUSCAKey(const Itin& itin, const type::Timestamp& ticketingDate);
  void buildDateDependantKeyPart(const Itin& itin,
                                 std::set<type::Nation>& fltNoDepTaxNationsFound,
                                 const type::Timestamp& ticketingDate,
                                 std::string& key);
  void buildFlightRangeKeyPart(const Itin& itin,
                               std::set<type::Nation>& fltNoDepTaxNationsFound,
                               const type::Timestamp& ticketingDate,
                               std::string& key);
  void buildStopoverKeyPart(const Itin& itin,
                            const type::Timestamp& ticketingDate,
                            std::string& key);
  void buildSameDayKeyPart(const Itin& itin, std::string& key);
  void buildFareAndYqYrKeyPart(const Itin& itin, std::string& key);

  char getTransitTimeChar(const type::Nation& nation,
                          const type::Timestamp& ticketingDate,
                          type::Timestamp& arrivalDate,
                          type::Timestamp& departureDate);
  std::set<int32_t>& getNationTransitTimes(const type::Nation& nation,
                                           const type::Timestamp& ticketingDate);

  Services& _services;
  Request& _request;

  DateSegmenterMap _dateSegmenters;
  FlightSegmenterMap _flightSegmenters;
  TransitTimesMap _transitTimes;
  AYApplicationCache _ayCache;
  bool _useFlightRanges;
  std::vector<type::Nation> _travelDateDependantTaxNations;
  std::vector<type::Nation> _flightNoDependantTaxNations;
  std::vector<type::Nation> _sameDayDepartureTaxNations;
};

} // namespace tax
