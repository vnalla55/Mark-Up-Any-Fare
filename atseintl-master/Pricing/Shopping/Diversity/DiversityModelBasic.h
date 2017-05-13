// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
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

#include "Pricing/Shopping/Diversity/DiversityItinerarySwapper.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"

#include <vector>

namespace tse
{
class DmcRequirementsFacade;
class ItinStatistic;
class Logger;
class Diag941Collector;

namespace shpq
{
class DiversityModelBasic_AdditionalNonStopsTest;
}

class DiversityModelBasic : public DiversityModel
{
public:
  struct BucketStatus
  {
    int goldStatus = 0;
    int uglyStatus = 0;
    int luxuryStatus = 0;
    int junkStatus = 0;
  };

  friend class shpq::DiversityModelBasic_AdditionalNonStopsTest;

  DiversityModelBasic(ShoppingTrx& trx, ItinStatistic& stats, DiagCollector* dc);

  PQItemAction getPQItemAction(const shpq::SoloPQItem* pqItem) override;

  SOPCombinationList::iterator
  getDesiredSOPCombination(SOPCombinationList& combinations, MoneyAmount score, size_t fpKey) override;

  bool getIsNewCombinationDesired(const SOPCombination& combination, MoneyAmount score) override;

  bool isNonStopNeededOnly() override;
  bool isNonStopNeededOnlyFrom(const shpq::SoloPQItem* pqItem) override;
  bool shouldPerformNSPreValidation(const shpq::SoloPQItem* pqItem) override;

  bool addSolution(const ShoppingTrx::FlightMatrix::value_type& solution,
                   ShoppingTrx::FlightMatrix& flightMatrix,
                   size_t farePathKey,
                   const DatePair* datePair) override;

  int getBucketStatus(const Diversity::BucketType bucket) const override;
  bool isNonStopOptionNeeded() const override;
  bool isAdditionalNonStopEnabled() const override;
  bool isAdditionalNonStopOptionNeeded() const override;
  void printSolutions() const override;
  void printSummary() const override;
  void handlePQStop() override;
  void handlePQHurryOutCondition() override;

  virtual std::unique_ptr<SOPInfos> filterSolutionsPerLeg(MoneyAmount score,
                                                          const SOPInfos& sopInfos,
                                                          SOPInfos* thrownSopInfos) override;

private:
  // Initialize stuff
  void initializeDiagnostic();
  void initializeStatistic();

  // Setup parameters stuff
  class FareCutoffCalcMethod;
  int32_t calcTravelTimeSeparator() const;
  void setupParameters();
  static bool getFareCutoffMultiplier(const MoneyAmount minPrice, uint16_t& multiplier);

  static bool getFareCutoffMultiplierExpPrefCxr(const MoneyAmount minPrice, uint16_t& multiplier);

  // Continue processing stuff
  bool continueProcessing(const MoneyAmount pqScore);
  bool setFareCutoffReachedGetNeedToContinue();
  void setFareCutoffReached(const bool isOldScore = true);
  int getCurrentStatus();
  void updateStatus();

  // getDesiredSOPCombination stuff
  SOPCombinationList::iterator getDesiredSOPCombinationImpl(SOPCombinationList& combinations,
                                                            MoneyAmount score,
                                                            size_t fpKey);

  enum UseCombinations
  {
    USE_NONE,
    USE_ALL,
    USE_SOME
  };

  UseCombinations checkEvaluateNewCombinationPrerequisites(MoneyAmount score);

  void preFilterSOPCombinationList(SOPCombinationList& combinations);

  // Non-stop stuff
  bool canProduceMoreNonStop() const;
  void updateNSParametersIfNeeded();
  void updateNSParametersUnconditionally();
  bool isAnyBucketChanged();

  void printPQItemAction(DiversityModel::PQItemAction result,
                         const shpq::SoloPQItem* pqItem,
                         int couldSatisfy) const;
  void printDesiredSOPCombination(size_t fpKey, MoneyAmount price, size_t combCount) const;
  void printSOPCombinationEvaluation(size_t fpKey,
                                     const DiversityModel::SOPCombination& comb,
                                     bool isSelected,
                                     int couldSatisfy,
                                     MoneyAmount price) const;
  void printAdditionalNonStopAdded(const ShoppingTrx::FlightMatrix::value_type&) const;
  void printContinueProcessing(const bool result,
      const bool fromSetFareCutoffReached = false,
      const bool carrierOptionNotMetButFarecutoffMet = false,
      const MoneyAmount pqScore = 0);
  void printDiscontinueProcessing();
  void printParameters(const FareCutoffCalcMethod& fareCutoff) const;
  void printStatusChanged(int oldStatus) const;

  // diagnostic 941
  void print941NSAdded(const std::vector<int>& comb) const;
  void print941AdditionalNSAdded(const std::vector<int>& comb) const;

  const ShoppingTrx& _trx;
  ItinStatistic& _stats;
  DiagCollector* _dc = nullptr;
  Diag941Collector* _dc941 = nullptr;
  Diversity& _diversityParams;
  DmcRequirementsFacade* _requirements = nullptr;
  DiversityItinerarySwapper _swapper;
  const size_t _numberOfSolutionsRequired;
  bool _isParametersSet = false;
  bool _isFareCutoffReached = false;
  int _currentStatus = 0;
  bool _diagNS = false;
  bool _diagPQ = false;
  bool _diagSOP = false;
  bool _diagSolutions = false;
  size_t _fpKey = 0;
  bool _newFarePath = true;
  bool _allSpecifiedCarrierOptionsMet = false;
  BucketStatus _lastBucketStatus;
  uint32_t _discontinueProcessingValue = 1000000;
  uint32_t _continueProcessingCount = 0;
  static Logger _logger;
};
}

