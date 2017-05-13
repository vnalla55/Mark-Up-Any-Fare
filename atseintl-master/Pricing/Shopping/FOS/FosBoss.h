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
#pragma once

#include "Common/TSELatencyData.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/Diag910Collector.h"
#include "Pricing/Shopping/FOS/FosBaseGenerator.h"
#include "Pricing/Shopping/FOS/FosCompositeBuilder.h"
#include "Pricing/Shopping/FOS/FosFilterComposite.h"
#include "Pricing/Shopping/FOS/FosStatistic.h"
#include "Pricing/Shopping/FOS/FosValidatorComposite.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"

#include <map>

namespace tse
{

namespace fos
{
class FosTaskScope;
class FosMatrixInserter;

class FosBoss
{
public:
  FosBoss(ShoppingTrx& trx, ItinStatistic* itinStatistic = nullptr);

  // GeneratorClass must implement the interface of FosBaseGenerator
  template <class GeneratorClass>
  void process(const FosTaskScope& task)
  {
    TSELatencyData metrics(_trx, "FOS BOSS PROCESS");

    if (_dc910)
    {
      _dc910->printFosProcessing(task);
    }

    FosCompositeBuilder fosCompositeBuilder(_trx, _dc910);
    FosFilterComposite fosFilterComposite(_statistic);
    GeneratorClass fosGenerator(_trx, fosFilterComposite, _dc910);
    fosGenerator.initGenerators();
    fosGenerator.addPredicates();

    fosCompositeBuilder.buildFosFilterComposite(task, fosFilterComposite);
    fosCompositeBuilder.buildFosValidatorComposite(
        task, fosGenerator, _statistic, _validatorComposite);
    _matrixInserter = fosCompositeBuilder.createFosMatrixInserter(task);

    TSE_ASSERT(_matrixInserter);

    SopIdVec comb;
    while (_statistic.getLackingValidators() &&
           fosGenerator.getNextCombination(_statistic.getLackingValidators(), comb))
    {
      processCombination(fosGenerator, comb);
    }

    if (_dc910)
    {
      _dc910->printFosDeferredProcessing(_statistic, fosGenerator.getStats());
    }

    processDeferredCombinations(VALIDATOR_ONLINE, _deferredOnlines);
    processDeferredCombinations(VALIDATOR_NONSTOP, _deferredDirects);

    if (_dc910)
    {
      _dc910->printFosProcessingFinished(_statistic);
      _dc910->flushMsg();
    }
  }

private:
  typedef std::map<CarrierCode, std::vector<SopIdVec> > DeferredCombs;

  void processCombination(FosBaseGenerator& fosGenerator, const SopIdVec& comb);
  void processDeferredCombinations(ValidatorType vt, const DeferredCombs& combinations);
  void updateItinStatistic(const SopIdVec& comb);
  bool isPaxTypeApplicable(const SopCombination& sopCombination) const;

  ShoppingTrx& _trx;
  ItinStatistic* const _itinStatistic;

  Diag910Collector* _dc910;

  FosStatistic _statistic;
  FosValidatorComposite _validatorComposite;
  FosMatrixInserter* _matrixInserter;

  DeferredCombs _deferredDirects;
  DeferredCombs _deferredOnlines;
};

} // fos
} // tse
