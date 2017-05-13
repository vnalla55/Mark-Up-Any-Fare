//-------------------------------------------------------------------
//
//  File:        CategoryRuleItem.cpp
//  Created:     Apr 8, 2003
//  Authors:     Devapriya SenGupta, Vladimir Koliasnikov
//
//  Description:
//
//
//  Updates:
//          04/19/04 - VK- file created.
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

#include "Rules/CategoryRuleItem.h"

#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/PenaltyInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/SalesRestriction.h"
#include "DBAccess/StopoversInfo.h"
#include "DBAccess/TransfersInfo1.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/Diag500Collector.h"
#include "Diagnostic/Diag550Collector.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleItemCaller.h"
#include "Rules/RuleProcessingData.h"
#include "Rules/SalesRestrictionRule.h"
#include "Rules/StopoversInfoWrapper.h"
#include "Rules/TransfersInfoWrapper.h"
#include "Util/BranchPrediction.h"

#include <cassert>
#include <vector>

#include <boost/logic/tribool.hpp>

namespace tse
{

FALLBACK_DECL(fallbackFVORuleDirectionalityTuning);
FALLBACK_DECL(r2sDirectionalityOpt)
FALLBACK_DECL(smpShoppingISCoreFix)
FALLBACK_DECL(cat9FixMaxTransfers)
FALLBACK_DECL(apo45023ApplyCat2DefaultsInOOJPU)

namespace
{
bool
isRexRule(uint16_t category)
{
  return category == RuleConst::VOLUNTARY_EXCHANGE_RULE ||
         category == RuleConst::VOLUNTARY_REFUNDS_RULE;
}

// enhancement hard coded for cat19 only now to solve SPR113907,
// might expand to other categories later
// expaned for cat13 - SPR 139971
bool
isDirectionalRule(uint32_t category)
{
  return category == RuleConst::CHILDREN_DISCOUNT_RULE ||
         category == RuleConst::ACCOMPANIED_PSG_RULE;
}

bool
isDirectionalRule2(uint32_t category)
{
  return true; 
}
}

struct ItinRuleItemCaller : public RuleItemCaller
{
  ItinRuleItemCaller(Itin& itin,
                     PricingTrx& trx,
                     const CategoryRuleInfo& cri,
                     const std::vector<CategoryRuleItemInfo>& cfrItemSet,
                     PaxTypeFare& paxTypeFare,
                     bool& isCat15Security,
                     RuleItem& ruleItem,
                     bool skipCat15Security)
    : RuleItemCaller(
          trx, cri, cfrItemSet, paxTypeFare, isCat15Security, ruleItem, skipCat15Security),
      _itin(itin)
  {
  }

  Record3ReturnTypes operator()(CategoryRuleItemInfo* cfrItem) const override
  {
    Record3ReturnTypes retval = SKIP;
    const RuleItemInfo* const ruleItemInfo = RuleUtil::getRuleItemInfo(
        _trx, &_cri, cfrItem, _paxTypeFare.fareMarket()->ruleApplicationDate());

    if (LIKELY(ruleItemInfo))
    {
      retval = _ruleItem.validate(_trx,
                                  _itin,
                                  _cri,
                                  _cfrItemSet,
                                  _paxTypeFare,
                                  cfrItem,
                                  ruleItemInfo,
                                  cfrItem->itemcat(),
                                  _isCat15Security,
                                  _skipCat15Security);
    }
    return retval;
  }

  Record3ReturnTypes operator()(CategoryRuleItemInfo* cfrItem, bool isInbound) const override
  {
    Record3ReturnTypes retval = SKIP;
    const RuleItemInfo* const ruleItemInfo = RuleUtil::getRuleItemInfo(
        _trx, &_cri, cfrItem, _paxTypeFare.fareMarket()->ruleApplicationDate());

    if (ruleItemInfo)
    {
      retval = _ruleItem.validate(_trx,
                                  _itin,
                                  _cri,
                                  _cfrItemSet,
                                  _paxTypeFare,
                                  cfrItem,
                                  ruleItemInfo,
                                  cfrItem->itemcat(),
                                  _isCat15Security,
                                  _skipCat15Security,
                                  isInbound);
    }
    return retval;
  }

  // Overloaded method to handle data set validation for Stopovers, Transfers
  //
  Record3ReturnTypes
  operator()(CategoryRuleItemInfo* cfrItem, const RuleItemInfo* ruleItem) const override
  {
    Record3ReturnTypes retval = _ruleItem.validate(_trx,
                                                   _itin,
                                                   _cri,
                                                   _cfrItemSet,
                                                   _paxTypeFare,
                                                   cfrItem,
                                                   ruleItem,
                                                   cfrItem->itemcat(),
                                                   _isCat15Security,
                                                   _skipCat15Security);

    return retval;
  }

  Record3ReturnTypes
  isDirectionPass(const CategoryRuleItemInfo* cfrItem, bool isLocationSwapped) const override
  {
    // Check for Fare Display transaction
    if (UNLIKELY(_trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX))
    {
      return (CategoryRuleItem::isDirectionPassForFD(_paxTypeFare, cfrItem, isLocationSwapped));
    }
    else
    {
      if (_ruleItem.phase() == tse::RuleItem::DynamicValidationPhase)
      {
        FareUsage fareUsage;
        fareUsage.paxTypeFare() = const_cast<PaxTypeFare*>(&_paxTypeFare);
        fareUsage.inbound() = (_paxTypeFare.directionality() == TO);
        return (
            CategoryRuleItem::isDirectionPass(_trx, fareUsage, _cri, cfrItem, isLocationSwapped));
      }
      return (CategoryRuleItem::isDirectionPass(
          _trx, _paxTypeFare, cfrItem, isLocationSwapped, &_itin));
    }
  }

  Record3ReturnTypes isR8DirectionPass(Indicator dir, bool isR8LocationSwapped) const override
  {
    // Check for Fare Display transaction
    if (_trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX)
    {
      return (CategoryRuleItem::isR8DirectionPassForFD(dir, isR8LocationSwapped));
    }

    return SOFTPASS;
  }

  Record3ReturnTypes validateT994DateOverride(const RuleItemInfo* r3, uint32_t catNo) const override
  {
    return RuleItem::preliminaryT994Validation(_trx, _cri, nullptr, _paxTypeFare, r3, catNo);
  }

private:
  Itin& _itin;
};

struct PricingUnitRuleItemCaller : public RuleItemCaller
{
  PricingUnitRuleItemCaller(FarePath& farePath,
                            const PricingUnit& pricingUnit,
                            FareUsage& fareUsage,
                            PricingTrx& trx,
                            const CategoryRuleInfo& cri,
                            const std::vector<CategoryRuleItemInfo>& cfrItemSet,
                            bool& isCat15Security,
                            RuleItem& ruleItem,
                            bool skipCat15Security)
    : RuleItemCaller(trx,
                     cri,
                     cfrItemSet,
                     *fareUsage.paxTypeFare(),
                     isCat15Security,
                     ruleItem,
                     skipCat15Security),
      _farePath(farePath),
      _pricingUnit(pricingUnit),
      _fareUsage(fareUsage)
  {
  }

  Record3ReturnTypes operator()(CategoryRuleItemInfo* cfrItem) const override
  {
    const RuleItemInfo* const ruleItemInfo = RuleUtil::getRuleItemInfo(
        _trx, &_cri, cfrItem, _paxTypeFare.fareMarket()->ruleApplicationDate());

    Record3ReturnTypes retval = _ruleItem.validate(_trx,
                                                   _cri,
                                                   _cfrItemSet,
                                                   _farePath,
                                                   _pricingUnit,
                                                   _fareUsage,
                                                   cfrItem,
                                                   ruleItemInfo,
                                                   cfrItem->itemcat(),
                                                   _isCat15Security,
                                                   _skipCat15Security);

    return retval;
  }

  Record3ReturnTypes operator()(CategoryRuleItemInfo* cfrItem, bool isInbound) const override
  {
    const RuleItemInfo* const ruleItemInfo = RuleUtil::getRuleItemInfo(
        _trx, &_cri, cfrItem, _paxTypeFare.fareMarket()->ruleApplicationDate());
    Record3ReturnTypes retval = _ruleItem.validate(_trx,
                                                   _cri,
                                                   _cfrItemSet,
                                                   _farePath,
                                                   _pricingUnit,
                                                   _fareUsage,
                                                   cfrItem,
                                                   ruleItemInfo,
                                                   cfrItem->itemcat(),
                                                   _isCat15Security,
                                                   _skipCat15Security);
    return retval;
  }

  // Overloaded method to handle data set validation for Stopovers, Transfers
  //
  Record3ReturnTypes
  operator()(CategoryRuleItemInfo* cfrItem, const RuleItemInfo* ruleItem) const override
  {
    Record3ReturnTypes retval = _ruleItem.validate(_trx,
                                                   _cri,
                                                   _cfrItemSet,
                                                   _farePath,
                                                   _pricingUnit,
                                                   _fareUsage,
                                                   cfrItem,
                                                   ruleItem,
                                                   cfrItem->itemcat(),
                                                   _isCat15Security,
                                                   _skipCat15Security);

    return retval;
  }

  Record3ReturnTypes
  isDirectionPass(const CategoryRuleItemInfo* cfrItem, bool isLocationSwapped) const override
  {
    return (CategoryRuleItem::isDirectionPass(_trx, _fareUsage, _cri, cfrItem, isLocationSwapped));
  }

  Record3ReturnTypes
  isR8DirectionPass(Indicator directionality, bool isR8LocationSwapped) const override
  { // lint !e578
    return (CategoryRuleItem::isR8DirectionPass(_fareUsage, directionality, isR8LocationSwapped));
  }

  Record3ReturnTypes validateT994DateOverride(const RuleItemInfo* r3, uint32_t catNo) const override
  {
    return RuleItem::preliminaryT994Validation(_trx, _cri, &_pricingUnit, _paxTypeFare, r3, catNo);
  }

private:
  FarePath& _farePath;
  const PricingUnit& _pricingUnit;
  FareUsage& _fareUsage;
};

struct PricingUnitWithoutFarePathRuleItemCaller : public RuleItemCaller
{
  PricingUnitWithoutFarePathRuleItemCaller(const Itin* itin,
                                           const PricingUnit& pricingUnit,
                                           FareUsage& fareUsage,
                                           PricingTrx& trx,
                                           const CategoryRuleInfo& cri,
                                           const std::vector<CategoryRuleItemInfo>& cfrItemSet,
                                           bool& isCat15Security,
                                           RuleItem& ruleItem,
                                           bool skipCat15Security)
    : RuleItemCaller(trx,
                     cri,
                     cfrItemSet,
                     *fareUsage.paxTypeFare(),
                     isCat15Security,
                     ruleItem,
                     skipCat15Security),
      _itin(itin),
      _pricingUnit(pricingUnit),
      _fareUsage(fareUsage)
  {
  }

  Record3ReturnTypes operator()(CategoryRuleItemInfo* cfrItem) const override
  {
    const RuleItemInfo* const ruleItemInfo = RuleUtil::getRuleItemInfo(
        _trx, &_cri, cfrItem, _paxTypeFare.fareMarket()->ruleApplicationDate());
    Record3ReturnTypes retval = _ruleItem.validate(_trx,
                                                   _cri,
                                                   _cfrItemSet,
                                                   _itin,
                                                   _pricingUnit,
                                                   _fareUsage,
                                                   cfrItem,
                                                   ruleItemInfo,
                                                   cfrItem->itemcat(),
                                                   _isCat15Security,
                                                   _skipCat15Security);

    return retval;
  }

  Record3ReturnTypes operator()(CategoryRuleItemInfo* cfrItem, bool isInbound) const override
  {
    const RuleItemInfo* const ruleItemInfo = RuleUtil::getRuleItemInfo(
        _trx, &_cri, cfrItem, _paxTypeFare.fareMarket()->ruleApplicationDate());
    Record3ReturnTypes retval = _ruleItem.validate(_trx,
                                                   _cri,
                                                   _cfrItemSet,
                                                   _itin,
                                                   _pricingUnit,
                                                   _fareUsage,
                                                   cfrItem,
                                                   ruleItemInfo,
                                                   cfrItem->itemcat(),
                                                   _isCat15Security,
                                                   _skipCat15Security);

    return retval;
  }

  // Overloaded method to handle data set validation for Stopovers, Transfers
  //
  Record3ReturnTypes
  operator()(CategoryRuleItemInfo* cfrItem, const RuleItemInfo* ruleItem) const override
  {
    Record3ReturnTypes retval = _ruleItem.validate(_trx,
                                                   _cri,
                                                   _cfrItemSet,
                                                   _itin,
                                                   _pricingUnit,
                                                   _fareUsage,
                                                   cfrItem,
                                                   ruleItem,
                                                   cfrItem->itemcat(),
                                                   _isCat15Security,
                                                   _skipCat15Security);

    return retval;
  }

  Record3ReturnTypes
  isDirectionPass(const CategoryRuleItemInfo* cfrItem, bool isLocationSwapped) const override
  {
    return (CategoryRuleItem::isDirectionPass(_trx, _fareUsage, _cri, cfrItem, isLocationSwapped));
  }

