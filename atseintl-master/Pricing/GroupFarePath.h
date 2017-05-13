//-------------------------------------------------------------------
//  Copyright Sabre 2004
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FarePath.h"
#include "DBAccess/DataHandle.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/PricingEnums.h"
#include "Pricing/PriorityStatus.h"

namespace tse
{
class PUPath;

class GroupFarePath
{
public:
  virtual ~GroupFarePath() = default;

  MoneyAmount getTotalNUCAmount() const { return _totalNUCAmount; }
  void setTotalNUCAmount(MoneyAmount amt) { _totalNUCAmount = amt; }
  void increaseTotalNUCAmount(MoneyAmount amt) { _totalNUCAmount += amt; }

  const MoneyAmount& getTotalNUCBaseFareAmount() const { return _totalNUCBaseFareAmount; }
  void setTotalNUCBaseFareAmount(MoneyAmount amount) { _totalNUCBaseFareAmount = amount; }

  const std::vector<FPPQItem*>& groupFPPQItem() const { return _groupFPPQItem; }
  std::vector<FPPQItem*>& groupFPPQItem() { return _groupFPPQItem; }

  const std::vector<uint32_t>& farePathIndices() const { return _farePathIndices; }
  std::vector<uint32_t>& farePathIndices() { return _farePathIndices; }

  std::vector<CarrierCode>& validatingCarriers() { return _validatingCarriers; }
  const std::vector<CarrierCode>& validatingCarriers() const { return _validatingCarriers; }

  const uint16_t& xPoint() const { return _xPoint; }
  uint16_t& xPoint() { return _xPoint; }

  PriorityStatus& mutablePriorityStatus() { return _priorityStatus; }
  const PriorityStatus& priorityStatus() const { return _priorityStatus; }

  const bool& valid() const { return _valid; }
  bool& valid() { return _valid; }

  const bool& divideParty() const { return _divideParty; }
  bool& divideParty() { return _divideParty; }

  const std::string& sourcePqName() const { return _sourcePqName; }
  void setSourcePqName(const std::string& sourcePqName) { _sourcePqName = sourcePqName; }

  bool isShortCutPricing() { return _shortCutPricing; }
  void setShortCutPricing(bool shortCutPricing) { _shortCutPricing = shortCutPricing; }

  bool cmdPrcWithWarning() const
  {
    for (const auto fpPQItem : _groupFPPQItem)
      if (fpPQItem->farePath()->cmdPrcWithWarning())
        return true;

    return false;
  }

  MoneyAmount getNUCAmountScore() const { return _NUCAmountScoreDeviation + _totalNUCAmount; }

  void increaseNUCAmountScore(MoneyAmount val) { _NUCAmountScoreDeviation += val; }

  // This function object is needed for GroupFarePath Priority Queue
  class Greater
  {
  public:
    bool operator()(const GroupFarePath* lhs, const GroupFarePath* rhs) const
    {
      const PriorityStatus& lps = lhs->priorityStatus();
      const PriorityStatus& rps = rhs->priorityStatus();

      if (lhs->_groupFPPQItem.size() != rhs->_groupFPPQItem.size())
      {
        // in complete Group_FP of XO command should be pushed down
        return lhs->_groupFPPQItem.size() < rhs->_groupFPPQItem.size();
      }

      // CXR vs YY Fare - carrier-pref-table
      if (lps.farePriority() != rps.farePriority())
      {
        return lps.farePriority() > rps.farePriority();
      }

      MoneyAmount diff = lhs->getNUCAmountScore() - rhs->getNUCAmountScore();
      if (diff > EPSILON)
      {
        // lhs price > rhs price
        return true;
      }
      else if (-diff > EPSILON)
      {
        return false;
      }

      if (UNLIKELY(lps.fareByRulePriority() != rps.fareByRulePriority()))
      {
        return lps.fareByRulePriority() > rps.fareByRulePriority();
      }

      if (lps.paxTypeFarePriority() != rps.paxTypeFarePriority())
      {
        return lps.paxTypeFarePriority() > rps.paxTypeFarePriority();
      }

      // BaseAmount without plus ups
      const MoneyAmount lhsBaseAmount = lhs->baseAmount();
      const MoneyAmount rhsBaseAmount = rhs->baseAmount();

      diff = lhsBaseAmount - rhsBaseAmount;

      if (diff > EPSILON)
      {
        return true;
      }
      else if (-diff > EPSILON)
      {
        return false;
      }

      if (UNLIKELY(lps.fareCxrTypePriority() != rps.fareCxrTypePriority()))
      {
        return lps.fareCxrTypePriority() > rps.fareCxrTypePriority();
      }

      if (lps.negotiatedFarePriority() != rps.negotiatedFarePriority())
      {
        return lps.negotiatedFarePriority() > rps.negotiatedFarePriority();
      }

      // Unlike others, In this case higher the number higher
      // the priority. That's why '<' is used here
      //
      return lps.ptfRank() < rps.ptfRank();
    }
  };

  GroupFarePath* createDuplicate(PricingTrx& trx) const
  {
    GroupFarePath* res;
    trx.dataHandle().get(res);
    *res = *this;

    res->groupFPPQItem().clear();

    for (const auto fpPQItem : _groupFPPQItem)
      res->groupFPPQItem().push_back(fpPQItem->createDuplicate(trx));

    return res;
  }

  MoneyAmount baseAmount() const
  {
    MoneyAmount baseAmt = 0;
    for (const auto fpPQItem : _groupFPPQItem)
    {
      const FarePath& fp = *fpPQItem->farePath();
      baseAmt += fp.getTotalNUCAmount();
      baseAmt -= fp.plusUpAmount();
    }
    return baseAmt;
  }

  void resetPriorities()
  {
    _priorityStatus.clear();
  }

  void reset()
  {
    _totalNUCAmount = 0.0;
    _totalNUCBaseFareAmount = 0.0;
    _NUCAmountScoreDeviation = 0;
    _valid = false;
    _divideParty = false;
    _groupFPPQItem.clear();
    _farePathIndices.clear();
    _xPoint = 0;
    _shortCutPricing = false;
    _validatingCarriers.clear();
    resetPriorities();
  }

private:
  MoneyAmount _totalNUCAmount = 0; // considering number of Pax of each type
  MoneyAmount _totalNUCBaseFareAmount = 0; // it doesn't include taxes and surcharges
  MoneyAmount _NUCAmountScoreDeviation = 0; // considering number of Pax of each type;
  // it accumalates additional fees like baggageCharge, yqyr

  bool _valid = false;
  bool _divideParty = false;

  std::vector<FPPQItem*> _groupFPPQItem;

  PriorityStatus _priorityStatus;

  PUPath* _puPath = nullptr;
  std::vector<uint32_t> _farePathIndices;
  uint16_t _xPoint = 0;

  bool _shortCutPricing = false;

  std::vector<CarrierCode> _validatingCarriers;

  std::string _sourcePqName; // Name/description of the ShoppingPQ that produced this GFP
};
} // tse namespace
