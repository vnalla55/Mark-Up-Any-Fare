// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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
#include "Pricing/Shopping/FOS/FosBoss.h"

#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag910Collector.h"
#include "Pricing/Shopping/FOS/FosCommonUtil.h"
#include "Pricing/Shopping/FOS/FosCompositeBuilder.h"
#include "Pricing/Shopping/FOS/FosFilterComposite.h"
#include "Pricing/Shopping/FOS/FosMatrixInserter.h"
#include "Pricing/Shopping/FOS/FosStatistic.h"
#include "Pricing/Shopping/FOS/FosTaskScope.h"
#include "Pricing/Shopping/FOS/FosTypes.h"
#include "Pricing/Shopping/FOS/FosValidatorComposite.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"

#include <string>

namespace tse
{
FALLBACK_DECL(fallbackNGSJCBPaxTuning);

namespace fos
{

FosBoss::FosBoss(ShoppingTrx& trx, ItinStatistic* itinStatistic)
  : _trx(trx), _itinStatistic(itinStatistic), _dc910(nullptr), _statistic(trx), _matrixInserter(nullptr)
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic910)
  {
    _dc910 = dynamic_cast<Diag910Collector*>(DCFactory::instance()->create(_trx));
    _dc910->initFosDiagnostic();
  }
}

bool
FosBoss::isPaxTypeApplicable(const SopCombination& sopCombination) const
{
  if (!ShoppingUtil::isJCBMatched(_trx))
    return true;

  // JCB pax type. Only valid for carriers in IS_JCB_CARRIER
  CarrierCode cxr = FosCommonUtil::detectCarrier(_trx, sopCombination);
  if (cxr == FosCommonUtil::INTERLINE_CARRIER)
    return false;

  if (TrxUtil::isJcbCarrier(&cxr))
    return true;

  return false;
}

void
FosBoss::processCombination(FosBaseGenerator& fosGenerator, const SopIdVec& comb)
{
  TSELatencyData metrics(_trx, "FOS BOSS PROCSESS COMBINATION");

  if (!fallback::fallbackNGSJCBPaxTuning(&_trx) && !isPaxTypeApplicable(comb))
    return;

  ValidatorBitMask validBitMask = 0;
  ValidatorBitMask deferredBitMask = 0;
  ValidatorBitMask invalidSopDetails = 0;

  _validatorComposite.validate(comb, validBitMask, deferredBitMask, invalidSopDetails);

  if (_dc910)
  {
    _dc910->printTracedFosStatus(_trx, comb, validBitMask, deferredBitMask, invalidSopDetails);
  }

  if (_validatorComposite.isThrowAway(comb, validBitMask))
  {
    if (UNLIKELY(_dc910))
    {
      _dc910->printTracedFosThrownAway(_trx, comb);
    }
    return;
  }

  if (!validBitMask && !deferredBitMask && invalidSopDetails)
  {
    fosGenerator.addCombinationForReuse(comb);
    return;
  }

  if (validBitMask)
  {
    _matrixInserter->addCombination(validBitMask, comb);
    _statistic.addFOS(validBitMask, comb);
    updateItinStatistic(comb);
  }
  else if (deferredBitMask)
  {
    CarrierCode cxr = FosCommonUtil::detectCarrier(_trx, comb);
    if (LIKELY(deferredBitMask & validatorBitMask(VALIDATOR_ONLINE)))
      _deferredOnlines[cxr].push_back(comb);
    else if (deferredBitMask & validatorBitMask(VALIDATOR_NONSTOP))
      _deferredDirects[cxr].push_back(comb);
  }
}

struct NumCarrierCombinations
{
  CarrierCode _cxr;
  uint32_t _num;

  NumCarrierCombinations(const CarrierCode& cxr, uint32_t num) : _cxr(cxr), _num(num) {}

  bool operator<(const NumCarrierCombinations& numCxrCombs) const
  {
    if (_cxr == FosCommonUtil::INTERLINE_CARRIER ||
        numCxrCombs._cxr == FosCommonUtil::INTERLINE_CARRIER)
    {
      return _cxr != FosCommonUtil::INTERLINE_CARRIER;
    }

    return _num < numCxrCombs._num;
  }
};

void
FosBoss::processDeferredCombinations(ValidatorType vt, const DeferredCombs& combinations)
{
  TSELatencyData metrics(_trx, "FOS BOSS PROCSES DEFERRED COMBINATIONS");

  typedef std::vector<NumCarrierCombinations> NumCarrierCombinationsVec;
  typedef NumCarrierCombinationsVec::const_iterator NumCarrierCombinationsCI;

  uint32_t counter = _statistic.getCounter(vt);
  uint32_t limit = _statistic.getCounterLimit(vt);
  if (counter >= limit)
    return;

  NumCarrierCombinationsVec overfilledCarriers;
  for (const auto& combination : combinations)
    overfilledCarriers.push_back(
        NumCarrierCombinations(combination.first, combination.second.size()));
  std::sort(overfilledCarriers.begin(), overfilledCarriers.end());

  uint32_t numLackingCombs = limit - counter;
  uint32_t numCarriersToProcess = overfilledCarriers.size();

  for (NumCarrierCombinationsCI it = overfilledCarriers.begin(); it != overfilledCarriers.end();
       ++it, --numCarriersToProcess)
  {
    const NumCarrierCombinations& numCxrCombs = *it;
    const std::vector<SopIdVec>& carrierCombinations = combinations.find(numCxrCombs._cxr)->second;

    size_t numCombsForCxrRequired = numLackingCombs / numCarriersToProcess;

    for (size_t i = 0; i < carrierCombinations.size() && numCombsForCxrRequired > 0; ++i)
    {
      const SopIdVec& combination = carrierCombinations[i];
      if (_validatorComposite.isThrowAway(combination, validatorBitMask(vt)))
      {
        if (_dc910)
        {
          _dc910->printTracedFosThrownAway(_trx, combination);
        }
        continue;
      }

      _matrixInserter->addCombination(validatorBitMask(vt), combination);
      _statistic.addFOS(validatorBitMask(vt), combination);
      updateItinStatistic(combination);
      --numCombsForCxrRequired;
      --numLackingCombs;
    }
  }
}

void
FosBoss::updateItinStatistic(const SopIdVec& comb)
{
  if (_trx.getRequest()->isBrandedFaresRequest() && _itinStatistic)
  {
    // update statistics with newly created Fos
    _itinStatistic->addFOS(comb);
    // ann new option and mark it as not fulfilling any IBF bucket ( hence 0 )
    _itinStatistic->addToBucketMatchingVec(std::make_pair(comb, 0));
  }
}

} // fos
} // tse
