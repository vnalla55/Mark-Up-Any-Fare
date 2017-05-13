//-------------------------------------------------------------------
//
//  File:           CategoryRuleItem.h
//  Authors:        Devapriya SenGupta
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

#include "Common/TseEnums.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FareUsage.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "Rules/RuleItem.h"
#include "Rules/RuleUtil.h"

#include <vector>

namespace tse
{
class PricingTrx;
class PaxTypeFare;
class RuleItem;
class Itin;
class FarePath;
class PricingUnit;
class StopoversInfoWrapper;
class TransfersInfoWrapper;
class RuleProcessingData;
struct RuleItemCaller;
struct ItinRuleItemCaller;
struct PricingUnitRuleItemCaller;
class RuleControllerDataAccess;

enum CategoryPhase
{ FCORuleValidation = 0,
  PreValidation, // 1
  NormalValidation, // 2
  PURuleValidationIS, // 3
  PURuleValidation, // 4
  FPRuleValidation, // 5
  ShoppingComponentValidation, // 6
  ShoppingAcrossStopOverComponentValidation, // 7
  ShoppingComponentWithFlightsValidation, // 8
  ShoppingASOComponentWithFlightsValidation, // 9
  DynamicValidation, // 10
  ShoppingComponentValidateQualifiedCat4, // 11
  ShoppingItinBasedValidation, // 12
  ShoppingItinFlightIndependentValidation, // 13
  FareGroupSRValidation, // 14
  FareDisplayValidation, // 15
  RuleDisplayValidation, // 16
  PreCombinabilityValidation, // 17
  PURuleValidationISALT, // 18
  FPRuleValidationISALT, // 19
  VolunExcPrevalidation, // 20
  ShoppingAltDateItinBasedValidation, // 21
  FCORuleValidationMIPALT, // 22
  RevalidatePassResultReusedFU, // 23
  FPRuleFamilyLogicChildrenValidation,
  FootNotePrevalidation,
  FootNotePrevalidationALT,
  LoadRecords,
  FBRBaseFarePrevalidation
};

/**
 * @class CategoryRuleItem
 *
 * @brief Defines a wrapper for a Record 2 segment.
 */
class CategoryRuleItem
{
  friend class CategoryRuleItem_doMainCategoryTest;

public:
  CategoryRuleItem() : _categoryPhase(NormalValidation)
  {
  }

  CategoryRuleItem(CategoryPhase phase) : _categoryPhase(phase)
  {
  }

  Record3ReturnTypes process(PricingTrx&,
                             Itin&,
                             const CategoryRuleInfo&,
                             PaxTypeFare&,
                             const std::vector<CategoryRuleItemInfo>&,
                             bool isLocationSwapped,
                             bool& isCat15Security,
                             RuleProcessingData& rpData,
                             bool isFareRule,
                             bool skipCat15Security,
                             RuleControllerDataAccess& da);

  Record3ReturnTypes process(PricingTrx&,
                             const CategoryRuleInfo&,
                             FarePath& farePath,
                             const PricingUnit& pricingUnit,
                             /*const*/ FareUsage& fareUsage,
                             const std::vector<CategoryRuleItemInfo>&,
                             bool isLocationSwapped,
                             bool& isCat15Security,
                             RuleProcessingData& rpData,
                             bool isFareRule,
                             bool skipCat15Security,
                             RuleControllerDataAccess& da);

  Record3ReturnTypes process(PricingTrx&,
                             const CategoryRuleInfo&,
                             const Itin* itin,
                             const PricingUnit& pricingUnit,
                             /*const*/ FareUsage& fareUsage,
                             const std::vector<CategoryRuleItemInfo>&,
                             bool isLocationSwapped,
                             bool& isCat15Security,
                             RuleProcessingData& rpData,
                             bool isFareRule,
                             bool skipCat15Security,
                             RuleControllerDataAccess& da);

  static Record3ReturnTypes isDirectionPass(const PricingTrx& trx,
                                            const PaxTypeFare&,
                                            const CategoryRuleItemInfo* cfrItem,
                                            bool isLocationSwapped,
                                            const Itin* itin = nullptr);

