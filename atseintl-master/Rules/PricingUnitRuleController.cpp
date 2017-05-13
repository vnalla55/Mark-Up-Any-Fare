//----------------------------------------------------------------------------
//
// Copyright Sabre 2007
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#include "Rules/PricingUnitRuleController.h"

#include "Common/ExchangeUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/SurchargeData.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/FarePUResultContainer.h"
#include "Rules/PricingUnitDataAccess.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleProcessingData.h"
#include "Rules/RuleSetPreprocessor.h"
#include "Rules/RuleUtil.h"
#include "Rules/StopoversInfoWrapper.h"
#include "Rules/TransfersInfoWrapper.h"
#include "Util/BranchPrediction.h"

#include <functional>
#include <vector>

using namespace std;

namespace tse
{
FALLBACK_DECL(fallbackGfrR2Optimization);
//------------------------------------------------------------------------------------------
FALLBACK_DECL(conversionDateSSDSP1154);
FALLBACK_DECL(cat9unusedCode);
// During fallback removal please remove also ExchnageUtill::RaiiProcessingDate::_isFallback
//------------------------------------------------------------------------------------------

namespace
{
bool
isDiagForMaxPenalty(const Diagnostic& diag)
{
  return diag.diagParamIsSet(Diagnostic::DISPLAY_DETAIL,
                             Diagnostic::MAX_PEN);
}
}

bool
PricingUnitRuleController::validate(PricingTrx& trx, FarePath& farePath, PricingUnit& pricingUnit)
{
  if (UNLIKELY(_categorySequence.empty()))
  {
    // no category to validate, should not happen unless
    // we remove all category from cfg
    return true;
  }

  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(trx);
  DiagCollector& diag = *diagPtr;
  bool isDiagDeactivatedForExcItin = false;
  FareUsage* failedKeepFareFu = nullptr;

  if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX || trx.excTrxType() == PricingTrx::AF_EXC_TRX))
  {
    RexBaseTrx& rexBaseTrx = static_cast<RexBaseTrx&>(trx);
    rexBaseTrx.skipRulesOnExcItin(_categorySequence);
    isDiagDeactivatedForExcItin = inactiveExcDiag(rexBaseTrx, diag);

    if (trx.excTrxType() == PricingTrx::AF_EXC_TRX)
      _categorySequence.erase(std::remove(_categorySequence.begin(),
                                          _categorySequence.end(),
                                          RuleConst::ADVANCE_RESERVATION_RULE),
                              _categorySequence.end());
  }

  const string& diagFareClass = trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);

  for (FareUsage* const fareUsage : pricingUnit.fareUsage())
  {
    const PaxTypeFare* ptf = fareUsage->paxTypeFare();

    if (UNLIKELY(!diagFareClass.empty()))
    {
      if (!trx.diagnostic().matchFareClass(diagFareClass, *ptf))
      {
        diagPtr->rootDiag()->deActivate();
      }
    }

    if (UNLIKELY(TrxUtil::reuseFarePUResult()))
      print555Fare(trx, diag, *ptf);

    else if (UNLIKELY(diag.diagnosticType() == Diagnostic555 &&
                       !isDiagForMaxPenalty(trx.diagnostic())))
    {
      if (((typeid(trx) == typeid(ShoppingTrx)) &&
           (_categoryPhase == ShoppingItinBasedValidation)) ||
          ((typeid(trx) == typeid(ShoppingTrx)) &&
           (_categoryPhase == ShoppingAltDateItinBasedValidation)) ||
          ((typeid(trx) == typeid(ShoppingTrx)) && (_categoryPhase == FPRuleValidationISALT)) ||
          ((typeid(trx) == typeid(PricingTrx)) && (_categoryPhase == FPRuleValidation)))
      {
        diag.enable(Diagnostic555);
        diag << "   " << std::setw(3) << (std::string)ptf->fareMarket()->boardMultiCity() << "-"
             << std::setw(2) << ptf->fareMarket()->governingCarrier() << "-" << std::setw(3)
             << (std::string)ptf->fareMarket()->offMultiCity() << "  :  " << ptf->fareClass()
             << endl;
        diag.flushMsg();
      }
    }

    bool validCatSeq = true;
    uint16_t resultKey = (_fareResultContainer && trx.getRequest()->isLowFareRequested())
                             ? _fareResultContainer->buildRebookStatBitMap(pricingUnit)
                             : 0;
    if (UNLIKELY(_fareResultContainer &&
        _fareResultContainer->getPreviousResult(*fareUsage, resultKey, validCatSeq)))
    {
      if (((_categoryPhase == FPRuleValidation) || (_categoryPhase == FPRuleValidationISALT)))
      {
        if (diag.diagnosticType() == Diagnostic555 &&
            !isDiagForMaxPenalty(trx.diagnostic()))
        {
          diag.enable(Diagnostic555);
          diag << "    RESULT REUSED - FARE ALREADY VALIDATED\n";
          diag.flushMsg();
        }
        else if (diag.diagnosticType() == Diagnostic605 || diag.diagnosticType() == Diagnostic500)
        {
          diag.enable(fareUsage, Diagnostic605, Diagnostic500);
          diag << *fareUsage->paxTypeFare();
          diag << "    RESULT REUSED - FARE ALREADY VALIDATED\n";
          diag.flushMsg();
        }
      }
    }
    else
    {
      validCatSeq =
          processCategorySequence(trx, farePath, pricingUnit, *fareUsage, _categorySequence);
      if (UNLIKELY(_fareResultContainer)) // && fareUsage->fpResultReusable())
      {
        _fareResultContainer->saveResultForReuse(*fareUsage, resultKey, validCatSeq);
      }
    }

    if (!validCatSeq)
    {
      if ((_categoryPhase == FPRuleValidation || _categoryPhase == FPRuleValidationISALT) &&
          diag.diagnosticType() == Diagnostic555 &&
          !isDiagForMaxPenalty(trx.diagnostic()))
      {
        diag.enable(Diagnostic555);
        diag << "     - FAIL - REVALIDATION" << endl << endl;
        diag.flushMsg();
      }

      if (UNLIKELY(fareUsage->isKeepFare()))
      {
        fareUsage->ruleFailed() = true;
        pricingUnit.ruleFailedButSoftPassForKeepFare() = true;
        failedKeepFareFu = fareUsage;
        continue; // continue check if non-keep fare can fail PU
      }
      else
      {
        pricingUnit.ruleFailedButSoftPassForKeepFare() = false;
      }

      if (LIKELY(!isDiagDeactivatedForExcItin))
        diagPtr->rootDiag()->activate();

      return false;
    }
    else
    {
      if ((_categoryPhase == FPRuleValidation || _categoryPhase == FPRuleValidationISALT) &&
          diag.diagnosticType() == Diagnostic555 &&
          _categorySequence.front() != RuleConst::SURCHARGE_RULE &&
          !isDiagForMaxPenalty(trx.diagnostic()))
      {
        diag.enable(Diagnostic555);
        diag << "     - PASS - REVALIDATION" << endl << endl;
        diag.flushMsg();
      }
    }
    if (LIKELY(!isDiagDeactivatedForExcItin))
      diagPtr->rootDiag()->activate();
  }

  if (UNLIKELY(failedKeepFareFu != nullptr))
  {
    if (!isDiagDeactivatedForExcItin)
      diagPtr->rootDiag()->activate();
    return false; // Only keep fare fails PU
  }

  if ((_categoryPhase == FPRuleValidation || _categoryPhase == FPRuleValidationISALT) &&
      diag.diagnosticType() == Diagnostic555 &&
      !isDiagForMaxPenalty(trx.diagnostic()))
  {
    diag.enable(Diagnostic555);
    diag << "\n \n";
    diag << "  -- ALL FARES IN PRICING UNIT - PASS RULE VALIDATION -- \n";
    diag << " \n";
    diag.flushMsg();
  }

  if (UNLIKELY(_categoryPhase == FPRuleValidation && pricingUnit.isCmdPricing()))
  {
    if (pricingUnit.cpFailedStatus().isSet(PaxTypeFare::PTFF_Cat8) &&
        pricingUnit.mostRestrictiveMaxStop() >= 0)
      adjustStopoversCharges(farePath, pricingUnit);

    if (pricingUnit.cpFailedStatus().isSet(PaxTypeFare::PTFF_Cat9) &&
        pricingUnit.mostRestrictiveMaxTransfer() >= 0 && !pricingUnit.hasTransferFCscope())
      adjustTransfersCharges(farePath, pricingUnit);
  }

  return true;
}

