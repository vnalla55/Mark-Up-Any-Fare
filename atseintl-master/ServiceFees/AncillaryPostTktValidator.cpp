//-------------------------------------------------------------------
//  Copyright Sabre 2011
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
#include "ServiceFees/AncillaryPostTktValidator.h"

#include "DBAccess/OptionalServicesInfo.h"

namespace tse
{
bool
AncillaryPostTktValidator::checkAdvPurchaseTktInd(const OptionalServicesInfo& optSrvInfo) const
{
  return optSrvInfo.advPurchTktIssue() != 'X';
}
}
