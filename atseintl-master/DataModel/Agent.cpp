//-------------------------------------------------------------------
//
//  File:        Agent.cpp
//  Created:     March 10, 2004
//  Authors:
//
//  Description: Ticketing agent
//
//  Updates:
//          03/10/04 - VN - file created.
//          04/05/04 - Mike Carroll - Initializers for new members
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "DataModel/Agent.h"

#include "Common/TrxUtil.h"
#include "DBAccess/Customer.h"

namespace tse
{

const bool
Agent::abacusUser() const
{
  return (TrxUtil::isAbacusEnabled() && (_agentTJR != nullptr) && (_agentTJR->crsCarrier() == "1B") &&
          (_agentTJR->hostName() == "ABAC"));
}

// Returns if the Agent is a CWT agent located in France.
// Some CWT agents located in France (like F8RA) are excluded
const bool
Agent::cwtUser() const
{
  return ((_agentTJR != nullptr) && (_agentTJR->ssgGroupNo() == CWT_GROUP_NUMBER));
}

const bool
Agent::axessUser() const
{
  return ((_agentTJR != nullptr) && (_agentTJR->crsCarrier() == "1J") &&
          (_agentTJR->hostName() == "AXES"));
}

TJRGroup
Agent::tjrGroup()
{
  if (_agentTJR)
    return _agentTJR->ssgGroupNo();
  return 0;
}

const bool
Agent::sabre1SUser() const
{
  if (!abacusUser() && !axessUser() && !infiniUser())
    return true;

  return false;
}

const bool
Agent::infiniUser() const
{
  return ((_agentTJR != nullptr) && (_agentTJR->crsCarrier() == "1F") &&
          (_agentTJR->hostName() == "INFI"));
}

bool
Agent::isArcUser() const
{
  return _agentTJR && 'Y' == _agentTJR->pricingApplTag2();
}

bool
Agent::isMultiSettlementPlanUser() const
{
  return agentTJR() && agentTJR()->isMultiSettlementPlanUser();
}

void
Agent::getMultiSettlementPlanTypes(std::vector<SettlementPlanType>& settlementPlanTypes)
{
  static const size_t SETTLEMENT_PLAN_SIZE = 3;
  if ( agentTJR() && agentTJR()->settlementPlans().size() >= SETTLEMENT_PLAN_SIZE )
  {
    for ( size_t i = 0; i < agentTJR()->settlementPlans().size(); i += SETTLEMENT_PLAN_SIZE )
    {
      const std::string settlementPlanType = agentTJR()->settlementPlans().substr( i, SETTLEMENT_PLAN_SIZE );
      if ( settlementPlanType.size() == SETTLEMENT_PLAN_SIZE )
        settlementPlanTypes.push_back( settlementPlanType );
    }
  }
}

} // tse namespace
