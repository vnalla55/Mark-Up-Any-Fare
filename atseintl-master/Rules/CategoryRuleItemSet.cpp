//-------------------------------------------------------------------
//
//  File:        CategoryRuleItemSet.cpp
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

#include "Rules/CategoryRuleItemSet.h"

#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PosPaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/QualifyFltAppRuleData.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleProcessingData.h"
#include "Rules/RuleSetPreprocessor.h"
#include "Rules/RuleUtil.h"
#include "Rules/SalesRestrictionRule.h"
#include "Rules/StopoversInfoWrapper.h"
#include "Rules/TransfersInfoWrapper.h"


#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace tse
{
FALLBACK_DECL(cat9unusedCode);

//----------------------------------------------------------------------------
// process()
//----------------------------------------------------------------------------
Record3ReturnTypes
CategoryRuleItemSet::process(PricingTrx& trx,
                             Itin& itin,
                             const CategoryRuleInfo& cri,
                             PaxTypeFare& paxTypeFare,
                             const std::vector<CategoryRuleItemInfoSet*>& crInfo,
                             RuleProcessingData& rpData,
                             bool isLocationSwapped,
                             bool isFareRule,
                             bool skipCat15Security,
                             RuleControllerDataAccess& da)
{
  Record3ReturnTypes retCode = SKIP;
  Record3ReturnTypes rc = SKIP;
  Record3ReturnTypes seasonRet = SKIP;
  bool seasonTemplate = false;
  bool isCat15Security = false;
  bool reuseCheck = true;
  bool shoppingTrx(trx.isShopping());

  // This check is done for SALE_RESTRICTIONS_RULE only.
  if (cri.categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE &&
      trx.getRequest()->fareGroupRequested() == false)
  {
    if (trx.getOptions()->isWeb() ||
        (shoppingTrx &&
         ((trx.getRequest()->ticketingAgent()->agentTJR() != nullptr) &&
          (trx.getRequest()->ticketingAgent()->agentTJR()->tvlyLocation() == YES)) &&
         (paxTypeFare.fcasPaxType() == ADULT || paxTypeFare.fcasPaxType() == NEG ||
          paxTypeFare.fcasPaxType().empty())))
    {
      if (!paxTypeFare.isWebFare())
      {
        setWebFare(trx, cri, paxTypeFare, crInfo, reuseCheck);
      }
    }
  }

  // For Category 25 - Fare By Rule
  if (cri.categoryNumber() == RuleConst::FARE_BY_RULE)
  {
    // lint -e{530}
    const PaxTypeFareRuleData* paxTypeFareRuleData =
        paxTypeFare.paxTypeFareRuleData(RuleConst::FARE_BY_RULE);

    retCode = _categoryRuleItem.process(
        trx,
        itin,
        cri,
        paxTypeFare,
        *paxTypeFareRuleData->categoryRuleItemInfoSet(),
        paxTypeFareRuleData->isLocationSwapped(),
        isCat15Security,
        rpData,
        false,
        skipCat15Security,
        da);

    if (retCode == PASS)
    {
      return PASS;
    }
    else if (retCode == SOFTPASS)
    {
      return SOFTPASS;
    }
    else if (retCode == STOP)
    {
      return FAIL;
    }
    else if (retCode == FAIL)
    {
      return FAIL;
    }
    else if (retCode == SKIP && isCat15Security)
    { //  Cat15 was failed by Security
      return FAIL;
    }
    return SKIP;
  }

  // For other Category Rules
  // set the PaxTypeFareRuleData in the PaxTypeFare
  // but only for FareRule data
  PaxTypeFareRuleData* paxTypeFareRuleData = nullptr;

  if (isFareRule)
  {
    // If this is repeating we do not need allocate new memory, and we are safe
    // because fares in each FareMarket are not duplicated, no multithreading
    paxTypeFareRuleData = paxTypeFare.setCatRuleInfo(cri, trx.dataHandle(), isLocationSwapped);
  }

  // Shopping
  // do not validate yet, if qualifier is cat 4
  if (shoppingTrx ||
      (trx.getTrxType() == PricingTrx::MIP_TRX && (_categoryPhase == FCORuleValidation || (_categoryPhase == PreValidation && paxTypeFare.fareMarket()->hasDuplicates()) )) )

  {
    Record3ReturnTypes retProcess = processQualifyCat4(trx, cri, crInfo, paxTypeFare);
    if (retProcess == NOTPROCESSED)
    {
      return NOTPROCESSED;
    }
  }
  FareDisplayTrx* fdt;
  FareDisplayUtil::getFareDisplayTrx(&trx, fdt);
  if (UNLIKELY(fdt))
    seasonTemplate = (fdt->isSeasonTemplate() && (fdt->isFQ() || fdt->isRD()));

  std::vector<CategoryRuleItemInfoSet*>::const_iterator crInfoB = crInfo.begin();
  std::vector<CategoryRuleItemInfoSet*>::const_iterator crInfoE = crInfo.end();

  for (; crInfoB != crInfoE; ++crInfoB)
  {
    if (cri.categoryNumber() != RuleConst::NEGOTIATED_RULE && isFareRule)
    {
      // set the set we are working on in the PaxTypeFareRuleData in the PaxTypeFare
      // only for FareRule data
      paxTypeFareRuleData->categoryRuleItemInfoSet() = *crInfoB;
    }
    retCode = _categoryRuleItem.process(trx,
                                        itin,
                                        cri,
                                        paxTypeFare,
                                        *(*crInfoB),
                                        isLocationSwapped,
                                        isCat15Security,
                                        rpData,
                                        isFareRule,
                                        skipCat15Security,
                                        da);

    if (UNLIKELY(cri.categoryNumber() == RuleConst::SURCHARGE_RULE))
    {
      if (retCode == SOFTPASS)
      {
        PaxTypeFare& nonConstPtf = paxTypeFare.cat25Fare()
                                       ? const_cast<PaxTypeFare&>(*paxTypeFare.cat25Fare())
                                       : const_cast<PaxTypeFare&>(paxTypeFare);

        nonConstPtf.needRecalculateCat12() = true;

        nonConstPtf.nucTotalSurchargeFareAmount() =
            std::accumulate(paxTypeFare.surchargeData().begin(),
                            paxTypeFare.surchargeData().end(),
                            paxTypeFare.nucTotalSurchargeFareAmount(),
                            [](MoneyAmount initial, const SurchargeData* sd)
                            { return initial - sd->amountNuc(); });

        nonConstPtf.surchargeData().clear();

        return SOFTPASS;
      }
    }

    // cat 35 logic
    if (cri.categoryNumber() == RuleConst::NEGOTIATED_RULE)
    {
      if (*crInfoB == paxTypeFareRuleData->categoryRuleItemInfoSet())
      {
        return retCode;
      }

      // if previous set passed we must fail this one for cat 35
      if (retCode == PASS)
      {
        return FAIL;
      }
      else
      {
        continue;
      }
    }

    // For FQ and RD if Season Templates we need to process seasons in all Data Sets
    if (UNLIKELY((cri.categoryNumber() == RuleConst::SEASONAL_RULE) && seasonTemplate))
    {
      if (retCode == PASS)
      {
        seasonRet = retCode;
        retCode = SKIP; // Need to process all Sets
      }
      else if (retCode == FAIL)
        rc = retCode;
      else if (retCode == STOP)
        return FAIL;
    }
    else
    {
      if (retCode == PASS)
      {
        return PASS;
      }
      else if (UNLIKELY(retCode == STOP_SKIP)) // stop processing to apply system assumption
      { // when the current set has a 'passed' qualified
        return SKIP; // and 'skipped' main for the Stopover rule.
      }
      else if (retCode == STOP)
      {
        return FAIL;
      }
      else if (retCode == STOP_SOFT)
      {
        return SOFTPASS;
      }
      else if (retCode == SOFTPASS)
      {
        if (cri.categoryNumber() == RuleConst::TRANSFER_RULE)
        {
          rc = SOFTPASS;
          continue;
        }

        return SOFTPASS;
      }
      else if (retCode == FAIL) // set was failed
      {
        if (_categoryPhase == ShoppingComponentValidateQualifiedCat4)
        {
          paxTypeFare.setCategoryValid(cri.categoryNumber(), false);
        }
        rc = retCode; // save FAIL and check next set.
      }
    } // else
  } // For loop

  if (retCode == SKIP &&
      (isCat15Security || (cri.categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE &&
                           paxTypeFare.isNegotiated() && paxTypeFare.cat35FailIf1OfThen15()) ||
       (cri.categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE &&
        paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF && isFareRule && !isCat15Security &&
        !skipCat15Security)))
  { //  Cat15 was failed by Security
    return FAIL;
  }

  if (UNLIKELY(seasonRet == PASS))
    return PASS; // return PASS if we pass at least 1 season.

  if (rc == FAIL)
    return FAIL;

  if (rc == SOFTPASS)
    return SOFTPASS;

  return SKIP;
}

//----------------------------------------------------------------------------
// process()
//----------------------------------------------------------------------------
Record3ReturnTypes
CategoryRuleItemSet::process(PricingTrx& trx,
                             const CategoryRuleInfo& cri,
                             FarePath& farePath,
                             const PricingUnit& pricingUnit,
                             FareUsage& fareUsage,
                             const std::vector<CategoryRuleItemInfoSet*>& crInfo,
                             RuleProcessingData& rpData,
                             bool isLocationSwapped,
                             bool isFareRule,
                             bool skipCat15Security,
                             RuleControllerDataAccess& da)
{
  Record3ReturnTypes retCode = SKIP;
  Record3ReturnTypes rc = SKIP;
  bool isCat15Security = false;

  // lint -e{530}
  PaxTypeFare& paxTypeFare = *(fareUsage.paxTypeFare());

  // cleanup E_TKT indicator for cat12 paper ticket surcharge: then12, if15, else12.
  // to separate FNotes/FRule/GRule.
  //  paxTypeFare.setElectronicTktRequired(false);
  fareUsage.setPaperTktSurchargeMayApply(false);

  PaxTypeFareRuleData* paxTypeFareRuleData = paxTypeFare.paxTypeFareRuleData(cri.categoryNumber());

  RuleSetPreprocessor ruleSetPreprocessor;
  ruleSetPreprocessor.process(trx, &cri, pricingUnit, fareUsage, &_categoryRuleItem, isFareRule);

  rpData.soInfoWrapper()->doRuleSetPreprocessing(trx, ruleSetPreprocessor, pricingUnit);

  rpData.trInfoWrapper()->doRuleSetPreprocessing(trx, ruleSetPreprocessor, pricingUnit);

  // This check is done for SALE_RESTRICTIONS_RULE only.
  if (cri.categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE &&
      trx.getRequest()->fareGroupRequested() == false)
  {
    if (trx.getOptions()->isWeb() || trx.itin().size() > 1 ||
        (trx.isShopping() &&
         ((trx.getRequest()->ticketingAgent()->agentTJR() != nullptr) &&
          (trx.getRequest()->ticketingAgent()->agentTJR()->tvlyLocation() == YES)) &&
         (paxTypeFare.fcasPaxType() == ADULT || paxTypeFare.fcasPaxType() == NEG ||
          paxTypeFare.fcasPaxType().empty())))
    {
      setWebFare(trx, cri, paxTypeFare, crInfo);
    }
  }

  //  if (paxTypeFareRuleData == 0 && isFareRule)
  //    return rc;

  // For Category 25 - Fare By Rule
  if (cri.categoryNumber() == RuleConst::FARE_BY_RULE)
  {
    // If the set that built the fare for cat25 started with an else
    // we need to call process for the preceding sets to see if they would pass,
    // if any of them pass fail this fare
    if (paxTypeFareRuleData->categoryRuleItemInfoSet()->at(0).relationalInd() == CategoryRuleItemInfo::ELSE)
    {
      // loop through the sets and call process.
      std::vector<CategoryRuleItemInfoSet*>::const_iterator crInfoB = crInfo.begin();
      std::vector<CategoryRuleItemInfoSet*>::const_iterator crInfoE = crInfo.end();

      for (; crInfoB != crInfoE; ++crInfoB)
      {
        retCode = _categoryRuleItem.process(trx,
                                            cri,
                                            farePath,
                                            pricingUnit,
                                            fareUsage,
                                            *(*crInfoB),
                                            paxTypeFareRuleData->isLocationSwapped(),
                                            isCat15Security,
                                            rpData,
                                            false,
                                            skipCat15Security,
                                            da);

        if (*crInfoB == paxTypeFareRuleData->categoryRuleItemInfoSet())
        {
          return retCode;
        }

        // if previous set passed we must fail this one for cat 25
        if (retCode == PASS)
        {
          return FAIL;
        }
        else
        {
          continue;
        }
      }
    }
    else
    {
      retCode = _categoryRuleItem.process(
          trx,
          cri,
          farePath,
          pricingUnit,
          fareUsage,
          *paxTypeFareRuleData->categoryRuleItemInfoSet(),
          paxTypeFareRuleData->isLocationSwapped(),
          isCat15Security,
          rpData,
          false,
          skipCat15Security,
          da);
    }

    if (UNLIKELY(retCode == SKIP && isCat15Security))
    { //  Cat15 was failed by Security
      return FAIL;
    }
    if (retCode == PASS)
    {
      return PASS;
    }
    if (retCode == STOP)
    {
      return FAIL;
    }
    if (LIKELY(retCode == FAIL))
    {
      return FAIL;
    }

    return SKIP;
  }

  //
  if (UNLIKELY(isFareRule && paxTypeFareRuleData == nullptr))
    paxTypeFareRuleData =
        fareUsage.paxTypeFare()->setCatRuleInfo(cri, trx.dataHandle(), isLocationSwapped);

  bool stopOverPass = false;
  bool transfersPass = false;
  StopoversInfoWrapper* soInfoWrapper = rpData.soInfoWrapper();
  TransfersInfoWrapper* trInfoWrapper = rpData.trInfoWrapper();

  std::vector<CategoryRuleItemInfoSet*>::const_iterator crInfoB = crInfo.begin();
  std::vector<CategoryRuleItemInfoSet*>::const_iterator crInfoE = crInfo.end();

  for (; crInfoB != crInfoE; ++crInfoB)
  {
    stopOverPass = false; // every new set, reset the pass status
    // set the set we are working on in the PaxTypeFareRuleData in the PaxTypeFare
    if (cri.categoryNumber() != RuleConst::NEGOTIATED_RULE && isFareRule)
    {
      paxTypeFareRuleData->categoryRuleItemInfoSet() = *crInfoB;
    }
    retCode = _categoryRuleItem.process(trx,
                                        cri,
                                        farePath,
                                        pricingUnit,
                                        fareUsage,
                                        *(*crInfoB),
                                        isLocationSwapped,
                                        isCat15Security,
                                        rpData,
                                        isFareRule,
                                        skipCat15Security,
                                        da);

    // cat 35 logic
    if (cri.categoryNumber() == RuleConst::NEGOTIATED_RULE)
    {
      if (*crInfoB == paxTypeFareRuleData->categoryRuleItemInfoSet())
      {
        return retCode;
      }

      continue;
    }

    if ((retCode == PASS) && (cri.categoryNumber() == RuleConst::TRANSFER_RULE))
    {
      // have to PASS to get final PU validation
      transfersPass = true;
      bool isCmdPricing = false;
      if (UNLIKELY(((fareUsage.paxTypeFare()->isFareByRule() || !fareUsage.cat25Fare()) &&
           fareUsage.paxTypeFare()->isCmdPricing()) ||
          (fareUsage.cat25Fare() && fareUsage.cat25Fare()->isCmdPricing())))
      {
        isCmdPricing = true;
      }
      if (trInfoWrapper->checkAllPassed() || isCmdPricing)
      {
        return PASS;
      }
      continue;
    }
    if ((retCode == PASS) && (cri.categoryNumber() == RuleConst::STOPOVER_RULE))
    {
      stopOverPass = true;
      bool isCmdPricing = false;
      if (UNLIKELY(((fareUsage.paxTypeFare()->isFareByRule() || !fareUsage.cat25Fare()) &&
           fareUsage.paxTypeFare()->isCmdPricing()) ||
          (fareUsage.cat25Fare() && fareUsage.cat25Fare()->isCmdPricing())))
      {
        isCmdPricing = true;
      }
      if (soInfoWrapper->checkAllPassed() || isCmdPricing)
      {
        return PASS;
      }
      continue;
    }

    if (retCode == PASS || retCode == STOP || retCode == STOP_SKIP)
    {
      break;
    }
    if (retCode == FAIL) // set was failed
    {
      rc = retCode; // save FAIL and check next set.
    }
  }

  if (retCode == SKIP &&
      (isCat15Security || (cri.categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE &&
                           paxTypeFare.isNegotiated() && paxTypeFare.cat35FailIf1OfThen15()) ||
       (cri.categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE &&
        paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF && isFareRule && !isCat15Security &&
        !skipCat15Security)))
  {
    if (cri.categoryNumber() == RuleConst::SURCHARGE_RULE)
    {
      return SKIP; // cat12 does not kill the fare.
    }
    //  Cat15 was failed by Security
    return FAIL;
  }
  if (retCode == PASS)
    return PASS;
  if (retCode == STOP)
  {
    if (cri.categoryNumber() == RuleConst::SURCHARGE_RULE)
    {
      return SKIP; // cat12 does not kill the fare.
    }
    return FAIL;
  }
  if (UNLIKELY(retCode == STOP_SKIP)) // Stopover condition
    return SKIP; // apply system assumption

  if (stopOverPass || transfersPass)
    return PASS;

  if (rc == FAIL)
  {
    if (UNLIKELY(cri.categoryNumber() == RuleConst::SURCHARGE_RULE))
    {
      return SKIP; // cat12 does not kill the fare.
    }
    return FAIL;
  }
  return SKIP;
}

//----------------------------------------------------------------------------
// process()
//----------------------------------------------------------------------------
Record3ReturnTypes
CategoryRuleItemSet::process(PricingTrx& trx,
                             const CategoryRuleInfo& cri,
                             const Itin* itin,
                             const PricingUnit& pricingUnit,
                             FareUsage& fareUsage,
                             const std::vector<CategoryRuleItemInfoSet*>& crInfo,
                             RuleProcessingData& rpData,
                             bool isLocationSwapped,
                             bool isFareRule,
                             bool skipCat15Security,
                             RuleControllerDataAccess& da)
{
  Record3ReturnTypes retCode = SKIP;
  Record3ReturnTypes rc = SKIP;
  bool isCat15Security = false;

  // lint -e{578, 530}
  PaxTypeFare& paxTypeFare = *(fareUsage.paxTypeFare());

  // cleanup E_TKT indicator for cat12 paper ticket surcharge: then12, if15, else12.
  // to separate FNotes/FRule/GRule.
  //  paxTypeFare.setElectronicTktRequired(false);
  fareUsage.setPaperTktSurchargeMayApply(false);

  // lint -e{578}
  PaxTypeFareRuleData* paxTypeFareRuleData = paxTypeFare.paxTypeFareRuleData(cri.categoryNumber());

  if (UNLIKELY(isFareRule && paxTypeFareRuleData == nullptr))
  {
    paxTypeFareRuleData = paxTypeFare.setCatRuleInfo(cri, trx.dataHandle(), isLocationSwapped);
  }

  RuleSetPreprocessor ruleSetPreprocessor;

  if (fallback::cat9unusedCode(&trx))
    ruleSetPreprocessor.process(trx, &cri, pricingUnit, fareUsage);

  // This check is done for SALE_RESTRICTIONS_RULE only.
  if (UNLIKELY(cri.categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE &&
      trx.getRequest()->fareGroupRequested() == false))
  {
    if (trx.getOptions()->isWeb() || trx.itin().size() > 1 ||
        (trx.isShopping() &&
         ((trx.getRequest()->ticketingAgent()->agentTJR() != nullptr) &&
          (trx.getRequest()->ticketingAgent()->agentTJR()->tvlyLocation() == YES)) &&
         (paxTypeFare.fcasPaxType() == ADULT || paxTypeFare.fcasPaxType() == NEG ||
          paxTypeFare.fcasPaxType().empty())))
    {
      setWebFare(trx, cri, paxTypeFare, crInfo);
    }
  }

  std::vector<CategoryRuleItemInfoSet*>::const_iterator crInfoB = crInfo.begin();
  std::vector<CategoryRuleItemInfoSet*>::const_iterator crInfoE = crInfo.end();

  for (; crInfoB != crInfoE; ++crInfoB)
  {
    // set the set we are working on in the PaxTypeFareRuleData in the PaxTypeFare
    if (cri.categoryNumber() != RuleConst::NEGOTIATED_RULE && isFareRule)
    {
      paxTypeFareRuleData->categoryRuleItemInfoSet() = *crInfoB;
    }

    retCode = _categoryRuleItem.process(trx,
                                        cri,
                                        itin,
                                        pricingUnit,
                                        fareUsage,
                                        *(*crInfoB),
                                        isLocationSwapped,
                                        isCat15Security,
                                        rpData,
                                        isFareRule,
                                        skipCat15Security,
                                        da);

    if (retCode == PASS || retCode == STOP || retCode == STOP_SKIP)
    {
      break;
    }
    if (retCode == FAIL) // set was failed
    {
      rc = retCode; // save FAIL and check next set.
    }
  }

  if (retCode == SKIP &&
      (isCat15Security || (cri.categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE &&
                           paxTypeFare.isNegotiated() && paxTypeFare.cat35FailIf1OfThen15()) ||
       (cri.categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE &&
        paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF && isFareRule && !isCat15Security &&
        !skipCat15Security)))
  { //  Cat15 was failed by Security
    return FAIL;
  }
  if (retCode == PASS)
    return PASS;
  if (retCode == STOP)
  {
    if (cri.categoryNumber() == RuleConst::SURCHARGE_RULE)
    {
      return SKIP; // cat12 does not kill the fare.
    }
    return FAIL;
  }
  if (UNLIKELY(retCode == STOP_SKIP)) // Stopover condition
    return SKIP; // apply system assumption
  if (rc == FAIL)
  {
    if (UNLIKELY(cri.categoryNumber() == RuleConst::SURCHARGE_RULE))
    {
      return SKIP; // cat12 does not kill the fare.
    }
    return FAIL;
  }

  return SKIP;
}

//  Shopping :
//   Do not validate Qualified cat4 just save the information, then validate during
// phase::shoppingComponentValidateQualified cat4
//
Record3ReturnTypes
CategoryRuleItemSet::processQualifyCat4(PricingTrx& trx,
                                        const CategoryRuleInfo& cri,
                                        const std::vector<CategoryRuleItemInfoSet*>& crInfo,
                                        PaxTypeFare& paxTypeFare)
{ // lint !e578
  if (_categoryPhase != ShoppingComponentValidateQualifiedCat4)
  {
    std::vector<CategoryRuleItemInfoSet*>::const_iterator crInfoB = crInfo.begin();
    std::vector<CategoryRuleItemInfoSet*>::const_iterator crInfoE = crInfo.end();

    for (; crInfoB != crInfoE; ++crInfoB)
    {
      const std::vector<CategoryRuleItemInfo>& cfrItemVec = *(*crInfoB);
      std::vector<CategoryRuleItemInfo>::const_iterator cfrItem = cfrItemVec.begin();
      std::vector<CategoryRuleItemInfo>::const_iterator cfrItemEnd = cfrItemVec.end();
      for (; cfrItem < cfrItemEnd; ++cfrItem)
      {
        if (cfrItem->relationalInd() ==
            CategoryRuleItemInfo::IF) // Does a RuleSet contain a qualified rule?
        {
          if (cfrItem->itemcat() == RuleConst::FLIGHT_APPLICATION_RULE)
          {
            if (trx.getTrxType() == PricingTrx::MIP_TRX && (_categoryPhase == FCORuleValidation || _categoryPhase == PreValidation))
            {
              // To track that there is qual 4
              paxTypeFare.qualifyFltAppRuleDataMap()[cri.categoryNumber()] = nullptr;
            }
            else
            {
              // lint --e{530}
              QualifyFltAppRuleData* qualifyFltAppRuleData;
              trx.dataHandle().get(qualifyFltAppRuleData);
              paxTypeFare.qualifyFltAppRuleDataMap()[cri.categoryNumber()] = qualifyFltAppRuleData;
              paxTypeFare.setCategoryProcessed(cri.categoryNumber());
              qualifyFltAppRuleData->categoryRuleInfo() = &cri;
            }

            return NOTPROCESSED;
          }
        }
      }
    } // for crInfo
  } // if not
  else if (LIKELY(_categoryPhase == ShoppingComponentValidateQualifiedCat4))
  {
    if (UNLIKELY(paxTypeFare.qualifyFltAppRuleDataMap().size() == 0))
    {
      return SKIP;
    }
    typedef uint32_t Key;
    typedef VecMap<Key, QualifyFltAppRuleData*> QMap;
    QMap::const_iterator iter = paxTypeFare.qualifyFltAppRuleDataMap().begin();
    QMap::const_iterator iterEnd = paxTypeFare.qualifyFltAppRuleDataMap().end();

    while (iter != iterEnd)
    {
      if (iter->first == cri.categoryNumber())
      {
        break;
      }
      iter++;
    }
    // not the save category
    if (UNLIKELY(iter == iterEnd))
    {
      return SKIP;
    }

  } // else ShoppingComponentValidateQulaifiedCat4
  return PASS;
}

//----------------------------------------------------------------------------
// processFareGroup()
//----------------------------------------------------------------------------
Record3ReturnTypes
CategoryRuleItemSet::processFareGroup(PricingTrx& trx,
                                      const CategoryRuleInfo& cri,
                                      PaxTypeFare& paxTypeFare,
                                      const std::vector<CategoryRuleItemInfoSet*>& crInfo)
{ // lint !e578
  Record3ReturnTypes retCode = FAIL;

  //  create the actualPaxTypeItem that need pcc validation.
  //  if pcc validation is not required, make it 0. This item will be skip.
  PosPaxType* posPaxType;
  std::vector<uint16_t> actualPaxTypeItem, actualPaxTypeItemOrig, actualPaxTypeItemTemp;
  size_t numPaxToProcess = 0;
  std::vector<PaxTypeBucket>& paxTypeCortege = paxTypeFare.fareMarket()->paxTypeCortege();
  std::vector<uint16_t>::iterator actualPaxTypeItemIt = paxTypeFare.actualPaxTypeItem().begin();
  std::vector<uint16_t>::iterator actualPaxTypeItemItEnd = paxTypeFare.actualPaxTypeItem().end();
  uint16_t paxTypeNum = 0;
  uint16_t itemIdx;
  for (; actualPaxTypeItemIt != actualPaxTypeItemItEnd; ++actualPaxTypeItemIt, ++paxTypeNum)
  {
    itemIdx = *actualPaxTypeItemIt;
    if (itemIdx != PaxTypeFare::PAXTYPE_FAIL)
    {
      if (itemIdx >= paxTypeCortege[paxTypeNum].actualPaxType().size())

      {
        itemIdx = PaxTypeFare::PAXTYPE_FAIL; // so we do not process this item
      }
      else
      {
        posPaxType = dynamic_cast<PosPaxType*>(paxTypeCortege[paxTypeNum].actualPaxType()[itemIdx]);
        if (posPaxType == nullptr)
          return FAIL;

        if (posPaxType->pcc().empty())
        {
          retCode = PASS;
          itemIdx = PaxTypeFare::PAXTYPE_FAIL;
        }
        else
        {
          ++numPaxToProcess;
        }
      }
    }
    actualPaxTypeItem.push_back(itemIdx);
  }

  const DateTime& applDate = paxTypeFare.fareMarket()->ruleApplicationDate();
  actualPaxTypeItemOrig = actualPaxTypeItem; // Keep original info set
  std::vector<CategoryRuleItemInfoSet*>::const_iterator crInfoIt = crInfo.begin();
  std::vector<CategoryRuleItemInfoSet*>::const_iterator crInfoItEnd = crInfo.end();
  // loop through category rule item info set
  for (; crInfoIt != crInfoItEnd && numPaxToProcess > 0; ++crInfoIt)
  {
    const std::vector<CategoryRuleItemInfo>& cfrItem = *(*crInfoIt);
    std::vector<CategoryRuleItemInfo>::const_iterator cfrItemIt = cfrItem.begin();
    std::vector<CategoryRuleItemInfo>::const_iterator cfrItemItEnd = cfrItem.end();
    // loop category rule item for each set
    for (; cfrItemIt != cfrItemItEnd && numPaxToProcess > 0; ++cfrItemIt)
    {
      const RuleItemInfo* const ruleItemInfo =
          RuleUtil::getRuleItemInfo(trx, &cri, &(*cfrItemIt), applDate);

      // Does a RuleSet contain a qualified rule?
      if (cfrItemIt->relationalInd() == CategoryRuleItemInfo::IF)
      {
        break; // Yes. Job is done for this set.
      }
      if (cfrItemIt->itemcat() == RuleConst::SALE_RESTRICTIONS_RULE)
      {
        if (ruleItemInfo == nullptr)
        {
          continue;
        }

        SalesRestrictionRule srr;
        const SalesRestriction* sr =
            trx.dataHandle().getSalesRestriction(cri.vendorCode(), cfrItemIt->itemNo());
        if (sr == nullptr)
        {
          continue;
        }
        // Validate faregroup pcc for all paxType if needed
        // srr return PAXTYPE_NO_MATCHED if no matched found
        // srr return PAXTYPE_FAIL if matched on negative fare group
        if (srr.validateFareGroup(trx, paxTypeFare, sr, actualPaxTypeItem))
        {
          actualPaxTypeItemTemp = actualPaxTypeItem;
          numPaxToProcess = 0;
          actualPaxTypeItemIt = actualPaxTypeItem.begin();
          actualPaxTypeItemItEnd = actualPaxTypeItem.end();
          for (paxTypeNum = 0; actualPaxTypeItemIt != actualPaxTypeItemItEnd;
               ++actualPaxTypeItemIt, ++paxTypeNum)
          {
            if (*actualPaxTypeItemIt == PaxTypeFare::PAXTYPE_NO_MATCHED)
            {
              // return original item for the next srr
              *actualPaxTypeItemIt = paxTypeFare.actualPaxTypeItem()[paxTypeNum];
              ++numPaxToProcess;
            }
            else if (*actualPaxTypeItemIt != PaxTypeFare::PAXTYPE_FAIL || // match positive item
                     (*actualPaxTypeItemIt == PaxTypeFare::PAXTYPE_FAIL &&
                      actualPaxTypeItemOrig[paxTypeNum] !=
                          PaxTypeFare::PAXTYPE_FAIL)) // match negative item
            {
              // update matched pcc item to actualPaxTypeItemIt in paxTypeFare
              // remove the iterator from actualPaxTypeItemIt temp. So this one is done
              paxTypeFare.actualPaxTypeItem()[paxTypeNum] = *actualPaxTypeItemIt;
              actualPaxTypeItem[paxTypeNum] = PaxTypeFare::PAXTYPE_FAIL;
              if (*actualPaxTypeItemIt != PaxTypeFare::PAXTYPE_FAIL)
              {
                retCode = PASS;
              }
            }
          }
        } // validateFareGroup
      } // srr
    }
  }
  // if number of passenger left to process. It means no macthed for that passenger.
  // It can pass the negative item. Locate the 1st negative item in the list for the
  // same pax type and corp id.
  if (numPaxToProcess)
  {
    actualPaxTypeItemIt = actualPaxTypeItemTemp.begin();
    actualPaxTypeItemItEnd = actualPaxTypeItemTemp.end();
    for (paxTypeNum = 0; actualPaxTypeItemIt != actualPaxTypeItemItEnd;
         ++actualPaxTypeItemIt, ++paxTypeNum)
    {
      if (*actualPaxTypeItemIt == PaxTypeFare::PAXTYPE_NO_MATCHED)
      {
        uint16_t actPaxTypeItem = paxTypeFare.actualPaxTypeItem()[paxTypeNum];
        PaxTypeCode& paxType =
            paxTypeCortege[paxTypeNum].actualPaxType()[actPaxTypeItem]->paxType(); // keep original
        // paxType
        std::vector<PaxType*>::iterator atPaxTypeItEnd =
            paxTypeCortege[paxTypeNum].actualPaxType().end();
        std::vector<PaxType*>::iterator atPaxTypeIt =
            paxTypeCortege[paxTypeNum].actualPaxType().begin() + actPaxTypeItem;
        bool matched = false;

        while (atPaxTypeIt != atPaxTypeItEnd && paxType == (*atPaxTypeIt)->paxType())
        {
          posPaxType = dynamic_cast<PosPaxType*>(*atPaxTypeIt);
          if (posPaxType == nullptr)
            return FAIL;

          // negative pcc and matched corp id whether we need it or not
          if (!posPaxType->positive() &&
              paxTypeFare.matchedCorpID() == !posPaxType->corpID().empty())
          {
            paxTypeFare.actualPaxTypeItem()[paxTypeNum] = actPaxTypeItem;
            matched = true;
            retCode = PASS;
            break;
          }
          ++actPaxTypeItem;
          ++atPaxTypeIt;
        }
        if (!matched)
        {
          paxTypeFare.actualPaxTypeItem()[paxTypeNum] = PaxTypeFare::PAXTYPE_FAIL;
        }
      }
    }
  }
  return retCode;
}

//----------------------------------------------------------------------------
// setWebFare()
//----------------------------------------------------------------------------
void
CategoryRuleItemSet::setWebFare(PricingTrx& trx,
                                const CategoryRuleInfo& cri,
                                PaxTypeFare& paxTypeFare,
                                const std::vector<CategoryRuleItemInfoSet*>& crInfo,
                                bool reuseCheck)
{ // lint !e578
  // bound fare:
  // std::cerr << "CategoryRuleItemSet::setWebFare" << std::endl;
  if (UNLIKELY(trx.getUseWebFareFlagBF()))
  {
    bool bTVLYLoc(trx.getRequest()->ticketingAgent() != nullptr &&
                  trx.getRequest()->ticketingAgent()->agentTJR() != nullptr &&
                  trx.getRequest()->ticketingAgent()->agentTJR()->tvlyLocation() == YES);
    if (bTVLYLoc && paxTypeFare.fare()->fareInfo()->isWebFare(bTVLYLoc))
    {
      paxTypeFare.setWebFare(true);
    }
    return;
  }
  bool webFareSet = false;

  const DateTime& applDate = paxTypeFare.fareMarket()->ruleApplicationDate();
  // Set the web fare indicator in the paxTypeFare if any of the Cat 15 locales contain
  // web agencies.
  // This check is done for Travelocity pseudos only.
  std::map<std::string, bool>& webFareMatchSet = paxTypeFare.fareMarket()->getWebFareMatchSet();

  for (CategoryRuleItemInfoSet* criInfoSet : crInfo)
  {
    const std::vector<CategoryRuleItemInfo>& cfrItem = *criInfoSet;
    for (uint i = 0; i < cfrItem.size(); ++i)
    {
      char buffer[32] = "";
      sprintf(buffer, "%s%u", cri.vendorCode().c_str(), cfrItem[i].itemNo());
      std::string tmpStr(buffer);
      if (reuseCheck)
      {
        std::map<std::string, bool>::const_iterator itWeb = webFareMatchSet.find(tmpStr);
        if (itWeb != webFareMatchSet.end())
        {
          if (UNLIKELY(itWeb->second == true))
          {
            paxTypeFare.setWebFare(true);
          }
          return;
        }
      }

      const RuleItemInfo* const ruleItemInfo =
          RuleUtil::getRuleItemInfo(trx, &cri, &cfrItem[i], applDate);

      // Does a RuleSet contain a qualified rule?
      if (cfrItem[i].relationalInd() == CategoryRuleItemInfo::IF)
      {
        break; // Yes. Job is done.
      }

      if (LIKELY(cfrItem[i].itemcat() == RuleConst::SALE_RESTRICTIONS_RULE))
      {
        if (ruleItemInfo == nullptr)
        {
          continue;
        }

        SalesRestrictionRule srr;
        const SalesRestriction* sr =
            trx.dataHandle().getSalesRestriction(cri.vendorCode(), cfrItem[i].itemNo());
        if (UNLIKELY(sr == nullptr))
        {
          if (reuseCheck)
          {
            webFareMatchSet.insert(std::make_pair(tmpStr, false));
          }

          continue;
        }

        if (srr.isWebFare(trx, sr))
        {
          if (reuseCheck)
          {
            webFareMatchSet.insert(std::make_pair(tmpStr, true));
          }

          paxTypeFare.setWebFare(true);
          webFareSet = true;
          break;
        }
        if (reuseCheck)
        {
          webFareMatchSet.insert(std::make_pair(tmpStr, false));
        }

      } // if (cfrItem[i]->itemcat() == RuleConst::SALE_RESTRICTIONS_RULE)
    } // for (uint i = 0; i < numElements; ++i)
    if (webFareSet)
      break;
  } // for ( ; crInfoB != crInfoE ; ++crInfoB )
  return;
}
} // tse
