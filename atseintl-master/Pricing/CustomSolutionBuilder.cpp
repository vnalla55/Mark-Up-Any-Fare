/*
 * CustomSolutionBuilder.cpp
 *
 *  Created on: March 4, 2013
 *      Author: Masud Khan
 *
 * The copyright to the computer program(s) herein
 * is the property of Sabre.
 * The program(s) may be used and/or copied only with
 * the written permission of Sabre or in accordance
 * with the terms and conditions stipulated in the
 * agreement/contract under which the program(s)
 * have been supplied.
 *
 */

#include "Pricing/CustomSolutionBuilder.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TrxAborter.h"
#include "Pricing/ShoppingPQ.h"

namespace tse
{
FALLBACK_DECL(reworkTrxAborter);

static Logger
logger("atseintl.CustomSolutionBuilder");

void
CustomSolutionBuilder::getAdditionalSolutionsFromExistingGFP()
{
  bool mustHurry = false;
  // 1. Do nothing if no additional solution needed
  if (!isMoreCustomSolutionsNeeded())
    return;

  TSELatencyData metrics(_trx, "PO GET MORE FROM EXISTING");

  if (fallback::reworkTrxAborter(&_trx))
    mustHurry = checkTrxMustHurry(_trx);
  else
    mustHurry = _trx.checkTrxMustHurry();

  ShoppingTrx::ShoppingPQVector& shoppingPQVector = _trx.shoppingPQVector();

  // 2. Get all GFPs from all PQ and insert them in sorted list if they can produce custom solution.
  SortedGfpSet candidateGfpSet;
  for (ShoppingTrx::ShoppingPQVector::iterator task = shoppingPQVector.begin();
       task != shoppingPQVector.end() && !mustHurry;
       ++task)
  {
    (*task)->resetProcessDoneWithCond();
    std::vector<GroupFarePath*> results;
    (*task)->getSortedSolutionsGFP(results);

    if (results.empty())
      continue;

    ShoppingPQ* pq = (*task).get();

    for (std::vector<GroupFarePath*>::iterator it = results.begin();
         it != results.end() && !mustHurry;
         ++it)
    {
      if (ShoppingUtil::isCustomSolutionGfp(_trx, *it))
      {
        candidateGfpSet.insert(GFPInPQ(pq, *it));
      }
      if (fallback::reworkTrxAborter(&_trx))
        mustHurry = checkTrxMustHurry(_trx) || (*task)->checkProcessDoneWithCond();
      else
        mustHurry = _trx.checkTrxMustHurry() || (*task)->checkProcessDoneWithCond();
    }
  }

  // 3. Process items from sorted list until we meet our goal of Additional Solutions
  // or the list is exhausted

  for (SortedGfpSet::iterator gfpIt = candidateGfpSet.begin();
       gfpIt != candidateGfpSet.end() && !mustHurry;
       ++gfpIt)
  {
    ShoppingPQ* pq = gfpIt->first;

    pq->customSolutionStateSet(true);
    pq->processSolution(gfpIt->second, true);
    pq->customSolutionStateSet(false);
    // Check if we need any more additional custom solution
    if (!isMoreCustomSolutionsNeeded())
      break;
    pq->customSolutionStateSet(false);

    if (fallback::reworkTrxAborter(&_trx))
      mustHurry = checkTrxMustHurry(_trx) || pq->checkProcessDoneWithCond();
    else
      mustHurry = _trx.checkTrxMustHurry() || pq->checkProcessDoneWithCond();
  }
}

void
CustomSolutionBuilder::getAdditionalSolutionsFromQueue()
{
  // Do nothing if no additional solution needed
  if (!isMoreCustomSolutionsNeeded())
    return;

  TSELatencyData metrics(_trx, "PO GET MORE FROM QUEUES");

  bool mustHurry = false;
  if (fallback::reworkTrxAborter(&_trx))
    mustHurry = checkTrxMustHurry(_trx);
  else
    mustHurry = _trx.checkTrxMustHurry();
  ShoppingTrx::ShoppingPQVector& shoppingPQVector = _trx.shoppingPQVector();

  for (ShoppingTrx::ShoppingPQVector::reverse_iterator task = shoppingPQVector.rbegin();
       task != shoppingPQVector.rend() && !mustHurry;
       ++task)
  {
    if (!isMoreCustomSolutionsNeeded())
      break;
    const uint32_t nbrSolutionRequired = _trx.pqDiversifierResult()._minNumCustomSolutions -
                                         _trx.pqDiversifierResult().currentCustomOptionCount();
    // Try this PQ for more custom solutions
    (*task)->customSolutionStateSet(true);
    (*task)->getAdditionalSolutions(nbrSolutionRequired);
    (*task)->customSolutionStateSet(false);

    if (fallback::reworkTrxAborter(&_trx))
      mustHurry = checkTrxMustHurry(_trx) || (*task)->checkProcessDoneWithCond();
    else
      mustHurry = _trx.checkTrxMustHurry() || (*task)->checkProcessDoneWithCond();
  }
}

void
CustomSolutionBuilder::getFlightOnlySolutions()
{
  // Do nothing if no additional solution needed
  if (!isMoreCustomSolutionsNeeded())
    return;

  TSELatencyData metrics(_trx, "PO GET FOS");

  bool mustHurry = false;

  if (fallback::reworkTrxAborter(&_trx))
    mustHurry = checkTrxMustHurry(_trx) && _trx.pqDiversifierResult().currentCustomOptionCount();
  else
    mustHurry = _trx.checkTrxMustHurry() && _trx.pqDiversifierResult().currentCustomOptionCount();

  ShoppingTrx::ShoppingPQVector& shoppingPQVector = _trx.shoppingPQVector();

  for (ShoppingTrx::ShoppingPQVector::iterator task = shoppingPQVector.begin();
       task != shoppingPQVector.end() && !mustHurry;
       ++task)
  {
    if (!isMoreCustomSolutionsNeeded())
      break;

    // Generate Flight only solutions from this PQ
    (*task)->customSolutionStateSet(true);
    GroupFarePath* gfp = _trx.dataHandle().create<GroupFarePath>();
    gfp->setTotalNUCAmount(1000000);
    (*task)->generateSolutionsWithNoFares(gfp);
    (*task)->customSolutionStateSet(false);

    if (fallback::reworkTrxAborter(&_trx))
      mustHurry = checkTrxMustHurry(_trx) && _trx.pqDiversifierResult().currentCustomOptionCount();
    else
      mustHurry = _trx.checkTrxMustHurry() && _trx.pqDiversifierResult().currentCustomOptionCount();
  }
}

bool
CustomSolutionBuilder::isMoreCustomSolutionsNeeded()
{
  return (_trx.pqDiversifierResult().currentCustomOptionCount() <
          _trx.pqDiversifierResult()._minNumCustomSolutions);
}

} /* namespace tse */
