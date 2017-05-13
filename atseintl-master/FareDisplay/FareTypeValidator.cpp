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

#include "FareDisplay/FareTypeValidator.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareDispInclFareType.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "DBAccess/PaxTypeInfo.h"

namespace tse
{
static Logger
logger("atseintl.FareDisplay.FareTypeValidator");

using namespace std;

bool
FareTypeValidator::initialize(const FareDisplayTrx& trx)
{
  if (hasInclusionCode(trx))
  {
    setInclusionCode(trx.fdResponse()->fareDisplayInclCd());
    setFlags();
    return populateFareTypes(trx);
  }
  else
  {
    return false;
  }
}

bool
FareTypeValidator::validate(const PaxTypeFare& fare)
{
  LOG4CXX_INFO(logger, "Entered FareTypeValidator::validateFareType()");
  if (fare.actualPaxType()->paxTypeInfo().isAdult())
  {

    return validateFareType(fare.fcaFareType(), _exceptFareData);
  }
  else
  {
    if (isDiscounted(fare))
    {
      return validateFareType(getBaseFareFareType(fare), _exceptFareData);
    }
    return validateFareType(fare.fcaFareType(), _exceptFareData);
  }
}

bool
FareTypeValidator::populateFareTypes(const FareDisplayTrx& trx)
{
  const std::vector<FareDispInclFareType*>& fareTypes =
      trx.dataHandle().getFareDispInclFareType(_fareDisplayInclCd->userApplType(),
                                               _fareDisplayInclCd->userAppl(),
                                               _fareDisplayInclCd->pseudoCityType(),
                                               _fareDisplayInclCd->pseudoCity(),
                                               _fareDisplayInclCd->inclusionCode());
  // There may not be any
  if (!fareTypes.empty())
  {
    std::vector<FareDispInclFareType*>::const_iterator iter = fareTypes.begin();
    std::vector<FareDispInclFareType*>::const_iterator iterEnd = fareTypes.end();
    for (; iter != iterEnd; iter++)
    {
      _fareTypes.insert((*iter)->fareType());
    }
    return true;
  }
  return false;
}

void
FareTypeValidator::setFlags()
{
  _exceptFareData = _fareDisplayInclCd->exceptFareType() == Validator::INCL_YES ? true : false;
}

bool
FareTypeValidator::validateFareType(const FareType& fareType, bool exceptFareData) const
{

  if (!_fareTypes.empty())
  {
    std::set<FareType>::const_iterator fareIter = _fareTypes.find(fareType);
    if (exceptFareData)
    {
      if (fareIter != _fareTypes.end())
      {
        LOG4CXX_INFO(logger, "Except FARE TYPE: " << fareType);
        return false;
      }
    }
    else
    {
      if (fareIter == _fareTypes.end())
      {
        LOG4CXX_INFO(logger, "Non-Except FARE TYPE: " << fareType);
        return false;
      }
    }
  }
  return true;
}
const FareType&
FareTypeValidator::getBaseFareFareType(const PaxTypeFare& fare) const
{

  return (fare.fareWithoutBase())->fcaFareType();
}

void
FareTypeValidator::failValidator(bool& invalidDT, bool& invalidFT, bool& invalidPT)
{
  invalidFT = true;
}
}
