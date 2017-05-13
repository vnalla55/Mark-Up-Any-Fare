//----------------------------------------------------------------------------
//  File:        Diag689Collector.h
//  Authors:     Grzegorz Cholewiak
//  Created:     Sep 06, 2007
//
//  Description: Diagnostic 689 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2007
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/Money.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/RefundPenalty.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class FarePath;
class FareUsage;
class PenaltyAdjuster;
class PenaltyFee;
class ProcessTagPermutation;
class RefundPermutation;
class RefundProcessInfo;
class VoluntaryRefundsInfo;

class Diag689Collector : public DiagCollector
{
  friend class Diag689CollectorTest;

public:
  Diag689Collector()
    : _minRange(0),
      _maxRange(0),
      _permutationIndex(0),
      _t988ItemNo(0),
      _t988SeqNo(0),
      _fpFilterPassed(true),
      _permFilterPassed(true),
      _isCAT31OptionN(false),
      _isRebookSolution(false),
      _filterRebook(false),
      _filterAsbook(false),
      _cat5info(false)
  {
  }

  void printPermutationInfo(const ProcessTagInfo& pti, bool info, const FarePath& excFp);
  void printPermutationValidationResult(const ProcessTagPermutation& permutation,
                                        const std::string& result);

  Diag689Collector& operator<<(const FarePath& farePath) override;
  Diag689Collector& operator<<(const ProcessTagPermutation& perm);
  void printResultInformation(RepriceFareValidationResult r,
                              const ProcessTagInfo& pti,
                              const PaxTypeFare* ptf = nullptr);
  void filterByFarePath(const FarePath& farePath);
  bool filterPassed() const;
  void initialize();
  bool& permutationFilterPassed() { return _permFilterPassed; }
  bool permutationFilterPassed() const { return _permFilterPassed; }
  bool& setCAT31OptionN() { return _isCAT31OptionN; }

  bool& isRebookSolution() { return _isRebookSolution; }
  bool isRebookSolution() const { return _isRebookSolution; }

  void print(const ProcessTagInfo& pti,
             Indicator applicationScenario,
             const std::string& applicationStatus);
  void print(const PenaltyFee* fee, bool charged);
  void printTotalChangeFee(const Money& totalFee, Indicator applicationScenario);

  void printNarrowPtf(const PaxTypeFare& fu);
  void printAdvResOverrideData(const AdvResOverride& aro);
  inline bool printCat5Info() const { return _cat5info; }
  // --- cat33 refund specyfic ---

  void print(const FarePath& farePath);
  void print(const RefundPermutation& permutation);
  void printHeader(const RefundProcessInfo& processInfo);
  void print(const RefundProcessInfo& processInfo);
  void printMapping(const RefundProcessInfo& processInfo, const std::vector<FareUsage*>& mappedFus);
  void printMapping(const PaxTypeFare& ptf);
  void printPenalty(const std::vector<PricingUnit*>& excPUs,
                    const RefundPermutation& perm,
                    const MoneyAmount farePathAmount);
  virtual void printHeader() override;
  void printForFullRefund(const std::vector<PricingUnit*>& excPUs,
                          const std::vector<RefundPermutation*>& perm,
                          const MoneyAmount farePathAmount);

  void printNewTicketEqualOrHigherValidation(const Money& excTotalAmount,
                                             const Money& newTotalAmount,
                                             const Money& newAdjustedTotalAmount,
                                             const Money& excNonRefAmount,
                                             const Money& newNonRefAmout,
                                             const Money& newAdjustedNonrefAmount,
                                             const bool isExcNet,
                                             const bool isNewNet,
                                             const Indicator byte,
                                             const bool status);
  void printNewFareEqualOrHigherValidation(int itemNo,
                                           int seqNo,
                                           Indicator indicator,
                                           bool status,
                                           MoneyAmount newFareAmount,
                                           MoneyAmount prevFareAmount,
                                           CurrencyCode currency);
  virtual void printFareUsagesInfo(const PricingUnit& pu) override;

protected:
  void filterByPermutationNumber(const int& number);
  void filterByT988(const ProcessTagPermutation& permutation);
  const char* fareApplticationToSymbol(FareApplication fa);
  void displayRetrievalDate(const PaxTypeFare& fare) override;
  void printNarrowHeader(const FareMarket& fm);
  void printRecord3PenaltyPart(const VoluntaryRefundsInfo& r3);

  void printMoney(const Money& money);
  void printMoney(const MoneyAmount& amount, const CurrencyCode& currency)
  {
    printMoney(Money(amount, currency));
  }

  void printFareComponent(const FareUsage& fu, const PenaltyAdjuster& adjuster);
  void printPricingUnit(const PricingUnit& pu, const RefundPermutation& perm);
  void printPricingUnitPenaltys(const RefundPenalty& penalty);
  void printFee(const RefundPenalty::Fee& fee, unsigned nr);
  const PenaltyAdjuster getAdjuster(const PricingUnit& pu) const;
  void printNewTicketEqualOrHigherAmounts(const Money& excAmount,
                                          const Money& newAmount,
                                          const Money& newAdjustedAmount,
                                          const bool isExcNet,
                                          const bool isNewNet);
  void printNonRefInfo(const FareUsage& fu, bool rounding);

  struct T988Finder;

private:
  static const std::string PERMUTATION_ID;
  static const std::string RANGE_START;
  static const std::string RANGE_END;
  static const std::string TABLE_ITEMNO;
  static const std::string TABLE_SEQNO;
  static const std::string ADV_RES;

  int _minRange;
  int _maxRange;
  int _permutationIndex;
  int _t988ItemNo;
  int _t988SeqNo;
  bool _fpFilterPassed;
  bool _permFilterPassed;
  bool _isCAT31OptionN;
  bool _isRebookSolution;
  bool _filterRebook;
  bool _filterAsbook;
  bool _cat5info;
};

} // tse

