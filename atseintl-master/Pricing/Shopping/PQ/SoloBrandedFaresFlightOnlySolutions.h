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

#include "Common/TsePrimitiveTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Diversity/DmcScheduleRepeatLimitRequirement.h"
#include "Pricing/Shopping/IBF/IbfDiag910Collector.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SoloFlightOnlySolutionsDataTypes.h"
#include "Pricing/Shopping/PQ/SoloFlightOnlySolutionsValidators.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

namespace tse
{
class ItinStatistic;
class ShoppingTrx;
}

namespace tse
{
namespace fos
{

const shpq::SopIdxVec INVALID_COMBINATION = shpq::SopIdxVec(0);
const size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

using Predicate = utils::INamedPredicate<utils::SopCombination>;
using CombinationWithPredicate = std::pair<utils::SopCombination, Predicate*>;
using CombinationsWithPredicateVec = std::vector<CombinationWithPredicate>;

//*********************************************************************************************************
class MissingCombinationsIterator
{
public:
  MissingCombinationsIterator(int bucketSatisfaction) : _bucketSatisfaction(bucketSatisfaction) {}
  virtual ~MissingCombinationsIterator() = default;

  virtual void reset() = 0;
  virtual bool moveNext() = 0;
  virtual int getCurrentOptionSatisfaction() { return _bucketSatisfaction; }
  const CombinationsWithPredicateVec& getFailedCombinations() const { return _failedCombinations; }
  void setCollectFailedCombinations(bool value) { _collectFailedCombinations = value; }
  SopsCombination getCurrentCombination() { return _currentCombination; }

protected:
  bool isCombinationValid(SopsCombination combination);

  const int _bucketSatisfaction;
  std::vector<Predicate*> _validators;
  CombinationsWithPredicateVec _failedCombinations;
  bool _collectFailedCombinations = false;
  shpq::SopIdxVec _currentCombination;
};

//*********************************************************************************************************

class MissingRCOnlinesIterator : public MissingCombinationsIterator
{
public:
  MissingRCOnlinesIterator(ShoppingTrx& trx, ItinStatistic& stats);
  void reset() override;
  bool moveNext() override;

private:
  bool _isReset;
  DmcScheduleRepeatLimitRequirement _scheduleRepeatLimit;
  ItinStatistic::UnusedRCOnlineCombs::iterator _unusedRCOIterator;
  ItinStatistic::UnusedRCOnlineCombs _unusedRCOnlineCombinations;
};

//*********************************************************************************************************

class MissingDirectsIterator : public MissingCombinationsIterator
{
  public:
    MissingDirectsIterator(ShoppingTrx& trx, ItinStatistic& stats);
    void reset() override;
    bool moveNext() override;

  private:
    bool _isReset;
    DmcScheduleRepeatLimitRequirement _scheduleRepeatLimit;
    ItinStatistic::UnusedDirectCombinations::iterator _unusedDirectsIterator;
    ItinStatistic::UnusedDirectCombinations _unusedDirectCombinations;
};

//*********************************************************************************************************

class MissingFlightsIterator : public MissingCombinationsIterator
{
public:
  enum class Strategy : uint8_t
  {
    CHEAPEST_SOLUTIONS,
    SMALLEST_RESULT_SET,
  };
  MissingFlightsIterator(ShoppingTrx& trx,
                         const ItinStatistic::UnusedSopIdsPerLeg& unusedSopIdsPerLeg,
                         Strategy strategy = Strategy::SMALLEST_RESULT_SET);
  void reset() override;
  bool moveNext() override;

private:
  class IsUnusedSop
  {
  public:
    IsUnusedSop(const ItinStatistic::UnusedSopIds& unusedSopIds) : _unusedSopIds(unusedSopIds) {}
    bool operator()(size_t sopId) { return _unusedSopIds.find(sopId) != _unusedSopIds.end(); }

  private:
    const ItinStatistic::UnusedSopIds _unusedSopIds;
  };
  bool isFinished();
  size_t nextSop(size_t sopIdx);
  void prepareOtherLegSops(size_t legIdx, std::vector<size_t>& sops);
  void updateCurrentCombination();

  size_t _currentLeg = 0;
  size_t _legsCount = 0;
  size_t _currentSop = 0;
  std::tr1::array<size_t, 2> _sopsCountPerLeg;
  ItinStatistic::UnusedSopIdsPerLeg _unusedSopIdsPerLeg;
  Strategy _strategy = Strategy::SMALLEST_RESULT_SET;
};

//*********************************************************************************************************

class SopOccurenceComparator
{
public:
  SopOccurenceComparator(ItinStatistic& stats) : _stats(stats) {};
  bool operator()(const shpq::SopIdxVec& comb1, const shpq::SopIdxVec& comb2);

private:
  ItinStatistic& _stats;
};

//*********************************************************************************************************

class MissingOptionsIterator : public MissingCombinationsIterator
{
public:
  MissingOptionsIterator(ShoppingTrx& trx, ItinStatistic& stats);
  void reset() override;
  bool moveNext() override;

private:
  bool _isReset = false;
  std::vector<shpq::SopIdxVec> _unusedCombinations;
  SopOccurenceComparator _comparator;
};

//*********************************************************************************************************

class SoloBrandedFaresFlightOnlySolutions
{
public:
  SoloBrandedFaresFlightOnlySolutions(ShoppingTrx& trx, ItinStatistic& stat);
  void operator()(MissingCombinationsIterator& it,
                  size_t numOfOptionsNeeded = std::numeric_limits<size_t>::max());

private:
  ShoppingTrx& _trx;
  ItinStatistic& _stats;
};

} // ns fos
} // ns tse

