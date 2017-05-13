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
#include <vector>

#include "DataModel/RequestResponse/InputItin.h"
#include "DataModel/RequestResponse/InputFlightPath.h"
#include "DataModel/RequestResponse/InputFlightUsage.h"
#include "DomainDataObjects/Itin.h"
#include "Factories/ChangeFeeFactory.h"
#include "Factories/FactoryUtils.h"
#include "Factories/FlightUsageFactory.h"
#include "Factories/ItinFactory.h"

namespace tax
{
Itin
ItinFactory::createFromInput(const InputItin& inputItin,
                             const InputFlightPath& flightPath,
                             std::vector<TicketingFee>& ticketingFees)
{
  Itin result;
  result.id() = inputItin._id;
  result.geoPathRefId() = inputItin._geoPathRefId;
  result.farePathRefId() = inputItin._farePathRefId;
  result.yqYrPathRefId() = inputItin._yqYrPathRefId;
  result.optionalServicePathRefId() = inputItin._optionalServicePathRefId;
  result.passengerRefId() = inputItin._passengerRefId;
  result.pointOfSaleRefId() = inputItin._pointOfSaleRefId;
  result.farePathGeoPathMappingRefId() = inputItin._farePathGeoPathMappingRefId;
  result.yqYrPathGeoPathMappingRefId() = inputItin._yqYrPathGeoPathMappingRefId;
  result.optionalServicePathGeoPathMappingRefId() =
      inputItin._optionalServicePathGeoPathMappingRefId;
  result.travelOriginDate() = type::Date(inputItin._travelOriginDate);
  result.label() = inputItin._label;

  result.geoPath() = nullptr;
  result.farePath() = nullptr;
  result.yqYrPath() = nullptr;
  result.optionalServicePath() = nullptr;
  result.passenger() = nullptr;
  result.pointOfSale() = nullptr;
  result.geoPathMapping() = nullptr;

  create<FlightUsageFactory>(flightPath._flightUsages, result.flightUsages());

  result.changeFeeRefId() = inputItin._changeFeeRefId;
  result.ticketingFees().swap(ticketingFees);

  return result;
}

} // namespace tax