  Record3ReturnTypes
  isR8DirectionPass(Indicator directionality, bool isR8LocationSwapped) const override
  { // lint !e578
    return SOFTPASS;
  }

  Record3ReturnTypes validateT994DateOverride(const RuleItemInfo* r3, uint32_t catNo) const override
  {
    return RuleItem::preliminaryT994Validation(_trx, _cri, &_pricingUnit, _paxTypeFare, r3, catNo);
  }

private:
  const Itin* _itin;
  const PricingUnit& _pricingUnit;
  FareUsage& _fareUsage;
};

//----------------------------------------------------------------------------
// process()
//----------------------------------------------------------------------------
Record3ReturnTypes
CategoryRuleItem::process(PricingTrx& trx,
                          Itin& itin,
                          const CategoryRuleInfo& cri,
                          PaxTypeFare& paxTypeFare,
                          const std::vector<CategoryRuleItemInfo>& cfrItem,
                          bool isLocationSwapped,
                          bool& isCat15Security,
                          RuleProcessingData& rpData,
                          bool isFareRule,
                          bool skipCat15Security,
                          RuleControllerDataAccess& da)
{
  Record3ReturnTypes retCode = SKIP;
  Record3ReturnTypes currRetCode = SKIP;
  bool isQualified = false;

  _ruleItem.setRuleControllerDataAccess(&da);

  // check the qualified portion of the RuleSet
  isQualified = isQualifiedCategory(trx,
                                    itin,
                                    cri,
                                    paxTypeFare,
                                    cfrItem,
                                    currRetCode,
                                    isLocationSwapped,
                                    rpData,
                                    isCat15Security,
                                    skipCat15Security);

  if (isQualified)
  {
    if (currRetCode == STOP || currRetCode == SKIP)
      return currRetCode;

    if (currRetCode == FAIL)
    {
      if (cri.categoryNumber() == RuleConst::FARE_BY_RULE ||
          cri.categoryNumber() == RuleConst::NEGOTIATED_RULE)
      {
        return FAIL;
      }
      else
      {
        return SKIP;
      }
    }

    if (currRetCode == SOFTPASS)
    {
      if (LIKELY(trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX ||
          (cri.categoryNumber() != RuleConst::SALE_RESTRICTIONS_RULE &&
           cri.categoryNumber() != RuleConst::NEGOTIATED_RULE)))
        return SOFTPASS;
    }
  }

  // only process main category for cat25 & cat35 if this is the set that built the fare
  if (cri.categoryNumber() == RuleConst::FARE_BY_RULE ||
      cri.categoryNumber() == RuleConst::NEGOTIATED_RULE)
  {
    // lint --e{530}
    PaxTypeFareRuleData* paxTypeFareRuleData =
        paxTypeFare.paxTypeFareRuleData(cri.categoryNumber());

    // IF the Category Rule info didn't get created for some reason
    if (UNLIKELY(!paxTypeFareRuleData || !paxTypeFareRuleData->categoryRuleItemInfoSet()))
    {
      // return an indication to skip or fail based on the category number.
      return ((cri.categoryNumber() == RuleConst::FARE_BY_RULE) ? FAIL : SKIP);
    }
    // if not the set that built the fare don't process the main category
    if (cfrItem != *(paxTypeFareRuleData->categoryRuleItemInfoSet()))
    {
      return currRetCode;
    }
  }

  //  do business logic for main portion of the RuleSet
  retCode = doMainCategory(trx,
                           itin,
                           cri,
                           paxTypeFare,
                           cfrItem,
                           isLocationSwapped,
                           isCat15Security,
                           rpData,
                           isFareRule,
                           skipCat15Security,
                           da);

  if (UNLIKELY(retCode == PASS && isRexRule(cri.categoryNumber())))
  {
    return SKIP; // need to check next set.
  }

  if (!isQualified)
  {
    if (cri.categoryNumber() == RuleConst::BLACKOUTS_RULE)
    {
      if (retCode == PASS)
        return SKIP; // need to check next set.
      else if (retCode == FAIL)
        return STOP;
    }

    /* FareDisplayTrx* fdt;
     FareDisplayUtil::getFareDisplayTrx(&trx,fdt);
     bool seasonTemplate=false;
     if (fdt)
     {
       seasonTemplate=(fdt->isSeasonTemplate() && (fdt->isRD() ||fdt->isFQ()));
       if (retCode == PASS &&
           cri.categoryNumber() == RuleConst::SEASONAL_RULE &&
           seasonTemplate )
        return SKIP;      // For Season templates need all valid seasons
     }  */

    return retCode; // return=FAIL          - check next set
    // return=SKIP          - ckeck next set
    // return=PASS/SOFTPASS - job is done.
  }
  // The current set has a qualified
  if (retCode == PASS)
    return PASS;
  if (retCode == SOFTPASS)
    return SOFTPASS;
  if (retCode == STOP_SOFT)
    return STOP_SOFT;

  // Special logic for Fare Display
  // When qualified category, we don't want to stop processing if the
  // qualifying portion was not processed.
  // Note: A category may not be processed if not supported by Fare Display,
  //       or when No Validation is requested (#VN)
  if (UNLIKELY(trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX && currRetCode == NOTPROCESSED &&
      retCode == FAIL))
  {
    return FAIL;
  }
  //  return ( (retCode == FAIL) ? STOP : FAIL );

  // Stop processing if the qualified portion was passed
  //                 and the main category was failed/skipped.
  // "STOP_SKIP" is used for the Stopover only to stop processing in the CategoryRuleItemSet
  //                when the main was skipped
  //                (to apply a SystemDefaultAssumption).
  // "PASS" is used for other Rules when the main was skipped.
  // "STOP" is used for all Rules when the main was failed.
  if (retCode == SKIP)
  {
    if (cri.categoryNumber() == RuleConst::STOPOVER_RULE)
      return STOP_SKIP;

    return PASS;
  }

  return STOP;
}

//----------------------------------------------------------------------------
// process()
//----------------------------------------------------------------------------
Record3ReturnTypes
CategoryRuleItem::process(PricingTrx& trx,
                          const CategoryRuleInfo& cri,
                          FarePath& farePath,
                          const PricingUnit& pricingUnit,
                          FareUsage& fareUsage,
                          const std::vector<CategoryRuleItemInfo>& cfrItem,
                          bool isLocationSwapped,
                          bool& isCat15Security,
                          RuleProcessingData& rpData,
                          bool isFareRule,
                          bool skipCat15Security,
                          RuleControllerDataAccess& da)
{
  Record3ReturnTypes retCode = SKIP;
  Record3ReturnTypes currRetCode = SKIP;
  bool isQualified = false;
  _ruleItem.setRuleControllerDataAccess(&da);
  // check the qualified portion of the RuleSet
  isQualified = isQualifiedCategory(trx,
                                    cri,
                                    &farePath,
                                    nullptr,
                                    pricingUnit,
                                    fareUsage,
                                    cfrItem,
                                    currRetCode,
                                    isLocationSwapped,
                                    isCat15Security,
                                    rpData,
                                    skipCat15Security,
                                    da);

  if (isQualified)
  {
    if (cri.categoryNumber() == RuleConst::FARE_BY_RULE ||
        cri.categoryNumber() == RuleConst::NEGOTIATED_RULE)
    {
      if (currRetCode != PASS)
        return FAIL;
    }
    else
    {
      if (currRetCode == STOP || currRetCode == SKIP)
        return currRetCode;

      if (currRetCode == FAIL)
        return ((cri.categoryNumber() == RuleConst::FARE_BY_RULE) ? FAIL : SKIP);
    }
  }

  // only process main category for cat25 & cat35 if this is the set that built the fare
  if (cri.categoryNumber() == RuleConst::FARE_BY_RULE ||
      cri.categoryNumber() == RuleConst::NEGOTIATED_RULE)
  {
    // lint --e{530}
    PaxTypeFareRuleData* paxTypeFareRuleData =
        fareUsage.paxTypeFare()->paxTypeFareRuleData(cri.categoryNumber());

    // IF the Category Rule info didn't get created for some reason
    if (UNLIKELY(!paxTypeFareRuleData || !paxTypeFareRuleData->categoryRuleItemInfoSet()))
    {
      // return an indication to skip or fail based on the category number.
      return ((cri.categoryNumber() == RuleConst::FARE_BY_RULE) ? FAIL : SKIP);
    }
    // if not the set that built the fare don't process the main category
    if (cfrItem != *(paxTypeFareRuleData->categoryRuleItemInfoSet()))
    {
      return currRetCode;
    }
  }

  //  do business logic for main portion of the RuleSet
  retCode = doMainCategory(trx,
                           cri,
                           farePath,
                           pricingUnit,
                           fareUsage,
                           cfrItem,
                           isLocationSwapped,
                           isCat15Security,
                           rpData,
                           isFareRule,
                           skipCat15Security,
                           da);
  if (UNLIKELY(retCode == PASS && isRexRule(cri.categoryNumber())))
  {
    return SKIP; // need to check next set.
  }

  if (!isQualified)
  {
    if (cri.categoryNumber() == RuleConst::BLACKOUTS_RULE)
    {
      if (retCode == PASS)
        return SKIP; // need to check next set.
      else if (retCode == FAIL)
        return STOP;
    }

    return retCode; // return=FAIL          - check next set
    // return=SKIP          - ckeck next set
    // return=PASS          - job is done.
  }
  if (retCode == PASS)
    return PASS;

  // return ( (retCode == FAIL) ? STOP : FAIL );

  // Stop processing if the qualified portion was passed
  //                 and the main category was failed/skipped.
  // "STOP_SKIP" is used for the Stopover only to stop processing in the CategoryRuleItemSet
  //                when the main was skipped
  //                (to apply a SystemDefaultAssumption).
  // "PASS" is used for other Rules when the main was skipped.
  // "STOP" is used for all Rules when the main was failed.
  if (retCode == SKIP)
  {
    if (UNLIKELY(cri.categoryNumber() == RuleConst::STOPOVER_RULE))
      return STOP_SKIP;

    return PASS;
  }

  return STOP;
}

//----------------------------------------------------------------------------
// process()
//----------------------------------------------------------------------------
Record3ReturnTypes
CategoryRuleItem::process(PricingTrx& trx,
                          const CategoryRuleInfo& cri,
                          const Itin* itin,
                          const PricingUnit& pricingUnit,
                          FareUsage& fareUsage,
                          const std::vector<CategoryRuleItemInfo>& cfrItem,
                          bool isLocationSwapped,
                          bool& isCat15Security,
                          RuleProcessingData& rpData,
                          bool isFareRule,
                          bool skipCat15Security,
                          RuleControllerDataAccess& da)
{
  Record3ReturnTypes retCode = SKIP;
  Record3ReturnTypes currRetCode = SKIP;
  bool isQualified = false;
  _ruleItem.setRuleControllerDataAccess(&da);

  // check the qualified portion of the RuleSet
  isQualified = isQualifiedCategory(trx,
                                    cri,
                                    nullptr,
                                    itin,
                                    pricingUnit,
                                    fareUsage,
                                    cfrItem,
                                    currRetCode,
                                    isLocationSwapped,
                                    isCat15Security,
                                    rpData,
                                    skipCat15Security,
                                    da);

  if (isQualified)
  {
    if (currRetCode == STOP || currRetCode == SKIP)
      return currRetCode;

    if (currRetCode == FAIL)
      return SKIP;
  }

  //  do business logic for main portion of the RuleSet
  retCode = doMainCategory(trx,
                           cri,
                           itin,
                           pricingUnit,
                           fareUsage,
                           cfrItem,
                           isLocationSwapped,
                           isCat15Security,
                           rpData,
                           isFareRule,
                           skipCat15Security,
                           da);
  if (UNLIKELY(retCode == PASS && isRexRule(cri.categoryNumber())))
  {
    return SKIP; // need to check next set.
  }

  if (!isQualified)
  {
    if (retCode == PASS && cri.categoryNumber() == RuleConst::BLACKOUTS_RULE)
      return SKIP; // need to check next set.

    return retCode;
  }
  if (retCode == PASS)
    return PASS;

  // return ( (retCode == FAIL) ? STOP : FAIL );

  // Stop processing if the qualified portion was passed
  //                 and the main category was failed/skipped.
  // "STOP_SKIP" is used for the Stopover only to stop processing in the CategoryRuleItemSet
  //                when the main was skipped
  //                (to apply a SystemDefaultAssumption).
  // "PASS" is used for other Rules when the main was skipped.
  // "STOP" is used for all Rules when the main was failed.
  if (retCode == SKIP)
  {
    if (cri.categoryNumber() == RuleConst::STOPOVER_RULE)
      return STOP_SKIP;

    return PASS;
  }

  return STOP;
}

//----------------------------------------------------------------------------
// isQualifiedCategory()
// return true  - if there is a qualified category in this Ruleset data
//        false -    there is no qualified category.
// statusRule = PASS  - qualified category appears and is passed
//              FAIL  - qualified category appears and is failed
//              SKIP  - there is no qualified category or
//                      qualified category not matched.
//              STOP  - Some problem in qualified category.
//----------------------------------------------------------------------------

bool
CategoryRuleItem::isQualifiedCategory(PricingTrx& trx,
                                      Itin& itin,
                                      const CategoryRuleInfo& cri,
                                      PaxTypeFare& paxTypeFare,
                                      const std::vector<CategoryRuleItemInfo>& cfrItem,
                                      Record3ReturnTypes& retCodeForQualified,
                                      bool isLocationSwapped,
                                      RuleProcessingData& rpData,
                                      bool& isCat15Security,
                                      bool skipCat15Security)
{ // lint !e578
  retCodeForQualified = SKIP;

  bool isQualified = false;
  // bool   matchQualified  = false;
  bool failIfCat1OfThen15 = false;

  StopoversInfoWrapper* soInfoWrapper = rpData.soInfoWrapper();
  TransfersInfoWrapper* trInfoWrapper = rpData.trInfoWrapper();

  std::vector<CategoryRuleItemInfo>::const_iterator cfrIter = cfrItem.begin();
  std::vector<CategoryRuleItemInfo>::const_iterator cfrIterEnd = cfrItem.end();

  const DateTime& applDate = paxTypeFare.fareMarket()->ruleApplicationDate();

  // do the qualified portion of the RuleSet
  for (; cfrIter != cfrIterEnd; ++cfrIter)
  {
    const CategoryRuleItemInfo& cfr = *cfrIter;

    uint32_t itemcat = cfr.itemcat();
    uint32_t relationalInd = cfr.relationalInd();

    const RuleItemInfo* const ruleItemInfo = RuleUtil::getRuleItemInfo(trx, &cri, &cfr, applDate);
    if (relationalInd == CategoryRuleItemInfo::IF) // Does a RuleSet contain a qualified rule?
    {
      isQualified = true; // Yes.
    }

    if (ruleItemInfo == nullptr)
      continue; // RuleUtil has logged the error.

    if (isQualified) // Check qualified rule.
    {
      if (retCodeForQualified == FAIL && relationalInd == CategoryRuleItemInfo::AND)
      {
        break; // job is done. fail this qualified portion.
      }
      if (retCodeForQualified == PASS)
      {
        if (relationalInd == CategoryRuleItemInfo::OR)
        {
          if (UNLIKELY(itemcat == RuleConst::STOPOVER_RULE))
          {
            if (soInfoWrapper->checkAllPassed())
            {
              break;
            }
            else
            {
              soInfoWrapper->clearResults();
            }
          }
          else if (UNLIKELY(itemcat == RuleConst::TRANSFER_RULE))
          {
            if (trInfoWrapper->checkAllPassed())
            {
              break;
            }
            else
            {
              trInfoWrapper->clearResults();
            }
          }
          else
          {
            break; // job is done. pass this qualified portion.
          }
        }
      }

      Record3ReturnTypes currRetCode = SKIP;

      Record3ReturnTypes directionCode;

      // Check for Fare Display transaction
      if (UNLIKELY(trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX))
      {
        directionCode = isDirectionPassForFD(paxTypeFare, &cfr, isLocationSwapped);
      }
      else
      {
        directionCode = isDirectionPass(trx, paxTypeFare, &cfr, isLocationSwapped);
      }

      if (directionCode == PASS)
      {
        // matchQualified = true;              // keep it for the future...

        if (UNLIKELY(itemcat == RuleConst::STOPOVER_RULE))
        {
          const StopoversInfo* soInfo = dynamic_cast<const StopoversInfo*>(ruleItemInfo);

          // use the wrapper class
          soInfoWrapper->soInfo(soInfo);
          soInfoWrapper->stopoversRuleExistsInSet() = true;

          const RuleItemInfo* ruleInfo = soInfoWrapper;

          currRetCode = _ruleItem.validate(trx,
                                           itin,
                                           cri,
                                           cfrItem,
                                           paxTypeFare,
                                           &cfr,
                                           ruleInfo,
                                           itemcat,
                                           isCat15Security,
                                           skipCat15Security);
        }
        else if (UNLIKELY(itemcat == RuleConst::TRANSFER_RULE))
        {
          trInfoWrapper->setCurrentTrInfo(dynamic_cast<const TransfersInfo1*>(ruleItemInfo));
          trInfoWrapper->transfersRuleExistsInSet() = true;

          const RuleItemInfo* ruleInfo = trInfoWrapper;

          currRetCode = _ruleItem.validate(trx,
                                           itin,
                                           cri,
                                           cfrItem,
                                           paxTypeFare,
                                           &cfr,
                                           ruleInfo,
                                           itemcat,
                                           isCat15Security,
                                           skipCat15Security);
        }
        // Do not re-validate IF 1 of THEN Cat 25,
        // it was completely validated during Fare Collector
        else if (cri.categoryNumber() == RuleConst::FARE_BY_RULE &&
                 itemcat == RuleConst::ELIGIBILITY_RULE)
        {
          currRetCode = PASS;
        }
        // Do not validate IF 18 of THEN Cat 25 during Fare Validation.
        // It will be validated during PRicing Unit Rule validation
        else if (UNLIKELY(cri.categoryNumber() == RuleConst::FARE_BY_RULE &&
                 itemcat == RuleConst::TICKET_ENDORSMENT_RULE))
        {
          currRetCode = SKIP;
        }
        else
        {
          currRetCode = _ruleItem.validate(trx,
                                           itin,
                                           cri,
                                           cfrItem,
                                           paxTypeFare,
                                           &cfr,
                                           ruleItemInfo,
                                           itemcat,
                                           isCat15Security,
                                           skipCat15Security);
        }

        if (UNLIKELY(trx.diagnostic().diagnosticType() != DiagnosticNone))
          diagR2sResult(trx, paxTypeFare, cfr, currRetCode);

        if (currRetCode == PASS)
        {
          // Special logic for Fare Display
          // If a category has not been processed, we are not absolutely
          // sure if it would pass. So we leave it alone when AND condition
          if (UNLIKELY(trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX &&
              retCodeForQualified == NOTPROCESSED && relationalInd == CategoryRuleItemInfo::AND))
          {
            currRetCode = NOTPROCESSED;
          }

          retCodeForQualified = currRetCode;

          if (relationalInd == CategoryRuleItemInfo::OR)
          {
            if (UNLIKELY(itemcat == RuleConst::STOPOVER_RULE))
            {
              if (soInfoWrapper->checkAllPassed())
              {
                break;
              }
            }
            else if (UNLIKELY(itemcat == RuleConst::TRANSFER_RULE))
            {
              if (trInfoWrapper->checkAllPassed())
              {
                break;
              }
            }
            else
            {
              break; // job is done. pass this qualified portion
            }
          }
        }
        else if (currRetCode == FAIL)
        {
          // Special logic for Fare Display
          // If a category has not been processed we want to keep checking
          // the OR conditions for a real PASS. But if it fails, we don't want
          // to change the status (NOTPROCESSED is better than FAIL)
          if (UNLIKELY(trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX &&
              retCodeForQualified == NOTPROCESSED && relationalInd == CategoryRuleItemInfo::OR))
          {
            currRetCode = NOTPROCESSED;
          }

          // Special logic for Cat 35 fare with THEN 15 IF 1
          if ((paxTypeFare.fcaDisplayCatType() == RuleConst::SELLING_FARE ||
               paxTypeFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
               paxTypeFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD) &&
              cri.categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE &&
              itemcat == RuleConst::ELIGIBILITY_RULE)
          {
            failIfCat1OfThen15 = true;
          }

          retCodeForQualified = currRetCode;
          if (relationalInd == CategoryRuleItemInfo::AND)
          {
            break; // job is done. fail this qualified portion.
          }
        }
        else if (UNLIKELY(currRetCode == STOP))
        {
          retCodeForQualified = STOP;
          break;
        }
        else if (currRetCode == SOFTPASS)
        {
          retCodeForQualified = currRetCode;
          paxTypeFare.setCategorySoftPassed(cri.categoryNumber());
          break;
        }
        else if (LIKELY(currRetCode == SKIP))
        {
          continue; // check next item. Save the current status.
        }
        else if (trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX && currRetCode == NOTPROCESSED)
        {
          retCodeForQualified = currRetCode;
          continue;
        }
        else // NOTPROCESSED conditions
        {
          retCodeForQualified = SKIP;
          break;
        }
      }
      else // Directionality is not match
      {
        if (UNLIKELY(trx.diagnostic().diagnosticType() != DiagnosticNone))
          diagR2sResult(trx, paxTypeFare, cfr, SKIP);

        if (directionCode == SOFTPASS)
        {
          retCodeForQualified = SOFTPASS;

          // Special logic for Fare Display
          if (trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX)
          {
            retCodeForQualified = NOTPROCESSED;
          }

          paxTypeFare.setCategorySoftPassed(cri.categoryNumber());
          break;
        }
      } // direction
    } // isQualified
  } // for

  if (UNLIKELY((retCodeForQualified == PASS) && (soInfoWrapper->stopoversRuleExistsInSet())))
  {
    retCodeForQualified = soInfoWrapper->processResults(trx, paxTypeFare);
  }

  if (UNLIKELY((retCodeForQualified == PASS) && (trInfoWrapper->transfersRuleExistsInSet())))
  {
    retCodeForQualified = trInfoWrapper->processResults(trx, paxTypeFare);
  }

  // Special logic for Cat 35 fare with THEN 15 IF 1
  if (failIfCat1OfThen15 && retCodeForQualified == FAIL)
  {
    const PaxTypeFareRuleData* paxTypeFareRuleData = nullptr;

    paxTypeFareRuleData = paxTypeFare.paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE);

    if (paxTypeFareRuleData)
    {
      // Check THEN 35 has no IF 1
      bool then35HasIf1 = false;
      const std::vector<CategoryRuleItemInfo>* crItem =
          paxTypeFareRuleData->categoryRuleItemInfoVec();
      std::vector<CategoryRuleItemInfo>::const_iterator crItemIt = crItem->begin();
      std::vector<CategoryRuleItemInfo>::const_iterator crItemItEnd = crItem->end();
      for (; crItemIt != crItemItEnd; ++crItemIt)
      {
        const CategoryRuleItemInfo* cfr = &(*crItemIt);
        uint32_t itemcat = cfr->itemcat();

        if (itemcat == RuleConst::ELIGIBILITY_RULE)
        {
          then35HasIf1 = true;
          break;
        }
      }

      // Check THEN 15 has 1S
      if (!then35HasIf1)
      {
        bool then15Has1S = false;
        cfrIter = cfrItem.begin();
        cfrIterEnd = cfrItem.end();

        for (; cfrIter != cfrIterEnd; ++cfrIter)
        {
          const CategoryRuleItemInfo* cfr = &(*cfrIter);

          uint32_t itemcat = cfr->itemcat();
          uint32_t relationalInd = cfr->relationalInd();

          if (relationalInd == CategoryRuleItemInfo::IF)
          {
            break;
          }

          if (itemcat == RuleConst::SALE_RESTRICTIONS_RULE)
          {
            const RuleItemInfo* const ruleItemInfo = RuleUtil::getRuleItemInfo(trx, &cri, cfr);
            if (ruleItemInfo)
            {
              const SalesRestriction* srInfo = dynamic_cast<const SalesRestriction*>(ruleItemInfo);
              if (srInfo && srInfo->otherCarrier() == RuleConst::SABRE1S)
              {
                then15Has1S = true;
              }
              break;
            }
          }
        }

        if (then15Has1S)
        {
          paxTypeFare.setCat35FailIf1OfThen15(true);
        }
      }
    }
    else // Cat 35 fare has not been created yet
    {
      retCodeForQualified = SOFTPASS;
    }
  }

