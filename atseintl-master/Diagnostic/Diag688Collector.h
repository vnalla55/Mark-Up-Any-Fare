//----------------------------------------------------------------------------
//  File:        Diag688Collector.h
//  Authors:     Grzegorz Cholewiak
//  Created:     Aug 13, 2007
//
//  Description: Diagnostic 688 formatter
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

#include "Diagnostic/DiagCollector.h"
#include "RexPricing/PermutationGenerator.h"
#include "Util/CartesianProduct.h"

#include <string>

namespace tse
{
class ProcessTagPermutation;
class RefundPermutation;
class RefundProcessInfo;

class Diag688Collector : public DiagCollector
{
protected:
  class PTFcmp : public std::binary_function<PaxTypeFare*, PaxTypeFare*, bool>
  {
  public:
    bool operator()(const PaxTypeFare* x, const PaxTypeFare* y)
    {
      if (x->fareMarket()->travelSeg().size() == y->fareMarket()->travelSeg().size())
        return x->fareMarket()->travelSeg().front()->pnrSegment() <
               y->fareMarket()->travelSeg().front()->pnrSegment();

      return x->fareMarket()->travelSeg().size() < y->fareMarket()->travelSeg().size();
    }
  };

public:
  explicit Diag688Collector(Diagnostic& root)
    : DiagCollector(root),
      _minRange(0),
      _maxRange(0),
      _permNumber(0),
      _permStatus(true),
      _withOverridenData(false)
  {
  }
  Diag688Collector()
    : _minRange(0), _maxRange(0), _permNumber(0), _permStatus(true), _withOverridenData(false)
  {
  }

  int& minRange() { return _minRange; }
  int minRange() const { return _minRange; }

  int& maxRange() { return _maxRange; }
  int maxRange() const { return _maxRange; }

  int& permNumber() { return _permNumber; }
  int permNumber() const { return _permNumber; }

  bool& permStatus() { return _permStatus; }
  bool permStatus() const { return _permStatus; }

  std::set<FCChangeStatus>& fcStatuses() { return _fcStatuses; }

  void printPermStatus();
  //    void printSummary(int validPermCount, int generatedPermCount);
  void printSummary(RexPricingTrx& trx, int generatedPermCount);
  Diag688Collector& operator<<(const std::string& x) override;
  Diag688Collector& operator<<(const RexPricingTrx& trx);
  Diag688Collector& operator<<(const ProcessTagPermutation* p);
  Diag688Collector& operator<<(
      std::pair<CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType*, bool> permInfo);
  void printKeepFareMapping();
  void printRefundPermutations(const std::vector<RefundPermutation*>& perm);

  typedef std::multimap<const PaxTypeFare*, const FareMarket*, PTFcmp> SortKeepMap;

protected:
  void printKeepFareMap(const SortKeepMap& map, const std::string& title);
  bool isVisible() const { return _active && _permNumber >= _minRange && _permNumber <= _maxRange; }

  Diag688Collector& operator<<(const ProcessTagInfo& pti);
  Diag688Collector& operator<<(
      CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType& permSelector);

  void printRefundProcessInfo(const RefundProcessInfo& info);
  void printRefundPermutation(const RefundPermutation& perm);

private:
  int _minRange;
  int _maxRange;
  int _permNumber;
  bool _permStatus;
  bool _withOverridenData;
  std::set<FCChangeStatus> _fcStatuses;

  std::string GetFareApplicationForTagWar(FareApplication fa);
  std::string GetFCChangeStatusForTagWar(FCChangeStatus cs);

  friend class Diag688CollectorTest;
};

} // namespace tse

