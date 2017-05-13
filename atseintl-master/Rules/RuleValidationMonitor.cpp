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

#include "Rules/RuleValidationMonitor.h"

#include "Common/TseEnums.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleValidationObserver.h"

namespace tse
{

void
RuleValidationMonitor::subscribe(ObserverPtr observer, const Event& event)
{
  _observers[event].push_back(observer);
}

void
RuleValidationMonitor::unsubscribe(ObserverPtr observer, const Event& event)
{
  std::map<Event, std::vector<ObserverPtr> >::iterator it;
  it = _observers.find(event);

  if (it != _observers.end())
  {
    it->second.erase(std::remove(it->second.begin(), it->second.end(), observer), it->second.end());
  }
}

const RuleValidationMonitor::ObserversVec&
RuleValidationMonitor::getObservers(const Event& event) const
{
  const ObserversMap::const_iterator it = _observers.find(event);

  if (it == _observers.end())
    return _emptyObserversVector;

  return it->second;
}

void
RuleValidationMonitor::notify(const Event& event,
                              const RuleValidationContext& rvc,
                              flexFares::ValidationStatusPtr validationStatus,
                              const uint16_t& catNumber,
                              const Record3ReturnTypes& ret)
{
  for (ObserverPtr itv : getObservers(event))
  {
    Observer& ob = *itv;
    ob.update(rvc, validationStatus, catNumber, ret);
  }
}

void
RuleValidationMonitor::notify(const Event& event,
                              const uint16_t& catNumber,
                              const RuleValidationPolicy& policy)
{
  for (ObserverPtr observer : getObservers(event))
  {
    observer->update(catNumber, policy);
  }
}

void
RuleValidationMonitor::notify(const Event& event,
                              const RuleValidationContext& rvc,
                              const std::string& acctCodeOrCorpIdString,
                              flexFares::ValidationStatusPtr validationStatus,
                              const bool isAccountCode)
{
  for (ObserverPtr itv : getObservers(event))
  {
    Observer& ob = *itv;
    ob.update(rvc, acctCodeOrCorpIdString, validationStatus, isAccountCode);
  }
}

} // tse
