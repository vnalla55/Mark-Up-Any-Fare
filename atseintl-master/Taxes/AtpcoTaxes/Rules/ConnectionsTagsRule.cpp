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
#include "Rules/ConnectionsTagsRule.h"

#include "Rules/ConnectionsTagsApplicator.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/Services.h"

namespace tax
{

ConnectionsTagsRule::ConnectionsTagsRule(const std::set<type::ConnectionsTag>& connectionsTags,
                                         bool alternateTurnaroundDeterminationLogic,
                                         bool caSurfaceException)
  : _connectionsTagsSet(connectionsTags),
    _alternateTurnaroundDeterminationLogic(alternateTurnaroundDeterminationLogic),
    _caSurfaceException(caSurfaceException)
{
}

ConnectionsTagsRule::~ConnectionsTagsRule() {}

ConnectionsTagsRule::ApplicatorType
ConnectionsTagsRule::createApplicator(const type::Index& itinIndex,
                                      const Request& request,
                                      Services& services,
                                      RawPayments& /*itinPayments*/) const
{
  const Itin& itin = request.getItinByIndex(itinIndex);

  return ApplicatorType(*this,
                        itin,
                        type::Timestamp(itin.travelOriginDate(), type::Time(0, 0)),
                        services.mileageService());
}

std::string
ConnectionsTagsRule::getDescription(Services&) const
{
  std::string description = " ConnectionsTagsApplicator\n";

  return description;
}

} // namespace tax