bool
PricingUnitRuleController::validate(PricingTrx& trx,
                                    FarePath& farePath,
                                    PricingUnit& pricingUnit,
                                    FareUsage& fareUsage)
{
  if (UNLIKELY(_categorySequence.empty()))
  {
    // no category to validate, should not happen unless
    // we remove all category from cfg
    return true;
  }

  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(trx);
  DiagCollector& diag = *diagPtr;

  bool isDiagDeactivatedForExcItin =
      (trx.excTrxType() == PricingTrx::AR_EXC_TRX || trx.excTrxType() == PricingTrx::AF_EXC_TRX)
          ? inactiveExcDiag(static_cast<RexBaseTrx&>(trx), diag)
          : false;

  const string& diagFareClass = trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);

  const PaxTypeFare* ptf = fareUsage.paxTypeFare();

  if (UNLIKELY(!diagFareClass.empty()))
  {
    if (!trx.diagnostic().matchFareClass(diagFareClass, *ptf))
    {
      diagPtr->rootDiag()->deActivate();
    }
  }

  // go throuth the Categories to validate rules
  if (UNLIKELY(TrxUtil::reuseFarePUResult()))
    print555Fare(trx, diag, *ptf);

  else if (UNLIKELY(diag.diagnosticType() == Diagnostic555 &&
                     !isDiagForMaxPenalty(trx.diagnostic())))
  {
    if (((typeid(trx) == typeid(ShoppingTrx)) && (_categoryPhase == ShoppingItinBasedValidation)) ||
        ((typeid(trx) == typeid(ShoppingTrx)) &&
         (_categoryPhase == ShoppingAltDateItinBasedValidation)) ||
        ((typeid(trx) == typeid(ShoppingTrx)) && (_categoryPhase == FPRuleValidationISALT)) ||
        ((typeid(trx) == typeid(PricingTrx)) && (_categoryPhase == FPRuleValidation)))
    {
      diag.enable(Diagnostic555);
      diag << "   " << std::setw(3) << (std::string)ptf->fareMarket()->boardMultiCity() << "-"
           << std::setw(2) << ptf->fareMarket()->governingCarrier() << "-" << std::setw(3)
           << (std::string)ptf->fareMarket()->offMultiCity() << "  :  " << ptf->fareClass() << endl;
      diag.flushMsg();
    }
  }

  if (!processCategorySequence(trx, farePath, pricingUnit, fareUsage, _categorySequence))

  {
    if ((_categoryPhase == FPRuleValidation || _categoryPhase == FPRuleValidationISALT) &&
        diag.diagnosticType() == Diagnostic555 &&
        !isDiagForMaxPenalty(trx.diagnostic()))
    {
      diag.enable(Diagnostic555);
      diag << "     - FAIL - REVALIDATION" << endl << endl;
      diag.flushMsg();
    }
    if (LIKELY(!isDiagDeactivatedForExcItin))
      diagPtr->rootDiag()->activate();
    return false;
  }

  if (UNLIKELY((_categoryPhase == FPRuleValidation || _categoryPhase == FPRuleValidationISALT) &&
                _categorySequence.front() != RuleConst::SURCHARGE_RULE &&
                diag.diagnosticType() == Diagnostic555 &&
                !isDiagForMaxPenalty(trx.diagnostic()) ))
  {
    diag.enable(Diagnostic555);
    diag << "     - PASS - REVALIDATION\n";
    diag.flushMsg();
  }

  if (LIKELY(!isDiagDeactivatedForExcItin))
    diagPtr->rootDiag()->activate();
  return true;
}

