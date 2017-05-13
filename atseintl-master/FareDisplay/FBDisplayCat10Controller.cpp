//----------------------------------------------------------------------------
//  File: FBDisplayCat10Controller.cpp
//
//  Author: Partha Kumar Chakraborti
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#include "FareDisplay/FBDisplayCat10Controller.h"

#include "Common/Logger.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/GeneralRuleApp.h"

namespace tse
{
static Logger
logger("atseintl.Pricing.FBDisplayCat10Controller");

bool
FBDisplayCat10Controller::collectRecord2Info()
{

  if (!_trx.isRD() && !_trx.isSDSOutputType())
    return false;

  if (_trx.allPaxTypeFare().size() == 0)
    return false;

  std::vector<PaxTypeFare*>::const_iterator ptfIter = _trx.allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::const_iterator ptfIterEnd = _trx.allPaxTypeFare().end();
  for (; ptfIter != ptfIterEnd; ptfIter++)
  {
    PaxTypeFare& paxTypeFare = **ptfIter;

    FareDisplayInfo* fdPtr(nullptr);

    fdPtr = paxTypeFare.fareDisplayInfo();

    if (!fdPtr)
    {
      LOG4CXX_DEBUG(logger, "Fare display Info object not found. Stopped gathering rec2 info");
      return false;
    }

    // -----------------------------------------
    // Collect Rec2 info of Combinibility Scoreboard: CAT 10
    // -----------------------------------------
    const CombinabilityRuleInfo* pCat10 =
        collectCombinabilityScoreboardRec2Info(*fdPtr, paxTypeFare, RuleConst::COMBINABILITY_RULE);

    fdPtr->setFBDisplayData(pCat10, paxTypeFare.fareClass(), _trx.dataHandle(), _trx.isDomestic());
  }
  return true;
}

// --------------------------------------
// Methods for Combinibility Scoreboard
// --------------------------------------
const CombinabilityRuleInfo*
FBDisplayCat10Controller::collectCombinabilityScoreboardRec2Info(FareDisplayInfo& fareDisplayInfo,
                                                                 PaxTypeFare& paxTypeFare,
                                                                 uint16_t cat)
{

  DiagCollector diag;
  CombinabilityRuleInfo* pCat10;
  //  bool samePoint         = false;
  bool isLocationSwapped = false;

  pCat10 = RuleUtil::getCombinabilityRuleInfo(_trx, paxTypeFare, isLocationSwapped);

  if (!pCat10)
    return nullptr;

  //'X' is Combinations::UNAVAILABLE_DATA

  if (pCat10->applInd() == 'X')
    return nullptr;

  // TODO: Do we require this validation?
  LocCode cOrig;
  LocCode cDest;

  cOrig = paxTypeFare.fareMarket()->boardMultiCity();
  cDest = paxTypeFare.fareMarket()->offMultiCity();

  if (cOrig != cDest)
  {
    RuleUtil::validateSamePoint(_trx,
                                pCat10->vendorCode(),
                                pCat10->samepointstblItemNo(),
                                cOrig,
                                cDest,
                                paxTypeFare.fareMarket()->travelDate(),
                                &diag,
                                Diagnostic653);
  }

  return pCat10;
}
}