  return isQualified;
}

bool
CategoryRuleItem::isQualifiedCategory(PricingTrx& trx,
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
                                      RuleControllerDataAccess& da)
{
  retCodeForQualified = SKIP;

  _ruleItem.setRuleControllerDataAccess(&da);

  bool isQualified = false;
  // bool   matchQualified  = false;
  bool failIfCat1OfThen15 = false;

  StopoversInfoWrapper* soInfoWrapper = rpData.soInfoWrapper();
  TransfersInfoWrapper* trInfoWrapper = rpData.trInfoWrapper();

  std::vector<CategoryRuleItemInfo>::const_iterator cfrIter = cfrItem.begin();
  std::vector<CategoryRuleItemInfo>::const_iterator cfrIterEnd = cfrItem.end();

  PaxTypeFare& paxTypeFare = *fareUsage.paxTypeFare();
  const DateTime& applDate = paxTypeFare.fareMarket()->ruleApplicationDate();

  // do the qualified portion of the RuleSet
  for (; cfrIter != cfrIterEnd; ++cfrIter)
  {
    const CategoryRuleItemInfo* cfr = &(*cfrIter);

    uint32_t itemcat = cfr->itemcat();
    uint32_t relationalInd = cfr->relationalInd();

    const RuleItemInfo* const ruleItemInfo = RuleUtil::getRuleItemInfo(trx, &cri, cfr, applDate);
    if (relationalInd == CategoryRuleItemInfo::IF) // Does a RuleSet contain a qualified rule?
    {
      isQualified = true; // Yes.
    }

    if (ruleItemInfo == nullptr)
      continue; // RuleUtil has logged the error

    if (isQualified) // Check qualified rule.
    {
      if (retCodeForQualified == FAIL && relationalInd == CategoryRuleItemInfo::AND)
      {
        break; // job is done. fail this qualified portion.
      }
      if (retCodeForQualified == PASS)
      {
        if (relationalInd == CategoryRuleItemInfo::OR)
        {
          if (LIKELY(itemcat != RuleConst::STOPOVER_RULE && itemcat != RuleConst::TRANSFER_RULE))
          {
            break; // job is done. pass this qualified portion.
          }
          else if (itemcat == RuleConst::STOPOVER_RULE)
          {
            if (soInfoWrapper->checkAllPassed())
            {
              break;
            }
            else
            {
              soInfoWrapper->clearResults();
            }
          }
          else if (itemcat == RuleConst::TRANSFER_RULE)
          {
            if (trInfoWrapper->checkAllPassed())
            {
              break;
            }
            else
            {
              trInfoWrapper->clearResults();
            }
          }
        }
      }

      Record3ReturnTypes currRetCode = SKIP;

      Record3ReturnTypes directionCode =
          isDirectionPass(trx, fareUsage, cri, cfr, isLocationSwapped);

      if (directionCode == PASS)
      {
        // matchQualified = true;  // keep it for the future...

        if (UNLIKELY(itemcat == RuleConst::STOPOVER_RULE))
        {
          const StopoversInfo* soInfo = dynamic_cast<const StopoversInfo*>(ruleItemInfo);

          // use the wrapper class
          soInfoWrapper->soInfo(soInfo);
          soInfoWrapper->stopoversRuleExistsInSet() = true;

          const RuleItemInfo* ruleInfo = soInfoWrapper;

          if (farePath)
          {
            currRetCode = _ruleItem.validate(trx,
                                             cri,
                                             cfrItem,
                                             *farePath,
                                             pricingUnit,
                                             fareUsage,
                                             cfr,
                                             ruleInfo,
                                             itemcat,
                                             isCat15Security,
                                             skipCat15Security);
          }
          else if (itin)
          {
            currRetCode = _ruleItem.validate(trx,
                                             cri,
                                             cfrItem,
                                             itin,
                                             pricingUnit,
                                             fareUsage,
                                             cfr,
                                             ruleInfo,
                                             itemcat,
                                             isCat15Security,
                                             skipCat15Security);
          }
        }
        else if (UNLIKELY(itemcat == RuleConst::TRANSFER_RULE))
        {
          trInfoWrapper->setCurrentTrInfo(dynamic_cast<const TransfersInfo1*>(ruleItemInfo));
          trInfoWrapper->transfersRuleExistsInSet() = true;

          const RuleItemInfo* ruleInfo = trInfoWrapper;

          if (farePath)
            currRetCode = _ruleItem.validate(trx,
                                             cri,
                                             cfrItem,
                                             *farePath,
                                             pricingUnit,
                                             fareUsage,
                                             cfr,
                                             ruleInfo,
                                             itemcat,
                                             isCat15Security,
                                             skipCat15Security);
          else if (itin)
          {
            currRetCode = _ruleItem.validate(trx,
                                             cri,
                                             cfrItem,
                                             itin,
                                             pricingUnit,
                                             fareUsage,
                                             cfr,
                                             ruleInfo,
                                             itemcat,
                                             isCat15Security,
                                             skipCat15Security);
          }
        }
        // Do not re-validate IF 1 of THEN Cat 25,
        // it was completely validated during Fare Collector
        else if (UNLIKELY(cri.categoryNumber() == RuleConst::FARE_BY_RULE &&
                 itemcat == RuleConst::ELIGIBILITY_RULE))
        {
          currRetCode = PASS;
        }
        // Do not re-validate IF 1 of THEN Cat 35,
        // it was completely validated during Fare Component Validation
        else if (cri.categoryNumber() == RuleConst::NEGOTIATED_RULE &&
                 itemcat == RuleConst::ELIGIBILITY_RULE)
        {
          PaxTypeFareRuleData* paxTypeFareRuleData =
              fareUsage.paxTypeFare()->paxTypeFareRuleData(cri.categoryNumber());

          // IF the Category Rule info didn't get created for some reason
          if (UNLIKELY(!paxTypeFareRuleData || !paxTypeFareRuleData->categoryRuleItemInfoSet()))
          {
            currRetCode = SKIP;
          }
          // if not the set that built the fare
          else if (cfrItem !=
                   (*paxTypeFareRuleData->categoryRuleItemInfoSet()))
          {
            currRetCode = SKIP;
          }
          else
          {
            currRetCode = PASS;
          }
        }
        else
        {
          if (farePath)
            currRetCode = _ruleItem.validate(trx,
                                             cri,
                                             cfrItem,
                                             *farePath,
                                             pricingUnit,
                                             fareUsage,
                                             cfr,
                                             ruleItemInfo,
                                             itemcat,
                                             isCat15Security,
                                             skipCat15Security);
          else if (LIKELY(itin))
            currRetCode = _ruleItem.validate(trx,
                                             cri,
                                             cfrItem,
                                             itin,
                                             pricingUnit,
                                             fareUsage,
                                             cfr,
                                             ruleItemInfo,
                                             itemcat,
                                             isCat15Security,
                                             skipCat15Security);
        }
        //APO39132: check if cat 12 is qualified with cat 4.
        //if cat4 returns SKIP, fail the rule set as we do not
        //want surcharges added when SKIP is returned.
        if ( (farePath) &&
             (cri.categoryNumber() == RuleConst::SURCHARGE_RULE) &&
             (itemcat == RuleConst::FLIGHT_APPLICATION_RULE) &&
             (currRetCode == SKIP) )
        {
           currRetCode = FAIL;
        }

        if (UNLIKELY(trx.diagnostic().diagnosticType() != DiagnosticNone))
          diagR2sResult(trx, *fareUsage.paxTypeFare(), *cfr, currRetCode);

        if (currRetCode == PASS)
        {
          retCodeForQualified = currRetCode;

          if (relationalInd == CategoryRuleItemInfo::OR)
          {
            if (itemcat == RuleConst::STOPOVER_RULE)
            {
              if (soInfoWrapper->checkAllPassed())
              {
                break;
              }
            }
            else if (itemcat == RuleConst::TRANSFER_RULE)
            {
              if (trInfoWrapper->checkAllPassed())
              {
                break;
              }
            }
            else
            {
              break; // job is done. pass this qualified portion
            }
          }
        }
        else if (currRetCode == FAIL)
        {
          // Special logic for Cat 35 fare with THEN 15 IF 1
          if (UNLIKELY(fareUsage.paxTypeFare()->isNegotiated() &&
              cri.categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE &&
              itemcat == RuleConst::ELIGIBILITY_RULE))
          {
            failIfCat1OfThen15 = true;
          }

          retCodeForQualified = currRetCode;
          if (relationalInd == CategoryRuleItemInfo::AND)
          {
            break; // job is done. fail this qualified portion.
          }
        }
        else if (UNLIKELY(currRetCode == STOP))
        {
          retCodeForQualified = STOP;
          break;
        }
        else if (LIKELY(currRetCode == SKIP))
        {
          continue; // check next item. Save the current status.
        }
        else // STOP/NOTPROCESSED conditions
        {
          retCodeForQualified = SKIP;
          break;
        }
      }
      else
      {
        if (UNLIKELY(trx.diagnostic().diagnosticType() != DiagnosticNone))
          diagR2sResult(trx, paxTypeFare, *cfr, SKIP);
      } // direction
    } // isQualified
  } // for

  if (UNLIKELY((retCodeForQualified == PASS) && (soInfoWrapper->stopoversRuleExistsInSet())))
  {
    if (farePath)
      retCodeForQualified =
          soInfoWrapper->processResults(trx, *farePath, pricingUnit, fareUsage, false);
  }

  if (UNLIKELY((retCodeForQualified == PASS) && (trInfoWrapper->transfersRuleExistsInSet())))
  {
    if (farePath)
      retCodeForQualified =
          trInfoWrapper->processResults(trx, *farePath, pricingUnit, fareUsage, false);
  }

  // Special logic for Cat 35 fare with THEN 15 IF 1
  if (UNLIKELY(failIfCat1OfThen15 && retCodeForQualified == FAIL))
  {
    PaxTypeFare& paxTypeFare = *fareUsage.paxTypeFare();

    const PaxTypeFareRuleData* paxTypeFareRuleData = nullptr;

    paxTypeFareRuleData = paxTypeFare.paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE);

    if (paxTypeFareRuleData)
    {
      // Check THEN 35 has no IF 1
      bool then35HasIf1 = false;
      const std::vector<CategoryRuleItemInfo>* crItem =
          paxTypeFareRuleData->categoryRuleItemInfoVec();
      std::vector<CategoryRuleItemInfo>::const_iterator crItemIt = crItem->begin();
      std::vector<CategoryRuleItemInfo>::const_iterator crItemItEnd = crItem->end();
      for (; crItemIt != crItemItEnd; ++crItemIt)
      {
        const CategoryRuleItemInfo* cfr = &(*crItemIt);

        uint32_t itemcat = cfr->itemcat();
        if (itemcat == RuleConst::ELIGIBILITY_RULE)
        {
          then35HasIf1 = true;
          break;
        }
      }

      // Check THEN 15 has 1S
      if (!then35HasIf1)
      {
        bool then15Has1S = false;
        cfrIter = cfrItem.begin();
        cfrIterEnd = cfrItem.end();

        for (; cfrIter != cfrIterEnd; ++cfrIter)
        {
          const CategoryRuleItemInfo* cfr = &(*cfrIter);

          uint32_t itemcat = cfr->itemcat();
          uint32_t relationalInd = cfr->relationalInd();

          if (relationalInd == CategoryRuleItemInfo::IF)
          {
            break;
          }

          if (itemcat == RuleConst::SALE_RESTRICTIONS_RULE)
          {
            const RuleItemInfo* const ruleItemInfo = RuleUtil::getRuleItemInfo(trx, &cri, cfr);
            if (ruleItemInfo)
            {
              const SalesRestriction* srInfo = dynamic_cast<const SalesRestriction*>(ruleItemInfo);
              if (srInfo && srInfo->otherCarrier() == RuleConst::SABRE1S)
              {
                then15Has1S = true;
              }
              break;
            }
          }
        }

        if (then15Has1S)
        {
          paxTypeFare.setCat35FailIf1OfThen15(true);
        }
      }
    }
  }

  return isQualified;
}

