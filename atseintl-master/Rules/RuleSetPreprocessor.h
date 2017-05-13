//-------------------------------------------------------------------
//
//  File:        RuleSetPreprocessor.h
//  Created:     October 28, 2004
//  Authors:     Andrew Ahmad
//
//  Description:
//
//
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

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Record2Types.h"

#include <set>
#include <vector>

namespace tse
{
class FareUsage;
class PricingTrx;
class PaxTypeFare;
class PricingUnit;
class CategoryRuleItem;

class RuleSetPreprocessor final
{
public:
  static constexpr Indicator CXR_PREF_APPLY_LEAST_RESTRICTION = 'Y';

  bool applyLeastRestrictiveStopovers() const { return _applyLeastRestrictiveStopovers; }
  bool& applyLeastRestrictiveStopovers() { return _applyLeastRestrictiveStopovers; }

  int16_t leastRestrictiveStopoversPermitted() const { return _leastRestrictiveStopoversPermitted; }
  int16_t& leastRestrictiveStopoversPermitted() { return _leastRestrictiveStopoversPermitted; }

  bool leastRestrictiveStopoversUnlimited() const { return _leastRestrictiveStopoversUnlimited; }
  bool& leastRestrictiveStopoversUnlimited() { return _leastRestrictiveStopoversUnlimited; }

  bool applyLeastRestrictiveTransfers() const { return _applyLeastRestrictiveTransfers; }
  bool& applyLeastRestrictiveTransfers() { return _applyLeastRestrictiveTransfers; }

  int16_t leastRestrictiveTransfersPermitted() const { return _leastRestrictiveTransfersPermitted; }
  int16_t& leastRestrictiveTransfersPermitted() { return _leastRestrictiveTransfersPermitted; }

  bool leastRestrictiveTransfersUnlimited() const { return _leastRestrictiveTransfersUnlimited; }
  bool& leastRestrictiveTransfersUnlimited() { return _leastRestrictiveTransfersUnlimited; }

  const CategoryRuleInfo* categoryRuleInfo() const { return _categoryRuleInfo; }

  void process(PricingTrx& trx,
               const CategoryRuleInfo* categoryRuleInfo,
               const PricingUnit& pu,
               const FareUsage& currentFareUsage,
               CategoryRuleItem* categoryRuleItem = nullptr,
               bool isFareRule = true);

  bool hasPricingUnitScope(const uint16_t category, const FareUsage* fareUsage) const;

  // The method below is in the public interface strictly to
  //  facilitate unit testing. It is not intended to be used
  //  for any other purpose.
  std::set<const FareUsage*>& fareUsagePricingUnitScopeForTransfers()
  {
    return _fareUsagePricingUnitScopeForTransfers;
  }

  const bool& transferFCscope() const { return _transferFCscope; }
  bool& transferFCscope() { return _transferFCscope; }

  const bool& transferFailPU() const { return _transferFailsPU; }
  bool& transferFailPU() { return _transferFailsPU; }

private:
  void processStopovers(PricingTrx& trx,
                        const CategoryRuleInfo* categoryRuleInfo,
                        const PricingUnit& pu);

  void processTransfers(PricingTrx& trx,
                        const CategoryRuleInfo& categoryRuleInfo,
                        const PricingUnit& pu,
                        const FareUsage& currentFareUsage,
                        bool isFareRule);

  const PaxTypeFare* getFareFromFU(const FareUsage* fu, uint16_t category, PricingTrx& trx);

  // Fare Tag Types
  // Tag 1 = Oneway may be doubled
  // Tag 2 = Round Trip may be halved
  // Tag 3 = Oneway may not be doubled
  //
  bool isTag1OnlyPU(const PricingUnit& pu);
  bool isTag2OnlyOrTag1AndTag2PU(const PricingUnit& pu);

  bool checkLeastRestrictiveStopoversApplies(PricingTrx& trx, const PricingUnit& pu);

  bool _applyLeastRestrictiveStopovers = false;
  bool _leastRestrictiveStopoversUnlimited = false;
  int16_t _leastRestrictiveStopoversPermitted = 0;

  bool _applyLeastRestrictiveTransfers = false;
  bool _leastRestrictiveTransfersUnlimited = false;
  int16_t _leastRestrictiveTransfersPermitted = 0;

  const CategoryRuleInfo* _categoryRuleInfo = nullptr;
  CategoryRuleItem* _categoryRuleItem = nullptr;

  std::set<const FareUsage*> _fareUsagePricingUnitScopeForTransfers;
  bool _transferFCscope = false;
  bool _transferFailsPU = false;

  struct FareUsageTransfersScopeInfo;
  void detectTransfersRecord3s(PricingTrx& trx,
                               const PricingUnit& pu,
                               const PaxTypeFare& ptf,
                               const bool isLocationSwapped,
                               const CategoryRuleInfo& fareOrGeneralRuleCatRuleInfo,
                               const FareUsage& fareUsage,
                               FareUsageTransfersScopeInfo& /* out*/ fareUsageTransfersScopeInfo);

  void detectTransfersRecord3s(PricingTrx& trx,
                               const PricingUnit& pu,
                               const CategoryRuleInfo& categoryRuleInfo,
                               const bool isFareRule,
                               const FareUsage& fareUsage,
                               FareUsageTransfersScopeInfo& /* out*/ fareUsageTransfersScopeInfo);

  void addTransfersDiagnostics(
      PricingTrx& trx,
      const PricingUnit& pu,
      const FareUsage& currentFareUsage,
      const FareUsageTransfersScopeInfo& currentFareUsageScopeInfo,
      const std::vector<FareUsageTransfersScopeInfo>* const transfersScopeInfoVector) const;
};

} // namespace tse

