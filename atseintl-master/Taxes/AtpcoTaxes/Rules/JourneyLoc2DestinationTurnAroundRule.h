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

#include "Common/LocZone.h"
#include "DataModel/Common/Types.h"
#include "Rules/BusinessRule.h"

namespace tax
{

class JourneyLoc2DestinationTurnAroundApplicator;
class Request;
class Services;

class JourneyLoc2DestinationTurnAroundRule : public BusinessRule
{
public:
  typedef JourneyLoc2DestinationTurnAroundApplicator ApplicatorType;
  JourneyLoc2DestinationTurnAroundRule(const type::JrnyInd& jrnyInd,
                                       const LocZone& zone,
                                       const type::Vendor& vendor,
                                       bool alternateTurnaroundDeterminationLogic);
  virtual ~JourneyLoc2DestinationTurnAroundRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(const type::Index& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  const type::JrnyInd& getJrnyInd() const { return _jrnyInd; }
  const LocZone& getLocZone() const { return _locZone; }
  const type::Vendor& getVendor() const { return _vendor; }

  bool alternateTurnaroundDeterminationLogic() const
  {
    return _alternateTurnaroundDeterminationLogic;
  }

private:
  type::JrnyInd _jrnyInd;
  LocZone _locZone;
  type::Vendor _vendor;
  bool _alternateTurnaroundDeterminationLogic;
};
}
