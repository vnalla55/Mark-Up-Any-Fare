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

#include "Common/SafeEnumToString.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/CarrierApplicationService.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/ValidatingCarrierApplicator.h"
#include "Rules/ValidatingCarrierRule.h"
#include "DataModel/Common/CodeIO.h"

namespace tax
{

ValidatingCarrierRule::ValidatingCarrierRule(type::Index const& carrierAppl,
                                             type::Vendor const& vendor)
  : _carrierAppl(carrierAppl), _vendor(vendor)
{
}

ValidatingCarrierRule::~ValidatingCarrierRule() {}

ValidatingCarrierRule::ApplicatorType
ValidatingCarrierRule::createApplicator(type::Index const& itinIndex,
                                        const Request& request,
                                        Services& services,
                                        RawPayments& /*itinPayments*/) const
{
  Itin const& itin = request.getItinByIndex(itinIndex);
  type::Index const& farePathRefId = itin.farePathRefId();
  type::CarrierCode const& validatingCarrier =
      request.farePaths()[farePathRefId].validatingCarrier();
  std::shared_ptr<const CarrierApplication> carrierApplication =
      services.carrierApplicationService().getCarrierApplication(_vendor, _carrierAppl);
  return ApplicatorType(this, validatingCarrier, carrierApplication);
}

std::string
ValidatingCarrierRule::getDescription(Services& services) const
{
  std::ostringstream buf;
  buf << "VALIDATING CARRIER MUST SATISFY RESTRICTIONS FROM CARRIER\n"
      << " APPLICATION ITEM NUMBER " << _carrierAppl << " FROM VENDOR " << _vendor << "\n";

  std::shared_ptr<const CarrierApplication> carrierApplication =
      services.carrierApplicationService().getCarrierApplication(_vendor, _carrierAppl);
  buf << " ITEM " << _carrierAppl;
  if (carrierApplication != nullptr)
  {
    if (carrierApplication->entries.size() != 0)
    {
      buf << " CONTENT:\n";
      boost::format formatter(" %|=7| %|=7|\n");
      buf << formatter % "APPLTAG" % "CARRIER";
      for(CarrierApplicationEntry const & entry : carrierApplication->entries)
      {
        buf << formatter % entry.applind % entry.carrier;
      }
    }
    else
    {
      buf << " IS EMPTY\n";
    }
  }
  else // carrierApplication == 0
  {
    buf << " DOES NOT EXIST\n";
  }
  return buf.str();
}
}
