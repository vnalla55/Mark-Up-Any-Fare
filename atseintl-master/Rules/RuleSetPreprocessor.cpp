//-------------------------------------------------------------------
//
//  File:        RuleSetPreprocessor.cpp
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

#include "Rules/RuleSetPreprocessor.h"

#include "Common/FallbackUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/GeneralRuleApp.h"
#include "DBAccess/StopoversInfo.h"
#include "DBAccess/TransfersInfo1.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/CategoryRuleItem.h"
#include "Rules/RuleConst.h"
#include "Rules/Stopovers.h"
#include "Rules/Transfers1.h"

using namespace std;

namespace tse
{
FALLBACK_DECL(fallbackAPO37838Record1EffDateCheck);
FALLBACK_DECL(cat9LeastRestrictive)

void
RuleSetPreprocessor::process(PricingTrx& trx,
                             const CategoryRuleInfo* categoryRuleInfo,
                             const PricingUnit& pu,
                             const FareUsage& currentFareUsage,
                             CategoryRuleItem* categoryRuleItem,
                             bool isFareRule)
{
  _categoryRuleInfo = categoryRuleInfo;
  _categoryRuleItem = categoryRuleItem;

  processStopovers(trx, categoryRuleInfo, pu);
  if (categoryRuleInfo && RuleConst::TRANSFER_RULE == categoryRuleInfo->categoryNumber())
  {
    processTransfers(trx, *categoryRuleInfo, pu, currentFareUsage, isFareRule);
  }
}

bool
RuleSetPreprocessor::hasPricingUnitScope(const uint16_t category, const FareUsage* fareUsage) const
{
  if (LIKELY(category == RuleConst::TRANSFER_RULE))
  {
    if (_fareUsagePricingUnitScopeForTransfers.find(fareUsage) !=
        _fareUsagePricingUnitScopeForTransfers.end())
    {
      return true;
    }
  }
  return false;
}

void
RuleSetPreprocessor::processStopovers(PricingTrx& trx,
                                      const CategoryRuleInfo* categoryRuleInfo,
                                      const PricingUnit& pu)
{
  if (pu.puType() == PricingUnit::Type::UNKNOWN || pu.fareUsage().size() < 2)
  {
    applyLeastRestrictiveStopovers() = false;
    return;
  }

  leastRestrictiveStopoversUnlimited() = false;
  leastRestrictiveStopoversPermitted() = 0;

  if (!checkLeastRestrictiveStopoversApplies(trx, pu))
  {
    applyLeastRestrictiveStopovers() = false;
    return;
  }

  applyLeastRestrictiveStopovers() = true;

  std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuIterEnd = pu.fareUsage().end();

  for (; fuIter != fuIterEnd; ++fuIter)
  {
    const PaxTypeFare* ptf = getFareFromFU(*fuIter, categoryRuleInfo->categoryNumber(), trx);

    PaxTypeFareRuleData* ptfRuleData = ptf->paxTypeFareRuleData(categoryRuleInfo->categoryNumber());

    bool isLocationSwapped = false;

    const CategoryRuleInfo* fareCatRuleInfo = nullptr;

    if (ptfRuleData)
    {
      fareCatRuleInfo = ptfRuleData->categoryRuleInfo();
      isLocationSwapped = ptfRuleData->isLocationSwapped();
    }

    if (!fareCatRuleInfo)
    {
      fareCatRuleInfo = RuleUtil::getGeneralFareRuleInfo(
          trx, *ptf, categoryRuleInfo->categoryNumber(), isLocationSwapped);
    }

    if (fareCatRuleInfo)
    {
      for (const auto& setPtr : fareCatRuleInfo->categoryRuleItemInfoSet())
      {
        int16_t ruleSetStopoverTotal = 0;

        for (const auto& info : *setPtr)
        {
          if (info.itemcat() == RuleConst::STOPOVER_RULE)
          {
            if (LIKELY(_categoryRuleItem))
            {
              Record3ReturnTypes direct = _categoryRuleItem->isDirectionPass(
                  trx, *(*fuIter), *fareCatRuleInfo, &info, isLocationSwapped);
              if (direct == FAIL)
                continue;
            }

            const RuleItemInfo* const ruleItemInfo =
                trx.dataHandle().getRuleItemInfo(categoryRuleInfo, &info);
            const StopoversInfo* soInfo = dynamic_cast<const StopoversInfo*>(ruleItemInfo);

            if (LIKELY(soInfo))
            {
              string noStopsMax = soInfo->noStopsMax();

              if (noStopsMax.empty() || (noStopsMax == Stopovers::NUM_STOPS_UNLIMITED))
              {
                leastRestrictiveStopoversUnlimited() = true;
                return;
              }
              else
              {
                int16_t numStopsMax = atoi(noStopsMax.c_str());

                if (info.relationalInd() == CategoryRuleItemInfo::AND)
                {
                  ruleSetStopoverTotal += numStopsMax;
                }
                else if (UNLIKELY(info.relationalInd() == CategoryRuleItemInfo::OR))
                {
                  if (numStopsMax > ruleSetStopoverTotal)
                  {
                    ruleSetStopoverTotal = numStopsMax;
                  }
                }
                else // THEN, IF
                {
                  if (numStopsMax > ruleSetStopoverTotal)
                  {
                    ruleSetStopoverTotal = numStopsMax;
                  }
                }
              }
            }
          }
        }

        if (UNLIKELY(leastRestrictiveStopoversUnlimited()))
        {
          return;
        }
        if (ruleSetStopoverTotal > leastRestrictiveStopoversPermitted())
        {
          leastRestrictiveStopoversPermitted() = ruleSetStopoverTotal;
        }
      }
    }

    // Now process the General Rule / Alt General Rule...
    //

    // Not sure if this is right... no way to verify at this time.

    // Determine if we need to use Alt Gen Rule or General Rule

    TariffNumber genTariffNumber;
    RuleNumber genRuleNumber = RuleConst::NULL_GENERAL_RULE;

    const GeneralFareRuleInfo* gfrRuleInfo = nullptr;

    if (ptfRuleData)
    {
      gfrRuleInfo = dynamic_cast<const GeneralFareRuleInfo*>(ptfRuleData->categoryRuleInfo());

      if (gfrRuleInfo)
      {
        genTariffNumber = gfrRuleInfo->generalRuleTariff();
        genRuleNumber = gfrRuleInfo->generalRule();
      }
    }

    if (genRuleNumber == RuleConst::NULL_GENERAL_RULE && gfrRuleInfo &&
        gfrRuleInfo->generalRuleAppl() == RuleConst::RECORD_0_NOT_APPLY)
    {
      continue;
    }

    CarrierCode carrier = ptf->carrier();
    if (UNLIKELY(ptf->fare()->isIndustry()))
    {
      carrier = INDUSTRY_CARRIER;
    }

    if ((genRuleNumber == RuleConst::NULL_GENERAL_RULE &&
         (gfrRuleInfo && gfrRuleInfo->generalRuleAppl() != RuleConst::RECORD_0_NOT_APPLY)) ||
        (!gfrRuleInfo))
    {
      // Alt General Rule...
      //
      if (!fallback::fallbackAPO37838Record1EffDateCheck(&trx))
      {
        trx.dataHandle().getGeneralRuleAppTariffRuleByTvlDate(ptf->vendor(),
                                                              carrier,
                                                              ptf->tcrRuleTariff(),
                                                              RuleConst::NULL_GENERAL_RULE,
                                                              categoryRuleInfo->categoryNumber(),
                                                              genRuleNumber,
                                                              genTariffNumber,
                                                              ptf->fareMarket()->travelDate());
      }
      else
      {
        trx.dataHandle().getGeneralRuleAppTariffRule(ptf->vendor(),
                                                     carrier,
                                                     ptf->tcrRuleTariff(),
                                                     RuleConst::NULL_GENERAL_RULE,
                                                     categoryRuleInfo->categoryNumber(),
                                                     genRuleNumber,
                                                     genTariffNumber);
      }
    }

    if (genRuleNumber != RuleConst::NULL_GENERAL_RULE)
    {
      // lint -e{645}
      const vector<GeneralFareRuleInfo*>& gfrList =
          trx.dataHandle().getGeneralFareRule(ptf->vendor(),
                                              carrier,
                                              genTariffNumber,
                                              genRuleNumber,
                                              categoryRuleInfo->categoryNumber(),
                                              ptf->fareMarket()->travelDate());

      vector<GeneralFareRuleInfo*>::const_iterator iterGfr = gfrList.begin();
      vector<GeneralFareRuleInfo*>::const_iterator iterGfrEnd = gfrList.end();

      for (; iterGfr != iterGfrEnd; ++iterGfr)
      {
        const GeneralFareRuleInfo* gfrRule = *iterGfr;

        bool isLocationSwapped = false;
        if (ptfRuleData)
        {
          isLocationSwapped = ptfRuleData->isLocationSwapped();
        }
        if (gfrRule->hasCat(RuleConst::STOPOVER_RULE) &&
            RuleUtil::matchGeneralFareRule(trx, *ptf, *gfrRule, isLocationSwapped))
        {
          for (const auto& setPtr : gfrRule->categoryRuleItemInfoSet())
          {
            int16_t ruleSetStopoverTotal = 0;
            for (const auto& item : *setPtr)
            {
              if (item.itemcat() == RuleConst::STOPOVER_RULE)
              {
                if (_categoryRuleItem)
                {
                  Record3ReturnTypes direct = _categoryRuleItem->isDirectionPass(
                      trx, *(*fuIter), *gfrRule, &item, isLocationSwapped);
                  if (direct == FAIL)
                    continue;
                }

                const RuleItemInfo* const ruleItemInfo =
                    trx.dataHandle().getRuleItemInfo(categoryRuleInfo, &item);
                const StopoversInfo* soInfo = dynamic_cast<const StopoversInfo*>(ruleItemInfo);

                if (soInfo)
                {
                  string noStopsMax = soInfo->noStopsMax();

                  if (noStopsMax.empty() || (noStopsMax == Stopovers::NUM_STOPS_UNLIMITED))
                  {
                    leastRestrictiveStopoversUnlimited() = true;
                    return;
                  }
                  else
                  {
                    int16_t numStopsMax = atoi(noStopsMax.c_str());

                    if (item.relationalInd() == CategoryRuleItemInfo::AND)
                    {
                      ruleSetStopoverTotal += numStopsMax;
                    }
                    else if (item.relationalInd() == CategoryRuleItemInfo::OR)
                    {
                      if (numStopsMax > ruleSetStopoverTotal)
                      {
                        ruleSetStopoverTotal = numStopsMax;
                      }
                    }
                    else // THEN, IF
                    {
                      if (numStopsMax > ruleSetStopoverTotal)
                      {
                        ruleSetStopoverTotal = numStopsMax;
                      }
                    }
                  }
                }
              }
            }
            if (leastRestrictiveStopoversUnlimited())
            {
              return;
            }
            if (ruleSetStopoverTotal > leastRestrictiveStopoversPermitted())
            {
              leastRestrictiveStopoversPermitted() = ruleSetStopoverTotal;
            }
          }
        }
      }
    }
  }
  if ((!leastRestrictiveStopoversUnlimited()) && (leastRestrictiveStopoversPermitted() == 0))
  {
    applyLeastRestrictiveStopovers() = false;
  }
}

struct RuleSetPreprocessor::FareUsageTransfersScopeInfo
{
  // Note that the two following pointers could be just booleans;
  // they are stored as pointers however for diagnostics purposes.
  const TransfersInfo1* _fcScope = nullptr; // a record3 that has FC validation
  const TransfersInfo1* _puScope = nullptr; // a record3 that has PU validation
  CarrierCode _carrier; // carrier from fare usage
  bool _maxZero = false; // true when all records3 do not permit any transfers

