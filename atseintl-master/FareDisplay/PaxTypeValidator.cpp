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

#include "FareDisplay/PaxTypeValidator.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "DBAccess/FareDispRec1PsgType.h"
#include "DBAccess/PaxTypeInfo.h"

namespace tse
{

static Logger
logger("atseintl.FareDisplay.PaxTypeValidator");

using namespace std;

bool
PaxTypeValidator::initialize(const FareDisplayTrx& trx)
{

  _trx = &trx;
  if (hasInclusionCode(trx))
  {
    setInclusionCode(trx.fdResponse()->fareDisplayInclCd());
    setFlags();
    return populatePaxType(trx);
  }
  return true;
}
//------------------------------------------------------
// PaxTypeValidator::qualify()
//------------------------------------------------------

bool
PaxTypeValidator::validate(const PaxTypeFare& fare)
{
  // for short request, displayPassengerType is cleared, so we
  // may go to PaxTypeValidator instead of RequestedPassengerTypeValidator
  if (const_cast<FareDisplayTrx*>(_trx)->isShortRequest() &&
      (*_trx->getRequest()->inputPassengerTypes().begin()) == fare.actualPaxType()->paxType())
  {
    return true;
  }

  if (_inclCdPaxSet.empty())
  {
    return fare.actualPaxType()->paxTypeInfo().isAdult();
  }
  else
  {
    return validatePsgType(fare.actualPaxType()->paxType(), _inclCdPaxSet, _exceptPsgData);
  }
}

//------------------------------------------------------
// validatePsgType()
//------------------------------------------------------
bool
PaxTypeValidator::validatePsgType(const PaxTypeCode& paxType,
                                  const std::set<PaxTypeCode>& validPaxTypes,
                                  bool exceptPsgData) const
{
  LOG4CXX_INFO(logger, "Entered PaxTypeValidator::validatePsgType()");

  if (!validPaxTypes.empty())
  {
    const PaxTypeCode& code = paxType.empty() ? ADULT : paxType;

    std::set<PaxTypeCode>::const_iterator paxIter = validPaxTypes.find(code);

    if (exceptPsgData && paxIter != validPaxTypes.end())
    {
      if (paxIter != validPaxTypes.end())
      {
        LOG4CXX_INFO(logger, "Except psg: " << code << " was found.");
        return false;
      }
    }
    else
    {
      if (paxIter == validPaxTypes.end())
      {
        LOG4CXX_INFO(logger, "Non-Except psg: " << code << " was not found.");
        return false;
      }
    }
  }
  return true;
}

bool
PaxTypeValidator::populatePaxType(const FareDisplayTrx& trx)
{

  // Get the inclusion code ADULT REC1 passenger types
  const std::vector<FareDispRec1PsgType*>& rec1PsgTypes =
      trx.dataHandle().getFareDispRec1PsgType(_fareDisplayInclCd->userApplType(),
                                              _fareDisplayInclCd->userAppl(),
                                              _fareDisplayInclCd->pseudoCityType(),
                                              _fareDisplayInclCd->pseudoCity(),
                                              _fareDisplayInclCd->inclusionCode());
  // There may not be any
  LOG4CXX_DEBUG(logger, "Size of REC1 psgTypes: " << rec1PsgTypes.size());
  if (!rec1PsgTypes.empty())
  {
    std::vector<FareDispRec1PsgType*>::const_iterator iter = rec1PsgTypes.begin();
    std::vector<FareDispRec1PsgType*>::const_iterator iterEnd = rec1PsgTypes.end();
    for (; iter != iterEnd; iter++)
    {
      _inclCdPaxSet.insert((*iter)->psgType());
    }
    return true;
  }
  return true; // we always want to initialize a PaxTypeValidator to ensure we only pass Adult
               // fares.
}

void
PaxTypeValidator::setFlags()
{
  if (_fareDisplayInclCd != nullptr)
  {
    _exceptPsgData = _fareDisplayInclCd->exceptPsgType() == Validator::INCL_YES ? true : false;
  }
  else
  {

    _exceptPsgData = false;
  }
}

bool
PaxTypeValidator::validateChildInfantFare(const PaxTypeFare& fare) const
{

  if (isDiscounted(fare))
  {
    const PaxTypeCode& code = getBaseFarePaxType(fare);
    return validatePsgType(code, _inclCdPaxSet);
  }
  else
    return (!_publishedChildInfant.empty() &&
            validatePsgType(fare.actualPaxType()->paxType(), _publishedChildInfant));
}

bool
PaxTypeValidator::validateAdultFare(const PaxTypeFare& fare) const
{
  return validatePsgType(fare.actualPaxType()->paxType(), _inclCdPaxSet);
}

const PaxTypeCode&
PaxTypeValidator::getBaseFarePaxType(const PaxTypeFare& fare) const
{
  if ((fare.fareWithoutBase())->actualPaxType() == nullptr)
    return fare.actualPaxType()->paxType();
  else
    return (fare.fareWithoutBase())->actualPaxType()->paxType();
}

bool
PaxTypeValidator::isMatchQualifier(const PaxTypeInfo& info) const
{
  return info.isAdult();
}

void
PaxTypeValidator::failValidator(bool& invalidDT, bool& invalidFT, bool& invalidPT)
{
  invalidPT = true;
}
}
