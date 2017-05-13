//----------------------------------------------------------------------------
//  File: FBDisplayController.cpp
//
//  Author: Partha Kumar Chakraborti
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#include "Fares/FBDisplayController.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/TSEException.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FBCategoryRuleRecord.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DBAccess/GeneralRuleApp.h"
#include "DBAccess/GeneralFareRuleInfo.h"

#include <vector>

namespace tse
{
static Logger
logger("atseintl.Fares.FBDisplayController");

GeneralFareRuleInfo*
FBDisplayController::getGeneralFareRuleInfo(FareDisplayTrx& trx,
                                            PaxTypeFare& paxTypeFare,
                                            uint16_t cat,
                                            bool& isLocationSwapped,
                                            const TariffNumber* overrideTcrRuleTariff = nullptr,
                                            const RuleNumber* overrideRuleNumber = nullptr)
{
  return RuleUtil::getGeneralFareRuleInfo(
      trx, paxTypeFare, cat, isLocationSwapped, overrideTcrRuleTariff, overrideRuleNumber);
}

bool
FBDisplayController::collectRecord2Info()
{

  if (!_trx.isRD() && !_trx.isSDSOutputType())
    return false;

  // most rec2 info already stored by RuleController
  // only need to call when looking at details of one fare
  // trimming list to one fare done by FareSelector
  if (_trx.allPaxTypeFare().size() < 1)
    return false;

  std::vector<PaxTypeFare*>::const_iterator ptfIter = _trx.allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::const_iterator ptfIterEnd = _trx.allPaxTypeFare().end();
  for (; ptfIter != ptfIterEnd; ptfIter++)
  {
    PaxTypeFare& paxTypeFare = **ptfIter;
    FareDisplayInfo* fdPtr(nullptr);

    fdPtr = paxTypeFare.fareDisplayInfo();
    if (!fdPtr)
    {
      LOG4CXX_DEBUG(logger, "Fare display Info object not found. Stopped gathering rec2 info");
      return false;
    }

    // prep for Cat25
    const FareByRuleItemInfo* fbrItemInfo = nullptr;
    if (paxTypeFare.isFareByRule() /*&& beginCat !=19 && beginCat !=35*/)
    {
      fbrItemInfo = &paxTypeFare.fareByRuleInfo();
    }

    std::vector<CatNumber>::const_iterator beginCat = _trx.getOptions()->ruleCategories().begin();
    std::vector<CatNumber>::const_iterator endCat = _trx.getOptions()->ruleCategories().end();

    for (; beginCat != endCat; beginCat++)
    {
      // Skip Cat10. We process it seperately
      if (*beginCat == RuleConst::COMBINABILITY_RULE)
        continue;

      // Collect Rec2 info of Discounted Fare: CAT19 to CAT22
      // has special fare/gen/alt logic
      if (*beginCat >= RuleConst::CHILDREN_DISCOUNT_RULE &&
          *beginCat <= RuleConst::OTHER_DISCOUNT_RULE)
      {
        collectDiscountedFareRec2Info(*fdPtr, paxTypeFare, *beginCat);
        continue;
      }

      // Collect extra Rec2 info of Fare By Rule CAT25
      if (*beginCat == RuleConst::FARE_BY_RULE)
      {
        collectCat25Rec2Info(*fdPtr, paxTypeFare);
        continue;
      }
      // for a Fbr fare, may need other category rules from base fare
      // FbrSpecified fares do not have a base fare
      if (fbrItemInfo && fbrItemInfo->fareInd() != FareByRuleItemInfo::SPECIFIED)
      {
        Indicator i;
        switch (*beginCat)
        {
        case 1:
          i = fbrItemInfo->ovrdcat1();
          break;
        case 2:
          i = fbrItemInfo->ovrdcat2();
          break;
        case 3:
          i = fbrItemInfo->ovrdcat3();
          break;
        case 4:
          i = fbrItemInfo->ovrdcat4();
          break;
        case 5:
          i = fbrItemInfo->ovrdcat5();
          break;
        case 6:
          i = fbrItemInfo->ovrdcat6();
          break;
        case 7:
          i = fbrItemInfo->ovrdcat7();
          break;
        case 8:
          i = fbrItemInfo->ovrdcat8();
          break;
        case 9:
          i = fbrItemInfo->ovrdcat9();
          break;
        case 10:
          i = fbrItemInfo->ovrdcat10();
          break;
        case 11:
          i = fbrItemInfo->ovrdcat11();
          break;
        case 12:
          i = fbrItemInfo->ovrdcat12();
          break;
        case 13:
          i = fbrItemInfo->ovrdcat13();
          break;
        case 14:
          i = fbrItemInfo->ovrdcat14();
          break;
        case 15:
          i = fbrItemInfo->ovrdcat15();
          break;
        case 16:
          i = fbrItemInfo->ovrdcat16();
          break;
        case 17:
          i = fbrItemInfo->ovrdcat17();
          break;
        case 18:
          i = fbrItemInfo->ovrdcat18();
          break;
        case 19:
          i = fbrItemInfo->ovrdcat19();
          break;
        case 20:
          i = fbrItemInfo->ovrdcat20();
          break;
        case 21:
          i = fbrItemInfo->ovrdcat21();
          break;
        case 22:
          i = fbrItemInfo->ovrdcat22();
          break;
        case 23:
          i = fbrItemInfo->ovrdcat23();
          break;
        case 24:
          i = fbrItemInfo->ovrdcat24();
          break;
        case 26:
          i = fbrItemInfo->ovrdcat26();
          break;
        case 27:
          i = fbrItemInfo->ovrdcat27();
          break;
        case 28:
          i = fbrItemInfo->ovrdcat28();
          break;
        case 29:
          i = fbrItemInfo->ovrdcat29();
          break;
        case 30:
          i = fbrItemInfo->ovrdcat30();
          break;
        case 31:
          i = fbrItemInfo->ovrdcat31();
          break;
        case 32:
          i = fbrItemInfo->ovrdcat32();
          break;
        case 33:
          i = fbrItemInfo->ovrdcat33();
          break;
        case 34:
          i = fbrItemInfo->ovrdcat34();
          break;
        case 35:
          i = fbrItemInfo->ovrdcat35();
          break;
        case 36:
          i = fbrItemInfo->ovrdcat36();
          break;
        case 37:
          i = fbrItemInfo->ovrdcat37();
          break;
        case 38:
          i = fbrItemInfo->ovrdcat38();
          break;
        case 39:
          i = fbrItemInfo->ovrdcat39();
          break;
        case 40:
          i = fbrItemInfo->ovrdcat40();
          break;
        case 41:
          i = fbrItemInfo->ovrdcat41();
          break;
        case 42:
          i = fbrItemInfo->ovrdcat42();
          break;
        case 43:
          i = fbrItemInfo->ovrdcat43();
          break;
        case 44:
          i = fbrItemInfo->ovrdcat44();
          break;
        case 45:
          i = fbrItemInfo->ovrdcat45();
          break;
        case 46:
          i = fbrItemInfo->ovrdcat46();
          break;
        case 47:
          i = fbrItemInfo->ovrdcat47();
          break;
        case 48:
          i = fbrItemInfo->ovrdcat48();
          break;
        case 49:
          i = fbrItemInfo->ovrdcat49();
          break;
        case 50:
          i = fbrItemInfo->ovrdcat50();
          break;
        default:
          i = 'X';
          break;
        }
        if (i == 'B')
        {

          // check if base fare category FBData is filled in (if it's reused)
          const PaxTypeFare* baseFare = paxTypeFare.fareWithoutBase();
          if (baseFare && baseFare->fareDisplayInfo())
          {
            FBCategoryRuleRecord* fbCatData =
                baseFare->fareDisplayInfo()->fbDisplay().getRuleRecordData(*beginCat);
            // is FB category record is filled, do nothing
            if (fbCatData &&
                (fbCatData->hasFareRuleRecord2Info() || fbCatData->hasFootNoteRecord2Info() ||
                 fbCatData->hasGeneralRuleRecord2Info()))
              continue;
            // nothing in FB category record, this can be because of
            // rule reuse for base fare
            collectAllOtherFareRec2Info(*fdPtr, const_cast<PaxTypeFare&>(*baseFare), *beginCat);
          }
          // already done in RuleController
          continue;
        }
      } // endif - is FBR

      collectAllOtherFareRec2Info(*fdPtr, paxTypeFare, *beginCat);

    } // endfor - all cat
  }
  return true;
}

//----------------------------------
// collectDiscountedFareRec2Info
//----------------------------------

// The way it works is taken from DiscountedFareController::process().
// From the implementation it seems that Discounted Fares can't
// have FootNote and FareRule/GeneralRule info together. If we have
// FootNote info then go back. Otherwise collect FareRule/GeneralRule.
//
// Any change in DiscountedFareController::process() should be reflected
// here too.

bool
FBDisplayController::collectDiscountedFareRec2Info(FareDisplayInfo& fareDisplayInfo,
                                                   PaxTypeFare& paxTypeFare,
                                                   uint16_t cat)
{

  if (cat < RuleConst::CHILDREN_DISCOUNT_RULE || cat > RuleConst::OTHER_DISCOUNT_RULE)
    return false;

  bool isLocationSwapped = false;

  DiscountedFareController dfc(_trx, *(_trx.itin().front()), *(paxTypeFare.fareMarket()));
  // --------------------------
  // Collect Footnote Info
  // --------------------------
  std::vector<Footnote> footnoteCodes;
  std::vector<TariffNumber> fareTariffs;
  RuleUtil::getFootnotes(paxTypeFare, footnoteCodes, fareTariffs);

  if (!footnoteCodes.empty())
  {
    const FootNoteCtrlInfo* footNote = nullptr;
    std::vector<Footnote>::const_iterator ftnt = footnoteCodes.begin();
    std::vector<Footnote>::const_iterator ftntEnd = footnoteCodes.end();
    std::vector<TariffNumber>::const_iterator fareTariffI = fareTariffs.begin();
    std::vector<TariffNumber>::const_iterator fareTariffEnd = fareTariffs.end();

    // get ALL footnotes
    for (; (ftnt != ftntEnd) && (fareTariffI != fareTariffEnd); ++ftnt, ++fareTariffI)
    {
      FootNoteCtrlInfoVec fnCtrlInfoVec;
      dfc.getFootNoteCtrlInfos(paxTypeFare, cat, *ftnt, *fareTariffI, fnCtrlInfoVec);

      // TODO: Do we need to call getDiscInfoFromRule ? I m not sure
      if (!fnCtrlInfoVec.empty())
      {
        footNote = fnCtrlInfoVec.front().first;
        isLocationSwapped = fnCtrlInfoVec.front().second;
        fareDisplayInfo.setFBDisplayData(
            cat, nullptr, footNote, nullptr, paxTypeFare.fareClass(), _trx.dataHandle());
        // FootNote always overriedes FareRule/General Rule both.
        // As we have FootNote then go back.
        return true;
      }
    }
  }

  // ----------------------------------------
  // No FootNote found. Collect FareRule Info
  // ----------------------------------------
  GeneralFareRuleInfo* ruleInfo(nullptr);

  // Get Fare Rule Record 2 info
  ruleInfo = getGeneralFareRuleInfo(_trx, paxTypeFare, cat, isLocationSwapped);
  if (ruleInfo)
  {
    fareDisplayInfo.setFBDisplayData(
        cat, ruleInfo, nullptr, nullptr, paxTypeFare.fareClass(), _trx.dataHandle());
  }

  TariffNumber genTariff;
  RuleNumber genNumber;

  // Do we have General Rule Record 2 info ? If not go back.
  if (fareRuleTakesPrecedenceOverGeneralRule(ruleInfo, cat) ||
      !dfc.getGenParam(ruleInfo, paxTypeFare, cat, genTariff, genNumber))
    return true;

  // -----------------------------------------
  // Yes. Collect General Rule Record 2 info
  // -----------------------------------------

  ruleInfo =
      getGeneralFareRuleInfo(_trx, paxTypeFare, cat, isLocationSwapped, &genTariff, &genNumber);
  if (ruleInfo)
  {
    fareDisplayInfo.setFBDisplayData(
        cat, nullptr, nullptr, ruleInfo, paxTypeFare.fareClass(), _trx.dataHandle());
  }

  return true;
}

// --------------------------------
// extra CAT 25 ctrl info
// ------------------------------
bool
FBDisplayController::collectCat25Rec2Info(FareDisplayInfo& fareDisplayInfo,
                                          PaxTypeFare& paxTypeFare)
{
  try
  {
    if (paxTypeFare.isFareByRule())
    {
      const FBRPaxTypeFareRuleData* fbrPaxTypeFareRuleData = paxTypeFare.getFbrRuleData();

      if (!fbrPaxTypeFareRuleData)
        return false;

      const FareByRuleCtrlInfo* pCat25 =
          dynamic_cast<const FareByRuleCtrlInfo*>(fbrPaxTypeFareRuleData->categoryRuleInfo());

      if (!pCat25)
        return false;

      fareDisplayInfo.setFBDisplayData(pCat25, paxTypeFare.fareClass(), _trx.dataHandle());
    } // endif - is really cat25
    return true;
  }
  catch (tse::TSEException& e) { LOG4CXX_ERROR(logger, "Cat25 PaxTypeFare not fully populated"); }
  return false;
}

// --------------------------------
// All other Categories
// ------------------------------

bool
FBDisplayController::collectAllOtherFareRec2Info(FareDisplayInfo& fareDisplayInfo,
                                                 PaxTypeFare& paxTypeFare,
                                                 uint16_t cat)
{
  const GeneralFareRuleInfo* ruleInfo(nullptr);
  bool isLocationSwapped = false;
  DiscountedFareController dfc(_trx, *(_trx.itin().front()), *(paxTypeFare.fareMarket()));

  PaxTypeFareRuleData* paxTypeFareRuleData = paxTypeFare.paxTypeFareRuleData(cat);

  if (paxTypeFareRuleData)
  {
    isLocationSwapped = paxTypeFareRuleData->isLocationSwapped();
    const CategoryRuleInfo* categoryRuleInfo = paxTypeFareRuleData->categoryRuleInfo();

    fareDisplayInfo.setFBDisplayLocationSwapped(cat, isLocationSwapped, _trx.dataHandle());

    if (categoryRuleInfo)
    {
      const FootNoteRecord2Info* footNote = castToFootNote(categoryRuleInfo);

      if (footNote)
      {
        fareDisplayInfo.setFBDisplayData(
            cat, nullptr, footNote, nullptr, paxTypeFare.fareClass(), _trx.dataHandle());
        return true;
      }

      const FareRuleRecord2Info* fareRule = castToFareRule(categoryRuleInfo, paxTypeFare);

      if (fareRule)
      {
        ruleInfo = fareRule;

        fareDisplayInfo.setFBDisplayData(
            cat, fareRule, nullptr, nullptr, paxTypeFare.fareClass(), _trx.dataHandle());
      }
    }
  }
  else
  {
    // --------------------------
    // Collect Footnote Info
    // --------------------------
    std::vector<Footnote> footnoteCodes;
    std::vector<TariffNumber> fareTariffs;
    RuleUtil::getFootnotes(paxTypeFare, footnoteCodes, fareTariffs);

    if (!footnoteCodes.empty())
    {
      const FootNoteCtrlInfo* footNote = nullptr;
      std::vector<Footnote>::const_iterator ftnt = footnoteCodes.begin();
      std::vector<Footnote>::const_iterator ftntEnd = footnoteCodes.end();
      std::vector<TariffNumber>::const_iterator fareTariffI = fareTariffs.begin();
      std::vector<TariffNumber>::const_iterator fareTariffEnd = fareTariffs.end();

      // get ALL footnotes
      for (; (ftnt != ftntEnd) && (fareTariffI != fareTariffEnd); ++ftnt, ++fareTariffI)
      {
        FootNoteCtrlInfoVec fnCtrlInfoVec;
        dfc.getFootNoteCtrlInfos(paxTypeFare, cat, *ftnt, *fareTariffI, fnCtrlInfoVec);

        if (!fnCtrlInfoVec.empty())
        {
          footNote = fnCtrlInfoVec.front().first;
          isLocationSwapped = fnCtrlInfoVec.front().second;
          fareDisplayInfo.setFBDisplayData(
              cat, nullptr, footNote, nullptr, paxTypeFare.fareClass(), _trx.dataHandle());
          return true;
        }
      }
    }

    // Get Fare Rule Record 2 info
    ruleInfo = getGeneralFareRuleInfo(_trx, paxTypeFare, cat, isLocationSwapped);
    if (ruleInfo)
    {
      fareDisplayInfo.setFBDisplayLocationSwapped(cat, isLocationSwapped, _trx.dataHandle());

      fareDisplayInfo.setFBDisplayData(
          cat, ruleInfo, nullptr, nullptr, paxTypeFare.fareClass(), _trx.dataHandle());
    }
  }

  TariffNumber genTariff;
  RuleNumber genNumber;

  // Do we have General Rule Record 2 info ? If not go back.
  if (fareRuleTakesPrecedenceOverGeneralRule(ruleInfo, cat) ||
      !dfc.getGenParam(ruleInfo, paxTypeFare, cat, genTariff, genNumber))
    return true;

  // -----------------------------------------
  // Yes. Collect General Rule Record 2 info
  // -----------------------------------------

  ruleInfo =
      getGeneralFareRuleInfo(_trx, paxTypeFare, cat, isLocationSwapped, &genTariff, &genNumber);
  if (ruleInfo)
  {
    fareDisplayInfo.setFBDisplayLocationSwapped(cat, isLocationSwapped, _trx.dataHandle());

    fareDisplayInfo.setFBDisplayData(
        cat, nullptr, nullptr, ruleInfo, paxTypeFare.fareClass(), _trx.dataHandle());
  }

  return true;
}

const FareRuleRecord2Info*
FBDisplayController::castToFareRule(const CategoryRuleInfo* categoryRuleInfo,
                                    const PaxTypeFare& paxTypeFare)
{

  const FareRuleRecord2Info* fareRule = dynamic_cast<const FareRuleRecord2Info*>(categoryRuleInfo);

  if (fareRule == nullptr)
    return fareRule;

  if (categoryRuleInfo->tariffNumber() == paxTypeFare.tcrRuleTariff() &&
      categoryRuleInfo->ruleNumber() == paxTypeFare.ruleNumber())
    return fareRule;

  return nullptr;
}

const FootNoteRecord2Info*
FBDisplayController::castToFootNote(const CategoryRuleInfo* categoryRuleInfo)
{
  return dynamic_cast<const FootNoteRecord2Info*>(categoryRuleInfo);
}

bool
FBDisplayController::fareRuleTakesPrecedenceOverGeneralRule(const GeneralFareRuleInfo* ruleInfo,
                                                            uint16_t category) const
{
  return ruleInfo &&
         (category == RuleConst::TRANSFER_RULE || category == RuleConst::STOPOVER_RULE) &&
         (ruleInfo->segcount() > 0 || ruleInfo->generalRule() == RuleConst::NULL_GENERAL_RULE);
}
}