//----------------------------------------------------------------------------
// doMainCategory()
//
// return     = PASS  - when the main category is passed
//              FAIL  - when the main category appears is failed
//              SKIP  - when the main category not matched.
//              STOP  - Some problem in main category
//----------------------------------------------------------------------------
Record3ReturnTypes
CategoryRuleItem::doMainCategory(PricingTrx& trx,
                                 Itin& itin,
                                 const CategoryRuleInfo& cri,
                                 PaxTypeFare& paxTypeFare,
                                 const std::vector<CategoryRuleItemInfo>& cfrItem,
                                 bool isLocationSwapped,
                                 bool& isCat15Security,
                                 RuleProcessingData& rpData,
                                 bool isFareRule,
                                 bool skipCat15Security,
                                 RuleControllerDataAccess& da)
{ // lint !e578
  ItinRuleItemCaller ruleCaller(
      itin, trx, cri, cfrItem, paxTypeFare, isCat15Security, _ruleItem, skipCat15Security);

  Record3ReturnTypes retCode = doMainCategoryCommon(trx,
                                                    paxTypeFare,
                                                    cri,
                                                    cfrItem,
                                                    isLocationSwapped,
                                                    ruleCaller,
                                                    rpData.soInfoWrapper(),
                                                    rpData.trInfoWrapper(),
                                                    isFareRule,
                                                    da);

  if (retCode == SOFTPASS || retCode == STOP_SOFT)
    paxTypeFare.setCategorySoftPassed(cri.categoryNumber());

  return retCode;
}

Record3ReturnTypes
CategoryRuleItem::doMainCategory(PricingTrx& trx,
                                 const CategoryRuleInfo& cri,
                                 FarePath& farePath,
                                 const PricingUnit& pricingUnit,
                                 FareUsage& fareUsage,
                                 const std::vector<CategoryRuleItemInfo>& cfrItem,
                                 bool isLocationSwapped,
                                 bool& isCat15Security,
                                 RuleProcessingData& rpData,
                                 bool isFareRule,
                                 bool skipCat15Security,
                                 RuleControllerDataAccess& da)
{
  PricingUnitRuleItemCaller ruleCaller(farePath,
                                       pricingUnit,
                                       fareUsage,
                                       trx,
                                       cri,
                                       cfrItem,
                                       isCat15Security,
                                       _ruleItem,
                                       skipCat15Security);

  Record3ReturnTypes retCode = doMainCategoryCommon(trx,
                                                    *fareUsage.paxTypeFare(),
                                                    cri,
                                                    cfrItem,
                                                    isLocationSwapped,
                                                    ruleCaller,
                                                    rpData.soInfoWrapper(),
                                                    rpData.trInfoWrapper(),
                                                    isFareRule,
                                                    da,
                                                    &pricingUnit,
                                                    &fareUsage);

  return retCode;
}

