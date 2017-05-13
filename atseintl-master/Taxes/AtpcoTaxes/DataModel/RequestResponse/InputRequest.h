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

#include "DataModel/RequestResponse/InputChangeFee.h"
#include "DataModel/RequestResponse/InputDiagnosticCommand.h"
#include "DataModel/RequestResponse/InputFare.h"
#include "DataModel/RequestResponse/InputFarePath.h"
#include "DataModel/RequestResponse/InputFlight.h"
#include "DataModel/RequestResponse/InputFlightPath.h"
#include "DataModel/RequestResponse/InputGeoPath.h"
#include "DataModel/RequestResponse/InputGeoPathMapping.h"
#include "DataModel/RequestResponse/InputItin.h"
#include "DataModel/RequestResponse/InputOptionalService.h"
#include "DataModel/RequestResponse/InputOptionalServicePath.h"
#include "DataModel/RequestResponse/InputPassenger.h"
#include "DataModel/RequestResponse/InputPointOfSale.h"
#include "DataModel/RequestResponse/InputProcessingOptions.h"
#include "DataModel/RequestResponse/InputTicketingFee.h"
#include "DataModel/RequestResponse/InputTicketingFeePath.h"
#include "DataModel/RequestResponse/InputTicketingFeeUsage.h"
#include "DataModel/RequestResponse/InputTicketingOptions.h"
#include "DataModel/RequestResponse/InputYqYr.h"
#include "DataModel/RequestResponse/InputYqYrPath.h"
#include "DataModel/Common/Types.h"

namespace tax
{
class InputRequest
{
public:
  typedef boost::ptr_vector<InputGeoPathMapping> InputGeoPathMappings;
  typedef boost::ptr_vector<InputYqYr> InputYqYrs;
  typedef boost::ptr_vector<InputChangeFee> InputChangeFees;

  boost::ptr_vector<InputPointOfSale>& pointsOfSale() { return _pointsOfSale; };
  const boost::ptr_vector<InputPointOfSale>& pointsOfSale() const { return _pointsOfSale; };

  boost::ptr_vector<InputGeo>& posTaxPoints() { return _posTaxPoints; };
  const boost::ptr_vector<InputGeo>& posTaxPoints() const { return _posTaxPoints; };

  boost::ptr_vector<InputGeoPath>& geoPaths() { return _geoPaths; };
  const boost::ptr_vector<InputGeoPath>& geoPaths() const { return _geoPaths; };

  InputPrevTicketGeoPath& prevTicketGeoPath() { return _prevTicketGeoPath; };
  const InputPrevTicketGeoPath& prevTicketGeoPath() const { return _prevTicketGeoPath; };

  boost::ptr_vector<InputFare>& fares() { return _fares; };
  const boost::ptr_vector<InputFare>& fares() const { return _fares; }

  InputYqYrs& yqYrs() { return _yqYrs; };
  const InputYqYrs& yqYrs() const { return _yqYrs; }

  boost::ptr_vector<InputOptionalService>& optionalServices() { return _optionalServices; };
  const boost::ptr_vector<InputOptionalService>& optionalServices() const
  {
    return _optionalServices;
  }

  boost::ptr_vector<InputFarePath>& farePaths() { return _farePaths; };
  const boost::ptr_vector<InputFarePath>& farePaths() const { return _farePaths; };

  boost::ptr_vector<InputFlightPath>& flightPaths() { return _flightPaths; };
  const boost::ptr_vector<InputFlightPath>& flightPaths() const { return _flightPaths; };

  boost::ptr_vector<InputYqYrPath>& yqYrPaths() { return _yqYrPaths; };

  const boost::ptr_vector<InputYqYrPath>& yqYrPaths() const { return _yqYrPaths; };

  boost::ptr_vector<InputOptionalServicePath>& optionalServicePaths()
  {
    return _optionalServicePaths;
  };

  const boost::ptr_vector<InputOptionalServicePath>& optionalServicePaths() const
  {
    return _optionalServicePaths;
  };