bool
PricingUnitRuleController::validate(PricingTrx& trx,
                                    PricingUnit& pricingUnit,
                                    FareUsage& fareUsage,
                                    Itin& itin)
{
  if (UNLIKELY(_categorySequence.empty()))
  {
    // no category to validate, can happen if we move Rule-re-validation
    // out of PU-Factory and do all in FarePath-Factory
    return true;
  }

  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(trx);
  DiagCollector& diag = *diagPtr;
  bool isDiagDeactivatedForExcItin = false;

  if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX || trx.excTrxType() == PricingTrx::AF_EXC_TRX))
  {
    RexBaseTrx& rexBaseTrx = static_cast<RexBaseTrx&>(trx);
    rexBaseTrx.skipRulesOnExcItin(_categorySequence);
    isDiagDeactivatedForExcItin = inactiveExcDiag(rexBaseTrx, diag);

    if (trx.excTrxType() == PricingTrx::AF_EXC_TRX)
      _categorySequence.erase(std::remove(_categorySequence.begin(),
                                          _categorySequence.end(),
                                          RuleConst::ADVANCE_RESERVATION_RULE),
                              _categorySequence.end());
  }

  const string& diagFareClass = trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);

  const PaxTypeFare* ptf = fareUsage.paxTypeFare();

  if (!diagFareClass.empty())
  {
    if (!trx.diagnostic().matchFareClass(diagFareClass, *ptf))
    {
      diagPtr->rootDiag()->deActivate();
    }
  }

  // go throuth the Categories to validate rules
  if (diag.diagnosticType() == Diagnostic555 &&
      !isDiagForMaxPenalty(trx.diagnostic()))
  {
    diag.enable(Diagnostic555);
    diag << "   " << std::setw(3) << (std::string)ptf->fareMarket()->boardMultiCity() << "-"
         << std::setw(2) << ptf->fareMarket()->governingCarrier() << "-" << std::setw(3)
         << (std::string)ptf->fareMarket()->offMultiCity() << "  :  " << ptf->fareClass() << endl;
    diag.flushMsg();
  }

  if (!processCategorySequence(trx, pricingUnit, fareUsage, _categorySequence, itin))

  {
    if (UNLIKELY((_categoryPhase == PURuleValidation || _categoryPhase == PURuleValidationIS ||
                 _categoryPhase == PURuleValidationISALT) &&
                  diag.diagnosticType() == Diagnostic555 &&
                  !isDiagForMaxPenalty(trx.diagnostic())))
    {
      diag.enable(Diagnostic555);
      diag << "     - FAIL - REVALIDATION" << endl << endl;
      diag.flushMsg();
    }
    if (LIKELY(!isDiagDeactivatedForExcItin))
      diagPtr->rootDiag()->activate();

    return false;
  }
  else
  {
    if (UNLIKELY((_categoryPhase == PURuleValidation || _categoryPhase == PURuleValidationIS ||
                  _categoryPhase == PURuleValidationISALT) &&
                  _categorySequence.front() != RuleConst::SURCHARGE_RULE &&
                  diag.diagnosticType() == Diagnostic555 &&
                  !isDiagForMaxPenalty(trx.diagnostic())))
    {
      diag.enable(Diagnostic555);
      diag << "     - PASS - REVALIDATION" << endl << endl;
      diag.flushMsg();
    }
  }

  if (LIKELY(!isDiagDeactivatedForExcItin))
    diagPtr->rootDiag()->activate();
  return true;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    PricingUnitRuleController::validate()
