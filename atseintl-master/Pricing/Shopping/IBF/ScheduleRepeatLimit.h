//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TNBrands/TNBrandsUtils.h"
#include "Pricing/Shopping/IBF/ISopUsageTracker.h"
#include "Pricing/Shopping/IBF/ScheduleRepeatLimitRatingPolicy.h"
#include "Pricing/Shopping/IBF/SopCountingAppraiser.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"
#include "Pricing/Shopping/Swapper/IAppraiser.h"
#include "Pricing/Shopping/Swapper/IObservableItemSet.h"
#include "Pricing/Shopping/Utils/ILogger.h"
#include "Pricing/Shopping/Utils/LoggerHandle.h"
#include "Pricing/Shopping/Utils/PrettyPrint.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include <memory>
#include <sstream>
#include <vector>

namespace tse
{

// Makes the coefficient of variation threshold grow as the number of swapper
// iterations grows.
// For a small number of iterations (e.g. < 300), we have a very small COV
// threshold (e.g. < 300 * 0.0004 = 0.12). Only very "balanced" usages of SOPs
// will produce COV <= 0.12. Since we have done very little computation yet, we
// are very strict about the results: we still have time to produce more
// balanced SOP usage. Example passing usage sequence: cov([6 8 7 7]) ~= 0.10

// For a moderate number of iterations (e.g. 1000), we produce a reasonable COV
// threshold (e.g. 1000 * 0.0004 = 0.4). Such SOP usages look "nice" which makes
// sensible limit for the most of computations. Example passing usage sequence:
// cov([3 6 9 4 5]) ~= 0.38

// For a big number of iterations (e.g. 5000) we really want to hurry since we
// are very permissive in terms of the COV threshold (e.g. 5000 * 0.0004 = 2).
// Example passing usage sequence: cov([1 5 400]) ~= 1.38
struct BasicCovThresholdCalculationPolicy
{
  double operator()(size_t swapperIterationsSoFar) const
  {
    return COV_THRESHOLD_PER_ITERATIONS * swapperIterationsSoFar;
  }

