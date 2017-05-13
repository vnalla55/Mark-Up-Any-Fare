// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Pricing/Shopping/PQ/OwrtFmPatternSummaryInitializer.h"

#include "DataModel/OwrtFareMarket.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "Pricing/Combinations.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"
#include "Rules/RuleUtil.h"

namespace tse
{
namespace shpq
{

void
OwrtFmPatternSummaryInitializer::initSummary(OwrtFareMarket& fm)
{
  typedef OwrtFmPatternSummary Summary;

  Summary& summary = fm.getMutableSolPatternSummary();
  if (summary.isInitialized())
    return;

  summary.setLowerBoundForPattern(Summary::EOE, UNKNOWN_MONEY_AMOUNT);
  summary.setLowerBoundForPattern(Summary::NOT_EOE, UNKNOWN_MONEY_AMOUNT);

  for (PaxTypeFare* ptf : fm)
  {
    if (!isEoeNotPermitted(*ptf)) // valid for EOE
    {
      summary.setLowerBoundForPattern(Summary::EOE, ptf->nucFareAmount());
      break;
    }
  }

  for (PaxTypeFare* ptf : fm)
  {
    if (!isEoeRequired(*ptf)) // valid for NOT_EOE
    {
      summary.setLowerBoundForPattern(Summary::NOT_EOE, ptf->nucFareAmount());
      break;
    }
  }

  summary.setInitialized(true);
}

bool
OwrtFmPatternSummaryInitializer::isEoeRequired(PaxTypeFare& ptf) const
{
  if (!ptf.rec2Cat10())
    ptf.rec2Cat10() = RuleUtil::getCombinabilityRuleInfo(_trx, ptf);

  if (ptf.rec2Cat10())
  {
    return ptf.rec2Cat10()->eoeInd() == Combinations::REQUIRED ||
           ptf.rec2Cat10()->eoeInd() == Combinations::EOE_REQUIRED_SIDE_TRIP_NOT_PERMITTED;
  }

  return false;
}

bool
OwrtFmPatternSummaryInitializer::isEoeNotPermitted(PaxTypeFare& ptf) const
{
  if (!ptf.rec2Cat10())
    ptf.rec2Cat10() = RuleUtil::getCombinabilityRuleInfo(_trx, ptf);

  if (ptf.rec2Cat10())
  {
    return ptf.rec2Cat10()->eoeInd() == Combinations::NOT_PERMITTED ||
           ptf.rec2Cat10()->eoeInd() == Combinations::EOE_NOT_PERMITTED_SIDE_TRIP_NOT_PERMITTED;
  }

  return false;
}
}
}