Record3ReturnTypes
CategoryRuleItem::doMainCategory(PricingTrx& trx,
                                 const CategoryRuleInfo& cri,
                                 const Itin* itin,
                                 const PricingUnit& pricingUnit,
                                 FareUsage& fareUsage,
                                 const std::vector<CategoryRuleItemInfo>& cfrItem,
                                 bool isLocationSwapped,
                                 bool& isCat15Security,
                                 RuleProcessingData& rpData,
                                 bool isFareRule,
                                 bool skipCat15Security,
                                 RuleControllerDataAccess& da)
{
  PricingUnitWithoutFarePathRuleItemCaller ruleCaller(itin,
                                                      pricingUnit,
                                                      fareUsage,
                                                      trx,
                                                      cri,
                                                      cfrItem,
                                                      isCat15Security,
                                                      _ruleItem,
                                                      skipCat15Security);

  Record3ReturnTypes retCode = doMainCategoryCommon(trx,
                                                    *fareUsage.paxTypeFare(),
                                                    cri,
                                                    cfrItem,
                                                    isLocationSwapped,
                                                    ruleCaller,
                                                    rpData.soInfoWrapper(),
                                                    rpData.trInfoWrapper(),
                                                    isFareRule,
                                                    da,
                                                    &pricingUnit,
                                                    &fareUsage);

  return retCode;
}