  const double COV_THRESHOLD_PER_ITERATIONS = 0.0004;
};



template <typename CoefficientOfVariationF = utils::CoefficientOfVariation,
          typename CovThresholdCalculationF = BasicCovThresholdCalculationPolicy>
class ScheduleRepeatLimitT :
    public swp::IAppraiser<utils::SopCombination, swp::BasicAppraiserScore>,
    boost::noncopyable
{
public:

  ScheduleRepeatLimitT(unsigned int limit, unsigned int requestedSolutionsCount,
      ISopUsageCounter& sopUsageCounter, const swp::ISwapperInfo& swapperInfo,
      utils::ILogger* logger = nullptr, CoefficientOfVariationF* covFunc = 0,
      CovThresholdCalculationF* thresholdFunc = 0)
    : _policy(limit), _appraiser(_policy),
      _requestedSolutionsCount(requestedSolutionsCount),
      _sopUsageCounter(sopUsageCounter),
      _swapperInfo(swapperInfo),
      _logger(logger)
  {
    TSE_ASSERT(requestedSolutionsCount > 0);
    skipper::assignValidObject(covFunc, _covFunc);
    skipper::assignValidObject(thresholdFunc, _thresholdFunc);
  }

  // Report on the number of SOPs on a given leg. This will let the appraiser
  // determine if SRL for a given leg is feasible (which happens only if
  // SOPcount * SRL >= requestedNumberOfSolutions). If SRL for a given leg
  // is infeasible, this appraiser only ensures that the standard
  // deviation of the SOP counts in the result set is small enough.
  void reportSopCount(unsigned int legId, unsigned int count)
  {
    if (_logger)
    {
      _logger->info(utils::Fmt("Reported %u SOPs on leg %u") % count % legId);
    }

    if (count * getLimit() < _requestedSolutionsCount)
    {
      if (_logger)
      {
        _logger->info(utils::Fmt("[SOP count = %u] X [SRL = %u] "
                                 "less than [Q0S = %u]. SRL infeasible on this leg.") %
                      count % getLimit() % _requestedSolutionsCount);
      }
      _policy.addIgnoredLeg(legId);
    }
  }

  // Throws on item duplicate
  swp::BasicAppraiserScore
  beforeItemAdded(const utils::SopCombination& item,
                  ImplIScoreBlackboard& blackboard) override
  {
    return _appraiser.beforeItemAdded(item, blackboard);
  }

  // Throws if removed item does not exist
  void beforeItemRemoved(const utils::SopCombination& item,
                         ImplIScoreBlackboard& blackboard) override
  {
    _appraiser.beforeItemRemoved(item, blackboard);
  }

  bool isSatisfied() const override
  {
    if (!_appraiser.isSatisfied())
    {
      return false;
    }

    return isSopCountBalancedOnSrlInfeasibleLegs();
  }

  std::string toString() const override
  {
    std::ostringstream out;
    out << "Schedule repeat limit = " << getLimit();
    return out.str();
  }

  unsigned int getLimit() const { return _policy.getLimit(); }

private:

  bool isSopCountBalancedOnSrlInfeasibleLegs() const
  {
    if (_policy.getIgnoredLegs().empty())
    {
      return true;
    }

    if (_logger)
    {
      _logger->info("SOP USAGE BALANCE CHECK");
    }

    for (const auto& elem : _policy.getIgnoredLegs())
    {
      if (!isSopUsageBalancedOnLeg(elem))
      {
        return false;
      }
    }
    return true;
  }

  bool isSopUsageBalancedOnLeg(unsigned int legId) const
  {
    const ISopUsageCounter::SopUsages usagesPerSop =
        _sopUsageCounter.getSopUsagesOnLeg(legId);

    if (usagesPerSop.empty())
    {
      if (_logger)
      {
        _logger->info(utils::Fmt("No SOP usages on leg %u. Usage balanced.") % legId);
      }
      return true;
    }

    if (usagesPerSop.size() == 1)
    {
      if (_logger)
      {
        _logger->info(utils::Fmt("Only one SOP used on leg %u. Usage balanced.") % legId);
      }
      return true;
    }

    std::vector<unsigned int> rawUsages;
    for (const auto& elem : usagesPerSop)
    {
      rawUsages.push_back(elem.second);
    }

    const double threshold = (*_thresholdFunc)(
        _swapperInfo.getTotalIterationsCount());
    const double cov = (*_covFunc)(rawUsages);
    const bool balanced = (cov < threshold);

    if (_logger && (_logger->enabled(utils::LOGGER_LEVEL::INFO)))
    {
      std::ostringstream out;
      out << "Leg id: " << legId << std::endl;
      out << "SOP usages: " << utils::toStr(usagesPerSop) << std::endl;
      out << "Iterations so far: " <<
          _swapperInfo.getTotalIterationsCount() << std::endl;
      out << "COV threshold: " << threshold << std::endl;
      out << "coefficient of variation: " << cov << std::endl;
      out << "balanced?: " << (balanced ? "true" : "false") << std::endl;
      _logger->info(out.str());
    }
    return balanced;
  }

  ScheduleRepeatLimitRatingPolicy _policy;
  SopCountingAppraiser<ScheduleRepeatLimitRatingPolicy> _appraiser;
  unsigned int _requestedSolutionsCount;
  ISopUsageCounter& _sopUsageCounter;
  const swp::ISwapperInfo& _swapperInfo;
  utils::ILogger* _logger;
  std::shared_ptr<CoefficientOfVariationF> _covFunc;
  std::shared_ptr<CovThresholdCalculationF> _thresholdFunc;
};

typedef ScheduleRepeatLimitT<> ScheduleRepeatLimit;

} // namespace tse


