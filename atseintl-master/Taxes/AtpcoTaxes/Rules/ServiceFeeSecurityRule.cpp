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
#include <sstream>
#include <boost/format.hpp>

#include "Common/SafeEnumToString.h"
#include "DataModel/Common/CodeIO.h"
#include "DataModel/Services/ServiceFeeSecurity.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/Services.h"
#include "ServiceInterfaces/ServiceFeeSecurityService.h"
#include "Rules/ServiceFeeSecurityRule.h"
#include "Rules/ServiceFeeSecurityApplicatorFacade.h"

namespace tax
{

std::string
ServiceFeeSecurityRule::getDescription(Services& services) const
{
  std::ostringstream buf;
  buf << "THE SELLER MUST SATISFY RESTRICTIONS FROM SERVICE FEE\n"
      << " SECURITY ITEM NUMBER " << _itemNo << " FROM VENDOR " << _vendor << "\n";

  std::shared_ptr<const ServiceFeeSecurityItems> sfss =
      services.serviceFeeSecurityService().getServiceFeeSecurity(_vendor, _itemNo);
  buf << " ITEM " << _itemNo;

  if (sfss->size() != 0)
  {
    buf << " CONTENT:\n";
    boost::format formatter(" %|=15| %|=10| %|=8| %|=8| %|=8| %|=4|\n");
    buf << formatter % "TRAVELAGENCYIND" % "CXRGDSCODE" % "DUTYCODE" % "LOCATION" % "CODETYPE" %
               "CODE";
    for(const ServiceFeeSecurityItem & sfs : *sfss)
    {
      buf << formatter % sfs.travelAgencyIndicator % sfs.carrierGdsCode % sfs.dutyFunctionCode %
                 sfs.location.toString() % sfs.codeType % sfs.code;
    }
  }
  else
  {
    buf << " IS EMPTY\n";
  }
  return buf.str();
}

ServiceFeeSecurityRule::ApplicatorType
ServiceFeeSecurityRule::createApplicator(const type::Index& itinIndex,
                                         const Request& request,
                                         Services& services,
                                         RawPayments& /*itinPayments*/) const
{
  std::shared_ptr<const ServiceFeeSecurityItems> sfs =
      services.serviceFeeSecurityService().getServiceFeeSecurity(_vendor, _itemNo);
  Itin const& itin = request.getItinByIndex(itinIndex);

  return ApplicatorType(
      this, request.pointsOfSale()[itin.pointOfSaleRefId()], services.locService(), sfs);
}

} // namespace tax