Record3ReturnTypes
CategoryRuleItem::doMainCategoryCommon(PricingTrx& trx,
                                       PaxTypeFare& paxTypeFare,
                                       const CategoryRuleInfo& cri,
                                       const std::vector<CategoryRuleItemInfo>& cfrItem,
                                       bool isLocSwapped,
                                       RuleItemCaller& ruleCaller,
                                       StopoversInfoWrapper* soInfoWrapper,
                                       TransfersInfoWrapper* trInfoWrapper,
                                       bool isFareRule,
                                       RuleControllerDataAccess& da,
                                       const PricingUnit* pu,
                                       const FareUsage* fu)
{ // lint !e578
  Record3ReturnTypes currRetCode = SKIP;
  Record3ReturnTypes retCode = SKIP;
  // bool matchRuleItem               = false;
  bool softPass = false;
  bool isInbound = false;

  // Special logic for Fare Display

  FareDisplayTrx* fdt;
  FareDisplayUtil::getFareDisplayTrx(&trx, fdt);

  if (UNLIKELY(fdt))
  {
    if (cri.categoryNumber() == RuleConst::BLACKOUTS_RULE ||
        cri.categoryNumber() == RuleConst::DAY_TIME_RULE ||
        cri.categoryNumber() == RuleConst::SEASONAL_RULE)
    {
      retCode = doMainCategoryCommon(trx,
                                     paxTypeFare,
                                     cri,
                                     cfrItem,
                                     isLocSwapped,
                                     ruleCaller,
                                     soInfoWrapper,
                                     trInfoWrapper,
                                     isFareRule,
                                     isInbound);
      /*
              if ((((retCode == FAIL || retCode == SKIP||(retCode == PASS && seasonTemplate )) &&
                     cri.categoryNumber() == RuleConst::SEASONAL_RULE) ||
                  ((retCode == FAIL || retCode == SKIP) &&
                     cri.categoryNumber() == RuleConst::DAY_TIME_RULE)) &&
                  paxTypeFare.fare()->owrt() != ONE_WAY_MAYNOT_BE_DOUBLED &&
                  fdt->getRequest()->returnDate().isValid())
      */
      if ((retCode == FAIL || retCode == SKIP) &&
          (cri.categoryNumber() == RuleConst::SEASONAL_RULE ||
           cri.categoryNumber() == RuleConst::DAY_TIME_RULE ||
           cri.categoryNumber() == RuleConst::BLACKOUTS_RULE) &&
          paxTypeFare.fare()->owrt() != ONE_WAY_MAYNOT_BE_DOUBLED &&
          fdt->getRequest()->returnDate().isValid())
      {
        isInbound = true;
        Record3ReturnTypes saveRetCode = retCode;
        bool isSwappedLocation = (isLocSwapped ? false : true);
        retCode = doMainCategoryCommon(trx,
                                       paxTypeFare,
                                       cri,
                                       cfrItem,
                                       isSwappedLocation,
                                       ruleCaller,
                                       soInfoWrapper,
                                       trInfoWrapper,
                                       isFareRule,
                                       isInbound);

        //  If outbound - FAIL and inbound - SKIP then return FAIL
        if (retCode == SKIP && saveRetCode == FAIL)
          return saveRetCode;
      }
      return retCode;
    }
  }

  // For Category 25 - Fare By Rule
  if (cri.categoryNumber() == RuleConst::FARE_BY_RULE ||
      cri.categoryNumber() == RuleConst::NEGOTIATED_RULE ||
      cri.categoryNumber() == RuleConst::CHILDREN_DISCOUNT_RULE ||
      cri.categoryNumber() == RuleConst::TOUR_DISCOUNT_RULE ||
      cri.categoryNumber() == RuleConst::AGENTS_DISCOUNT_RULE ||
      cri.categoryNumber() == RuleConst::OTHER_DISCOUNT_RULE)
  {
    const PaxTypeFareRuleData* paxTypeFareRuleData = nullptr;

    if (cri.categoryNumber() == RuleConst::FARE_BY_RULE)
    {
      paxTypeFareRuleData = paxTypeFare.paxTypeFareRuleData(RuleConst::FARE_BY_RULE);
      const FBRPaxTypeFareRuleData* fbrPaxTypeFareRuleData =
          paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);

      if (fbrPaxTypeFareRuleData->fbrApp()->directionality() != RuleConst::ALWAYS_APPLIES)
      {
        retCode = ruleCaller.isR8DirectionPass(fbrPaxTypeFareRuleData->fbrApp()->directionality(),
                                               fbrPaxTypeFareRuleData->isR8LocationSwapped());
      }
    }

    if (cri.categoryNumber() == RuleConst::NEGOTIATED_RULE)
    {
      paxTypeFareRuleData = paxTypeFare.paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE);
    }
    else if (UNLIKELY(cri.categoryNumber() == RuleConst::CHILDREN_DISCOUNT_RULE ||
             cri.categoryNumber() == RuleConst::TOUR_DISCOUNT_RULE ||
             cri.categoryNumber() == RuleConst::AGENTS_DISCOUNT_RULE ||
             cri.categoryNumber() == RuleConst::OTHER_DISCOUNT_RULE))
    {
      paxTypeFareRuleData = paxTypeFare.paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE);
    }

    if (UNLIKELY(!paxTypeFareRuleData))
      throw std::runtime_error(
          "PaxTypeFareRuleData not set in CategoryRuleItem::doMainCategoryCommon");
    const CategoryRuleItemInfo* fbrCfrItem = paxTypeFareRuleData->categoryRuleItemInfo();
    bool isLocationSwapped = paxTypeFareRuleData->isLocationSwapped();

    if ((retCode == SKIP || retCode == PASS) && fbrCfrItem != nullptr)
    {
      if ((fbrCfrItem->directionality() == RuleConst::ALWAYS_APPLIES ||
           fbrCfrItem->directionality() == RuleConst::FROM_LOC1_TO_LOC2 ||
           fbrCfrItem->directionality() == RuleConst::TO_LOC1_FROM_LOC2) &&
          fbrCfrItem->inOutInd() == RuleConst::ALWAYS_APPLIES)
      {
        retCode = PASS;
      }
      else
      {
        retCode = ruleCaller.isDirectionPass(fbrCfrItem, isLocationSwapped);
      }
    }

    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic500))
    {
      DCFactory* factory = DCFactory::instance();
      Diag500Collector* diag = dynamic_cast<Diag500Collector*>(factory->create(trx));
      diag->enable(Diagnostic500);
      if (diag->givenValidationPhase() == Diag500Collector::ANY ||
          diag->givenValidationPhase() == diag->validationPhase())
      {
        diag->displayRelation(paxTypeFare, fbrCfrItem, retCode);
      }
      diag->flushMsg();
    }
    return retCode;
  }

  PaxTypeFareRuleData* paxTypeFareRuleData = nullptr;
  if (isFareRule)
  {
    // For other Category Rules
    // set the PaxTypeFareRuleData in the PaxTypeFare
    // for FareRule only
    paxTypeFareRuleData = paxTypeFare.paxTypeFareRuleData(cri.categoryNumber());

    if (LIKELY(paxTypeFareRuleData))
    {
      paxTypeFareRuleData->categoryRuleItemInfoVec() = &cfrItem;
    }
  }

  std::vector<CategoryRuleItemInfo>::const_iterator cfrIter = cfrItem.begin();
  std::vector<CategoryRuleItemInfo>::const_iterator cfrIterEnd = cfrItem.end();

  bool transferSoftPass = false;

  bool isCmdPricing = false;
  const PaxTypeFare* cat25Fare = fu ? fu->cat25Fare() : nullptr;
  if (UNLIKELY(((paxTypeFare.isFareByRule() || !cat25Fare) && paxTypeFare.isCmdPricing()) ||
      (cat25Fare && cat25Fare->isCmdPricing())))
  {
    isCmdPricing = true;
  }

  bool ruleTuningISSimple = false;
  if ((trx.getTrxType() == PricingTrx::IS_TRX) && (trx.isAltDates()))
  {
    ShoppingTrx* shopTrx = static_cast<ShoppingTrx*>(&trx);
    if (LIKELY((shopTrx != nullptr) && (shopTrx->isSimpleTrip()) && (shopTrx->isRuleTuningISProcess())))
    {
      ruleTuningISSimple = true;
    }
  }

  const DateTime& applDate = paxTypeFare.fareMarket()->ruleApplicationDate();

  // do the main category portion of the RuleSet
  for (; cfrIter != cfrIterEnd; ++cfrIter)
  {
    const CategoryRuleItemInfo* cfr = &(*cfrIter);

    uint32_t itemcat = cfr->itemcat();
    uint32_t relationalInd = cfr->relationalInd();

    if ((itemcat == RuleConst::TRANSFER_RULE) && transferSoftPass)
      continue;

    const RuleItemInfo* const ruleItemInfo = RuleUtil::getRuleItemInfo(trx, &cri, cfr, applDate);
    if (ruleItemInfo == nullptr)
      continue;

    Record3ReturnTypes directionCode = isDirectionalityMatch(trx, paxTypeFare, cfr, cri, ruleCaller,
                                                             ruleTuningISSimple, itemcat, isLocSwapped);
    //APO-45023: if direction fails, check if intl OOJ pu. In ooj pu if fare usage is inbound and
    //cat 2 io byte is outbound, pass the directionality for cat 2 only
    if (!fallback::apo45023ApplyCat2DefaultsInOOJPU(&trx) )
    {
       if ((directionCode == FAIL) && pu)
       {
       //check if it an intl OOJ
          bool oojPu = (pu->puSubType() == PricingUnit::ORIG_OPENJAW) ;
          if ( oojPu && !(pu->sameNationOJ()) &&
               (cfr->inOutInd() == RuleConst::FARE_MARKET_OUTBOUND) &&
               (fu->isInbound())  &&
               (cfr->itemcat() == RuleConst::DAY_TIME_RULE) )
          directionCode = PASS;
       }
    }
    if (_categoryPhase == LoadRecords &&
        (!fallback::smpShoppingISCoreFix(&trx) ||
         (paxTypeFare.paxType() && paxTypeFare.paxType()->maxPenaltyInfo())) &&
        directionCode == PASS)
    {
      switch(cfr->itemcat())
      {
        case RuleConst::PENALTIES_RULE:
        {
          const PenaltyInfo* record = static_cast<const PenaltyInfo*>(ruleItemInfo);
          paxTypeFare.addPenaltyInfo(record);
          continue;
        }

        case RuleConst::VOLUNTARY_EXCHANGE_RULE:
        {
          if (smp::validateOverrideDateTable(trx, ruleItemInfo->vendor(),
                                             ruleItemInfo->overrideDateTblItemNo(), applDate))
          {
            VoluntaryChangesInfoW* voluntChgInfoW = trx.dataHandle().create<VoluntaryChangesInfoW>();
            voluntChgInfoW->orig() = static_cast<const VoluntaryChangesInfo*>(ruleItemInfo);
            paxTypeFare.addVoluntaryChangesInfo(voluntChgInfoW);
          }
          continue;
        }

        case RuleConst::VOLUNTARY_REFUNDS_RULE:
        {
          if (smp::validateOverrideDateTable(trx, ruleItemInfo->vendor(),
                                             ruleItemInfo->overrideDateTblItemNo(), applDate))
          {
            const VoluntaryRefundsInfo* volRef =
                                            static_cast<const VoluntaryRefundsInfo*>(ruleItemInfo);
            paxTypeFare.addVoluntaryRefundsInfo(volRef);
          }
          continue;
        }
      }
    }

    // Every next THEN set for CAT8 needs to be cleared
    // of the latest set data stored
    if ((relationalInd == CategoryRuleItemInfo::THEN ||
         relationalInd == CategoryRuleItemInfo::ELSE) &&
        (itemcat == RuleConst::STOPOVER_RULE))
    {
      if (!(soInfoWrapper->checkAllPassed()))
      {
        soInfoWrapper->clearResults(true);
      }
    }
    // Every next THEN set for CAT9 needs to be cleared
    // of the latest set data stored
    if ((relationalInd == CategoryRuleItemInfo::THEN ||
         relationalInd == CategoryRuleItemInfo::ELSE) &&
        (itemcat == RuleConst::TRANSFER_RULE))
    {
      if (!(trInfoWrapper->checkAllPassed()))
      {
        trInfoWrapper->clearResults();
      }
    }
    // Does a RuleSet contain a qualified rule?
    if (relationalInd == CategoryRuleItemInfo::IF)
    {
      if (softPass)
      {
        return SOFTPASS;
      }
      // for Stopover/Transfer, if there is qualifier category, need to stop process next set
      if (cri.categoryNumber() == RuleConst::STOPOVER_RULE)
      {
        //SSDP:1205: there is a bug in checkAllPassed method below.
        //if the main portion returned fail, then return fail here.
        if (retCode == FAIL)
           return  FAIL;
        return soInfoWrapper->checkAllPassed()? PASS:FAIL;
      }
      else if (cri.categoryNumber() == RuleConst::TRANSFER_RULE)
      {
        return trInfoWrapper->checkAllPassed()? PASS:FAIL;
      }

      return retCode; // Job is done.
    }

    if (retCode == FAIL)
    {
      if (UNLIKELY(itemcat == RuleConst::BLACKOUTS_RULE))
      {
        // once fail on any Cat11, fail the fare
        return FAIL; // job is done.
      }

      if (UNLIKELY(itemcat == RuleConst::STOPOVER_RULE && pu))
      {
        if (isCmdPricing)
        {
          std::vector<CategoryRuleItemInfo>::const_iterator currentCfrIter = cfrIter;
          ++currentCfrIter;
          if (relationalInd == CategoryRuleItemInfo::AND || currentCfrIter == cfrIterEnd)
          {
            const StopoversInfo* soInfo = dynamic_cast<const StopoversInfo*>(ruleItemInfo);
            soInfoWrapper->setMostRestrictiveMaxStop(const_cast<PricingUnit*>(pu), soInfo);
          }
        }
      }

      if (relationalInd == CategoryRuleItemInfo::AND && itemcat != RuleConst::TOURS_RULE)
      {
        return FAIL; // job is done. fail this main portion.
      }
    }
    else if (retCode == PASS)
    {
      if (relationalInd == CategoryRuleItemInfo::OR)
      {
        if (itemcat != RuleConst::STOPOVER_RULE && itemcat != RuleConst::TRANSFER_RULE &&
            itemcat != RuleConst::SURCHARGE_RULE && itemcat != RuleConst::BLACKOUTS_RULE &&
            itemcat != RuleConst::PENALTIES_RULE && // OR is processed like AND
            itemcat != RuleConst::TICKET_ENDORSMENT_RULE && // for cat16/18
            itemcat != RuleConst::VOLUNTARY_EXCHANGE_RULE &&
            itemcat != RuleConst::VOLUNTARY_REFUNDS_RULE)
        {
          return PASS; // job is done. pass this main portion.
        }
        else if (itemcat == RuleConst::SURCHARGE_RULE)
        {
          return PASS; // job is done. pass this main portion
        }
        else if (itemcat == RuleConst::STOPOVER_RULE)
        {
          if (soInfoWrapper->checkAllPassed())
          {
            return PASS;
          }
          else
          {
            soInfoWrapper->clearResults(true);
          }
        }
        else if (itemcat == RuleConst::TRANSFER_RULE)
        {
          if (trInfoWrapper->checkAllPassed())
          {
            return PASS;
          }
          else
          {
            trInfoWrapper->clearResults();
          }
        }
      }
      /* add 'else if' after removal of fallback */
      if (UNLIKELY(itemcat == RuleConst::TOURS_RULE && relationalInd == CategoryRuleItemInfo::AND))
        return PASS;
    }
    else if (retCode == SOFTPASS)
    {
      if (relationalInd == CategoryRuleItemInfo::AND)
      {
        if (itemcat != RuleConst::STOPOVER_RULE && itemcat != RuleConst::TRANSFER_RULE)
        {
          return SOFTPASS;
        }
      }
      else if (LIKELY(relationalInd == CategoryRuleItemInfo::OR))
      {
        if (itemcat == RuleConst::STOPOVER_RULE)
        {
          if (soInfoWrapper->checkAllPassed())
          {
            return SOFTPASS;
          }
          else
          {
            soInfoWrapper->clearResults();
          }
        }
        else if (itemcat == RuleConst::TRANSFER_RULE)
        {
          if (trInfoWrapper->checkAllPassed())
          {
            return SOFTPASS;
          }
          else
          {
            trInfoWrapper->clearResults();
          }
        }
      }
    }

    currRetCode = SKIP;

    if (directionCode == PASS)
    {
      // matchRuleItem = true; // keep it for the future...

      if (itemcat == RuleConst::STOPOVER_RULE)
      {
        const StopoversInfo* soInfo = dynamic_cast<const StopoversInfo*>(ruleItemInfo);

        // APO-36991 ifrelational Ind is THEN or OR,clear results inwrapper.
        if ( ((relationalInd == CategoryRuleItemInfo::OR) ||
             (relationalInd == CategoryRuleItemInfo::THEN)))
        {
          soInfoWrapper->clearResults(false);
        }
        soInfoWrapper->setRtw(trx.getOptions() && trx.getOptions()->isRtw());
        soInfoWrapper->sumNumStopoversMax(relationalInd, soInfo);

        // use the wrapper class
        soInfoWrapper->soInfo(soInfo);
        soInfoWrapper->stopoversRuleExistsInSet() = true;

        currRetCode = ruleCaller(const_cast<CategoryRuleItemInfo*>(cfr), soInfoWrapper);
      }
      else if (itemcat == RuleConst::TRANSFER_RULE)
      {
        const TransfersInfo1* trInfo = dynamic_cast<const TransfersInfo1*>(ruleItemInfo);
        // use the wrapper class
        if (fallback::cat9FixMaxTransfers(&trx))
          trInfoWrapper->setCurrentTrInfo(trInfo);
        else
          trInfoWrapper->setCurrentTrInfo(trInfo, ruleCaller, isLocSwapped);
        trInfoWrapper->transfersRuleExistsInSet() = true;

        currRetCode = ruleCaller(const_cast<CategoryRuleItemInfo*>(cfr), trInfoWrapper);
      }
      else
      {
        currRetCode = ruleCaller(const_cast<CategoryRuleItemInfo*>(cfr));
      }

      if (UNLIKELY(trx.diagnostic().diagnosticType() != DiagnosticNone))
        diagR2sResult(trx, paxTypeFare, *cfr, currRetCode);

      if (currRetCode == FAIL)
      {
        retCode = currRetCode;
        if (itemcat == RuleConst::BLACKOUTS_RULE)
        {
          // once fail on any Cat11, fail the fare
          return FAIL; // job is done.
        }

        if (itemcat == RuleConst::STOPOVER_RULE && pu)
        {
          if (isCmdPricing)
          {
            std::vector<CategoryRuleItemInfo>::const_iterator currentCfrIter = cfrIter;
            ++currentCfrIter;
            if (relationalInd == CategoryRuleItemInfo::AND || currentCfrIter == cfrIterEnd)
            {
              const StopoversInfo* soInfo = dynamic_cast<const StopoversInfo*>(ruleItemInfo);
              soInfoWrapper->setMostRestrictiveMaxStop(const_cast<PricingUnit*>(pu), soInfo);
            }
          }
          // apo38826:  check if the OR rule is the last one in the rule set. if so return fail.
          if (relationalInd == CategoryRuleItemInfo::OR)
          {
             std::vector<CategoryRuleItemInfo>::const_iterator nextCfrIter = cfrIter;
             ++nextCfrIter;
             //if the next item is not linked by OR then this R3 is the last one, so fail it
             if ((nextCfrIter == cfrIterEnd) ||
                 (nextCfrIter->relationalInd()!=CategoryRuleItemInfo::OR) )
                return FAIL;
          }
        }

        if (relationalInd == CategoryRuleItemInfo::AND && itemcat != RuleConst::TOURS_RULE)
        {
          return FAIL; // job is done. fail this main portion.
        }
      }
      else if (currRetCode == PASS)
      {
        // set the two elements of the PaxTypeFareRuleData in the PaxTypeFare in the loop
        if (paxTypeFareRuleData)
        {
          paxTypeFareRuleData->categoryRuleItemInfo() = cfr;
          paxTypeFareRuleData->ruleItemInfo() = ruleItemInfo;
        }

        if (trx.isValidatingCxrGsaApplicable())
        {
          if (itemcat == RuleConst::SALE_RESTRICTIONS_RULE && ruleItemInfo->textTblItemNo() != 0)
          {
            if (da.getRuleType() == FootNote )
               paxTypeFare.setCat15HasT996FT(true);
            else if (da.getRuleType() == FareRule )
               paxTypeFare.setCat15HasT996FR(true);
            else if (LIKELY(da.getRuleType() == GeneralRule ))
               paxTypeFare.setCat15HasT996GR(true);
          }
        }
        retCode = currRetCode;

        if (relationalInd == CategoryRuleItemInfo::OR)
        {
          if (itemcat != RuleConst::STOPOVER_RULE && itemcat != RuleConst::TRANSFER_RULE &&
              itemcat != RuleConst::SURCHARGE_RULE && itemcat != RuleConst::BLACKOUTS_RULE &&
              itemcat != RuleConst::PENALTIES_RULE && // OR is processed like AND
              itemcat != RuleConst::TICKET_ENDORSMENT_RULE && // for cat16/18
              itemcat != RuleConst::VOLUNTARY_EXCHANGE_RULE &&
              itemcat != RuleConst::VOLUNTARY_REFUNDS_RULE)
          {
            return PASS; // job is done. pass this main portion.
          }
          else if (itemcat == RuleConst::STOPOVER_RULE)
          {
            if (soInfoWrapper->checkAllPassed())
            {
              return PASS;
            }
          }
          else if (itemcat == RuleConst::TRANSFER_RULE)
          {
            if (trInfoWrapper->checkAllPassed())
            {
              return PASS;
            }
          }
          else if (itemcat == RuleConst::SURCHARGE_RULE)
          {
            return PASS; // job is done. pass this main portion.
          }
        }
        /* add 'else if' after removal of fallback */
        if (UNLIKELY(itemcat == RuleConst::TOURS_RULE && relationalInd == CategoryRuleItemInfo::AND))
          return PASS;
      }
      else if (currRetCode == SOFTPASS)
      {
        softPass = true;
        retCode = SOFTPASS;
      }
      else if (currRetCode != SKIP) // STOP/NOTPROCESSED conditions
      {
        if (LIKELY(itemcat == RuleConst::ADVANCE_RESERVATION_RULE))
        {
          if (LIKELY(currRetCode != NOTPROCESSED))
            return currRetCode; // STOP/STOP_SOFT
        }

        retCode = SKIP;
        return SKIP;
      }
      else // SKIP condition
      {
        if (trx.isValidatingCxrGsaApplicable())
        {
          if (itemcat == RuleConst::SALE_RESTRICTIONS_RULE && ruleItemInfo->textTblItemNo() != 0)
          {
            if (UNLIKELY(da.getRuleType() == FootNote ))
               paxTypeFare.setCat15HasT996FT(true);
            else if (da.getRuleType() == FareRule )
               paxTypeFare.setCat15HasT996FR(true);
            else if (LIKELY(da.getRuleType() == GeneralRule ))
               paxTypeFare.setCat15HasT996GR(true);
          }
        }
        continue;
      }
    }
    else // Directionality is not match
    {
      if (UNLIKELY(trx.diagnostic().diagnosticType() != DiagnosticNone))
        diagR2sResult(trx, paxTypeFare, *cfr, SKIP);

      if (directionCode == SOFTPASS)
      {
        softPass = true;
        retCode = SOFTPASS;

        if ((itemcat == RuleConst::TRANSFER_RULE) && (pu == nullptr)) // Only for Fare Validation phase
          transferSoftPass = true;
      }
    }
  } // for

  if (softPass)
  {
    return SOFTPASS;
  }

  return retCode;
}