  void setCarrierFromPtf(const PaxTypeFare& ptf)
  {
    _carrier = ptf.carrier();
    if (ptf.fare()->isIndustry() || _carrier == INDUSTRY_CARRIER)
      _carrier = ptf.fareMarket()->governingCarrier();
  }
  bool isFCscopeOnly(const size_t puSize) const
  {
    // When fare usage can be FC-validated or PU-validated (depending on record3 we hit in later
    // processing) it seems to be safer to assume that the fare usage is PU-validated.
    // However the previous version assumed FC-validation for pricing units consisting with
    // up to two fares, leaving - implicitly - PU-validation when there are more than two
    // fares - retain the logic here.
    // Also when there are no cat9 record3 for the fare usage, assume it is PU-validated.
    return _fcScope && (puSize <= 2 || !_puScope);
  }
};

namespace
{
const std::vector<GeneralFareRuleInfo*>&
getGeneralRuleInfoVector(PricingTrx& trx,
                         const uint16_t categoryNumber,
                         const PaxTypeFare* const ptf,
                         const PaxTypeFareRuleData* const ptfRuleData)
{
  static const std::vector<GeneralFareRuleInfo*> EMPTY;

  // Not sure if this is right... no way to verify at this time.

  // Determine if we need to use Alt Gen Rule or General Rule

  TariffNumber genTariffNumber = 0;
  RuleNumber genRuleNumber = RuleConst::NULL_GENERAL_RULE;

  const GeneralFareRuleInfo* gfrRuleInfo = nullptr;

  if (ptfRuleData)
  {
    gfrRuleInfo = dynamic_cast<const GeneralFareRuleInfo*>(ptfRuleData->categoryRuleInfo());

    if (LIKELY(gfrRuleInfo))
    {
      genTariffNumber = gfrRuleInfo->generalRuleTariff();
      genRuleNumber = gfrRuleInfo->generalRule();
    }
  }

  if (UNLIKELY(genRuleNumber == RuleConst::NULL_GENERAL_RULE && gfrRuleInfo &&
               gfrRuleInfo->generalRuleAppl() == RuleConst::RECORD_0_NOT_APPLY))
    return EMPTY;

  const CarrierCode carrier = ptf->fare()->isIndustry() ? INDUSTRY_CARRIER : ptf->carrier();

  if (genRuleNumber == RuleConst::NULL_GENERAL_RULE || !gfrRuleInfo)
  {
    // Alt General Rule...
    //
    if (!fallback::fallbackAPO37838Record1EffDateCheck(&trx))
    {
      trx.dataHandle().getGeneralRuleAppTariffRuleByTvlDate(ptf->vendor(),
                                                            carrier,
                                                            ptf->tcrRuleTariff(),
                                                            RuleConst::NULL_GENERAL_RULE,
                                                            categoryNumber,
                                                            genRuleNumber,
                                                            genTariffNumber,
                                                            ptf->fareMarket()->travelDate());
    }
    else
    {
      trx.dataHandle().getGeneralRuleAppTariffRule(ptf->vendor(),
                                                   carrier,
                                                   ptf->tcrRuleTariff(),
                                                   RuleConst::NULL_GENERAL_RULE,
                                                   categoryNumber,
                                                   genRuleNumber,
                                                   genTariffNumber);
    }
  }

  if (genRuleNumber == RuleConst::NULL_GENERAL_RULE)
    return EMPTY;

  return trx.dataHandle().getGeneralFareRule(ptf->vendor(),
                                             carrier,
                                             genTariffNumber,
                                             genRuleNumber,
                                             categoryNumber,
                                             ptf->fareMarket()->travelDate());
}

inline void
printItemNo(DiagManager& diag, const TransfersInfo1* const trInfo)
{
  if (trInfo)
    diag << std::setw(10) << trInfo->itemNo();
  else
    diag << " ---------";
}
} // anonymous namespace

void
RuleSetPreprocessor::addTransfersDiagnostics(
    PricingTrx& trx,
    const PricingUnit& pu,
    const FareUsage& currentFareUsage,
    const FareUsageTransfersScopeInfo& currentFareUsageScopeInfo,
    const std::vector<FareUsageTransfersScopeInfo>* const transfersScopeInfoVector) const
{
  DiagManager diag(trx, Diagnostic309);

  if (LIKELY(!diag.isActive() ||
             trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "RSP"))
    return;

  diag << "\nCATEGORY 09 - RULE SET PREPROCESSOR DIAGNOSTICS\n";
  diag << "   MKT    FARE      FLAG CX  REC3-FC   REC3-PU    FLAG  RESULT\n\n";

  TSE_ASSERT(!transfersScopeInfoVector ||
             pu.fareUsage().size() == transfersScopeInfoVector->size());
  for (size_t idx = 0; idx != pu.fareUsage().size(); ++idx)
  {
    const FareUsage* const fareUsage = pu.fareUsage()[idx];
    const PaxTypeFare* const ptf = fareUsage->paxTypeFare();
    diag << ' ' << ptf->fareMarket()->origin()->loc() << ' '
         << ptf->fareMarket()->destination()->loc() << ' ' << std::left << std::setw(12)
         << ptf->fareClass() << std::right;

    const FareUsageTransfersScopeInfo* const sci =
        transfersScopeInfoVector
            ? &(*transfersScopeInfoVector)[idx]
            : fareUsage == &currentFareUsage ? &currentFareUsageScopeInfo : nullptr;
    if (!sci)
    {
      diag << " SKIP\n"; // the fare was not analyzed
      continue;
    }
    diag << (fareUsage == &currentFareUsage ? " *" : " -");
    diag << "  " << sci->_carrier;

    printItemNo(diag, sci->_fcScope);
    printItemNo(diag, sci->_puScope);

    diag << (sci->_maxZero ? " MAXZERO" : " -------");
    diag << (!transferFCscope() && hasPricingUnitScope(RuleConst::TRANSFER_RULE, fareUsage)
                 ? "   PU"
                 : "   FC");
    diag << '\n';
  }

  diag.collector().printLine();
  diag << "\n";
}

void
RuleSetPreprocessor::detectTransfersRecord3s(
    PricingTrx& trx,
    const PricingUnit& pu,
    const PaxTypeFare& ptf,
    const bool isLocationSwapped,
    const CategoryRuleInfo& fareOrGeneralRuleCatRuleInfo,
    const FareUsage& fareUsage,
    FareUsageTransfersScopeInfo& /* out*/ fareUsageTransfersScopeInfo)
{
  bool allMaxZero = true; // all found record3s have `number of transfers permitted equal to 0'

  for (const auto& setPtr : fareOrGeneralRuleCatRuleInfo.categoryRuleItemInfoSet())
  {
    for (const auto& item : *setPtr)
    {
      if (item.itemcat() != RuleConst::TRANSFER_RULE)
        continue;

      const RuleItemInfo* const ruleItemInfo =
          trx.dataHandle().getRuleItemInfo(&fareOrGeneralRuleCatRuleInfo, &item);

      // call RuleUtil to validate T994
      if (UNLIKELY(ruleItemInfo && ruleItemInfo->overrideDateTblItemNo() != 0 &&
                   RuleItem::preliminaryT994Validation(
                       trx, fareOrGeneralRuleCatRuleInfo, &pu, ptf, ruleItemInfo, item.itemcat()) ==
                       SKIP))
        continue;

      if ((item.relationalInd() == CategoryRuleItemInfo::THEN ||
           item.relationalInd() == CategoryRuleItemInfo::ELSE) &&
          CategoryRuleItem::isDirectionPass(
              trx, fareUsage, fareOrGeneralRuleCatRuleInfo, &item, isLocationSwapped) != PASS)
        continue;

      const TransfersInfo1* const trInfo = dynamic_cast<const TransfersInfo1*>(ruleItemInfo);
      if (UNLIKELY(!trInfo))
        continue;

      const std::string& noTransfersMax = trInfo->noTransfersMax();

      if (noTransfersMax.empty())
      {
        fareUsageTransfersScopeInfo._fcScope = trInfo;
        allMaxZero = false;
      }
      else
      {
        fareUsageTransfersScopeInfo._puScope = trInfo;
        if (noTransfersMax == Transfers1::NUM_TRANSFERS_UNLIMITED)
        {
          if (fallback::cat9LeastRestrictive(&trx))
            leastRestrictiveTransfersUnlimited() = true;
          allMaxZero = false;
        }
        else if (allMaxZero && atoi(noTransfersMax.c_str()) != 0)
          allMaxZero = false;
      }
    }
  }

  fareUsageTransfersScopeInfo._maxZero = allMaxZero && fareUsageTransfersScopeInfo._puScope;
}

void
RuleSetPreprocessor::detectTransfersRecord3s(
    PricingTrx& trx,
    const PricingUnit& pu,
    const CategoryRuleInfo& categoryRuleInfo,
    const bool isFareRule,
    const FareUsage& fareUsage,
    FareUsageTransfersScopeInfo& /*out*/ fareUsageTransfersScopeInfo)
{
  const PaxTypeFare* const ptf = getFareFromFU(&fareUsage, categoryRuleInfo.categoryNumber(), trx);
  fareUsageTransfersScopeInfo.setCarrierFromPtf(*ptf);

  const PaxTypeFareRuleData* const ptfRuleData =
      ptf->paxTypeFareRuleData(categoryRuleInfo.categoryNumber());

  if (isFareRule)
  {
    bool isLocationSwapped = ptfRuleData && ptfRuleData->isLocationSwapped();
    const CategoryRuleInfo* fareCatRuleInfo =
        ptfRuleData ? ptfRuleData->categoryRuleInfo() : nullptr;

    if (!fareCatRuleInfo)
    {
      fareCatRuleInfo = RuleUtil::getGeneralFareRuleInfo(
          trx, *ptf, categoryRuleInfo.categoryNumber(), isLocationSwapped);
    }

    // The fareCatRuleInfo->categoryRuleItemInfoSet().empty() check was not in old version
    // however it seems to be needed: request from HPS-792 works incorrectly without it.
    if (fareCatRuleInfo && !fareCatRuleInfo->categoryRuleItemInfoSet().empty())
    {
      TSE_ASSERT(categoryRuleInfo.categoryNumber() == fareCatRuleInfo->categoryNumber());

      // Note: old code used to pass also categoryRuleInfo just to get vendorCode()
      // This might have been incorrect, especially is fare path like this:
      //   YEEIF4M       SITA33   YY MSSP  1793.00 AUD 2O EH XEX9000000 CT
      //   BRTAE7        ATP 33   EK EK46     7710 AED 2O EH EU      50 CT
      //   ELESAU6       ATP 8    CZ SP13   330.00 AUD 2I EH XEX4999900 CT
      // where one fare is of SITA vendor, and other fares are ATP.
      // Now the vendorCode() is taken from fareCatRuleInfo.
      detectTransfersRecord3s(trx,
                              pu,
                              *ptf,
                              isLocationSwapped,
                              *fareCatRuleInfo,
                              fareUsage,
                              fareUsageTransfersScopeInfo);

      return; // Cat9 allows fare rule take precedence of general rule.
    }
  }

  // Now process the General Rule / Alt General Rule...
  const std::vector<GeneralFareRuleInfo*>& gfrList =
      getGeneralRuleInfoVector(trx, categoryRuleInfo.categoryNumber(), ptf, ptfRuleData);
  for (const GeneralFareRuleInfo* const gfrRule : gfrList)
  {
    bool isLocationSwapped = ptfRuleData && ptfRuleData->isLocationSwapped();

    if (gfrRule->hasCat(RuleConst::TRANSFER_RULE) &&
        RuleUtil::matchGeneralFareRule(trx, *ptf, *gfrRule, isLocationSwapped))
    {
      detectTransfersRecord3s(
          trx, pu, *ptf, isLocationSwapped, *gfrRule, fareUsage, fareUsageTransfersScopeInfo);
      break; // found matching general rule, no need to find another one (note that previous code
      // didn't stop here)
    }
  }
}

void
RuleSetPreprocessor::processTransfers(PricingTrx& trx,
                                      const CategoryRuleInfo& categoryRuleInfo,
                                      const PricingUnit& pu,
                                      const FareUsage& currentFareUsage,
                                      const bool isFareRule)
{
  // The following code, based on the contents of transfersScopeInfoVector tries to implement the
  // rules from section 4.0 of ATPCO Cat9 documentation:
  //
  // <quote>
  //   When two fare components are used in a round trip pricing unit and both have pricing unit
  //   application, both carriers' transfers data is validated against the pricing unit to which
  //   they belong. When two fare components are used in a round trip pricing unit and one of
  //   them has fare component application (and the other has pricing unit), both carriers'
  //   transfers data is validated against their own fare component only.
  //
  //   When there are more than two fare components in a pricing unit and one of the carriers in the
  //   pricing solution is fare component, the overall assumption is that all components should be
  //   treated as fare component. The exception is that when two or more of the fare components have
  //   the same carrier and that carrier states a pricing unit application in their transfers
  //   provisions for Category 9, then those fare components can be measured as pricing unit and
  //   only their transfers should be used to validate their fare  components – any other
  //   carrier fare component should not have their transfers count towards validating the same
  //   carrier "pricing unit."
  // </quote>
  //
  // The basic idea of the algorithm is: find possible cat9 record3 for all fare components in
  // pricing unit, check if the records have PU or FC validation, and set transferFCscope() and
  // fill _fareUsagePricingUnitScopeForTransfers according to the rules given above.
  // When for given fare usage we can find both PU-validated and FC-validated records3, we cannot
  // be sure which record3 will be taken for validation, so the worse case (PU-validation) is
  // assumed. This assumption can sometimes lead to incorrect results, but unless the real records3
  // used for validation are stored somewhere,  we can do nothing about it.

  leastRestrictiveTransfersUnlimited() = false;
  leastRestrictiveTransfersPermitted() = 0;
  applyLeastRestrictiveTransfers() = false;
  TSE_ASSERT(_fareUsagePricingUnitScopeForTransfers.empty());

  const size_t puSize = pu.fareUsage().size();

  if (puSize <= 1u)
  {
    // We could potentially try to find out if the fare has PU or FC records3, but since there
    // is one fare component only there should be no difference.
    transferFCscope() = true;
    return;
  }

  // First try to find the scope of the current fare usage
  FareUsageTransfersScopeInfo currentFUtransfersScopeInfo;
  detectTransfersRecord3s(
      trx, pu, categoryRuleInfo, isFareRule, currentFareUsage, currentFUtransfersScopeInfo);
  if (currentFUtransfersScopeInfo.isFCscopeOnly(puSize))
  {
    // All matching records3 have FC validation.
    transferFCscope() = true;
    addTransfersDiagnostics(trx, pu, currentFareUsage, currentFUtransfersScopeInfo, nullptr);
    return;
  }

  // Find scope of other fare usages
  std::vector<FareUsageTransfersScopeInfo> transfersScopeInfoVector(pu.fareUsage().size());
  for (size_t idx = 0u; idx != pu.fareUsage().size(); ++idx)
  {
    const FareUsage* const fareUsage = pu.fareUsage()[idx];
    if (&currentFareUsage == fareUsage)
      transfersScopeInfoVector[idx] = currentFUtransfersScopeInfo;
    else
      // Note usage of `true', instead of `isFareRule'. The value of isFareRule was determined
      // for currentFareUsage by the caller, and it is not applicable to other fare usages in
      // the pricing unit (i.e. the other fare usages can have fare rules, even though the
      // currently validated one has general rule only).
      detectTransfersRecord3s(
          trx, pu, categoryRuleInfo, true, *fareUsage, transfersScopeInfoVector[idx]);
  }

  applyLeastRestrictiveTransfers() = leastRestrictiveTransfersUnlimited();

  const size_t numberOfFCscope = std::count_if(transfersScopeInfoVector.begin(),
                                               transfersScopeInfoVector.end(),
                                               [&](const FareUsageTransfersScopeInfo& scopeInfo)
                                               { return scopeInfo.isFCscopeOnly(puSize); });
  if (numberOfFCscope == 0u) // All are PU-validated
  {
    _fareUsagePricingUnitScopeForTransfers.insert(pu.fareUsage().begin(), pu.fareUsage().end());
    transferFCscope() = false;

    // If all fare components are PU validated and at least one of them does not allow any
    // transfer, then we may earlier reject solution that contains a transfer
    const bool anyMaxZero = std::any_of(transfersScopeInfoVector.begin(),
                                        transfersScopeInfoVector.end(),
                                        [&](const FareUsageTransfersScopeInfo& scopeInfo)
                                        { return scopeInfo._maxZero; });
    if (anyMaxZero && puSize >= 2 && puSize != pu.travelSeg().size())
      transferFailPU() = true;
  }
  else if (numberOfFCscope >= puSize - 1) // all but one are FC-validated ==> all are FC-validated
  {
    transferFCscope() = true;
  }
  else
  {
    std::vector<FareUsageTransfersScopeInfo>::const_iterator iterTransfersScopeInfo =
        transfersScopeInfoVector.begin();
    std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
    const std::vector<FareUsage*>::const_iterator fuIterEnd = pu.fareUsage().end();
    for (; fuIter != fuIterEnd; ++fuIter, ++iterTransfersScopeInfo)
    {
      if (iterTransfersScopeInfo->_carrier == currentFUtransfersScopeInfo._carrier &&
          (!iterTransfersScopeInfo->isFCscopeOnly(puSize)))
        _fareUsagePricingUnitScopeForTransfers.insert(*fuIter);
    }
    transferFCscope() = _fareUsagePricingUnitScopeForTransfers.size() == 1u;
  }

  addTransfersDiagnostics(
      trx, pu, currentFareUsage, currentFUtransfersScopeInfo, &transfersScopeInfoVector);
}

bool
RuleSetPreprocessor::checkLeastRestrictiveStopoversApplies(PricingTrx& trx, const PricingUnit& pu)
{
  const bool isTag1Only = isTag1OnlyPU(pu);
  const bool isTag1AndOrTag2Only = !isTag1Only && isTag2OnlyOrTag1AndTag2PU(pu);

  if (!isTag1Only && !isTag1AndOrTag2Only)
    return false;

  for (FareUsage* fu : pu.fareUsage())
  {
    const FareMarket* const fm = fu->paxTypeFare()->fareMarket();

    if (UNLIKELY(!fm))
      continue;

    const CarrierPreference* cxrPref = trx.dataHandle().getCarrierPreference(
        fm->governingCarrier(), trx.getRequest()->ticketingDT());

    if (UNLIKELY(!cxrPref))
      return false;

    if (isTag1Only && cxrPref->applyleastRestrStOptopu() != CXR_PREF_APPLY_LEAST_RESTRICTION)
      return false;

    if (isTag1AndOrTag2Only &&
        cxrPref->applyleastRestrtrnsftopu() != CXR_PREF_APPLY_LEAST_RESTRICTION)
      return false;
  }

  return true;
}

bool
RuleSetPreprocessor::isTag1OnlyPU(const PricingUnit& pu)
{
  return std::all_of(pu.fareUsage().begin(),
                     pu.fareUsage().end(),
                     [](const FareUsage* fu)
                     { return fu->paxTypeFare()->owrt() == ONE_WAY_MAY_BE_DOUBLED; });
}

bool
RuleSetPreprocessor::isTag2OnlyOrTag1AndTag2PU(const PricingUnit& pu)
{
  bool tag2Exists = false;

  for (FareUsage* fu : pu.fareUsage())
  {
    const PaxTypeFare& ptf = *fu->paxTypeFare();

    if (ptf.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED) // Tag 3
      return false;

    tag2Exists |= (ptf.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED); // Tag 2
  }

  return tag2Exists; // tag2Exists || ( tag2Exists && tag1Exists );
}

const PaxTypeFare*
RuleSetPreprocessor::getFareFromFU(const FareUsage* fu, uint16_t category, PricingTrx& trx)
{
  const PaxTypeFare* ptf = fu->paxTypeFare();
  if (ptf->isFareByRule())
  {
    const FBRPaxTypeFareRuleData* fbrPTF = ptf->getFbrRuleData(RuleConst::FARE_BY_RULE);
    if (fbrPTF && !fbrPTF->isSpecifiedFare())
    {
      const FareByRuleItemInfo* fbrItemInfo =
          dynamic_cast<const FareByRuleItemInfo*>(fbrPTF->ruleItemInfo());
      if (LIKELY(fbrItemInfo))
      {
        Indicator i('X');
        if (category == 8)
          i = fbrItemInfo->ovrdcat8();
        else if (category == 9)
          i = fbrItemInfo->ovrdcat9();

        if (i == 'B')
          ptf = fbrPTF->baseFare();
      }
    }
  }
  return ptf;
}
}