  static Record3ReturnTypes isDirectionPassOld(const PricingTrx& trx,
                                               const PaxTypeFare&,
                                               const CategoryRuleItemInfo* cfrItem,
                                               bool isLocationSwapped,
                                               const Itin* itin = nullptr);

  //----------------------------------------------------------------------------
  // isDirectionPass()
  // return PASS  - pass directionality for the current RuleItem
  //        FAIL  - fail directionality for the current RuleItem
  //
  //----------------------------------------------------------------------------

  template <class T>
  static Record3ReturnTypes isDirectionPass(PricingTrx& trx,
                                            const FareUsage& fareUsage,
                                            const T& cri,
                                            const typename T::item_info_type* cfrItem,
                                            bool isLocationSwapped)
  {
    if (UNLIKELY(trx.getOptions()->isRtw()))
      return cfrItem->inOutInd() == RuleConst::ALWAYS_APPLIES ? PASS : FAIL;

    // check the outbound/inbound direction
    if (cfrItem->inOutInd() != RuleConst::ALWAYS_APPLIES)
    {
      if ((cfrItem->inOutInd() == RuleConst::FARE_MARKET_OUTBOUND && fareUsage.isInbound()) ||
          (cfrItem->inOutInd() == RuleConst::FARE_MARKET_INBOUND && fareUsage.isOutbound()))
      {
        return FAIL;
      }
    }

    const bool forwardLocMatch = !isLocationSwapped;
    const bool isOutbound = fareUsage.isOutbound() ^ fareUsage.fareDirectionReversed();
    const FareByRuleCtrlInfo* fbrCtrlInfo = nullptr;
    bool isLocSwapped = false;

    // check the GEO_location directionality
    switch (cfrItem->directionality())
    {
    case RuleConst::ALWAYS_APPLIES:
      return PASS;
    case RuleConst::FROM_LOC1_TO_LOC2:
      return forwardLocMatch ? PASS : FAIL;
    case RuleConst::TO_LOC1_FROM_LOC2:
      return !forwardLocMatch ? PASS : FAIL;
    case RuleConst::ORIGIN_FROM_LOC1_TO_LOC2:
      if (isOutbound == forwardLocMatch)
        return PASS;

      // I'm not sure why CAT25 is validated only in that case so I just left previous behavior
      if (!isOutbound && fareUsage.isInbound() && cri.categoryNumber() == RuleConst::FARE_BY_RULE)
      {
        fbrCtrlInfo = dynamic_cast<const FareByRuleCtrlInfo*>(&cri);
        TSE_ASSERT(fbrCtrlInfo != 0);
        if (RuleUtil::matchLocation(trx,
                                    fbrCtrlInfo->loc2(),
                                    fbrCtrlInfo->loc2zoneTblItemNo(),
                                    fbrCtrlInfo->loc1(),
                                    fbrCtrlInfo->loc1zoneTblItemNo(),
                                    fbrCtrlInfo->vendorCode(),
                                    *fareUsage.paxTypeFare()->fareMarket(),
                                    isLocSwapped,
                                    fbrCtrlInfo->carrierCode()) &&
            !isLocSwapped)
        {
          return PASS;
        }
      }

      return FAIL;
    case RuleConst::ORIGIN_FROM_LOC2_TO_LOC1:
      // Check if reversed by DMC process
      return (isOutbound != forwardLocMatch) ? PASS : FAIL;
    default:
      return FAIL;
    }
  }

  static Record3ReturnTypes isDirectionPassForShopping(PricingTrx&,
                                                       const PaxTypeFare& paxTypeFare,
                                                       const CategoryRuleInfo& cri,
                                                       const CategoryRuleItemInfo* cfrItem,
                                                       bool isLocationSwapped);

  static Record3ReturnTypes isDirectionPassForFD(const PaxTypeFare&,
                                                 const CategoryRuleItemInfo* cfrItem,
                                                 bool isLocationSwapped);

  static Record3ReturnTypes
  isR8DirectionPass(const FareUsage& fareUsage, Indicator directionality, bool isLocationSwapped);

  static Record3ReturnTypes
  isR8DirectionPassForFD(Indicator directionality, bool isLocationSwapped);