//
// This function is for Fare Display
//
Record3ReturnTypes
CategoryRuleItem::doMainCategoryCommon(PricingTrx& trx,
                                       PaxTypeFare& paxTypeFare,
                                       const CategoryRuleInfo& cri,
                                       const std::vector<CategoryRuleItemInfo>& cfrItem,
                                       bool isLocSwapped,
                                       RuleItemCaller& ruleCaller,
                                       StopoversInfoWrapper* soInfoWrapper,
                                       TransfersInfoWrapper* trInfoWrapper,
                                       bool isFareRule,
                                       bool isInbound)
{
  Record3ReturnTypes currRetCode = SKIP;
  Record3ReturnTypes retCode = SKIP;
  Record3ReturnTypes seasonRetCode = SKIP;
  bool softPass = false;
  bool seasonTemplate = false;

  PaxTypeFareRuleData* paxTypeFareRuleData = nullptr;
  if (isFareRule)
  {
    // For other Category Rules
    // set the PaxTypeFareRuleData in the PaxTypeFare
    // for FareRule only
    paxTypeFareRuleData = paxTypeFare.paxTypeFareRuleData(cri.categoryNumber());

    if (paxTypeFareRuleData)
    {
      paxTypeFareRuleData->categoryRuleItemInfoVec() = &cfrItem;
    }
  }

  FareDisplayTrx* fdt;
  FareDisplayUtil::getFareDisplayTrx(&trx, fdt);
  if (fdt)
    seasonTemplate = (fdt->isSeasonTemplate() && (fdt->isFQ() || fdt->isRD()));

  const DateTime& applDate = paxTypeFare.fareMarket()->ruleApplicationDate();

  // do the main category portion of the RuleSet
  for (const CategoryRuleItemInfo& cfr : cfrItem)
  {
    uint32_t itemcat = cfr.itemcat();
    uint32_t relationalInd = cfr.relationalInd();

    const RuleItemInfo* const ruleItemInfo = RuleUtil::getRuleItemInfo(trx, &cri, &cfr, applDate);
    if (ruleItemInfo == nullptr)
      continue;

    // Does a RuleSet contain a qualified rule?
    if (relationalInd == CategoryRuleItemInfo::IF)
    {
      if (seasonRetCode == PASS)
      {
        return seasonRetCode;
      }

      return retCode; // Job is done.
    }

    if (retCode == FAIL)
    {
      if (itemcat == RuleConst::BLACKOUTS_RULE)
      {
        // once fail on any Cat11, fail the fare
        return FAIL; // job is done.
      }

      if (relationalInd == CategoryRuleItemInfo::AND && itemcat != RuleConst::TOURS_RULE)
      {
        return FAIL; // job is done. fail this main portion.
      }
    }
    else if (retCode == PASS)
    {
      if (relationalInd == CategoryRuleItemInfo::OR)
      {
        if (itemcat != RuleConst::STOPOVER_RULE && itemcat != RuleConst::TRANSFER_RULE &&
            itemcat != RuleConst::BLACKOUTS_RULE && itemcat != RuleConst::SEASONAL_RULE &&
            itemcat != RuleConst::PENALTIES_RULE &&
            itemcat != RuleConst::TICKET_ENDORSMENT_RULE) // OR is processed like AND
        // for cat16/18
        {
          return PASS; // job is done. pass this main portion.
        }
        else if (itemcat == RuleConst::SEASONAL_RULE)
        {
          if (!seasonTemplate)
            return PASS;
        }
        else if (itemcat == RuleConst::STOPOVER_RULE)
        {
          if (soInfoWrapper->checkAllPassed())
          {
            return PASS;
          }
          else
          {
            soInfoWrapper->clearResults();
          }
        }
        else if (itemcat == RuleConst::TRANSFER_RULE)
        {
          if (trInfoWrapper->checkAllPassed())
          {
            return PASS;
          }
          else
          {
            trInfoWrapper->clearResults();
          }
        }
      }
      /* add 'else if' after removal of fallback */
      if (itemcat == RuleConst::TOURS_RULE && relationalInd == CategoryRuleItemInfo::AND)
        return PASS;
    }
    else if (retCode == SOFTPASS)
    {
      if (relationalInd == CategoryRuleItemInfo::AND)
      {
        if (itemcat != RuleConst::STOPOVER_RULE && itemcat != RuleConst::TRANSFER_RULE)
        {
          return SOFTPASS;
        }
      }
      else if (relationalInd == CategoryRuleItemInfo::OR)
      {
        if (itemcat == RuleConst::STOPOVER_RULE)
        {
          if (soInfoWrapper->checkAllPassed())
          {
            return SOFTPASS;
          }
          else
          {
            soInfoWrapper->clearResults();
          }
        }
        else if (itemcat == RuleConst::TRANSFER_RULE)
        {
          if (trInfoWrapper->checkAllPassed())
          {
            return SOFTPASS;
          }
          else
          {
            trInfoWrapper->clearResults();
          }
        }
      }
    }

    currRetCode = SKIP;
    Record3ReturnTypes directionCode = ruleCaller.isDirectionPass(&cfr, isLocSwapped);

    if (directionCode == PASS)
    {
      // matchRuleItem = true; // keep it for the future...

      if (itemcat == RuleConst::STOPOVER_RULE)
      {
        const StopoversInfo* soInfo = dynamic_cast<const StopoversInfo*>(ruleItemInfo);

        // use the wrapper class
        soInfoWrapper->soInfo(soInfo);
        soInfoWrapper->stopoversRuleExistsInSet() = true;

        currRetCode = ruleCaller(const_cast<CategoryRuleItemInfo*>(&cfr), soInfoWrapper);
      }
      else if (itemcat == RuleConst::TRANSFER_RULE)
      {
        const TransfersInfo1* trInfo = dynamic_cast<const TransfersInfo1*>(ruleItemInfo);
        // use the wrapper class
        if (fallback::cat9FixMaxTransfers(&trx))
          trInfoWrapper->setCurrentTrInfo(trInfo);
        else
          trInfoWrapper->setCurrentTrInfo(trInfo, ruleCaller, isLocSwapped);
        trInfoWrapper->transfersRuleExistsInSet() = true;

        currRetCode = ruleCaller(const_cast<CategoryRuleItemInfo*>(&cfr), trInfoWrapper);
      }
      else
      {
        currRetCode = ruleCaller(const_cast<CategoryRuleItemInfo*>(&cfr), isInbound);
      }

      if (UNLIKELY(trx.diagnostic().diagnosticType() != DiagnosticNone))
        diagR2sResult(trx, paxTypeFare, cfr, currRetCode);

      if (currRetCode == FAIL)
      {
        retCode = currRetCode;
        if (itemcat == RuleConst::BLACKOUTS_RULE)
        {
          // once fail on any Cat11, fail the fare
          return FAIL; // job is done.
        }

        if (relationalInd == CategoryRuleItemInfo::AND && itemcat != RuleConst::TOURS_RULE)
        {
          return FAIL; // job is done. fail this main portion.
        }
      }
      else if (currRetCode == PASS)
      {
        // set the two elements of the PaxTypeFareRuleData in the PaxTypeFare in the loop
        if (paxTypeFareRuleData)
        {
          paxTypeFareRuleData->categoryRuleItemInfo() = &cfr;
          paxTypeFareRuleData->ruleItemInfo() = ruleItemInfo;
        }

        retCode = currRetCode;
        if (itemcat == RuleConst::SEASONAL_RULE)
          seasonRetCode = PASS;

        if (relationalInd == CategoryRuleItemInfo::OR)
        {
          if (itemcat != RuleConst::STOPOVER_RULE && itemcat != RuleConst::TRANSFER_RULE &&
              itemcat != RuleConst::SURCHARGE_RULE && itemcat != RuleConst::BLACKOUTS_RULE &&
              itemcat != RuleConst::SEASONAL_RULE &&
              itemcat != RuleConst::PENALTIES_RULE && // OR is processed like AND
              itemcat != RuleConst::TICKET_ENDORSMENT_RULE) // for cat16/18
          {
            return PASS; // job is done. pass this main portion.
          }

          else if (itemcat == RuleConst::SEASONAL_RULE)
          {
            if (!seasonTemplate)
              return PASS;
          }
          else if (itemcat == RuleConst::STOPOVER_RULE)
          {
            if (soInfoWrapper->checkAllPassed())
            {
              return PASS;
            }
          }
          else if (itemcat == RuleConst::TRANSFER_RULE)
          {
            if (trInfoWrapper->checkAllPassed())
            {
              return PASS;
            }
          }
          else if (itemcat == RuleConst::SURCHARGE_RULE)
          {
            return PASS; // job is done. pass this main portion.
          }
        }
        /* add 'else if' after removal of fallback */
        if (itemcat == RuleConst::TOURS_RULE && relationalInd == CategoryRuleItemInfo::AND)
          return PASS;
      }
      else if (currRetCode == SOFTPASS)
      {
        softPass = true;
        retCode = SOFTPASS;
      }
      else if (currRetCode != SKIP) // STOP/NOTPROCESSED conditions
      {
        retCode = SKIP;
        return SKIP;
      }
      else // SKIP condition
      {
        continue;
      }
    }
    else // Directionality is not match
    {
      if (UNLIKELY(trx.diagnostic().diagnosticType() != DiagnosticNone))
        diagR2sResult(trx, paxTypeFare, cfr, SKIP);

      if (directionCode == SOFTPASS)
      {
        softPass = true;
        retCode = SOFTPASS;
      }
    }
  } // for

  if (softPass)
  {
    return SOFTPASS;
  }
  if (seasonRetCode == PASS)
  {
    return seasonRetCode;
  }
  return retCode;
}

//----------------------------------------------------------------------------
// isDirectionPass()
// return PASS - pass directionality for the current RuleItem
//        FAIL - fail directionality for the current RuleItem
//        SOFTPASS - can not validate at this time
//
//----------------------------------------------------------------------------

inline static bool
isCertainlyFirstFm(const PricingTrx& trx, const Itin& itin, const FareMarket& fm)
{
  assert(!itin.travelSeg().empty());

  if (trx.getTrxType() == PricingTrx::IS_TRX)
    return fm.legIndex() == 0 && fm.boardMultiCity() == itin.firstTravelSeg()->boardMultiCity();

  assert(!fm.travelSeg().empty());
  return fm.travelSeg().front() == itin.firstTravelSeg();
}

inline static bool
isCarnivalOwQueuePossible(const PricingTrx& trx)
{
  return trx.getOptions() && trx.getOptions()->isCarnivalSumOfLocal() &&
         trx.getOptions()->getAdditionalItinsRequestedForOWFares() > 0;
}

// If you're interested where the conditions below come from visit PricingUnitFactory code
// that initializes fareUsage.inbound() field. The code below should predict as accurately
// as possible that value during PU-level validation.

inline static FMDirection
guessFareComponentDir(const PricingTrx& trx, const PaxTypeFare& ptf, const Itin* itin)
{
  const Indicator fareDir = ptf.directionality();

  if (fareDir == TO && !isCarnivalOwQueuePossible(trx))
    return FMDirection::INBOUND;

  if (itin && isCertainlyFirstFm(trx, *itin, *ptf.fareMarket()))
    return FMDirection::OUTBOUND;

  // In case of Int. Open Jaws outbound fares can be used on inbound parts
  if (fareDir == FROM && itin && itin->geoTravelType() != GeoTravelType::International)
    return FMDirection::OUTBOUND;

  return FMDirection::UNKNOWN;
}

inline static boost::logic::tribool
checkR2sInOut(const CategoryRuleItemInfo& r2s, const FMDirection actualDir)
{
  if (r2s.inOutInd() == RuleConst::ALWAYS_APPLIES)
    return true;

  if (actualDir == FMDirection::UNKNOWN)
    return boost::indeterminate;

  if (actualDir == FMDirection::INBOUND &&
      r2s.inOutInd() == RuleConst::FARE_MARKET_INBOUND)
    return true;
  else if (actualDir == FMDirection::OUTBOUND &&
    r2s.inOutInd() == RuleConst::FARE_MARKET_OUTBOUND)
     return true;

  return false;
}

inline static boost::logic::tribool
checkR2sDirectionality(const CategoryRuleItemInfo& r2s,
                       const bool forwardLocMatch,
                       const FMDirection actualDir)
{
  switch (r2s.directionality())
  {
  case RuleConst::ALWAYS_APPLIES:
    return true;

  case RuleConst::FROM_LOC1_TO_LOC2:
    return forwardLocMatch;

  case RuleConst::TO_LOC1_FROM_LOC2:
    return !forwardLocMatch;

  case RuleConst::ORIGIN_FROM_LOC1_TO_LOC2:
    if (actualDir == FMDirection::UNKNOWN)
      return boost::indeterminate;
    return (actualDir == FMDirection::OUTBOUND) == forwardLocMatch;

  case RuleConst::ORIGIN_FROM_LOC2_TO_LOC1:
    if (actualDir == FMDirection::UNKNOWN)
      return boost::indeterminate;
    return (actualDir == FMDirection::OUTBOUND) == !forwardLocMatch;

  default:
    return false;
  }
}

