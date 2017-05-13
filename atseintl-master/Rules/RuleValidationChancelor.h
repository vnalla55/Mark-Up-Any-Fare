//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/Assert.h"
#include "DataModel/PaxTypeFare.h"
#include "Rules/RuleControllerDataAccess.h"
#include "Rules/RuleValidationContext.h"
#include "Rules/RuleValidationMonitor.h"
#include "Rules/RuleValidationPolicy.h"


namespace tse
{

class RuleValidationChancelor
{
  typedef std::map<const uint16_t, RuleValidationPolicy*> PoliciesMap;

public:
  virtual ~RuleValidationChancelor();

  RuleValidationMonitor& getMutableMonitor() { return _monitor; }

  const RuleValidationMonitor& getMonitor() const { return _monitor; }

  void setPolicy(const uint16_t category, RuleValidationPolicy* policy)
  {
    _policies[category] = policy;
  }

  virtual bool hasPolicy(const uint16_t category)
  {
    const PoliciesMap::const_iterator findIt = _policies.find(category);
    return findIt != _policies.end();
  }
  virtual RuleValidationPolicy& getPolicy(const uint16_t category) const
  {
    const PoliciesMap::const_iterator findIt = _policies.find(category);
    TSE_ASSERT(findIt != _policies.end());
    return *findIt->second;
  }

  // Methods to deal with context
  virtual const RuleValidationContext& getContext() const { return _context; }

  void updateContextGroupId(const uint16_t& id) { _context._groupId = id; }

  void updateContextType(const RuleValidationContext::ContextType& type)
  {
    _context._contextType = type;
  }

  void updateContext(RuleControllerDataAccess& da)
  {
    _context._trx = &da.trx();
    _context._paxTypeFare = &da.paxTypeFare();
    _context._paxTypeFare->initializeFlexFareValidationStatus(da.trx());
    _context._fareUsage = da.getFareUsage();
  }

private:
  RuleValidationContext _context;
  RuleValidationMonitor _monitor;
  PoliciesMap _policies;
};
}

