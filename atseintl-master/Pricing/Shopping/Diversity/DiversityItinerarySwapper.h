#pragma once

#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/Diversity/SwapperTypes.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

namespace tse
{

class Diag941Collector;
class DiagCollector;
class Diversity;
class ShoppingTrx;
class SwapperValidationStrategy;
class SwapperView;

class DiversityItinerarySwapper
{
public:
  DiversityItinerarySwapper(ShoppingTrx& trx,
                            ItinStatistic& stats,
                            DiversityModel* model,
                            DiagCollector* dc);
  void swapSolution(const ShoppingTrx::FlightMatrix::value_type& solution, size_t farePathKey);

  // returns true if any solution has been removed
  bool swapAdditionalNS(const ShoppingTrx::FlightMatrix::value_type& solution);

private:
  float calcTODDistance(size_t decreasedBkt, size_t increasedBkt) const;
  double calcSolutionScore(Diversity::BucketType bucket,
                           MoneyAmount price,
                           const ShoppingTrx::SchedulingOption* outbound,
                           const ShoppingTrx::SchedulingOption* inbound) const;
  void removeSolution(Diversity::BucketType bucket,
                      size_t pairing,
                      const ItinStatistic::Solution* sol,
                      const ShoppingTrx::SchedulingOption* outbound,
                      const ShoppingTrx::SchedulingOption* inbound);

  void printSwapSolution(size_t fpKey,
                         const std::vector<int>& sopVec,
                         const CarrierCode& cxr,
                         SopCombinationUtil::NonStopType isNonStop,
                         Diversity::BucketType bucket,
                         size_t todBucket) const;
  void printStartBucket(size_t fpKey, Diversity::BucketType bucket, int bktStatus) const;
  void printRemoveSolution(const std::vector<int>& sopVec, Diversity::BucketType bucket) const;
  void printNoSolutionForRemoval() const;
  void printSolutionEvaluation(size_t fpKey,
                               const std::vector<int>& sopVec,
                               size_t pairing,
                               size_t newTODBucket,
                               Diversity::BucketType bucket,
                               MoneyAmount price,
                               SwapperEvaluationResult result) const;
  void printSopCombination(const std::vector<int>& sopVec) const;
  const char* bucketToString(const Diversity::BucketType bucket) const;

  // Swap additional non-stop - auxiliary methods:
  // Get solution with the greatest score if it's greater or equal to minScore
  bool getMaxScoredSolution(const ItinStatistic::ScoredCombinations& sc,
                            size_t minScore,
                            const ItinStatistic::Solution** worstSolution,
                            size_t* worstScore);

  void removeAdditionalNS(Diversity::NSBucketType bucket,
                          size_t pairing,
                          const ItinStatistic::Solution* solution);

  void printNSRemoved(const std::vector<int>& sopVec);
  void printAdditionalNSRemoved(const std::vector<int>& sopVec);
  void printAdditionalNSAdded(const std::vector<int>& sopVec);

private:
  ShoppingTrx& _trx;
  DiagCollector* _dc;
  Diag941Collector* _dc941;
  const DiversityModel* _model;
  ItinStatistic& _stats;
  ShoppingTrx::FlightMatrix& _flightMatrix;
  const Diversity& _diversity;

  SwapperValidationStrategy* _validationStrategy;
  SwapperView* _swapperView;

  size_t _fpKey;
  bool _diagSwapper;
  bool _diagNonStops;
  bool _isBrandedFaresPath;
};
}