//
// this validate()   is called at PU_RULE_VALIDATION phase to validate all
//                   FareUsages on a PricingUnit. When any FareUsage failed,
//                   the validation failed and pointer to the FareUsage returned
//
//  @param PricingTrx&          - Pricing transaction
//  @param PricingUnit&         - Prcing Unit
//  @param FareUsage*&          - (Out) First Fare Usage that failed validation
//
//  @return bool -
//                 true      validation passed
//                 false     validation failed (failedFareUsage would be set)
//
// </PRE>
//-------------------------------------------------------------------
bool
PricingUnitRuleController::validate(PricingTrx& trx,
                                    PricingUnit& pricingUnit,
                                    FareUsage*& failedFareUsage,
                                    Itin& itin)
{
  failedFareUsage = nullptr;

  const string& diagFareClass = trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);

  bool displayDiag = false;
  FareUsage* failedKeepFareFu = nullptr;

  for (FareUsage* fareUsage : pricingUnit.fareUsage())
  {
    const PaxTypeFare* ptf = fareUsage->paxTypeFare();
    DiagCollector* diagPtr = nullptr;

    if (_categoryPhase == LoadRecords && (ptf->arePenaltyRecordsLoaded() ||
                                          ptf->isDummyFare()))
    {
      continue;
    }

    if (UNLIKELY(!diagFareClass.empty()))
    {
      if (trx.diagnostic().matchFareClass(diagFareClass, *ptf))
      {
        displayDiag = true;

        if ((_categoryPhase == PURuleValidation || _categoryPhase == PURuleValidationIS ||
            _categoryPhase == PURuleValidationISALT) &&
            trx.diagnostic().diagnosticType() == Diagnostic555 &&
            !isDiagForMaxPenalty(trx.diagnostic()))
        {
          DCFactory* factory = DCFactory::instance();
          diagPtr = factory->create(trx);
        }
      }
    }

    bool isValid = false;

    uint16_t resultKey = (_fareResultContainer && trx.getRequest()->isLowFareRequested())
                             ? _fareResultContainer->buildRebookStatBitMap(pricingUnit)
                             : 0;
    if (UNLIKELY(_fareResultContainer &&
        _fareResultContainer->getPreviousResult(*fareUsage, resultKey, isValid)))
    {
      if (isValid)
        pricingUnit.reusedResultFUs().push_back(fareUsage);
      if (UNLIKELY(diagPtr))
        doReuseDiag(*diagPtr, isValid, trx, ptf, *fareUsage);
    }
    else
    {
      isValid = validate(trx, pricingUnit, *fareUsage, itin);
      if (UNLIKELY(_fareResultContainer))
        _fareResultContainer->saveResultForReuse(*fareUsage, resultKey, isValid);
    }

    if (!isValid)
    {
      if (LIKELY(!fareUsage->failedCat5InAnotherFu()))
        failedFareUsage = fareUsage;

      if (UNLIKELY(fareUsage->isKeepFare()))
      {
        fareUsage->ruleFailed() = true;
        pricingUnit.ruleFailedButSoftPassForKeepFare() = true;
        failedKeepFareFu = fareUsage;
        continue; // continue check if non-keep fare can fail PU
      }
      else
      {
        pricingUnit.ruleFailedButSoftPassForKeepFare() = false;
      }

      return false;
    }
  }

  if (UNLIKELY(failedKeepFareFu != nullptr))
    return false; // Only keep fare fails PU

  if (UNLIKELY(displayDiag &&
               (_categoryPhase == PURuleValidation || _categoryPhase == PURuleValidationIS ||
                _categoryPhase == PURuleValidationISALT) &&
               trx.diagnostic().diagnosticType() == Diagnostic555 &&
               !isDiagForMaxPenalty(trx.diagnostic())))
  {
    DCFactory* factory = DCFactory::instance();
    DiagCollector* diagPtr = factory->create(trx);
    DiagCollector& diag = *diagPtr;
    diag.enable(Diagnostic555);
    diag << "\n \n";
    diag << "  -- ALL FARES IN PRICING UNIT - SOFTPASS VALIDATION -- \n";
    diag << " \n";
    diag.flushMsg();
  }

  return true;
}

bool
PricingUnitRuleController::validate(PricingTrx& trx, PricingUnit& pricingUnit, Itin& itin)
{
  if (_categorySequence.empty())
    return true;

  for (FareUsage* const fareUsage : pricingUnit.fareUsage())
  {
    if (!processCategorySequence(trx, pricingUnit, *fareUsage, _categorySequence, itin))
      return false;
  }
  return true;
}

bool
PricingUnitRuleController::processCategorySequence(PricingTrx& trx,
                                                   FarePath& farePath,
                                                   PricingUnit& pricingUnit,
                                                   FareUsage& fareUsage,
                                                   std::vector<uint16_t>& categorySequence)
{
  RexBaseTrx* rexBaseTrx = nullptr;

  if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX || trx.excTrxType() == PricingTrx::AF_EXC_TRX))
    rexBaseTrx = static_cast<RexBaseTrx*>(&trx);

  if (UNLIKELY((rexBaseTrx != nullptr) &&
      (rexBaseTrx->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE || // New itin fare
       fareUsage.paxTypeFare()->retrievalDate() != rexBaseTrx->fareApplicationDT()))) // Exc itin
                                                                                     // fare
  {
    ExchangeUtil::RaiiProcessingDate dateSetter(*rexBaseTrx,
                                                fareUsage.paxTypeFare()->retrievalDate(),
                                                false);
    return processCategorySequenceSub(trx, farePath, pricingUnit, fareUsage, categorySequence);
  }
  return processCategorySequenceSub(trx, farePath, pricingUnit, fareUsage, categorySequence);
}

bool
PricingUnitRuleController::processCategorySequence(PricingTrx& trx,
                                                   PricingUnit& pricingUnit,
                                                   FareUsage& fareUsage,
                                                   std::vector<uint16_t>& categorySequence,
                                                   Itin& itin)
{
  PricingUnitDataAccess da(trx, nullptr, pricingUnit, fareUsage, &itin);

  if (pricingUnit.validatingCarriers().size() == 1)
    da.setValidatingCxr( pricingUnit.validatingCarriers().front() );

  RexBaseTrx* rexBaseTrx = nullptr;

  if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX || trx.excTrxType() == PricingTrx::AF_EXC_TRX))
    rexBaseTrx = static_cast<RexBaseTrx*>(&trx);

  if (UNLIKELY((rexBaseTrx != nullptr) &&
      (rexBaseTrx->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE || // New itin fare
       fareUsage.paxTypeFare()->retrievalDate() != rexBaseTrx->fareApplicationDT()))) // Exc itin
                                                                                     // fare
  {
    boost::lock_guard<boost::mutex> guard(rexBaseTrx->dataHandleMutex());

    ExchangeUtil::RaiiProcessingDate dateSetter(*rexBaseTrx,
                                                fareUsage.paxTypeFare()->retrievalDate(),
                                                fallback::conversionDateSSDSP1154(&trx));
    return processCategorySequenceCommon(trx, da, categorySequence);
  }
  return processCategorySequenceCommon(trx, da, categorySequence);
}

bool
PricingUnitRuleController::processCategorySequenceSub(PricingTrx& trx,
                                                      FarePath& farePath,
                                                      PricingUnit& pricingUnit,
                                                      FareUsage& fareUsage,
                                                      std::vector<uint16_t>& categorySequence)
{
  PricingUnitDataAccess da(trx, &farePath, pricingUnit, fareUsage, nullptr);
  const FarePath& cfarePath = farePath;
  if (cfarePath.validatingCarriers().size() == 1)
    da.setValidatingCxr( cfarePath.validatingCarriers().front() );

  if (UNLIKELY(fareUsage.isSoftPassed()))
  {
    std::vector<uint16_t> newCatSequence;
    fareUsage.getSoftPassedCategories(newCatSequence);
    newCatSequence.insert(newCatSequence.end(), categorySequence.begin(), categorySequence.end());

    return processCategorySequenceCommon(trx, da, newCatSequence);
  }
  return processCategorySequenceCommon(trx, da, categorySequence);
}

