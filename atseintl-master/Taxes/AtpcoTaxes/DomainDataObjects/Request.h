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

#include "DataModel/Common/Types.h"
#include "DomainDataObjects/ChangeFee.h"
#include "DomainDataObjects/DiagnosticCommand.h"
#include "DomainDataObjects/Fare.h"
#include "DomainDataObjects/FarePath.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/OptionalService.h"
#include "DomainDataObjects/OptionalServicePath.h"
#include "DomainDataObjects/Passenger.h"
#include "DomainDataObjects/PointOfSale.h"
#include "DomainDataObjects/ProcessingOptions.h"
#include "DomainDataObjects/TicketingFee.h"
#include "DomainDataObjects/TicketingOptions.h"
#include "DomainDataObjects/XmlCache.h"
#include "DomainDataObjects/YqYr.h"
#include "DomainDataObjects/YqYrPath.h"

#include <boost/noncopyable.hpp>
#include <vector>

namespace tax
{
class TaxData;

class Request : public boost::noncopyable
{
public:
  Request() {}
  ~Request() {}

  std::vector<PointOfSale>& pointsOfSale() { return _pointsOfSale; };
  const std::vector<PointOfSale>& pointsOfSale() const { return _pointsOfSale; };

  std::vector<Geo>& posTaxPoints() { return _posTaxPoints; };
  const std::vector<Geo>& posTaxPoints() const { return _posTaxPoints; };

  std::vector<GeoPath>& geoPaths() { return _geoPaths; };
  const std::vector<GeoPath>& geoPaths() const { return _geoPaths; };

  std::vector<Fare>& fares() { return _fares; };
  const std::vector<Fare>& fares() const { return _fares; }

  std::vector<YqYr>& yqYrs() { return _yqYrs; };
  const std::vector<YqYr>& yqYrs() const { return _yqYrs; }

  std::vector<OptionalService>& optionalServices() { return _optionalServices; };
  const std::vector<OptionalService>& optionalServices() const { return _optionalServices; }

  std::vector<FarePath>& farePaths() { return _farePaths; };
  const std::vector<FarePath>& farePaths() const { return _farePaths; };

  std::vector<YqYrPath>& yqYrPaths() { return _yqYrPaths; };

  const std::vector<YqYrPath>& yqYrPaths() const { return _yqYrPaths; };

  std::vector<OptionalServicePath>& optionalServicePaths() { return _optionalServicePaths; };

  const std::vector<OptionalServicePath>& optionalServicePaths() const
  {
    return _optionalServicePaths;
  };

  std::vector<GeoPathMapping>& geoPathMappings() { return _geoPathMappings; };
  const std::vector<GeoPathMapping>& geoPathMappings() const { return _geoPathMappings; };

  const Itin& getItinByIndex(type::Index index) const { return _allItins.at(index); }

  std::vector<Itin*>& itins() { return _itins; }
  const std::vector<Itin*>& itins() const { return _itins; }

  std::vector<Itin>& allItins() { return _allItins; }
  const std::vector<Itin>& allItins() const { return _allItins; }

  std::vector<Flight>& flights() { return _flights; }
  const std::vector<Flight>& flights() const { return _flights; }

  std::vector<Passenger>& passengers() { return _passengers; }
  const std::vector<Passenger>& passengers() const { return _passengers; }

  std::vector<ChangeFee>& changeFees() { return _changeFees; }
  const std::vector<ChangeFee>& changeFees() const { return _changeFees; }

  DiagnosticCommand& diagnostic() { return _diagnostic; }
  const DiagnosticCommand& diagnostic() const { return _diagnostic; }

  XmlCache& xmlCache() { return _xmlCache; };
  const XmlCache& xmlCache() const { return _xmlCache; };

  ProcessingOptions& processing() { return _procOpts; }
  const ProcessingOptions& processing() const { return _procOpts; }

  TicketingOptions& ticketingOptions() { return _ticketingOptions; }
  const TicketingOptions& ticketingOptions() const { return _ticketingOptions; }

  std::string& buildInfo() { return _buildInfo; }
  const std::string& buildInfo() const { return _buildInfo; }

  std::string& echoToken() { return _echoToken; }
  const std::string& echoToken() const { return _echoToken; }

  GeoPath& prevTicketGeoPath() { return _prevTicketGeoPath; };
  const GeoPath& prevTicketGeoPath() const { return _prevTicketGeoPath; };

  std::ostream& print(std::ostream& out) const;

  using ContainersKey = std::pair<const type::Nation, const type::TaxPointTag>;
  using ContainersValue = std::shared_ptr<const boost::ptr_vector<TaxData>>;
  using ContainersMap = std::map<ContainersKey, ContainersValue>;

  ContainersMap& getContainersMap() { return _containersMap; }

  // this method is just to keep all data alive for as long as the request lives
  std::pair<ContainersValue, bool> setRulesContainers(const type::Nation& nation,
                                                      const type::TaxPointTag& taxPointTag,
                                                      ContainersValue containers);

  const GeoPath& getGeoPath(type::Index itinIndex) const;
  const std::vector<FlightUsage> getFlightUsages(type::Index itinIndex) const;

  std::map<std::string, type::Index>& keyToItinClassMap() { return _keyToItinClassMap; }
  const std::map<std::string, type::Index>& keyToItinClassMap() const { return _keyToItinClassMap; }

  std::map<type::Index, type::Index>& allItinsMap() { return _allItinsMap; }
  const std::map<type::Index, type::Index>& allItinsMap() const { return _allItinsMap; }

private:
  std::vector<PointOfSale> _pointsOfSale;
  std::vector<Geo> _posTaxPoints;
  std::vector<GeoPath> _geoPaths;
  std::vector<FarePath> _farePaths;
  std::vector<YqYrPath> _yqYrPaths;
  std::vector<OptionalServicePath> _optionalServicePaths;
  std::vector<GeoPathMapping> _geoPathMappings;
  std::vector<Itin*> _itins;
  std::vector<Itin> _allItins;
  std::vector<Flight> _flights;
  std::vector<Fare> _fares;
  std::vector<YqYr> _yqYrs;
  std::vector<OptionalService> _optionalServices;
  std::vector<Passenger> _passengers;
  std::vector<ChangeFee> _changeFees;
  ProcessingOptions _procOpts;
  TicketingOptions _ticketingOptions;
  DiagnosticCommand _diagnostic;
  XmlCache _xmlCache;
  std::string _buildInfo;
  std::string _echoToken;
  GeoPath _prevTicketGeoPath;

  std::map<std::string, type::Index> _keyToItinClassMap;
  std::map<type::Index, type::Index> _allItinsMap;

  ContainersMap _containersMap;
};
} // namespace tax

