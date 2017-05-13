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
#include "DataModel/RequestResponse/InputRequest.h"
#include "DataModel/RequestResponse/InputChangeFee.h"
#include "DomainDataObjects/Request.h"
#include "Factories/ChangeFeeFactory.h"
#include "Factories/DiagnosticCommandFactory.h"
#include "Factories/FactoryUtils.h"
#include "Factories/FareFactory.h"
#include "Factories/FarePathFactory.h"
#include "Factories/FlightFactory.h"
#include "Factories/GeoFactory.h"
#include "Factories/GeoPathFactory.h"
#include "Factories/GeoPathMappingFactory.h"
#include "Factories/ItinFactory.h"
#include "Factories/OptionalServiceFactory.h"
#include "Factories/OptionalServicePathFactory.h"
#include "Factories/PassengerFactory.h"
#include "Factories/PointOfSaleFactory.h"
#include "Factories/ProcessingOptionsFactory.h"
#include "Factories/RequestFactory.h"
#include "Factories/TicketingFeeFactory.h"
#include "Factories/TicketingOptionsFactory.h"
#include "Factories/YqYrFactory.h"
#include "Factories/YqYrPathFactory.h"

#include <cassert>

namespace tax
{

namespace
{

std::vector<TicketingFee>
getTicketingFees(const InputRequest& inputRequest, CompactOptional<type::Index> ticketingFeeRefId)
{
  std::vector<TicketingFee> result;

  if (ticketingFeeRefId.has_value())
  {
    for (const InputTicketingFee& ticketingFee : inputRequest.ticketingFees())
    {
      if (inputRequest.isTicketingFeeInPath(ticketingFee._id, ticketingFeeRefId.value()))
        result.push_back(TicketingFeeFactory::createFromInput(ticketingFee));
    }
  }

  return result;
}

} //namespace

RequestFactory::RequestFactory()
{
}

RequestFactory::~RequestFactory()
{
}

void
RequestFactory::createFromInput(const InputRequest& inputRequest, Request& result)
{
  create<PointOfSaleFactory>(inputRequest.pointsOfSale(), result.pointsOfSale());
  create<GeoFactory>(inputRequest.posTaxPoints(), result.posTaxPoints());

  result.processing() = ProcessingOptionsFactory::createFromInput(inputRequest.processing());
  result.ticketingOptions() =
      TicketingOptionsFactory::createFromInput(inputRequest.ticketingOptions());
  result.diagnostic() = DiagnosticCommandFactory::createFromInput(inputRequest.diagnostic());
  result.buildInfo() = inputRequest.buildInfo();
  result.echoToken() = inputRequest.echoToken();

  create<GeoPathFactory>(inputRequest.geoPaths(), result.geoPaths());
  result.prevTicketGeoPath() = GeoPathFactory::createFromInput(inputRequest.prevTicketGeoPath());

  create<FarePathFactory>(inputRequest.farePaths(), result.farePaths());
  create<YqYrPathFactory>(inputRequest.yqYrPaths(), result.yqYrPaths());
  create<OptionalServicePathFactory>(inputRequest.optionalServicePaths(), result.optionalServicePaths());
  create<GeoPathMappingFactory>(inputRequest.geoPathMappings(), result.geoPathMappings());
  create<ChangeFeeFactory>(inputRequest.changeFees(), result.changeFees());

  assert(result.itins().empty());
  result.itins().reserve(inputRequest.itins().size());
  for (const InputItin& inputItin : inputRequest.itins())
  {
    const InputFlightPath& flightPath =
        get<InputFlightPath>(inputItin._flightPathRefId, "FlightPath", inputRequest.flightPaths());

    std::vector<TicketingFee> ticketingFees =
        getTicketingFees(inputRequest, inputItin._ticketingFeeRefId);

    result.allItins().push_back(
        ItinFactory::createFromInput(inputItin, flightPath, ticketingFees));
  }

  create<FlightFactory>(inputRequest.flights(), result.flights());
  create<FareFactory>(inputRequest.fares(), result.fares());
  create<YqYrFactory>(inputRequest.yqYrs(), result.yqYrs());
  create<OptionalServiceFactory>(inputRequest.optionalServices(), result.optionalServices());
  create<PassengerFactory>(inputRequest.passengers(), result.passengers());
}

} // namespace tax
