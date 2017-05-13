//----------------------------------------------------------------------------
//  File: FBDisplayController.h
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/DBAForwardDecl.h"
#include "Fares/DiscountedFareController.h"
#include "Rules/RuleUtil.h"


#include <vector>

namespace tse
{

class FBDisplayController
{
  friend class FBDisplayControllerTest;

public:
  FBDisplayController(FareDisplayTrx& trx) : _trx(trx) {};
  virtual ~FBDisplayController() {};
  virtual bool collectRecord2Info();

protected:
  virtual bool collectDiscountedFareRec2Info(FareDisplayInfo& fareDisplayInfo,
                                             PaxTypeFare& paxTypeFare,
                                             uint16_t cat);

  virtual GeneralFareRuleInfo* getGeneralFareRuleInfo(FareDisplayTrx& trx,
                                                      PaxTypeFare& paxTypeFare,
                                                      uint16_t cat,
                                                      bool& isLocationSwapped,
                                                      const TariffNumber* overrideTcrRuleTariff,
                                                      const RuleNumber* overrideRuleNumber);

  virtual bool collectCat25Rec2Info(FareDisplayInfo& fareDisplayInfo, PaxTypeFare& paxTypeFare);

  virtual bool collectAllOtherFareRec2Info(FareDisplayInfo& fareDisplayInfo,
                                           PaxTypeFare& paxTypeFare,
                                           uint16_t cat);

  const FareRuleRecord2Info*
  castToFareRule(const CategoryRuleInfo* categoryRuleInfo, const PaxTypeFare& paxTypeFare);

  const FootNoteRecord2Info* castToFootNote(const CategoryRuleInfo* categoryRuleInfo);

  bool fareRuleTakesPrecedenceOverGeneralRule(const GeneralFareRuleInfo* ruleInfo,
                                              uint16_t category) const;

private:
  FareDisplayTrx& _trx;
};
}; // end of namespace
