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

#include "Rules/CategoryValidationObserver.h"

#include "Common/TseEnums.h"
#include "DataModel/FlexFares/ValidationStatus.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleValidationContext.h"
#include "Rules/RuleValidationMonitor.h"

namespace tse
{

CategoryValidationObserver::CategoryValidationObserver(Monitor& monitor)
  : RuleValidationObserver(monitor)
{
  _monitor->subscribe(this, RuleValidationMonitor::VALIDATION_RESULT);
}


void
CategoryValidationObserver::update(const RuleValidationContext& rvc,
                                   flexFares::ValidationStatusPtr validationStatus,
                                   const uint16_t& catNumber,
                                   const Record3ReturnTypes& ret)
{
  if (rvc._contextType != RuleValidationContext::FARE_MARKET)
    return;

  switch (catNumber)
  {
    case RuleConst::PENALTIES_RULE:
      validationStatus->setStatus<flexFares::NO_PENALTIES>(ret);
      break;

    case RuleConst::ADVANCE_RESERVATION_RULE :
      validationStatus->setStatus<flexFares::NO_ADVANCE_PURCHASE>(ret);
      break;

    case RuleConst::MINIMUM_STAY_RULE:
    case RuleConst::MAXIMUM_STAY_RULE:
      validationStatus->setStatus<flexFares::NO_MIN_MAX_STAY>(ret);
      break;

    default:
    //Categories not supported, do nothing
      break;
  }
}

void
CategoryValidationObserver::update(const RuleValidationContext& rvc,
                                   const std::string& acctCodeOrCorpIdString,
                                   flexFares::ValidationStatusPtr validationStatus,
                                   const bool isAccountCode)
{
  if (rvc._contextType != RuleValidationContext::FARE_MARKET)
    return;

  if (isAccountCode)
  {
    validationStatus->setValidAccCode(acctCodeOrCorpIdString);
  }
  else
  {
    validationStatus->setValidCorpId(acctCodeOrCorpIdString);
  }
}


CategoryValidationObserver::~CategoryValidationObserver()
{
  if (_monitor)
    _monitor->unsubscribe(this, RuleValidationMonitor::VALIDATION_RESULT);
}

} // tse