Record3ReturnTypes
PricingUnitRuleController::revalidateC15BaseFareForDisc(uint16_t category,
                                                        bool& checkFare,
                                                        PaxTypeFare* ptfDisc,
                                                        RuleControllerDataAccess& da)
{
  // Cat15 revalidation will be done only in FP scope
  // for the pure (not C25/35) Discounted fare when a base fare has SOFTPASS.
  PricingUnitDataAccess& pda(static_cast<PricingUnitDataAccess&>(da));
  PaxTypeFareRuleData* ruleData = ptfDisc->paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE);

  Record3ReturnTypes retResultOfRule = PASS;

  // we need to swap the Discounted PaxTypeFare in the fareUsage object to point
  // to the base fare for C15 rules validation

  if (ruleData)
    pda.fareUsage().paxTypeFare() = ruleData->baseFare();

  //  PaxTypeFare& currentBaseFare = pda.paxTypeFare();

  retResultOfRule = processRules(pda.trx(), pda, category, true, false, false);

  // This code swaps the paxTypeFare object back to the original fare
  if (ruleData)
  {
    pda.fareUsage().paxTypeFare() = ptfDisc;
    // set up "cat15SoftPass" indicator
    ptfDisc->setCat15SoftPass(ruleData->baseFare()->cat15SoftPass());
  }

  return retResultOfRule;
}

Record3ReturnTypes
PricingUnitRuleController::validateBaseFare(uint16_t category,
                                            const FareByRuleItemInfo* fbrItemInfo,
                                            bool& checkFare,
                                            PaxTypeFare* fbrFare,
                                            RuleControllerDataAccess& da)
{
  PricingUnitDataAccess& pda = dynamic_cast<PricingUnitDataAccess&>(da);
  bool checkFBRBaseFare = false;

  RuleUtil::determineRuleChecks(category, *fbrItemInfo, checkFare, checkFBRBaseFare);

  // Notify Cat 15 rule validation to skip Cat 15 security check on base fare
  // when Cat 25 has Cat 35 rule.
  bool skipCat15Security = false;

  // processFareRuleCaller.retrieveBaseFare(true);
  pda.retrieveBaseFare(true);

  PaxTypeFare* ptFare = nullptr;

  // For cat 25 we need to swap the PaxTypeFare in the fareUsage object to point
  // to the base fare for rules validation

  ptFare = pda.fareUsage().paxTypeFare();

  if (pda.fareUsage().segmentStatus().empty())
    pda.fareUsage().segmentStatus().insert(pda.fareUsage().segmentStatus().end(),
                                           ptFare->segmentStatus().begin(),
                                           ptFare->segmentStatus().end());
  if (pda.fareUsage().segmentStatusRule2().empty())
    pda.fareUsage().segmentStatusRule2().insert(pda.fareUsage().segmentStatusRule2().end(),
                                                ptFare->segmentStatusRule2().begin(),
                                                ptFare->segmentStatusRule2().end());

  const FBRPaxTypeFareRuleData* fbrPaxTypeFare = ptFare->getFbrRuleData(RuleConst::FARE_BY_RULE);
  if (LIKELY(fbrPaxTypeFare))
    pda.fareUsage().paxTypeFare() = fbrPaxTypeFare->baseFare();

  PaxTypeFare& currentBaseFare = pda.paxTypeFare();

  const VendorCode& vendor(currentBaseFare.vendor());

  if (ptFare->isNegotiated())
  {
    if (RuleUtil::isVendorPCC(ptFare->vendor(), pda.trx()) &&
        (vendor.equalToConst("ATP") || vendor.equalToConst("SITA") || vendor.equalToConst("SMFA") || vendor.equalToConst("SMFC")) &&
        !(currentBaseFare.fcaDisplayCatType() == RuleConst::SELLING_FARE ||
          currentBaseFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
          currentBaseFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD))
    {
      // For Cat 25/35 DFF and base fare non-Cat 35 ATP/SITA/SMFA/SMFC,
      // Do not skip Cat 15 security validation of this base fare
      skipCat15Security = false;
    }
    else
    {
      skipCat15Security = true;
    }
  }
  else // not negotiated
  {
    if (PricingTrx::MIP_TRX == da.trx().getTrxType())
    {
      PaxTypeFare* ptf = nullptr;
      ptf = &currentBaseFare;

      if ((ptf->fcaDisplayCatType() == RuleConst::SELLING_FARE) ||
          (ptf->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE) ||
          (ptf->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD))
      {
        skipCat15Security = true;
      }
    }
  }

  // We always need to check the FootNote rule for baseFares
  Record3ReturnTypes retResultOfRule;

  if ((currentBaseFare.isCategoryProcessed(category)) &&
      (!currentBaseFare.isCategorySoftPassed(category)))
  {
    // Do not revalidate the baseFare when this job was done.
    //
    // Min/Max stay rule should be revalidated because of a temporary problem
    //
    // Stopovers, Transfers and Surcharges must always be revalidated
    //  in order to apply surcharges to the FareUsage. We have to do
    //  this because multiple FareByRule fares can share the same
    //  PaxTypeFare of the base fare.
    //
    if (category == RuleConst::STOPOVER_RULE || category == RuleConst::TRANSFER_RULE ||
        category == RuleConst::SURCHARGE_RULE)
    {
      currentBaseFare.setCategoryProcessed(category, false);
      currentBaseFare.setCategorySoftPassed(category, false);
    }
    else if (category != RuleConst::MINIMUM_STAY_RULE && category != RuleConst::MAXIMUM_STAY_RULE)
    {
      // This code swaps the paxTypeFare object back to the original fare
      pda.fareUsage().paxTypeFare() = ptFare;
      return PASS;
    }
  }

  // Create Footnotes Vector for the base fare
  pda.footNoteTbl().clear();
  getFootnotes(currentBaseFare, pda.footNoteTbl());

  pda.fareUsage().cat25Fare() = ptFare;

  retResultOfRule =
      processRules(pda.trx(), pda, category, checkFBRBaseFare, skipCat15Security, checkFBRBaseFare);

  pda.fareUsage().cat25Fare() = nullptr;

  // This code swaps the paxTypeFare object back to the original fare
  pda.fareUsage().paxTypeFare() = ptFare;

  // Restore Footnotes vector for the original FBR fare
  pda.footNoteTbl().clear();
  getFootnotes(*ptFare, pda.footNoteTbl());

  return retResultOfRule;
}

