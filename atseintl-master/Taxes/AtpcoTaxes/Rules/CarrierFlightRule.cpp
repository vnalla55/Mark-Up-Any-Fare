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
#include <boost/format.hpp>

#include "DataModel/Common/CodeIO.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/CarrierFlightService.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/CarrierFlightApplicator.h"
#include "Rules/CarrierFlightRule.h"

namespace tax
{

class CarrierFlight;

CarrierFlightRule::CarrierFlightRule(type::Index const& carrierFlightItemBefore,
                                     type::Index const& carrierFlightItemAfter,
                                     type::TaxPointTag const& taxPointTag,
                                     type::Vendor const& vendor)
  : _carrierFlightItemBefore(carrierFlightItemBefore),
    _carrierFlightItemAfter(carrierFlightItemAfter),
    _taxPointTag(taxPointTag),
    _vendor(vendor)
{
}

CarrierFlightRule::~CarrierFlightRule() {}

CarrierFlightRule::ApplicatorType
CarrierFlightRule::createApplicator(type::Index const& itinIndex,
                                    const Request& request,
                                    Services& services,
                                    RawPayments& /*itinPayments*/) const
{
  Itin const& itin = request.getItinByIndex(itinIndex);
  type::Index const& geoPathRefId = itin.geoPathRefId();
  std::shared_ptr<CarrierFlight const> carrierFlightBefore =
      services.carrierFlightService().getCarrierFlight(_vendor, _carrierFlightItemBefore);
  std::shared_ptr<CarrierFlight const> carrierFlightAfter =
      services.carrierFlightService().getCarrierFlight(_vendor, _carrierFlightItemAfter);

  return ApplicatorType(*this,
                        request.geoPaths()[geoPathRefId],
                        itin.flightUsages(),
                        carrierFlightBefore,
                        carrierFlightAfter);
}

void
CarrierFlightRule::addItemDescription(std::ostringstream& buf, type::Index item, Services& services)
    const
{
  std::shared_ptr<CarrierFlight const> carrierFlight =
      services.carrierFlightService().getCarrierFlight(_vendor, item);
  buf << " ITEM " << item;
  if (carrierFlight != nullptr)
  {
    if (carrierFlight->segments.size() != 0)
    {
      buf << " CONTENT:\n";
      boost::format formatter(" %|=9| %|=9| %|=7| %|=5|\n");
      buf << formatter % "MARKETING" % "OPERATING" % "FLTFROM" % "FLTTO";
      for(CarrierFlightSegment const & cfs : carrierFlight->segments)
      {
        buf << formatter % cfs.marketingCarrier % cfs.operatingCarrier % cfs.flightFrom %
                   cfs.flightTo;
      }
    }
    else
    {
      buf << " IS EMPTY\n";
    }
  }
  else // carrierFlight == 0
  {
    buf << " DOES NOT EXIST\n";
  }
}

std::string
CarrierFlightRule::getDescription(Services& services) const
{
  std::ostringstream buf;
  buf << "FLIGHT BEFORE LOC1 MUST SATISFY RESTRICTIONS FROM CARRIER\n"
      << " FLIGHT ITEM NUMBER " << _carrierFlightItemBefore << " FROM VENDOR " << _vendor
      << " AND FLIGHT AFTER\n"
      << " LOC1 MUST SATISFY RESTRICTIONS FROM CARRIER FLIGHT ITEM\n"
      << " NUMBER " << _carrierFlightItemAfter << " FROM VENDOR " << _vendor << "\n";
  addItemDescription(buf, _carrierFlightItemBefore, services);
  addItemDescription(buf, _carrierFlightItemAfter, services);
  return buf.str();
}

} // namespace tax