  InputGeoPathMappings& geoPathMappings() { return _geoPathMappings; };
  const InputGeoPathMappings& geoPathMappings() const { return _geoPathMappings; };

  const InputItin& getItinByIndex(type::Index index) const { return _itins.at(index); }

  boost::ptr_vector<InputItin>& itins() { return _itins; }
  const boost::ptr_vector<InputItin>& itins() const { return _itins; }

  boost::ptr_vector<InputFlight>& flights() { return _flights; }
  const boost::ptr_vector<InputFlight>& flights() const { return _flights; }

  boost::ptr_vector<InputPassenger>& passengers() { return _passengers; }
  const boost::ptr_vector<InputPassenger>& passengers() const { return _passengers; }

  InputChangeFees& changeFees() { return _changeFees; }
  const InputChangeFees& changeFees() const { return _changeFees; }

  InputDiagnosticCommand& diagnostic() { return _diagnostic; }
  const InputDiagnosticCommand& diagnostic() const { return _diagnostic; }

  InputProcessingOptions& processing() { return _procOpts; }
  const InputProcessingOptions& processing() const { return _procOpts; }

  InputTicketingOptions& ticketingOptions() { return _ticketingOptions; }
  const InputTicketingOptions& ticketingOptions() const { return _ticketingOptions; }

  boost::ptr_vector<InputTicketingFee>& ticketingFees() { return _ticketingFees; }
  const boost::ptr_vector<InputTicketingFee>& ticketingFees() const { return _ticketingFees; }

  boost::ptr_vector<InputTicketingFeePath>& ticketingFeePaths() { return _ticketingFeePaths; }
  const boost::ptr_vector<InputTicketingFeePath>& ticketingFeePaths() const { return _ticketingFeePaths; }

  std::string& echoToken() { return _echoToken; }
  const std::string& echoToken() const { return _echoToken; }

  std::string& buildInfo() { return _buildInfo; }
  const std::string& buildInfo() const { return _buildInfo; }

  std::ostream& print(std::ostream& out) const;

  bool isTicketingFeeInPath(type::Index ticketingFeeIndex,
                            type::Index ticketingFeePathIndex) const
  {
    for (const InputTicketingFeePath& path : ticketingFeePaths())
    {
      if (path.index() == ticketingFeePathIndex)
      {
        for (const InputTicketingFeeUsage& usage : path.ticketingFeeUsages())
        {
          if (usage.index() == ticketingFeeIndex)
            return true;
        }
        return false;
      }
    }
    return false;
  }

private:
  boost::ptr_vector<InputPointOfSale> _pointsOfSale;
  boost::ptr_vector<InputGeo> _posTaxPoints;
  boost::ptr_vector<InputGeoPath> _geoPaths;
  boost::ptr_vector<InputFarePath> _farePaths;
  boost::ptr_vector<InputYqYrPath> _yqYrPaths;
  boost::ptr_vector<InputOptionalServicePath> _optionalServicePaths;
  boost::ptr_vector<InputFlightPath> _flightPaths;
  InputGeoPathMappings _geoPathMappings;
  boost::ptr_vector<InputItin> _itins;
  boost::ptr_vector<InputFlight> _flights;
  boost::ptr_vector<InputFare> _fares;
  InputYqYrs _yqYrs;
  boost::ptr_vector<InputOptionalService> _optionalServices;
  boost::ptr_vector<InputPassenger> _passengers;
  InputChangeFees _changeFees;
  boost::ptr_vector<InputTicketingFee> _ticketingFees;
  boost::ptr_vector<InputTicketingFeePath> _ticketingFeePaths;
  InputProcessingOptions _procOpts;
  InputTicketingOptions _ticketingOptions;
  InputDiagnosticCommand _diagnostic;
  std::string _echoToken;
  std::string _buildInfo;
  InputPrevTicketGeoPath _prevTicketGeoPath;
};
}
