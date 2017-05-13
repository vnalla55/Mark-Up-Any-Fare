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

#include "Pricing/Shopping/PQ/AltDatesStatistic.h"

namespace tse
{
class DiagCollector;
class ItinStatistic;

class AltDatesStatistic2 : public AltDatesStatistic
{
public:
  AltDatesStatistic2(ShoppingTrx&, ItinStatistic&);

  /**
   * @override
   */
  void addSolution(const FlightMatrixSolution& solution, const DatePair& datePair) override;
  /**
   * @override
   */
  Stat getStat(const DiversityModel::SOPCombination& comb, MoneyAmount score) const override;
  /**
   * @override
   */
  Stat getStat(const FlightMatrixSolution& solution, const DatePair& datePair) const override;
  /**
   * @override
   */
  std::tr1::tuple<std::size_t, std::size_t>
  filterDatePairsBy(MoneyAmount pqScore,
                    unsigned maxFareLevel,
                    float datePairFareCutoffCoef,
                    std::set<DatePair>& datePairSet) const override;
  /**
   * @override
   */
  bool findAnyDatePair(MoneyAmount findUsingPQScoreStat,
                       CxrSPInfo findUsingCxrSPInfoStat,
                       const DatePairSkipCallback& skipCallback,
                       const std::set<DatePair>& datePairsToCheck) const override;
  /**
   * @override
   */
  void dumpToDiag(DiagCollector& dc, const DumpToDiagParam& param) const override;

  void removeSolution(const FlightMatrixSolution& solution, const DatePair& datePair);

private:
  // AltDatesStatistic is collecting statistics in nodes,
  //  the nodes are organized into hierarchy
  class StatNode;
  class StatNodeFactory;
  class FareLevelStatNodeSharedCtx;

  // root level 0:
  class DatePairStatNode;
  // 1 \___ 1..* level 1:
  class FareLevelStatNode;
  //  1 \___ 1..* level 2:
  class FareLevelAuxStatNode;
  //      1 \___ 1 level 3:
  class SolKindStatNode;
  //        1 \___ 1..* level 5:
  class TravelSegCountStatNode;
  //          1 \___ 1..* level 6:
  class NumOptStatNode;

  struct DumpToDiagCtx;

  DatePairStatNode* _stat; // root level
  StatNodeFactory* _statNodeFactory;
};

} // ns tse

