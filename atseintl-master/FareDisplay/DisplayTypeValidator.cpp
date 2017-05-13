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

#include "FareDisplay/DisplayTypeValidator.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareDispInclDsplType.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "DBAccess/PaxTypeInfo.h"

namespace tse
{
static Logger
logger("atseintl.FareDisplay.DisplayTypeValidator");
using namespace std;

bool
DisplayTypeValidator::initialize(const FareDisplayTrx& trx)
{

  if (hasInclusionCode(trx))
  {
    setInclusionCode(trx.fdResponse()->fareDisplayInclCd());
    return getData(trx);
  }

  return false;
}
//------------------------------------------------------
// DisplayTypeValidator::qualify()
//------------------------------------------------------

bool
DisplayTypeValidator::validate(const PaxTypeFare& fare)
{
  LOG4CXX_INFO(logger, "Entered GeneralInclusionCodeValidator::validateFareType()");

  if (fare.actualPaxType()->paxTypeInfo().isAdult())
  {
    return validateDisplayType(fare.fcaDisplayCatType());
  }
  else
  {
    if (isDiscounted(fare))
      return validateDisplayType(fare.baseFare(19)->fcaDisplayCatType());

    return validateDisplayType(fare.fcaDisplayCatType());
  }
  return true;
}

bool
DisplayTypeValidator::getData(const FareDisplayTrx& trx)
{
  // Get the display types
  const std::vector<FareDispInclDsplType*>& displayTypes =
      trx.dataHandle().getFareDispInclDsplType(_fareDisplayInclCd->userApplType(),
                                               _fareDisplayInclCd->userAppl(),
                                               _fareDisplayInclCd->pseudoCityType(),
                                               _fareDisplayInclCd->pseudoCity(),
                                               _fareDisplayInclCd->inclusionCode());
  if (!displayTypes.empty())
  {
    std::vector<FareDispInclDsplType*>::const_iterator iter = displayTypes.begin();
    std::vector<FareDispInclDsplType*>::const_iterator iterEnd = displayTypes.end();
    for (; iter != iterEnd; iter++)
    {
      _displayTypes.insert((*iter)->displayCatType());
    }
    return true;
  }
  return false;
}

bool
DisplayTypeValidator::validateDisplayType(const Indicator& displayType) const
{
  if (!_displayTypes.empty())
  {
    std::set<Indicator>::const_iterator catIter = _displayTypes.find(displayType);

    if (catIter == _displayTypes.end())
    {
      LOG4CXX_INFO(logger, "Non-Except dspl: " << displayType << " was not found.");
      return false;
    }
    return true;
  }

  return true;
}

void
DisplayTypeValidator::failValidator(bool& invalidDT, bool& invalidFT, bool& invalidPT)
{
  invalidDT = true;
}
}
