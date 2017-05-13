//----------------------------------------------------------------------------
//
// Copyright Sabre 2014
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#include "FreeBagService/BTAValidationProcessObserver.h"

#include "DBAccess/OptionalServicesInfo.h"
#include "Diagnostic/Diag852Collector.h"
#include "FreeBagService/IS7RecordFieldsValidator.h"
#include "Util/BranchPrediction.h"

namespace tse
{

bool
BTAValidationProcessObserver::notifyCabinValidationFinished(bool cabinPassed) const
{
  if (UNLIKELY(_diag))
    _diag->printBTAFieldStatusCabin(cabinPassed);

  return cabinPassed;
}

bool
BTAValidationProcessObserver::notifyRBDValidationFinished(bool rbdPassed) const
{
  if (UNLIKELY(_diag))
    _diag->printBTAFieldStatusRBD(rbdPassed);

  return rbdPassed;
}

void
BTAValidationProcessObserver::notifySegmentValidationStarted(const TravelSeg& segment) const
{
  if (UNLIKELY(_diag))
    _diag->printBTASegmentHeader(segment);
}

StatusS7Validation
BTAValidationProcessObserver::notifySegmentValidationFinished(StatusS7Validation validationStatus,
                                                              const TravelSeg& segment) const
{
  if (UNLIKELY(_diag))
    _diag->printBTASegmentFooter(PASS_S7 == validationStatus, segment);

  return validationStatus;
}

bool
BTAValidationProcessObserver::notifyTable171ValidationFinished(bool resultingFareClassPasses) const
{
  if (UNLIKELY(_diag))
    _diag->printBTAStatusTableT171(resultingFareClassPasses);

  return resultingFareClassPasses;
}

bool
BTAValidationProcessObserver::notifyOutputTicketDesignatorValidationFinished(bool passed) const
{
  if (UNLIKELY(_diag))
    _diag->printBTAStatusOutputTicketDesignator(passed);

  return passed;
}

bool
BTAValidationProcessObserver::notifyRuleValidationFinished(bool rulePassed) const
{
  if (UNLIKELY(_diag))
    _diag->printBTAFieldStatusRule(rulePassed);

  return rulePassed;
}

bool
BTAValidationProcessObserver::notifyRuleTariffValidationFinished(bool ruleTariffPassed) const
{
  if (UNLIKELY(_diag))
    _diag->printBTAFieldStatusRuleTariff(ruleTariffPassed);

  return ruleTariffPassed;
}

bool
BTAValidationProcessObserver::notifyFareIndValidationFinished(bool fareIndPassed) const
{
  if (UNLIKELY(_diag))
    _diag->printBTAFieldStatusFareInd(fareIndPassed);

  return fareIndPassed;
}

bool
BTAValidationProcessObserver::notifyCarrierFlightApplT186Finished(bool t186Passed) const
{
  if (UNLIKELY(_diag))
    _diag->printBTAFieldStatusCarrierFlightApplT186(t186Passed);

  return t186Passed;
}

} // tse
