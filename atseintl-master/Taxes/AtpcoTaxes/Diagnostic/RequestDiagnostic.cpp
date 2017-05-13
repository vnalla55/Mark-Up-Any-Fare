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

#include <iomanip>
#include <sstream>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "Common/MoneyUtil.h"
#include "Common/OCUtil.h"
#include "Common/SafeEnumToString.h"
#include "DataModel/Common/CodeIO.h"
#include "DataModel/Common/Types.h"
#include "Diagnostic/RequestDiagnostic.h"
#include "DomainDataObjects/Request.h"

namespace tax
{
namespace
{
void
printGeo(Geo const& geo,
         const FlightUsage& flightUsage,
         boost::format& formatter,
         CopyableStream& result)
{
  std::string typeName;
  type::Time time;
  type::Date date;

  if (geo.loc().tag() == type::TaxPointTag::Departure)
  {
    typeName = "DEPARTURE";
    time = flightUsage.departureTime();
    date = flightUsage.departureDate();
  }
  else
  {
    typeName = "ARRIVAL";
    time = flightUsage.arrivalTime();
    date = flightUsage.arrivalDate();
  };

  bool specifiedDate = (flightUsage.openSegmentIndicator() != type::OpenSegmentIndicator::Open);
  bool specifiedTime = (flightUsage.openSegmentIndicator() == type::OpenSegmentIndicator::Fixed);

  std::ostringstream dateStream;
  dateStream << date;

  const char hidden = ((geo.unticketedTransfer() == type::UnticketedTransfer::Yes) ? 'X' : ' ');
  const std::string OPEN = "OPEN";

  result << formatter % (geo.id() + 1) % geo.loc().code() % typeName % geo.loc().nation() % hidden %
                (specifiedDate ? dateStream.str() : OPEN) % (specifiedTime ? time.str() : OPEN);
}

} // anonymous namespace

const uint32_t RequestDiagnostic::NUMBER = 830;

RequestDiagnostic::RequestDiagnostic(const Request& taxRequest,
                                     const boost::ptr_vector<Parameter>& parameters)
  : _taxRequest(taxRequest), _parameters(parameters)
{
}

RequestDiagnostic::~RequestDiagnostic(void)
{
}

void
RequestDiagnostic::runAll()
{
  _helpPrinter.print(*this);
  _taxPointsPrinter.print(*this);
  _farePathsPrinter.print(*this);
  _optionalServicesPrinter.print(*this);
  _baggagePrinter.print(*this);
  _requestPrinter.print(*this);
}

void
RequestDiagnostic::applyParameters()
{
  if (_parameters.empty())
  {
    _helpPrinter.enable();
  }

  for (Parameter const& parameter : _parameters)
  {
    if (parameter.name() == "HE" || parameter.name() == "HELP")
    {
      _helpPrinter.enable();
    }
    else if (parameter.name() == "TP")
    {
      taxPointsPrinter().enable();
    }
    else if (parameter.name() == "FP")
    {
      farePathsPrinter().enable();
    }
    else if (parameter.name() == "OC")
    {
      optionalServicesPrinter().enable();
    }
    else if (parameter.name() == "BG")
    {
      baggagePrinter().enable();
    }
    else if (parameter.name() == "RQ")
    {
      requestPrinter().enable();
    }
  }
}

void
RequestDiagnostic::printTaxPoints()
{
  printHeaderLong("TAX POINT ANALYSIS");

  for (uint32_t i = 0; i < _taxRequest.pointsOfSale().size(); i++)
  {
    const type::AirportCode loc = _taxRequest.pointsOfSale()[i].loc();
    const type::Nation nationCode = _taxRequest.posTaxPoints()[i].loc().nation();

    if (!loc.empty())
    {
      printHeaderShort("POINT OF SALE INFO");

      _result << "\n";

      boost::format formatter("    %|=3|  %|=6|  %|=10|\n");
      _result << formatter % "POS" % "NATION" % "DATE";

      _result << formatter % loc % nationCode % _taxRequest.ticketingOptions().ticketingDate();
      _result << "\n";
    }
  }

  printHeaderShort("ITINERARY INFO");

  std::vector<GeoPath> const& geoPaths = _taxRequest.geoPaths();
  for (uint32_t geoPathRefId = 0; geoPathRefId < geoPaths.size(); geoPathRefId++)
  {
    printHeaderShort("ITINERARY: " + boost::lexical_cast<std::string>(geoPathRefId + 1));
    _result << "\n";

    const GeoPath& geoPath = geoPaths[geoPathRefId];
    const Itin& itin = _taxRequest.getItinByIndex(geoPathRefId);

    boost::format formatter("%|3| %|=4|  %|=9| %|=6| %|=6| %|=10| %|=5|\n");
    _result << formatter % "" % "TXPT" % "TXPT TYPE" % "NATION" % "HIDDEN" % "DATE" % "TIME";

    for (uint32_t geoIndex = 0; geoIndex < geoPath.geos().size(); geoIndex++)
    {
      printGeo(geoPath.geos()[geoIndex], itin.flightUsages().at(geoIndex / 2), formatter, _result);
    }
    _result << "\n";
  }

  printLine('-');
}

void
RequestDiagnostic::printFarePaths()
{
  printHeaderLong("FARE PATH ANALYSIS");

  for (Itin const& itin : _taxRequest.allItins())
  {
    printHeaderShort(str(boost::format("ITIN: %|-3|") % (itin.id() + 1)));
    _result << "\n";

    FarePath const& farePath = _taxRequest.farePaths()[itin.farePathRefId()];

    _result << "VALIDATING CARRIER: " << farePath.validatingCarrier() << "\n";
    _result << "TOTAL AMOUNT: " << amountToDouble(farePath.totalAmount()) << "\n";
    _result << "\n";

    assert (itin.geoPathMapping());
    const std::vector<Mapping>& geoPathMappings = itin.geoPathMapping()->mappings();

    GeoPath const& geoPath = _taxRequest.geoPaths()[itin.geoPathRefId()];

    boost::format formatter("%|3| %|=7| %|=4| %|=9| %|=14| %|=6|\n");
    _result << formatter % "" % "BASIS" % "TYPE" % "ONEWAY/RT" % "DIRECTIONALITY" % "AMOUNT";
    _result << "\n";

    for (uint32_t j = 0; j < farePath.fareUsages().size(); j++)
    {
      if (geoPathMappings[j].maps().size() > 0)
      {
        std::vector<Map> const& maps = geoPathMappings[j].maps();
        Geo const& geo0 = geoPath.geos()[maps[0].index()];

        _result << boost::format("%|3|") % (j + 1) << "  FAREMARKET: " << geo0.loc().code();

        for (uint32_t k = 1; k < maps.size(); k++)
        {
          Geo const& geo = geoPath.geos()[maps[k].index()];
          _result << "-" << geo.loc().code();
        }
        _result << "\n";
      }

      FareUsage const& fareUsage = farePath.fareUsages()[j];

      Fare const& fare = _taxRequest.fares()[fareUsage.index()];
      _result << formatter % "" % fare.basis() % fare.type() % fare.oneWayRoundTrip() %
                     fare.directionality() % amountToDouble(fare.amount());

      _result << "\n";
    }
  }

  printLine('-');
}

void
RequestDiagnostic::printOptionalServices()
{
  printSelectedOS(std::string("OPTIONAL SERVICES ANALYSIS"), type::ProcessingGroup::OC);
}

void
RequestDiagnostic::printBaggage()
{
  printSelectedOS(std::string("BAGGAGE ANALYSIS"), type::ProcessingGroup::Baggage);
}

void
RequestDiagnostic::printRequest()
{
  printHeaderLong("REQUEST ANALYSIS");
  std::ostringstream output;
  _taxRequest.print(output);
  _result << output.str() << "\n";
}

void
RequestDiagnostic::printSelectedOS(const std::string& header,
                                   const type::ProcessingGroup& processingGroup)
{
  printHeaderLong(header);

  for (Itin const& itin : _taxRequest.allItins())
  {
    printHeaderShort(str(boost::format("ITIN: %|-3|") % (itin.id() + 1)));
    _result << "\n";

    if (!itin.optionalServicePathRefId().has_value())
      continue;

    OptionalServicePath const& optionalServicePath =
        _taxRequest.optionalServicePaths()[itin.optionalServicePathRefId().value()];

    if (!itin.optionalServicePathGeoPathMappingRefId().has_value()) // defensive
      continue;

    GeoPathMapping const& geoPathMapping =
        _taxRequest.geoPathMappings()[itin.optionalServicePathGeoPathMappingRefId().value()];

    GeoPath const& geoPath = _taxRequest.geoPaths()[itin.geoPathRefId()];

    boost::format formatter("%|3| %|=4| %|=14| %|=8| %|=11| %|=7| %|=6|\n");
    _result << formatter % "" % "TYPE" % "SVCSUBTYPECODE" % "SVCGROUP" % "SVCSUBGROUP" % "CARRIER" %
                   "AMOUNT";
    _result << "\n";

    uint32_t index = 0;
    for (uint32_t j = 0; j < optionalServicePath.optionalServiceUsages().size(); j++)
    {
      OptionalServiceUsage const& optionalServiceUsage =
          optionalServicePath.optionalServiceUsages()[j];
      OptionalService const& optionalService =
          _taxRequest.optionalServices()[optionalServiceUsage.index()];

      if (!OCUtil::isOCValidForGroup(optionalService.type(), processingGroup))
      {
        continue;
      }

      if (geoPathMapping.mappings()[j].maps().size() > 0)
      {
        std::vector<Map> const& maps = geoPathMapping.mappings()[j].maps();
        Geo const& geo0 = geoPath.geos()[maps[0].index()];

        _result << boost::format("%|3|") % (++index) << "  MARKET: " << geo0.loc().code();

        for (uint32_t k = 1; k < maps.size(); ++k)
        {
          Geo const& geo = geoPath.geos()[maps[k].index()];
          _result << "-" << geo.loc().code();
        }
        _result << "\n";
      }

      _result << formatter % "" % optionalService.type() % optionalService.subCode() %
                     optionalService.serviceGroup() % optionalService.serviceSubGroup() %
                     optionalService.ownerCarrier() % amountToDouble(optionalService.amount());
      _result << "\n";
    }
  }

  printLine('-');
}

void
RequestDiagnostic::printHelp()
{
  printHeaderLong("HELP");

  _result << "TP - TAX POINT ANALYSIS\n"
          << "FP - FARE PATH ANALYSIS\n"
          << "OC - OPTIONAL SERVICE ANALYSIS\n"
          << "BG - BAGGAGE ANALYSIS\n"
          << "RQ - REQUEST ANALYSIS\n"
          << "HELP - HELP INFO\n";
}

void
RequestDiagnostic::printHeader()
{
  printLine('*');
  printHeaderShort("DIAGNOSTIC " + boost::lexical_cast<std::string>(NUMBER) + " - REQUEST INFO");
  printLine('*');
}
}
