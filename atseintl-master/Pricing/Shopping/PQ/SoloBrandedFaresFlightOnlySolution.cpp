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

#include "Diagnostic/DiagManager.h"
#include "Pricing/Shopping/Diversity/DiversityModelBasic.h"
#include "Pricing/Shopping/Diversity/DmcBucketRequirement.h"
#include "Pricing/Shopping/Diversity/DmcCarrierRequirement.h"
#include "Pricing/Shopping/IBF/IbfDiag910Collector.h"
#include "Pricing/Shopping/PQ/SoloBrandedFaresFlightOnlySolutions.h"
#include "Pricing/Shopping/Predicates/CabinClassValidity.h"
#include "Pricing/Shopping/Predicates/FosPredicatesFactory.h"
#include "Pricing/Shopping/Predicates/IFosPredicatesFactory.h"
#include "Pricing/Shopping/Predicates/InterlineTicketingAgreement.h"
#include "Pricing/Shopping/Predicates/MinimumConnectTime.h"

#include <boost/iterator/counting_iterator.hpp>

#include <algorithm>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <tr1/array>

namespace tse
{

namespace fos
{

bool
MissingCombinationsIterator::isCombinationValid(SopsCombination combination)
{
  bool isCombinationValid = true;
  for (Predicate* predicate : _validators)
  {
    if (!((*predicate)(combination)))
    {
      isCombinationValid = false;
      if (_collectFailedCombinations)
        _failedCombinations.push_back(std::make_pair(combination, predicate));

      break;
    }
  }

  return isCombinationValid;
}

// The following class takes a list(s) of missing SOPs for each leg
// ang generates flight combinations containing those flights.
// There are two strategies for doing that:
// - generating possible cheapest solution (based on DSS estimation)
// - generating possible smallest result set (using missing SOPs
//   on both legs regardless it's expected to be cheapest or not)
//
// Example:
// We have 3 missing oboutbound SOPs: [3, 7, 8]
// and 2 missing inbound SOPs: [7, 9].
// We also assume that all possible SOP combinations are valid.
//
// CHEAPEST_SOLUTIONS strategy will take each missing SOP for given
// leg and combine it with cheapest SOP from other leg.
// In other words, for each missing SOP it iterates all SOPs
// from other leg until it finds valid SOP combination.
// As a result we will get 5 combinations:
//   [3, 1], [7, 1], [8, 1], [1, 7], [1, 9]
//
// SMALLEST_RESULT_SET strategy will use missing SOPs from both legs
// if it is possible. If not it will try to combine SOP with previously
// used SOPs from other leg. If a SOP is used in a combination
// it is removed from set of unused SOPs.
// As a result we get 3 combinations:
//   [3, 7], [7, 9], [8, 1]
// In the last combination we have used SOP 1 which was not initially
// present in the set of missing SOPs but as long as all SOPs have been
// used we're taking the cheapest one.

MissingRCOnlinesIterator::MissingRCOnlinesIterator(ShoppingTrx& trx, ItinStatistic& stats)
  : MissingCombinationsIterator(DmcRequirement::NEED_RC_ONLINES),
    _isReset(false),
    _scheduleRepeatLimit(stats, trx.getRequest()->getScheduleRepeatLimit()),
    _unusedRCOnlineCombinations(stats.getUnusedRCOnlineCombs())
{
  utils::FosPredicatesFactory factory(trx);

  _validators.push_back(&_scheduleRepeatLimit);
  _validators.push_back(factory.createFosPredicate(tse::utils::CABIN_CLASS_VALIDITY));
  _validators.push_back(factory.createFosPredicate(tse::utils::MINIMUM_CONNECT_TIME));
  _validators.push_back(factory.createFosPredicate(tse::utils::INTERLINE_TICKETING_AGREEMENT));

  reset();
}

void
MissingRCOnlinesIterator::reset()
{
  _currentCombination = INVALID_COMBINATION;
  _unusedRCOIterator = _unusedRCOnlineCombinations.begin();
  _isReset = true;
  _failedCombinations.clear();
}

bool
MissingRCOnlinesIterator::moveNext()
{
  if (_unusedRCOIterator == _unusedRCOnlineCombinations.end())
    return false;

  do // find next valid combination
  {
    if (_isReset)
    {
      _isReset = false;
    }
    else
    {
      ++_unusedRCOIterator;
    }
  } while (_unusedRCOIterator != _unusedRCOnlineCombinations.end() &&
           !isCombinationValid(*_unusedRCOIterator));

  if (_unusedRCOIterator != _unusedRCOnlineCombinations.end())
  {
    _currentCombination = *_unusedRCOIterator;
    return true;
  }
  else
  {
    return false;
  }
}

//************************************************************************************
  MissingDirectsIterator::MissingDirectsIterator(ShoppingTrx& trx, ItinStatistic& stats)
: MissingCombinationsIterator(DmcRequirement::NEED_IBF_DIRECTS),
  _isReset(false),
  _scheduleRepeatLimit(stats, trx.getRequest()->getScheduleRepeatLimit()),
  _unusedDirectCombinations(stats.getUnusedDirectCombinations())
{
  utils::FosPredicatesFactory factory(trx);

  _validators.push_back(&_scheduleRepeatLimit);
  _validators.push_back(factory.createFosPredicate(tse::utils::CABIN_CLASS_VALIDITY));
  _validators.push_back(factory.createFosPredicate(tse::utils::MINIMUM_CONNECT_TIME));
  _validators.push_back(factory.createFosPredicate(tse::utils::INTERLINE_TICKETING_AGREEMENT));

  reset();
}

