//-------------------------------------------------------------------
//
//  Copyright Sabre 2003
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

#include "FareDisplay/RequestedPaxTypeValidator.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareDispCldInfPsgType.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "DBAccess/PaxTypeInfo.h"

namespace tse
{

static Logger
logger("atseintl.FareDisplay.RequestedPaxTypeValidator");

//------------------------------------------------------
// RequestedPaxTypeValidator::validate()
// @precondition : user has requested specific passenger type(s).
// @requested pax type, either exists in the record1 passenger list
// or in child/infant passenger list or the fare is a discounted fare
// and the base fare of the passenger type exists in the record 1
// passenger type.
//------------------------------------------------------

bool
RequestedPaxTypeValidator::validate(const PaxTypeFare& fare)
{
  LOG4CXX_INFO(logger, "Entered RequestedPaxTypeValidator::validate()");

  if (_trx->isERD() && fare.fcasPaxType().empty())
  {
    return true;
  }

  if (!isMatchQualifier(fare.actualPaxType()->paxTypeInfo()))
  {
    LOG4CXX_DEBUG(logger,
                  fare.actualPaxType()->paxTypeInfo().paxType()
                      << " -- Didnt Match the Requested Pax Type");
    return false;
  }
  else if (hasInclusionCode(*_trx))
  {
    if (fare.actualPaxType()->paxTypeInfo().isAdult())
    {

      return validateAdultFare(fare);
    }
    else
    {

      return validateChildInfantFare(fare);
    }
  }
  LOG4CXX_DEBUG(logger, "Passed RequestedPaxTypeValidation " << fare.fareClass());
  return true;
}

bool
RequestedPaxTypeValidator::initialize(const FareDisplayTrx& trx)
{

  PaxTypeValidator::initialize(trx);

  if (trx.fdResponse()->fareDisplayInclCd() != nullptr)
  {
    const std::vector<FareDispCldInfPsgType*>& cldPsgTypes =
        trx.dataHandle().getFareDispCldInfPsgType(_fareDisplayInclCd->userApplType(),
                                                  _fareDisplayInclCd->userAppl(),
                                                  _fareDisplayInclCd->pseudoCityType(),
                                                  _fareDisplayInclCd->pseudoCity(),
                                                  _fareDisplayInclCd->inclusionCode(),
                                                  Validator::CHILD_IND);
    LOG4CXX_DEBUG(logger, "Size of CHILD psgTypes: " << cldPsgTypes.size());
    if (cldPsgTypes.size() != 0)
    {
      std::vector<FareDispCldInfPsgType*>::const_iterator iter = cldPsgTypes.begin();
      std::vector<FareDispCldInfPsgType*>::const_iterator iterEnd = cldPsgTypes.end();
      for (; iter != iterEnd; iter++)
      {
        _publishedChildInfant.insert((*iter)->psgType());
      }
    }

    const std::vector<FareDispCldInfPsgType*>& infPsgTypes =
        trx.dataHandle().getFareDispCldInfPsgType(_fareDisplayInclCd->userApplType(),
                                                  _fareDisplayInclCd->userAppl(),
                                                  _fareDisplayInclCd->pseudoCityType(),
                                                  _fareDisplayInclCd->pseudoCity(),
                                                  _fareDisplayInclCd->inclusionCode(),
                                                  Validator::INFANT_IND);
    LOG4CXX_DEBUG(logger, "Size of INFANT psgTypes: " << infPsgTypes.size());
    if (infPsgTypes.size() != 0)
    {
      std::vector<FareDispCldInfPsgType*>::const_iterator iter = infPsgTypes.begin();
      std::vector<FareDispCldInfPsgType*>::const_iterator iterEnd = infPsgTypes.end();
      for (; iter != iterEnd; iter++)
      {
        _publishedChildInfant.insert((*iter)->psgType());
      }
    }
  }
  return true;
}

bool
RequestedPaxTypeValidator::isMatchQualifier(const PaxTypeInfo& info) const
{
  std::vector<PaxTypeCode>::const_iterator i =
      std::find(_trx->getRequest()->displayPassengerTypes().begin(),
                _trx->getRequest()->displayPassengerTypes().end(),
                info.paxType().empty() ? ADULT : info.paxType());

  if (i == _trx->getRequest()->displayPassengerTypes().end())
  {
    LOG4CXX_DEBUG(logger, "FARE FAILED REQUESTED PAX TYPE VALIDATION");
    return false;
  }
  return true;
}

Validator::Restriction
RequestedPaxTypeValidator::restriction(const PaxTypeFare& fare) const
{
  LOG4CXX_DEBUG(logger, "RETURNING AND RESTRICTION FOR INCL VALIDATION");
  if (!isMatchQualifier((fare.actualPaxType()->paxTypeInfo())))
  {
    return Validator::AND;
  }
  return Validator::restriction(fare);
}
}
