//----------------------------------------------------------------------------
//  File: FBDisplayCat10Controller.h
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
#pragma once

#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Rules/RuleUtil.h"


#include <vector>

namespace tse
{

class FBDisplayCat10Controller
{
  friend class FBDisplayCat10ControllerTest;

public:
  FBDisplayCat10Controller(FareDisplayTrx& trx) : _trx(trx) {};

  virtual ~FBDisplayCat10Controller() {};
  virtual bool collectRecord2Info();

  // --------------------------------------
  // Methods for Combinibility Scoreboard
  // --------------------------------------
  virtual const CombinabilityRuleInfo*
  collectCombinabilityScoreboardRec2Info(FareDisplayInfo& fareDisplayInfo,
                                         PaxTypeFare& paxTypeFare,
                                         uint16_t cat);

private:
  FareDisplayTrx& _trx;
};
}; // end of namespace