Record3ReturnTypes
PricingUnitRuleController::doCategoryPostProcessing(PricingTrx& trx,
                                                    RuleControllerDataAccess& da,
                                                    const uint16_t category,
                                                    RuleProcessingData& rpData,
                                                    const Record3ReturnTypes preResult)
{
  Record3ReturnTypes result = preResult;

  PricingUnitDataAccess& pda = dynamic_cast<PricingUnitDataAccess&>(da);

  if ((result == PASS) && (rpData.soInfoWrapper()->stopoversRuleExistsInSet()))
  {
    if (LIKELY(pda.farePath()))
    {
      result = rpData.soInfoWrapper()->processResults(
          trx, *(pda.farePath()), pda.pricingUnit(), pda.fareUsage());
    }
  }

  if ((result == PASS) && (rpData.trInfoWrapper()->transfersRuleExistsInSet()))
  {
    if (LIKELY(pda.farePath()))
    {
      result = rpData.trInfoWrapper()->processResults(
          trx, *(pda.farePath()), pda.pricingUnit(), pda.fareUsage());
    }
  }

  return result;
}

void
PricingUnitRuleController::preparePtfVector(std::vector<PaxTypeFare*>& ptfVec,
                                            const PricingTrx& trx,
                                            const FareMarket& fm) const
{
  if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    const RexPricingTrx* rexTrx = static_cast<const RexPricingTrx*>(&trx);
    FareMarket* siblingFM = nullptr;
    if (rexTrx->trxPhase() == RexPricingTrx::REPRICE_EXCITIN_PHASE &&
        rexTrx->isPlusUpCalculationNeeded() &&
        (siblingFM = rexTrx->exchangeItin().front()->getSiblingMarket(&fm)))
    {
      ptfVec.insert(
          ptfVec.end(), siblingFM->allPaxTypeFare().begin(), siblingFM->allPaxTypeFare().end());
    }
  }
}

void
PricingUnitRuleController::applySurchargeGenRuleForFMS(PricingTrx& trx,
                                                       RuleControllerDataAccess& da,
                                                       uint16_t categoryNumber,
                                                       RuleControllerParam& rcParam,
                                                       bool skipCat15Security)
{
  PricingUnitDataAccess& puda = dynamic_cast<PricingUnitDataAccess&>(da);

  PaxTypeFare* ptFMS = puda.fareUsage().paxTypeFare(); // current PaxTypeFare

  if (LIKELY(!shouldApplyGeneralRuleSurcharges(trx, *ptFMS)))
    return;

  // At this point the current paxTypeFare is FMS_fare (because of non_private vendor)
  // Need to process a GeneralRule surcharges.

  std::vector<PaxTypeFare*> ptfVec = ptFMS->fareMarket()->allPaxTypeFare();
  preparePtfVector(ptfVec, trx, *ptFMS->fareMarket());
  for (PaxTypeFare* ptf : ptfVec)
  {
    if (!fareApplicableToGeneralRuleSurcharges(trx, *ptFMS, ptf))
      continue;

    if ((ptf->directionality() != BOTH) &&
        ((puda.fareUsage().isInbound() && (ptf->directionality() == FROM)) ||
         (puda.fareUsage().isOutbound() && (ptf->directionality() == TO))))
      continue; // skip fare with wrong directionality

    // swap the FMS paxTypeFare in the FareUsage object to point to the matched ptf.
    puda.fareUsage().paxTypeFare() = ptf; // do it for car12 processing

    // apply the GeneralRule processing of the Public vendor to the FMS fare.
    // SurchargesRule processing will add the GenRule surcharge to FMS fareUsage object.
    da.retrieveBaseFare(false); // To get paxTypeFare from fareUsage
    rcParam._processGenRuleForFMS = true; // To skip FareRule processing

    RuleProcessingData rpData;

    StopoversInfoWrapper soInfoWrapper(&puda.fareUsage());
    TransfersInfoWrapper trInfoWrapper(&puda.fareUsage());
    rpData.soInfoWrapper(&soInfoWrapper);
    rpData.trInfoWrapper(&trInfoWrapper);

    processFareRuleCommon(trx, da, categoryNumber, rcParam, rpData, skipCat15Security);

    Record3ReturnTypes retResultOfGRule =
        fallback::fallbackGfrR2Optimization(&trx)
            ? processGeneralRule<FallBackOnGfr>(
                  trx, da, categoryNumber, rcParam, rpData, skipCat15Security)
            : processGeneralRule<FallBackOffGfr>(
                  trx, da, categoryNumber, rcParam, rpData, skipCat15Security);
    // swap the paxTypeFare object back to the FMS fare
    puda.fareUsage().paxTypeFare() = ptFMS;
    rcParam._processGenRuleForFMS = false;
    if (retResultOfGRule == PASS)
      break; // job is done...
  }
  return;
}

