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
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/PassengerTypesService.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/PassengerTypeCodeRule.h"
#include "Rules/PassengerTypeCodeApplicatorFacade.h"
#include "Rules/RequestLogicError.h"

namespace tax
{

std::string
PassengerTypeCodeRule::getDescription(Services& services) const
{
  std::ostringstream buf;
  buf << "PASSENGER TYPE CODE MUST SATISFY RESTRICTIONS FROM\n"
      << " PASSENGER TYPE ITEM NUMBER " << _itemNo << " FROM VENDOR " << _vendor << "\n";

  std::shared_ptr<const PassengerTypeCodeItems> ptcs =
      services.passengerTypesService().getPassengerTypeCode(_vendor, _itemNo);
  buf << " ITEM " << _itemNo;

  if (ptcs && ptcs->size() != 0)
  {
    buf << " CONTENT:\n";
    boost::format formatter(" %|=7| %|=4| %|=6| %|=6| %|=6| %|=7| %|=5| %|=8|\n");
    buf << formatter % "APPLTAG" % "TYPE" % "MINAGE" % "MAXAGE" % "STATUS" % "LOCTYPE" % "LOC" %
               "MATCHIND";
    for(const PassengerTypeCodeItem & ptc : *ptcs)
    {
      buf << formatter % ptc.applTag % ptc.passengerType % ptc.minimumAge % ptc.maximumAge %
                 ptc.statusTag % ptc.location.type() % ptc.location.code() % ptc.matchIndicator;
    }
  }
  else
  {
    buf << " IS EMPTY\n";
  }
  return buf.str();
}

PassengerTypeCodeRule::ApplicatorType
PassengerTypeCodeRule::createApplicator(const type::Index& itinIndex,
                                        const Request& request,
                                        Services& services,
                                        RawPayments& /*itinPayments*/) const
{
  std::shared_ptr<const PassengerTypeCodeItems> ptc =
      services.passengerTypesService().getPassengerTypeCode(_vendor, _itemNo);

  const Itin& itin = request.getItinByIndex(itinIndex);

  type::Date referenceDate = (_taxRemittanceId == type::TaxRemittanceId::Sale)
                                 ? request.ticketingOptions().ticketingDate()
                                 : itin.travelOriginDate();

  return ApplicatorType(*this,
                        itin,
                        referenceDate,
                        services.passengerMapper(),
                        ptc,
                        services.locService(),
                        request.pointsOfSale()[itin.pointOfSaleRefId()].loc());
}

} // namespace tax
