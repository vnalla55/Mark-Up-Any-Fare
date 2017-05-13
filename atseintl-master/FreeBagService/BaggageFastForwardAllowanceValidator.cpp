//-------------------------------------------------------------------
//  Copyright Sabre 2012
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
#include "FreeBagService/BaggageFastForwardAllowanceValidator.h"

#include "DBAccess/OptionalServicesInfo.h"
#include "Diagnostic/Diag877Collector.h"
#include "ServiceFees/OCFees.h"


namespace tse
{

StatusS7Validation
BaggageFastForwardAllowanceValidator::checkServiceNotAvailNoCharge(const OptionalServicesInfo& info,
                                                                   OCFees& ocFees) const
{
  if (info.notAvailNoChargeInd() == SERVICE_FREE_NO_EMD_ISSUED)
    return PASS_S7;

  if (info.notAvailNoChargeInd() == DEFER_BAGGAGE_RULES_FOR_MC)
    return FAIL_S7_DEFER_BAGGAGE_RULE;

  if (info.notAvailNoChargeInd() == DEFER_BAGGAGE_RULES_FOR_OPC)
    return FAIL_S7_DEFER_BAGGAGE_RULE;

  // TODO: change enum to NO_CHARGE
  return FAIL_S7_NOT_AVAIL_NO_CHANGE;
}

} // tse
