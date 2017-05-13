//-------------------------------------------------------------------
//
//  File:        PrepareRexFareRules.h
//  Created:     April 30, 2007
//  Authors:     Simon Li
//
//  Updates:
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "DataModel/ExcItin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexBaseTrx.h"
#include "Rules/FareMarketRuleController.h"
#include "Rules/RuleConst.h"

namespace tse
{

/**
*   @class PrepareRexFareRules
*
*   Description:
*       for REX, collect fare rules and validate
*
*/
class PrepareRexFareRules
{
  friend class RexFarePrepareRuleTest;

public:
  PrepareRexFareRules(RexBaseTrx& rexTrx, RuleController* ruleController)
    : _rexTrx(rexTrx),
      _ruleController(ruleController),
      _subjectCategory((rexTrx.excTrxType() == PricingTrx::AF_EXC_TRX)
                           ? RuleConst::VOLUNTARY_REFUNDS_RULE
                           : RuleConst::VOLUNTARY_EXCHANGE_RULE)
  {
  }

  void process(PaxTypeFare* ptf);
  bool process();

protected:
  void skipRulesOnExcItin(PaxTypeFare& ptf) const;

private:
  RexBaseTrx& _rexTrx;
  RuleController* _ruleController;
  const uint16_t _subjectCategory;
};

} // namespace tse

