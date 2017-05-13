//-------------------------------------------------------------------
//
//  Copyright Sabre 2008
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/ALLPaxTypeValidator.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/PaxTypeInfo.h"

namespace tse
{

static Logger
logger("atseintl.FareDisplay.ALLPaxTypeValidator");

using namespace std;

//------------------------------------------------------
// RequestedPaxTypeValidator::validate()
// @precondition : user has requested specific passenger type(s).
// @requested pax type, either exists in the record1 passenger list
// or in child/infant passenger list or the fare is a discounted fare
// and the base fare of the passenger type exists in the record 1
// passenger type.
//------------------------------------------------------

bool
ALLPaxTypeValidator::validate(const PaxTypeFare& fare)
{

  LOG4CXX_INFO(logger, "Entered ALL::validate()");
  // requested passenger - use base class
  if (!_trx->getRequest()->displayPassengerTypes().empty())
    return RequestedPaxTypeValidator::validate(fare);
  // if no ACI, then pass all fares
  if (!_trx->getOptions()->isChildFares() && !_trx->getOptions()->isInfantFares() &&
      !_trx->getOptions()->isAdultFares())
    return true;

  // match fare passenger type to requested option
  if (fare.actualPaxType()->paxTypeInfo().isAdult() && _trx->getOptions()->isAdultFares())
    return true;
  if (fare.actualPaxType()->paxTypeInfo().isChild() && _trx->getOptions()->isChildFares())
    return true;
  if (fare.actualPaxType()->paxTypeInfo().isInfant() && _trx->getOptions()->isInfantFares())
    return true;

  return false;
}
Validator::Restriction
ALLPaxTypeValidator::restriction(const PaxTypeFare& fare) const
{
  LOG4CXX_DEBUG(logger, "RETURNING AND RESTRICTION FOR ALL INCL VALIDATION");
  if (_trx->getRequest()->displayPassengerTypes().empty())
  {
    return Validator::AND;
  }
  return RequestedPaxTypeValidator::restriction(fare);
}
}
