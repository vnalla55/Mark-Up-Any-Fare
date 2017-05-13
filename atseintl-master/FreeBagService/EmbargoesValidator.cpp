//-------------------------------------------------------------------
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
//-------------------------------------------------------------------
#include "FreeBagService/EmbargoesValidator.h"

#include "DBAccess/OptionalServicesInfo.h"

namespace tse
{

StatusS7Validation
EmbargoesValidator::checkServiceNotAvailNoCharge(const OptionalServicesInfo& info, OCFees& ocFees)
    const
{
  if (info.notAvailNoChargeInd() == SERVICE_NOT_AVAILABLE)
    return PASS_S7;
  else
    return FAIL_S7_NOT_AVAIL_NO_CHANGE;
}
}