  void
MissingDirectsIterator::reset()
{
  _currentCombination = INVALID_COMBINATION;
  _unusedDirectsIterator = _unusedDirectCombinations.begin();
  _isReset = true;
  _failedCombinations.clear();
}

  bool
MissingDirectsIterator::moveNext()
{
  if (_unusedDirectsIterator == _unusedDirectCombinations.end())
    return false;

  do // find next valid combination
  {
    if (_isReset)
    {
      _isReset = false;
    }
    else
    {
      ++_unusedDirectsIterator;
    }
  } while (_unusedDirectsIterator != _unusedDirectCombinations.end() &&
      !isCombinationValid(*_unusedDirectsIterator));

  if (_unusedDirectsIterator != _unusedDirectCombinations.end())
  {
    _currentCombination = *_unusedDirectsIterator;
    return true;
  }
  else
  {
    return false;
  }
}

//************************************************************************************

MissingFlightsIterator::MissingFlightsIterator(
    ShoppingTrx& trx,
    const ItinStatistic::UnusedSopIdsPerLeg& unusedSopIdsPerLeg,
    Strategy strategy)
  : MissingCombinationsIterator(DmcRequirement::NEED_INBOUNDS | DmcRequirement::NEED_OUTBOUNDS),
    _legsCount(trx.legs().size()),
    _unusedSopIdsPerLeg(unusedSopIdsPerLeg),
    _strategy(strategy)
{
  _sopsCountPerLeg[0] = trx.legs()[0].requestSops();
  _sopsCountPerLeg[1] = (_legsCount > 1) ? trx.legs()[1].requestSops() : 0;

  utils::FosPredicatesFactory factory(trx);
  _validators.push_back(factory.createFosPredicate(tse::utils::CABIN_CLASS_VALIDITY));
  _validators.push_back(factory.createFosPredicate(tse::utils::MINIMUM_CONNECT_TIME));
  _validators.push_back(factory.createFosPredicate(tse::utils::INTERLINE_TICKETING_AGREEMENT));

  reset();
}

void
MissingFlightsIterator::reset()
{
  _currentLeg = 0;
  _currentSop = INVALID_INDEX;
  _failedCombinations.clear();
}

bool
MissingFlightsIterator::moveNext()
{
  updateCurrentCombination();
  return !isFinished();
}

bool
MissingFlightsIterator::isFinished()
{
  return _currentLeg >= _legsCount;
}

size_t
MissingFlightsIterator::nextSop(size_t sopIdx)
{
  if (sopIdx == INVALID_INDEX)
    return 0;
  else
  {
    for (size_t i = 0; i < _legsCount; i++)
    {
      ItinStatistic::UnusedSopIds::iterator it =
          _unusedSopIdsPerLeg[i].find(_currentCombination[i]);
      if (it != _unusedSopIdsPerLeg[i].end())
        _unusedSopIdsPerLeg[i].erase(it);
    }
    return sopIdx;
  }
}

void
MissingFlightsIterator::prepareOtherLegSops(size_t legIdx, std::vector<size_t>& sops)
{
  // the only difference between two strategies is that
  // in the smallest result set strategy we move unused SOPs
  // to the beginning so they can be used in solutions as soon as possible
  sops.assign(boost::counting_iterator<size_t>(0),
              boost::counting_iterator<size_t>(_sopsCountPerLeg[legIdx]));
  if (_strategy == Strategy::SMALLEST_RESULT_SET)
  {
    std::vector<size_t>::iterator end_of_unused;
    end_of_unused =
        std::stable_partition(sops.begin(), sops.end(), IsUnusedSop(_unusedSopIdsPerLeg[legIdx]));
    if (legIdx == 0)
    {
      sops.erase(sops.begin(), end_of_unused);
    }
  }
}

void
MissingFlightsIterator::updateCurrentCombination()
{
  while (_currentLeg < _legsCount)
  {
    size_t otherLegIdx = 1 - _currentLeg;
    _currentSop = nextSop(_currentSop);
    std::vector<size_t> unusedSopIds(_unusedSopIdsPerLeg[_currentLeg].begin(),
                                     _unusedSopIdsPerLeg[_currentLeg].end());
    std::vector<size_t> otherLegSops;
    prepareOtherLegSops(otherLegIdx, otherLegSops);

    while (_currentSop < unusedSopIds.size())
    {
      _currentCombination[_currentLeg] = unusedSopIds[_currentSop];

      if (_legsCount == 1 && isCombinationValid(_currentCombination))
        return; // valid one way combination found

      for (auto& otherLegSop : otherLegSops)
      {
        _currentCombination[otherLegIdx] = otherLegSop;
        if (isCombinationValid(_currentCombination))
          return;
      }
      _currentSop++;
    }
    _currentSop = INVALID_INDEX;
    _currentLeg++;
  }
}

bool
SopOccurenceComparator::
operator()(const shpq::SopIdxVec& comb1, const shpq::SopIdxVec& comb2)
{
  size_t combRepeat1 = 0;
  size_t combRepeat2 = 0;

  for (unsigned int i = 0; i < comb1.size(); ++i)
  {
    combRepeat1 = std::max(combRepeat1, _stats.getSopPairing(i, comb1[i]));
    combRepeat2 = std::max(combRepeat2, _stats.getSopPairing(i, comb2[i]));
  }

  return combRepeat1 > combRepeat2; // sort it so the best candidates are at the end
}

MissingOptionsIterator::MissingOptionsIterator(ShoppingTrx& trx, ItinStatistic& stats)
  : MissingCombinationsIterator(DmcRequirement::NEED_JUNK), _comparator(stats)
{
  std::copy(stats.getUnusedCombinations().begin(),
            stats.getUnusedCombinations().end(),
            std::back_inserter(_unusedCombinations));
  utils::FosPredicatesFactory factory(trx);

  _validators.push_back(factory.createFosPredicate(tse::utils::CABIN_CLASS_VALIDITY));
  _validators.push_back(factory.createFosPredicate(tse::utils::MINIMUM_CONNECT_TIME));
  _validators.push_back(factory.createFosPredicate(tse::utils::INTERLINE_TICKETING_AGREEMENT));

  reset();
}

void
MissingOptionsIterator::reset()
{
  _currentCombination = INVALID_COMBINATION;
  _failedCombinations.clear();
}

bool
MissingOptionsIterator::moveNext()
{
  if (_unusedCombinations.empty())
    return false;
  // sort elements so the best option is at the back
  std::sort(_unusedCombinations.begin(), _unusedCombinations.end(), _comparator);

  while (!_unusedCombinations.empty() && !isCombinationValid(_unusedCombinations.back()))
  {
    _unusedCombinations.pop_back();
  }

  if (!_unusedCombinations.empty())
  {
    _currentCombination = _unusedCombinations.back();
    _unusedCombinations.pop_back();
    return true;
  }
  else
  {
    return false;
  }
}

SoloBrandedFaresFlightOnlySolutions::SoloBrandedFaresFlightOnlySolutions(ShoppingTrx& trx,
                                                                         ItinStatistic& stats)
  : _trx(trx), _stats(stats)
{
}

void
SoloBrandedFaresFlightOnlySolutions::
operator()(MissingCombinationsIterator& it, size_t numOfOptionsNeeded)
{
  std::ostringstream diagMessage;
  diagMessage << "Generating Additional Solutions to fulfill requirement: "
              << DmcRequirement::print(it.getCurrentOptionSatisfaction());

  IbfDiag910Collector diag910collector(_trx, diagMessage.str());
  if (diag910collector.areDetailsFor910Enabled())
    it.setCollectFailedCombinations(true);

  size_t numOfOptionsGenerated = 0;

  while (numOfOptionsGenerated < numOfOptionsNeeded && it.moveNext())
  {
    const ShoppingTrx::FlightMatrix::value_type item(it.getCurrentCombination(), nullptr);
    _trx.flightMatrix().insert(item);
    _stats.addFOS(item.first);
    _stats.addToBucketMatchingVec(std::make_pair(item.first, it.getCurrentOptionSatisfaction()));
    numOfOptionsGenerated++;
    diag910collector.collect(item.first);
  }
  if (diag910collector.areDetailsFor910Enabled())
  {
    CombinationsWithPredicateVec::const_iterator vecIter;
    for (vecIter = it.getFailedCombinations().begin(); vecIter != it.getFailedCombinations().end();
         ++vecIter)
    {
      diag910collector.elementInvalid(vecIter->first, *vecIter->second);
    }
  }
  diag910collector.setShouldPrintFailedSops(false);
  diag910collector.flush();
}

} // ns fos
} // ns tse
