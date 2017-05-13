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
#include "Rules/BusinessRule.h"

#include <set>
#include <string>

namespace tax
{
class ConnectionsTagsApplicator;
class Request;

class ConnectionsTagsRule : public BusinessRule
{
public:
  typedef ConnectionsTagsApplicator ApplicatorType;
  ConnectionsTagsRule(const std::set<type::ConnectionsTag>& connectionsTags,
                      bool alternateTurnaroundDeterminationLogic,
                      bool caSurfaceException);

  virtual ~ConnectionsTagsRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(const type::Index& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& itinPayments) const;

  const std::set<type::ConnectionsTag>& getConnectionsTagsSet() const
  {
    return _connectionsTagsSet;
  }

  bool alternateTurnaroundDeterminationLogic() const
  {
    return _alternateTurnaroundDeterminationLogic;
  }

  bool caSurfaceException() const
  {
    return _caSurfaceException;
  }

private:
  std::set<type::ConnectionsTag> _connectionsTagsSet;
  bool _alternateTurnaroundDeterminationLogic;
  bool _caSurfaceException;
};

} // namespace tax