Record3ReturnTypes
PricingUnitRuleController::callCategoryRuleItemSet(
    CategoryRuleItemSet& catRuleIS,
    const CategoryRuleInfo& ruleInfo,
    const std::vector<CategoryRuleItemInfoSet*>& crInfo,
    RuleControllerDataAccess& da,
    RuleProcessingData& rpData,
    bool isLocationSwapped,
    bool isFareRule,
    bool skipCat15Security)
{
  PricingUnitDataAccess& pda = dynamic_cast<PricingUnitDataAccess&>(da);

  if (pda.farePath())
  {
    return catRuleIS.process(pda.trx(),
                             ruleInfo,
                             *pda.farePath(),
                             pda.pricingUnit(),
                             pda.fareUsage(),
                             crInfo,
                             rpData,
                             isLocationSwapped,
                             isFareRule,
                             skipCat15Security,
                             da);
  }
  else
  {
    std::vector<CarrierCode>& validatingCarriers = pda.pricingUnit().validatingCarriers();

    if (pda.trx().isValidatingCxrGsaApplicable() && !validatingCarriers.empty() && existCat15QualifyCat15ValidatingCxrRest(pda.trx(), ruleInfo))
    {
      int skipCounter = validatingCarriers.size();
      std::vector<CarrierCode> failedCxrs;
      DiagManager diag500(da.trx(), Diagnostic500);
      for (CarrierCode& vcr : validatingCarriers)
      {
        if (UNLIKELY(diag500.isActive()))
        {
           diag500 << "      VALIDATING CXR - " << vcr << " :\n";
           diag500.collector().flushMsg();
        }
        da.setValidatingCxr(vcr);
        Record3ReturnTypes result = catRuleIS.process(pda.trx(),
                                                      ruleInfo,
                                                      pda.itin(),
                                                      pda.pricingUnit(),
                                                      pda.fareUsage(),
                                                      crInfo,
                                                      rpData,
                                                      isLocationSwapped,
                                                      isFareRule,
                                                      skipCat15Security,
                                                      da);
        if (result == FAIL)
        {
          failedCxrs.push_back(vcr);
        }
        if (result == SKIP)
           --skipCounter;
      }
      for (CarrierCode& vcr : failedCxrs)
      {
        auto vcrIt = std::find(validatingCarriers.begin(), validatingCarriers.end(), vcr);
        if (vcrIt != validatingCarriers.end())
            pda.pricingUnit().validatingCarriers().erase(vcrIt);
      }
      if (validatingCarriers.empty())
      {
        return FAIL;
      }
      else if (skipCounter == 0)
      {
         return SKIP;
      }
      return PASS;
    }
    else
    {
      return catRuleIS.process(pda.trx(),
                               ruleInfo,
                               pda.itin(),
                               pda.pricingUnit(),
                               pda.fareUsage(),
                               crInfo,
                               rpData,
                               isLocationSwapped,
                               isFareRule,
                               skipCat15Security,
                               da);
    }
  }
}

Record3ReturnTypes
PricingUnitRuleController::reValidDiscQualifiers(PaxTypeFare& paxTypeFare,
                                                 const uint16_t& category,
                                                 RuleControllerDataAccess& da)
{
  PaxTypeFareRuleData* ruleData =
      paxTypeFare.paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE);
  if (!ruleData || !ruleData->isSoftPassDiscount())
    return PASS;

  if (ruleData->categoryRuleInfo()->categoryNumber() != category)
    return PASS;

  PricingUnitDataAccess& pda = dynamic_cast<PricingUnitDataAccess&>(da);

  if (!pda.farePath())
    return PASS;

  const std::vector<CategoryRuleItemInfo>& seqQualifiers = *(ruleData->categoryRuleItemInfoVec());
  RuleSetPreprocessor ruleSetPreprocessor;
  if (fallback::cat9unusedCode(&pda.trx()))
    ruleSetPreprocessor.process(
        pda.trx(), ruleData->categoryRuleInfo(), pda.pricingUnit(), pda.fareUsage());

  RuleProcessingData rpData;

  StopoversInfoWrapper soInfoWrapper(&pda.fareUsage());
  TransfersInfoWrapper trInfoWrapper(&pda.fareUsage());
  rpData.soInfoWrapper(&soInfoWrapper);
  rpData.trInfoWrapper(&trInfoWrapper);

  bool isCat15Security = false;
  CategoryRuleItem categoryRuleItem;

  Record3ReturnTypes rtn = SKIP;
  bool isQualified = categoryRuleItem.isQualifiedCategory(pda.trx(),
                                                          *ruleData->categoryRuleInfo(),
                                                          pda.farePath(),
                                                          nullptr,
                                                          pda.pricingUnit(),
                                                          pda.fareUsage(),
                                                          seqQualifiers,
                                                          rtn,
                                                          ruleData->isLocationSwapped(),
                                                          isCat15Security,
                                                          rpData,
                                                          true,
                                                          da);

  if (!isQualified)
    return PASS;

  if (rtn == FAIL)
    return FAIL;
  else
    return PASS;
}