  bool isQualifiedCategory(PricingTrx& trx,
                           const CategoryRuleInfo& cri,
                           FarePath* farePath,
                           const Itin* itin,
                           const PricingUnit& pricingUnit,
                           FareUsage& fareUsage,
                           const std::vector<CategoryRuleItemInfo>& cfrItem,
                           Record3ReturnTypes& retCodeForQualified,
                           bool isLocationSwapped,
                           bool& isCat15Security,
                           RuleProcessingData& rpData,
                           bool skipCat15Security,
                           RuleControllerDataAccess& da);

  RuleItem& ruleItem() { return _ruleItem; }
  const RuleItem& ruleItem() const { return _ruleItem; }

private:
  bool isQualifiedCategory(PricingTrx&,
                           Itin&,
                           const CategoryRuleInfo&,
                           PaxTypeFare&,
                           const std::vector<CategoryRuleItemInfo>&,
                           Record3ReturnTypes&,
                           bool isLocationSwapped,
                           RuleProcessingData& rpData,
                           bool& isCat15Security,
                           bool skipCat15Security);

  Record3ReturnTypes doMainCategory(PricingTrx&,
                                    Itin&,
                                    const CategoryRuleInfo&,
                                    PaxTypeFare&,
                                    const std::vector<CategoryRuleItemInfo>&,
                                    bool isLocationSwapped,
                                    bool& isCat15Security,
                                    RuleProcessingData& rpData,
                                    bool isFareRule,
                                    bool skipCat15Security,
                                    RuleControllerDataAccess& da);

  Record3ReturnTypes doMainCategory(PricingTrx&,
                                    const CategoryRuleInfo&,
                                    FarePath& farePath,
                                    const PricingUnit& pricingUnit,
                                    FareUsage& fareUsage,
                                    const std::vector<CategoryRuleItemInfo>&,
                                    bool isLocationSwapped,
                                    bool& isCat15Security,
                                    RuleProcessingData& rpData,
                                    bool isFareRule,
                                    bool skipCat15Security,
                                    RuleControllerDataAccess& da);

  Record3ReturnTypes doMainCategory(PricingTrx&,
                                    const CategoryRuleInfo&,
                                    const Itin* itin,
                                    const PricingUnit& pricingUnit,
                                    FareUsage& fareUsage,
                                    const std::vector<CategoryRuleItemInfo>&,
                                    bool isLocationSwapped,
                                    bool& isCat15Security,
                                    RuleProcessingData& rpData,
                                    bool isFareRule,
                                    bool skipCat15Security,
                                    RuleControllerDataAccess& da);

  Record3ReturnTypes doMainCategoryCommon(PricingTrx&,
                                          PaxTypeFare&,
                                          const CategoryRuleInfo&,
                                          const std::vector<CategoryRuleItemInfo>&,
                                          bool isLocationSwapped,
                                          RuleItemCaller& ruleCaller,
                                          StopoversInfoWrapper* soInfoWrapper,
                                          TransfersInfoWrapper* trInfoWrapper,
                                          bool isFareRule,
                                          RuleControllerDataAccess& da,
                                          const PricingUnit* pu = nullptr,
                                          const FareUsage* fu = nullptr);

  Record3ReturnTypes doMainCategoryCommon(PricingTrx&,
                                          PaxTypeFare&,
                                          const CategoryRuleInfo&,
                                          const std::vector<CategoryRuleItemInfo>&,
                                          bool isLocationSwapped,
                                          RuleItemCaller& ruleCaller,
                                          StopoversInfoWrapper* soInfoWrapper,
                                          TransfersInfoWrapper* trInfoWrapper,
                                          bool isFareRule,
                                          bool isInbound);

  Record3ReturnTypes isDirectionalityMatch(PricingTrx& trx,
                                           const PaxTypeFare& paxTypeFare,
                                           const CategoryRuleItemInfo* cfr,
                                           const CategoryRuleInfo& cri,
                                           RuleItemCaller& ruleCaller,
                                           bool ruleTuningISSimple,
                                           uint32_t itemcat,
                                           bool isLocSwapped);

  void diagR2sResult(PricingTrx&,
                     const PaxTypeFare&,
                     const CategoryRuleItemInfo&,
                     const Record3ReturnTypes) const;

  RuleItem _ruleItem;
  CategoryPhase _categoryPhase;
};

} // tse namespace

