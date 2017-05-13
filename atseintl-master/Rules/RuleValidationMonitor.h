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

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "DataModel/FlexFares/Types.h"

#include <map>
#include <vector>

namespace tse
{

struct RuleValidationContext;
class RuleValidationObserver;
class RuleValidationPolicy;

class RuleValidationMonitor
{
public:
  enum Event
  {
    VALIDATION_START = 0,
    VALIDATION_RESULT
  };

  typedef RuleValidationObserver Observer;
  typedef Observer* ObserverPtr;
  typedef std::vector<ObserverPtr> ObserversVec;
  typedef std::map<Event, std::vector<ObserverPtr> > ObserversMap;

  void subscribe(ObserverPtr observer, const Event& event);
  void unsubscribe(ObserverPtr observer, const Event& event);

  void notify(const Event& event,
              const RuleValidationContext& rvc,
              flexFares::ValidationStatusPtr validationStatus,
              const uint16_t& catNumber,
              const Record3ReturnTypes& ret);

  void notify(const Event& event, const uint16_t& catNumber, const RuleValidationPolicy& policy);

  void notify(const Event& event,
              const RuleValidationContext& rvc,
              const std::string& acctCodeOrCorpIdString,
              flexFares::ValidationStatusPtr validationStatus,
              const bool isAccountCode);

private:
  const ObserversVec& getObservers(const Event& event) const;

  ObserversMap _observers;
  const ObserversVec _emptyObserversVector;
};
}

