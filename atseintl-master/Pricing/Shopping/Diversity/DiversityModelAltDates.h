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

#include "Common/Logger.h"
#include "Pricing/Shopping/Diversity/ADIsSolutionNeededCheck.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/PQ/AltDatesStatistic.h"

#include <boost/optional.hpp>

#include <set>

namespace tse
{
class DiagCollector;
class ItinStatistic;

class DiversityModelAltDates : public DiversityModel,
                               protected ADIsSolutionNeededCheck::DiversityModelCallback
{
public:
  DiversityModelAltDates(ShoppingTrx& shoppingTrx, ItinStatistic& stats, DiagCollector* dc);

  PQItemAction getPQItemAction(const shpq::SoloPQItem* pqItem) override;

  SOPCombinationList::iterator
  getDesiredSOPCombination(SOPCombinationList& combinations, MoneyAmount score, size_t fpKey) override;

  bool getIsNewCombinationDesired(const SOPCombination& combination, MoneyAmount score) override;

  bool isNonStopNeededOnly() override { return false; }

  bool addSolution(const ShoppingTrx::FlightMatrix::value_type& solution,
                   ShoppingTrx::FlightMatrix& flightMatrix,
                   size_t farePathKey,
                   const DatePair* datePair) override;

  int getBucketStatus(const Diversity::BucketType bucket) const override { return -1; }
  bool isNonStopOptionNeeded() const override { return false; }
  bool isAdditionalNonStopEnabled() const override { return false; }
  bool isAdditionalNonStopOptionNeeded() const override { return false; }
  void printSolutions() const override;
  void removeUnwantedSolutions(ShoppingTrx::FlightMatrix& flightMatrix) override;

protected:
  AltDatesStatistic::Stat
  getStat(const DiversityModel::SOPCombination& comb, MoneyAmount score) const override;
  AltDatesStatistic::Stat
  getStat(const ShoppingTrx::FlightMatrix::value_type& solution, const DatePair& datePair) const override;
  /**
   * @return true if date pair shall be skipped from processing and diagnostic output
   */
  bool checkDatePairFilt(DatePair datePair) const override;
  /**
   * @param datePairFirstFareLevelAmount < 0. means has not initialized yet
   */
  bool
  isDatePairFareCutoffReached(MoneyAmount datePairFirstFareLevelAmount, MoneyAmount price) const override;
  bool isFirstCxrFareLevel(const AltDatesStatistic::Stat&) const override;
  bool isNumOptionNeeded(const AltDatesStatistic::Stat&) const override;

  const ShoppingTrx* getTrx() const override { return &_trx; }

private:
  using CxrSPInfo = AltDatesStatistic::CxrSPInfo;
  using Carrier = AltDatesStatistic::Carrier;
  using IsSolutionNeededCheck = ADIsSolutionNeededCheck;

  bool continueProcessing(MoneyAmount pqScore);

  std::pair<bool, Carrier> skipPQItem(const shpq::SoloPQItem& pqItem) const;
  bool skipPQItemCallback(const AltDatesStatistic::Stat&) const;

  bool isFareCutoffReached(MoneyAmount price) const;
  static bool isFareCutoffReached(MoneyAmount min, MoneyAmount price, float coef);

  boost::optional<CxrSPInfo> getCxrSPInfo(const shpq::SoloPQItem& pqItem) const;

  void printParameters() const;
  void traceDatePairsToFullfill();
  void printContinueProcessing(bool result,
                               bool isMaxFareLevelReached,
                               bool isFareCutoffReached,
                               bool isDatePairFareCutoffReached,
                               MoneyAmount score) const;
  void printSolutionAction(const IsSolutionNeededCheck& details, const char* action);
  void printPQItemAction(DiversityModel::PQItemAction result,
                         const shpq::SoloPQItem& pqItem,
                         const Carrier* = nullptr) const;

  void initializeDiagnostic(const std::string& diagArg);
  void initializeDatePairFilt(const std::string& filtByDatePair);

  AltDatesStatistic::DumpToDiagParam dumpToDiagParam() const;

  bool isTraceSolutionActionEnabled() const { return _diagPQ || _diagSkips || _diagAll; }

  ShoppingTrx& _trx;
  ItinStatistic& _stats;
  Diversity& _diversityParams;
  DiagCollector* _dc;
  bool _isNewFarePath = true;
  bool _diagPQ = false;
  bool _diagSkips = false;
  bool _diagSops = false;
  bool _diagAll = false;
  DatePair _filterByDatePair;
  const bool _isFareCutOffUsed;
  const bool _isDatePairFareCutOffUsed;

  std::set<DatePair> _datePairsToFulfill;
  // whether to print AltDatesStatistic table in diag 942 on the next getPQItemAction call
  int _datePairsToFulfillTraceOnly = 0; // zero means not initialized
  const std::size_t _allDatePairsCount;
  static Logger _logger;
};
}