Record3ReturnTypes
CategoryRuleItem::isDirectionPass(const PricingTrx& trx,
                                  const PaxTypeFare& paxTypeFare,
                                  const CategoryRuleItemInfo* r2s,
                                  bool isLocationSwapped,
                                  const Itin* itin)
{
  if (fallback::r2sDirectionalityOpt(&trx))
    return isDirectionPassOld(trx, paxTypeFare, r2s, isLocationSwapped, itin);

  if (UNLIKELY(trx.getOptions()->isRtw()))
    return r2s->inOutInd() == RuleConst::ALWAYS_APPLIES ? PASS : FAIL;

  if (r2s->inOutInd() == RuleConst::ALWAYS_APPLIES &&
      r2s->directionality() == RuleConst::ALWAYS_APPLIES)
    return PASS;

  const FMDirection fareComponentDir = guessFareComponentDir(trx, paxTypeFare, itin);

  const boost::logic::tribool result =
      checkR2sInOut(*r2s, fareComponentDir) &&
      checkR2sDirectionality(*r2s, !isLocationSwapped, fareComponentDir);

  if (boost::indeterminate(result))
    return SOFTPASS;

  return result ? PASS : FAIL;
}

Record3ReturnTypes
CategoryRuleItem::isDirectionPassOld(const PricingTrx& trx,
                                     const PaxTypeFare& paxTypeFare,
                                     const CategoryRuleItemInfo* cfrItem,
                                     bool isLocationSwapped,
                                     const Itin* itin)
{
  if (UNLIKELY(trx.getOptions()->isRtw()))
  {
    return cfrItem->inOutInd() == RuleConst::ALWAYS_APPLIES ? PASS : FAIL;
  }

  // check the outbound/inbound direction
  if (cfrItem->inOutInd() != RuleConst::ALWAYS_APPLIES)
  {
    if (paxTypeFare.directionality() == BOTH)
    {
      return SOFTPASS;
    }
    if ((cfrItem->inOutInd() == RuleConst::FARE_MARKET_OUTBOUND &&
         paxTypeFare.directionality() == TO) ||
        (cfrItem->inOutInd() == RuleConst::FARE_MARKET_INBOUND &&
         paxTypeFare.directionality() == FROM))
    {
       //APO-35436: a not first fc  in a Intl.itin could form a  Orig. Open Jaw(OOJ) 
       //PU. A second fc in a OOJ PU is validated against inbound fc rules(not as 
       //outbound fc). So, dont kill the fare if the itin is intl and the fc is 
       //not the first. 
        bool isFirstFC = false;
        if ( itin  && (paxTypeFare.fareMarket()->travelSeg().front() == itin->travelSeg().front()))
            isFirstFC = true;
        const bool isDomesticFM =
            (paxTypeFare.fareMarket()->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA));
        if (isFirstFC || isDomesticFM)
           return FAIL;
         else  
         {  //APO-35436:this is an intl fc.so softpass it  as this fc may form an intl ooj pu
             return SOFTPASS;
         }
    }
  }

  // check the GEO_location directionality
  Indicator directionality = cfrItem->directionality();

  if (directionality == RuleConst::ALWAYS_APPLIES)
  {
    return PASS;
  }

  if (directionality == RuleConst::FROM_LOC1_TO_LOC2)
  {
    if (isLocationSwapped) // directionality was swapped on Loc1-Loc2 from Rule
    {
      return FAIL;
    }
  }
  else if (directionality == RuleConst::TO_LOC1_FROM_LOC2)
  {
    if (!isLocationSwapped) // directionality was not swapped on Loc1-Loc2 from Rule
    {
      return FAIL;
    }
  }
  else if (LIKELY(directionality == RuleConst::ORIGIN_FROM_LOC1_TO_LOC2 ||
           directionality == RuleConst::ORIGIN_FROM_LOC2_TO_LOC1))
  {
    if (itin && isDirectionalRule(cfrItem->itemcat()))
    {
      if (paxTypeFare.fareMarket()->travelSeg().front() == itin->travelSeg().front())
      {
        if ((directionality == RuleConst::ORIGIN_FROM_LOC2_TO_LOC1) == isLocationSwapped)
          return PASS;
        else
          return FAIL;
      }
      if (paxTypeFare.fareMarket()->offMultiCity() == itin->travelSeg().front()->boardMultiCity())
      {
        if ((directionality == RuleConst::ORIGIN_FROM_LOC1_TO_LOC2) == isLocationSwapped)
          return PASS;
        else
          return FAIL;
      }
    }

    if (!fallback::fallbackFVORuleDirectionalityTuning(&trx) &&
        isDirectionalRule2(cfrItem->itemcat()))
    {
      if (paxTypeFare.directionality() == FROM)
      {
        // Outbound
        if ((directionality == RuleConst::ORIGIN_FROM_LOC1_TO_LOC2) == isLocationSwapped)
          return FAIL;
        else
          return PASS;
      }
      else if (paxTypeFare.directionality() == TO)
      {
        if ((directionality == RuleConst::ORIGIN_FROM_LOC2_TO_LOC1) == isLocationSwapped)
          return FAIL;
        else
          return PASS;
      }
    }
    return SOFTPASS;
  }

  return PASS;
}

Record3ReturnTypes
CategoryRuleItem::isDirectionPassForShopping(PricingTrx& trx,
                                             const PaxTypeFare& paxTypeFare,
                                             const CategoryRuleInfo& cri,
                                             const CategoryRuleItemInfo* cfrItem,
                                             bool isLocationSwapped)
{
  // check the outbound/inbound direction
  FMDirection fmDirection = paxTypeFare.fareMarket()->direction();
  if (cfrItem->inOutInd() != RuleConst::ALWAYS_APPLIES)
  {
    if ((cfrItem->inOutInd() == RuleConst::FARE_MARKET_OUTBOUND && fmDirection == FMDirection::INBOUND) ||
        (cfrItem->inOutInd() == RuleConst::FARE_MARKET_INBOUND && fmDirection == FMDirection::OUTBOUND))
    {
      return FAIL;
    }
  }

  // check the GEO_location directionality
  Indicator directionality = cfrItem->directionality();
  if (directionality == RuleConst::ALWAYS_APPLIES)
  {
    return PASS;
  }

  if (directionality == RuleConst::FROM_LOC1_TO_LOC2)
  {
    if (isLocationSwapped) // directionality was swapped on Loc1-Loc2 from Rule
    {
      return FAIL;
    }
  }
  else if (directionality == RuleConst::TO_LOC1_FROM_LOC2)
  {
    if (!isLocationSwapped) // directionality was not swapped on Loc1-Loc2 from Rule
    {
      return FAIL;
    }
  }
  else if (directionality == RuleConst::ORIGIN_FROM_LOC1_TO_LOC2)
  {
    if (fmDirection == FMDirection::OUTBOUND)
    {
      if (isLocationSwapped)
      {
        return FAIL;
      }
    }

    else if (fmDirection == FMDirection::INBOUND)
    {
      if (!isLocationSwapped)
      {
        return FAIL;
      }
    }
    else
    {
      return SOFTPASS;
    }
  }

  else if (LIKELY(directionality == RuleConst::ORIGIN_FROM_LOC2_TO_LOC1))
  {
    if (fmDirection == FMDirection::OUTBOUND)
    {
      if (!isLocationSwapped)
      {
        return FAIL;
      }
    }
    else if (fmDirection == FMDirection::INBOUND)
    {
      if (isLocationSwapped)
      {
        return FAIL;
      }
    }
    else
    {
      return SOFTPASS;
    }
  }
  else
  {
    return FAIL;
  }

  return PASS;
}

//----------------------------------------------------------------------------
// isDirectionPassForFD()
// return PASS - pass directionality for the current RuleItem
//        FAIL - fail directionality for the current RuleItem
//
//----------------------------------------------------------------------------
Record3ReturnTypes
CategoryRuleItem::isDirectionPassForFD(const PaxTypeFare& paxTypeFare,
                                       const CategoryRuleItemInfo* cfrItem,
                                       bool isLocationSwapped)
{ // lint !e578

  // Directionility check for Surcharges is done in FDSurchargesRule
  if (cfrItem->itemcat() == RuleConst::SURCHARGE_RULE)
  {
    // Save isLocatioSwapped in FareDisplayInfo, so that it can be used for
    // Directionality check in FDSurcharges
    FareDisplayInfo* fareDisplayInfo = const_cast<FareDisplayInfo*>(paxTypeFare.fareDisplayInfo());
    if (fareDisplayInfo)
      fareDisplayInfo->isLocationSwappedForSurcharges() = isLocationSwapped;
    return PASS;
  }

  // check the inbound direction
  if (cfrItem->inOutInd() != RuleConst::ALWAYS_APPLIES)
  {
    if (cfrItem->inOutInd() == RuleConst::FARE_MARKET_INBOUND)
    {
      return FAIL;
    }
  }

  // check the GEO_location directionality
  Indicator directionality = cfrItem->directionality();

  if (directionality == RuleConst::ALWAYS_APPLIES)
  {
    return PASS;
  }

  if (directionality == RuleConst::FROM_LOC1_TO_LOC2 ||
      directionality == RuleConst::ORIGIN_FROM_LOC1_TO_LOC2)
  {
    if (isLocationSwapped) // directionality was swapped on Loc1-Loc2 from Rule
    {
      return FAIL;
    }
  }

  else if (directionality == RuleConst::TO_LOC1_FROM_LOC2 ||
           directionality == RuleConst::ORIGIN_FROM_LOC2_TO_LOC1)
  {
    if (!isLocationSwapped) // directionality was not swapped on Loc1-Loc2 from Rule
    {
      return FAIL;
    }
  }

  else
  {
    return FAIL;
  }

  return PASS;
}

//----------------------------------------------------------------------------
// isR8DirectionPass()
// return PASS  - pass directionality for the current RuleItem
//        FAIL  - fail directionality for the current RuleItem
//
//----------------------------------------------------------------------------
Record3ReturnTypes
CategoryRuleItem::isR8DirectionPass(const FareUsage& fareUsage,
                                    Indicator directionality,
                                    bool isR8LocationSwapped)
{
  // check the Record 8 directionality
  if (directionality == RuleConst::FROMTO)
  {
    if (fareUsage.isOutbound())
    {
      if (isR8LocationSwapped)
      {
        return FAIL;
      }
    }
    else if (fareUsage.isInbound())
    {
      if (!isR8LocationSwapped)
      {
        return FAIL;
      }
    }
    else
    {
      return FAIL;
    }
  }

  else if (directionality == RuleConst::TOFROM)
  {
    if (fareUsage.isOutbound())
    {
      if (!isR8LocationSwapped)
      {
        return FAIL;
      }
    }
    else if (fareUsage.isInbound())
    {
      if (isR8LocationSwapped)
      {
        return FAIL;
      }
    }
    else
    {
      return FAIL;
    }
  }
  else
  {
    return FAIL;
  }

  return PASS;
}

//----------------------------------------------------------------------------
// isR8DirectionPassForFD()
// return PASS  - pass directionality for the current RuleItem
//        FAIL  - fail directionality for the current RuleItem
//
//----------------------------------------------------------------------------
Record3ReturnTypes
CategoryRuleItem::isR8DirectionPassForFD(Indicator directionality, bool isR8LocationSwapped)
{
  // check the Record 8 directionality
  if (directionality == RuleConst::FROMTO)
  {
    if (isR8LocationSwapped)
    {
      return FAIL;
    }
  }

  else if (directionality == RuleConst::TOFROM)
  {
    if (!isR8LocationSwapped)
    {
      return FAIL;
    }
  }

  else
  {
    return FAIL;
  }

  return PASS;
}

Record3ReturnTypes
CategoryRuleItem::isDirectionalityMatch(PricingTrx& trx,
                                        const PaxTypeFare& paxTypeFare,
                                        const CategoryRuleItemInfo* cfr,
                                        const CategoryRuleInfo& cri,
                                        RuleItemCaller& ruleCaller,
                                        bool ruleTuningISSimple,
                                        uint32_t itemcat,
                                        bool isLocSwapped)
{
  if (!ruleTuningISSimple || (itemcat != RuleConst::SEASONAL_RULE))
  {
    return ruleCaller.isDirectionPass(cfr, isLocSwapped);
  }
  else
  {
    return CategoryRuleItem::isDirectionPassForShopping(trx, paxTypeFare, cri, cfr, isLocSwapped);
  }
}

void
CategoryRuleItem::diagR2sResult(PricingTrx& trx,
                                const PaxTypeFare& ptf,
                                const CategoryRuleItemInfo& r2s,
                                const Record3ReturnTypes rc) const
{
  DiagManager d500(trx, Diagnostic500);

  if (d500.isActive())
  {
    Diag500Collector& dc = static_cast<Diag500Collector&>(d500.collector());
    if (dc.givenValidationPhase() == Diag500Collector::ANY ||
        dc.givenValidationPhase() == dc.validationPhase())
      dc.displayRelation(ptf, &r2s, rc);
  }

  DiagManager d550(trx, Diagnostic550);

  if (d550.isActive())
  {
    Diag550Collector& dc = static_cast<Diag550Collector&>(d500.collector());
    dc.displayRelation(ptf, &r2s, rc);
  }
}


} //tse namespace