namespace
{
void
removeCharges(const MoneyAmount sosAmount, MoneyAmount& fuChargesAmt, FarePath& farePath)
{
  MoneyAmount chargesAmt = fuChargesAmt;
  const double diff1 = chargesAmt - sosAmount;

  if ((diff1 >= 0.0 && diff1 < EPSILON) || (diff1 <= 0.0 && -diff1 < EPSILON))
  {
    chargesAmt = 0;
  }
  else if (chargesAmt - sosAmount > EPSILON)
  {
    chargesAmt -= sosAmount;
  }
  else
  {
    throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }
  // Update new fareUsage->stopOverAmt() for Cat 8 or
  // Update new fareUsage->transferAmt() for Cat 9
  fuChargesAmt = chargesAmt;

  MoneyAmount totalNUCAmount = farePath.getTotalNUCAmount();
  const double diff2 = totalNUCAmount - sosAmount;

  if ((diff2 >= 0.0 && diff2 < EPSILON) || (diff2 <= 0.0 && -diff2 < EPSILON))
  {
    totalNUCAmount = 0;
  }
  else if (totalNUCAmount - sosAmount > EPSILON)
  {
    totalNUCAmount -= sosAmount;
  }
  else
  {
    throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }
  farePath.setTotalNUCAmount(totalNUCAmount);
}

template <typename T>
struct SurchargesCleaner
{
  void operator()(TravelSeg* ts)
  {
    auto scIter = surchargesMap.find(ts);
    if (scIter == surchargesMap.end())
      return;

    if (numMax > 0)
      --numMax;
    else
    {
      removeCharges((scIter->second)->amount(), surchargesAmount, farePath);
      surchargesMap.erase(scIter);
    }
  }

  MoneyAmount& surchargesAmount;
  FarePath& farePath;
  T& surchargesMap;
  int16_t& numMax;
};
}

void
PricingUnitRuleController::adjustStopoversCharges(FarePath& farePath, PricingUnit& pricingUnit)
{
  int16_t numStopsMax = pricingUnit.mostRestrictiveMaxStop();
  for (FareUsage* const fareUsage : pricingUnit.fareUsage())
  {
    SurchargesCleaner<FareUsage::StopoverSurchargeMultiMap> cleaner{
        fareUsage->stopOverAmt(), farePath, fareUsage->stopoverSurcharges(), numStopsMax};
    const auto& travelSegs = fareUsage->travelSeg();
    if (fareUsage->isInbound() && !chargeFromFirstInbound(fareUsage))
      std::for_each(travelSegs.rbegin(), travelSegs.rend(), std::move(cleaner));
    else
      std::for_each(travelSegs.begin(), travelSegs.end(), std::move(cleaner));
  }
}

void
PricingUnitRuleController::adjustTransfersCharges(FarePath& farePath, PricingUnit& pricingUnit)
{
  int16_t numTransfersMax = pricingUnit.mostRestrictiveMaxTransfer();
  for (FareUsage* const fareUsage : pricingUnit.fareUsage())
  {
    SurchargesCleaner<FareUsage::TransferSurchargeMultiMap> cleaner{
        fareUsage->transferAmt(), farePath, fareUsage->transferSurcharges(), numTransfersMax};
    const auto& travelSegs = fareUsage->travelSeg();
    std::for_each(travelSegs.begin(), travelSegs.end(), cleaner);
  }
}

bool
PricingUnitRuleController::chargeFromFirstInbound(FareUsage* fareUsage)
{
  for (const TravelSeg* const ts : fareUsage->travelSeg())
  {
    auto scIter = fareUsage->stopoverSurcharges().find(ts);
    if (scIter == fareUsage->stopoverSurcharges().end())
      continue;

    if (scIter->second->chargeFromFirstInbound())
      return true;
  }
  return false;
}

bool
PricingUnitRuleController::inactiveExcDiag(const RexBaseTrx& rexBaseTrx, const DiagCollector& diag)
    const
{
  return rexBaseTrx.trxPhase() != RexPricingTrx::PRICE_NEWITIN_PHASE &&
         !diag.rootDiag()->isActive();
}

Record3ReturnTypes
PricingUnitRuleController::applySystemDefaultAssumption(PricingTrx& trx,
                                                        RuleControllerDataAccess& da,
                                                        const uint16_t category,
                                                        bool& displayDiag)
{
  return RuleController::applySystemDefaultAssumption(trx, da, category, displayDiag);
}

void
PricingUnitRuleController::print555Fare(const PricingTrx& trx,
                                        DiagCollector& diag,
                                        const PaxTypeFare& ptf) const
{
  if (diag.diagnosticType() == Diagnostic555 &&
      !isDiagForMaxPenalty(trx.diagnostic()))
  {
    if (((typeid(trx) == typeid(ShoppingTrx)) && (_categoryPhase == ShoppingItinBasedValidation)) ||
        ((typeid(trx) == typeid(ShoppingTrx)) &&
         (_categoryPhase == ShoppingAltDateItinBasedValidation)) ||
        ((typeid(trx) == typeid(ShoppingTrx)) && (_categoryPhase == FPRuleValidationISALT)) ||
        ((typeid(trx) == typeid(PricingTrx)) && (_categoryPhase == FPRuleValidation)))
    {
      diag.enable(Diagnostic555);
      diag << "   " << std::setw(3) << (std::string)ptf.fareMarket()->boardMultiCity() << "-"
           << std::setw(2) << ptf.fareMarket()->governingCarrier() << "-" << std::setw(3)
           << (std::string)ptf.fareMarket()->offMultiCity() << "  :  " << ptf.fareClass() << endl;
      diag.flushMsg();
    }
  }
}

void
PricingUnitRuleController::doReuseDiag(DiagCollector& diag,
                                       bool isValid,
                                       PricingTrx& trx,
                                       const PaxTypeFare* ptf,
                                       const FareUsage& fareUsage)
{
  if (diag.diagnosticType() == Diagnostic555 &&
      !isDiagForMaxPenalty(trx.diagnostic()))
  {
    print555Fare(trx, diag, *ptf);
    diag << " RESULT REUSED - FARE ALREADY VALIDATED - " << (isValid ? "PASS" : "FAIL") << "\n";
    diag.flushMsg();
  }
  else if (diag.diagnosticType() == Diagnostic605 || diag.diagnosticType() == Diagnostic500)
  {
    diag.enable(&fareUsage, Diagnostic605, Diagnostic500);
    diag << *fareUsage.paxTypeFare();
    diag << " RESULT REUSED - FARE ALREADY VALIDATED\n";
    diag.flushMsg();
  }
}
}
