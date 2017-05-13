//-------------------------------------------------------------------
//
//  File:        FareController.cpp
//  Created:     Dec 4, 2003
//  Authors:     Abu Islam, Mark Kasprowicz
//
//  Description: Base class for all Fare Controllers
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

#include "Fares/FareController.h"

#include "AddonConstruction/FareDup.h"
#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TSELatencyData.h"
#include "Common/TseUtil.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PosPaxType.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/QualifyFltAppRuleData.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/EligibilityInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/SeasonalAppl.h"
#include "DBAccess/TariffInhibits.h"
#include "DBAccess/TravelRestriction.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag201Collector.h"
#include "Diagnostic/Diag204Collector.h"
#include "Diagnostic/Diag225Collector.h"
#include "Diagnostic/Diag258Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Fares/FareTypeMatcher.h"
#include "Fares/FareUtil.h"
#include "Rules/FareMarketDataAccess.h"
#include "Rules/FDDayTimeApplication.h"
#include "Rules/FDEligibility.h"
#include "Rules/FDSalesRestrictionRule.h"
#include "Rules/FDSeasonalApplication.h"
#include "Rules/FDTravelRestrictions.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Util/Algorithm/Container.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FALLBACK_DECL(fallbackFootNotePrevalidation)
FALLBACK_DECL(fallbackFootNotePrevalidationForAltDates)
FALLBACK_DECL(fallbackAPO37838Record1EffDateCheck)
FALLBACK_DECL(fallbackSimpleTripCorrectionOWFare)
FALLBACK_DECL(compDiffCmdPricing);
FALLBACK_DECL(reworkTrxAborter);
FALLBACK_DECL(dffOaFareCreation);
FALLBACK_DECL(createSMFOFaresForALlUsers);

static Logger
_logger("atseintl.Fares.FareController");

FareController::FareController(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
  : Record1Resolver(trx, itin, fareMarket),
    _itin(itin),
    _nuc("NUC"),
    _prevalidationRuleController(PreValidation, &trx),
    _fareGroupSRValidationRuleController(FareGroupSRValidation, &trx),
    _matchFaresDirectionality(boost::indeterminate)
{
  std::call_once(_footNotePrevalidationEnabledSet, &PricingTrx::setFootNotePrevalidationEnabled,
                 &_trx, !fallback::fallbackFootNotePrevalidation(&_trx));

  // set up to pre-validate the sales restriction - sales security

  RuleItem& prevalRI =
      _prevalidationRuleController.categoryRuleItemSet().categoryRuleItem().ruleItem();
  prevalRI.clearHandlers();
  prevalRI.setHandler(RuleConst::SALE_RESTRICTIONS_RULE, &RuleItem::handleSalesSecurity);
  prevalRI.setHandler(RuleConst::ELIGIBILITY_RULE, &RuleItem::handleEligibility);

  _fareGroupSRValidationRuleController.categoryRuleItemSet()
      .categoryRuleItem()
      .ruleItem()
      .clearHandlers();

  if (fallback::fallbackFootNotePrevalidationForAltDates(&_trx))
    _footNotePrevalidationRuleController = std::unique_ptr<FootNoteRuleController>(new FootNoteRuleController());
  else
    _footNotePrevalidationRuleController = std::unique_ptr<FootNoteRuleController>(new FootNoteRuleController(trx.isAltDates()));

  // cat23 controller prevalidation

  std::vector<uint16_t> categories;
  categories.push_back(RuleConst::MISC_FARE_TAG);

  _cat23PrevalidationRuleController = std::unique_ptr<tse::FareMarketRuleController>(
      new RuleControllerWithChancelor<FareMarketRuleController>(
          NormalValidation, categories, &trx));

  RuleItem& cat23RI =
      _cat23PrevalidationRuleController->categoryRuleItemSet().categoryRuleItem().ruleItem();
  cat23RI.clearHandlers();
  cat23RI.setHandler(RuleConst::MISC_FARE_TAG, &RuleItem::handleMiscFareTagsForPubl);

  _allowedVend.clear();

  std::vector<PaxTypeBucket>& ptCorteges = _fareMarket.paxTypeCortege();
  _matchedNonJCB = false;
  for (const auto& paxTypeCortege : ptCorteges)
  {
    for (const auto paxType : paxTypeCortege.actualPaxType())
    {
      if (paxType->paxType() != JCB && paxType->paxType() != JNN && paxType->paxType() != JNF)
      {
        _matchedNonJCB = true;
        break;
      }
    }
    if (_matchedNonJCB)
      break;
  }
}

bool
FareController::resolveTariffCrossRef(Fare& fare) const
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "FC RESOLVE TXR");
#endif

  bool isInternational = isInternationalFare(fare);

  Diag204Collector* diag204 = Diag204Collector::getDiag204(_trx);

  if (_lastTariffCrossRefFare != nullptr &&
      _lastTariffCrossRefFare->fareTariff() == fare.fareTariff() &&
      _lastTariffCrossRefFare->vendor() == fare.vendor() &&
      _lastTariffCrossRefFare->carrier() == fare.carrier() &&
      isInternationalFare(*_lastTariffCrossRefFare) == isInternational)
  {
    fare.setTariffCrossRefInfo(_lastTariffCrossRefFare->tariffCrossRefInfo());
  }
  else
  {

    // retrieve a vector of TariffCrossRef candidates

    const std::vector<TariffCrossRefInfo*>& tcrList =
        _trx.dataHandle().getTariffXRefByFareTariff(fare.vendor(),
                                                    fare.carrier(),
                                                    (isInternational ? INTERNATIONAL : DOMESTIC),
                                                    fare.fareTariff(),
                                                    _travelDate);
    if (!tcrList.empty())
    {
      fare.setTariffCrossRefInfo(tcrList.front());
    }
  }

  if (fare.tariffCrossRefInfo() != nullptr)
  {
    if (isInternational)
    {
      // match global directions of fare and fare market

      const std::string* globalDirStr = globalDirectionToStr(fare.globalDirection());

      if (globalDirStr != nullptr &&
          _fareMarket.getGlobalDirection() != fare.tariffCrossRefInfo()->globalDirection())
      {
        // Fail this fare and write to Diag204
        if (UNLIKELY(diag204))
        {
          diag204->writeBadGlobalDirection(_trx, fare, _fareMarket);
          diag204->flushMsg();

          return false;
        }
      }
    }

    if (!resolveTariffInhibits(fare))
    {
      return false;
    }
  }

  const bool result = !fare.isTariffCrossRefMissing();
  if (result)
  {
    _lastTariffCrossRefFare = &fare;
  }

  return result;
}

bool
FareController::resolveTariffInhibits(const Fare& fare) const
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "FC RESOLVE TXRINB");
#endif

  const TariffCrossRefInfo& tariffCrossRef = *fare.tariffCrossRefInfo();
  // retrieve a vector of TariffCrossRef candidates
  const Indicator tariffInhibit = _trx.dataHandle().getTariffInhibit(
      tariffCrossRef.vendor(),
      (tariffCrossRef.crossRefType() == INTERNATIONAL ? 'I' : 'D'),
      tariffCrossRef.carrier(),
      tariffCrossRef.fareTariff(),
      tariffCrossRef.ruleTariffCode());

  switch (tariffInhibit)
  {
  case NOT_INHIBIT:
    return true;
  case INHIBIT_PRICING_ONLY:
  case INHIBIT_ALL:
    if (UNLIKELY(_trx.diagnostic().diagnosticType() == Diagnostic201))
    {
      DCFactory* const factory = DCFactory::instance();
      Diag201Collector* const diagPtr = dynamic_cast<Diag201Collector*>(factory->create(_trx));

      if (diagPtr != nullptr)
      {
        Diag201Collector& dc = *diagPtr;
        dc.enable(Diagnostic201);
        dc << _fareMarket << tariffCrossRef;
        dc << std::setw(10) << fare.fareClass();
        dc.setf(std::ios::right, std::ios::adjustfield);
        dc.setf(std::ios::fixed, std::ios::floatfield);
        dc.precision(2);
        dc << std::setw(9) << fare.fareAmount() << " ";
        dc.setf(std::ios::left, std::ios::adjustfield);
        dc << fare.currency() << " " << tariffInhibit << " \n";
        dc.flushMsg();
      }
    }
    return false;
  default:
    return true;
  }
}



bool
FareController::resolveFareTypeMatrix(PaxTypeFare& paxTypeFare, const FareClassAppInfo& fcaInfo)
    const

{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "FC RESOLVE FTM");
#endif
  // retrieve a vector of FareTypeMatrix candidates

  const FareTypeMatrix* ftm = _trx.dataHandle().getFareTypeMatrix(fcaInfo._fareType, _travelDate);

  if (LIKELY(ftm != nullptr))
  {

    paxTypeFare.cabin() = ftm->cabin();
    paxTypeFare.fareTypeDesignator() = ftm->fareTypeDesig();
    paxTypeFare.fareTypeApplication() = ftm->fareTypeAppl();
  }
  else
  {
    LOG4CXX_ERROR(_logger, "DataHandle.resolveFareTypeMatrix() cannot retrieve fare type");

    LOG4CXX_ERROR(_logger, "  faretype: " << fcaInfo._fareType);
  }

  return (ftm != nullptr);
}

bool
FareController::matchGlobalDirectionality(const Fare& fare, const FareMarket& fareMarket) const
{
  if (fareMarket.getGlobalDirection() != GlobalDirection::XX &&
      fareMarket.getGlobalDirection() != GlobalDirection::ZZ &&
      fare.globalDirection() != GlobalDirection::ZZ)
  {
    if (fareMarket.getGlobalDirection() != fare.globalDirection())
      return false;
  }
  return true;
}

bool
FareController::createPaxTypeFares(Fare* fare)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "FC CREATE PTFARES");
#endif

  if (_fareMarket.getGlobalDirection() != GlobalDirection::XX &&
      _fareMarket.getGlobalDirection() != GlobalDirection::ZZ &&
      fare->globalDirection() != GlobalDirection::ZZ)
  {
    if (_fareMarket.getGlobalDirection() != fare->globalDirection())
    {
      return true;
    }
  }

  std::vector<PaxTypeFare*> ptFares;

  if (resolveFareClassApp(*fare, ptFares) && !ptFares.empty())
  {
    validateAndAddPaxTypeFares(ptFares);
  }
  return true;
}

void
FareController::createPaxTypeFareESV(const Fare* fare, PaxTypeBucket* paxTypeCortege)
{
  if (_fareMarket.getGlobalDirection() != GlobalDirection::XX &&
      _fareMarket.getGlobalDirection() != GlobalDirection::ZZ &&
      fare->globalDirection() != GlobalDirection::ZZ)
  {
    if (_fareMarket.getGlobalDirection() != fare->globalDirection())
    {
      return;
    }
  }

  PaxTypeFare* ptFare = nullptr;

  if ((true == resolveFareClassAppESV(*fare, ptFare)) && (ptFare != nullptr))
  {
    // Check if fare object is not null
    if ((nullptr == ptFare->fare()) || (nullptr == ptFare->fare()->fareInfo()))
    {
      LOG4CXX_ERROR(_logger, "FareController::addPaxTypeFares - PaxTypeFare object is NULL or "
                             "contains NULL pointers.");
      return;
    }

    if ((ptFare->paxType() == nullptr) || (ptFare->fareClassAppInfo() == nullptr) ||
        (ptFare->isFareClassAppMissing()) || (ptFare->fareClassAppSegInfo() == nullptr) ||
        (ptFare->isFareClassAppSegMissing()))
    {
      return;
    }

    if (ptFare->fcasPaxType().empty() ||
        (ptFare->fcasPaxType() == (_trx.paxType()[0])->paxType()) ||
        (ptFare->fcasPaxType().equalToConst("JCB")))
    {
      paxTypeCortege->paxTypeFare().push_back(ptFare);
    }
  }
}

bool
FareController::validateAndAddPaxTypeFares(const std::vector<PaxTypeFare*>& ptFares)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "FC VALIDATE_AND_ADD PTFARES");
#endif

  for (const auto paxTypeFare : ptFares)
  {
    if (UNLIKELY(!paxTypeFare))
      continue;

    if (!_fareMarket.hasCat35Fare() &&
        (_trx.getTrxType() == PricingTrx::MIP_TRX || _trx.getTrxType() == PricingTrx::IS_TRX) &&
        (paxTypeFare->fcaDisplayCatType() == RuleConst::SELLING_FARE ||
         paxTypeFare->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
         paxTypeFare->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD))
    {
      _fareMarket.setHasCat35Fare(true);
    }

    _fareMarket.allPaxTypeFare().push_back(paxTypeFare);

    // pre-validate the paxTypeFare with the ruleController
    // for MIP entry:
    //   set up to do cat15 sales security only and cat1 rule.
    // for regular WP/W' entry:
    //   set up to do all cat15/cat1 rules.

    if (paxTypeFare->paxType() == nullptr || paxTypeFare->fareClassAppInfo() == nullptr ||
        paxTypeFare->isFareClassAppMissing() || paxTypeFare->fareClassAppSegInfo() == nullptr ||
        paxTypeFare->isFareClassAppSegMissing())
      continue;

    const bool needChkCat1R3Psg = (ptFares.size() == 1) && (paxTypeFare->fcasPaxType().empty() ||
                                                            (paxTypeFare->fcasPaxType() == ADULT));

    prevalidatePaxTypeFare(_trx, _itin, *paxTypeFare);

    // For JCB request, do not keep non-Cat 35 published fare with PTC blank or ADT
    // in R1B when Cat 1 rule does not exist.
    if (!isFdTrx() && _matchedNonJCB == false &&
        (paxTypeFare->fcasPaxType().empty() || paxTypeFare->fcasPaxType() == ADULT) &&
        !(paxTypeFare->fcaDisplayCatType() == RuleConst::SELLING_FARE ||
          paxTypeFare->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
          paxTypeFare->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD))
    {
      PaxType* paxType = nullptr;
      paxType =
          const_cast<PaxType*>(PaxTypeUtil::isAnActualPaxInTrx(_trx, paxTypeFare->carrier(), JCB));
      if (LIKELY(paxType != nullptr))
      {
        const PaxTypeFareRuleData* paxTypeFareRuleData =
            paxTypeFare->paxTypeFareRuleData(RuleConst::ELIGIBILITY_RULE);

        if (!paxTypeFareRuleData)
        {
          continue;
        }
      }
    }

    if (needChkCat1R3Psg)
      paxTypeFare->setNeedChkCat1R3Psg(false); // only check cat1 r3 once

    // validate cat23 rule for the published (non SITA) fares only.
    // check the construction indicator (byte#8)
    // to make a right decision before the published/constructed fare elimination.
    if (paxTypeFare->vendor() != Vendor::SITA && !paxTypeFare->isConstructed())
    {
      _cat23PrevalidationRuleController->validate(_trx, _itin, *paxTypeFare);
    }

    if (UNLIKELY(_trx.getOptions() != nullptr && _trx.getOptions()->isWeb() == false && paxTypeFare->isWebFare()))
      continue;

    if (LIKELY(!needChkCat1R3Psg || !paxTypeFare->foundCat1R3NoADT()))
    {
      addFareToPaxTypeBucket(*paxTypeFare);
    }
    else
    {
      addMatchedFareToPaxTypeBucket(*paxTypeFare);
    }
  }

  return true;
}

void
FareController::addMatchedFareToPaxTypeBucket(PaxTypeFare& paxTypeFare)
{
  // When we have actual psgType of record3 of cat1 not ADULT,
  // we need more PaxTypeFares with actual PaxType in paxTypeCortege
  std::vector<PaxType*> candidatePaxTypes;
  findCandidatePaxTypes(candidatePaxTypes, paxTypeFare.foundCat1R3ChkAge());

  // candidatePaxTypes at least includes ADULT
  // create different fares to different PaxType bucket
  std::vector<PaxTypeFare*> candidatePTFares;

  for (const auto candidatePaxType : candidatePaxTypes)
  {
    PaxTypeFare* candidatePTFare = clonePTFare(paxTypeFare, *candidatePaxType);
    if (!candidatePTFare) // out of memory
      break;

    candidatePTFare->setFoundCat1R3NoADT(false); // reset matching status
    candidatePTFare->setNotFBRBaseFare(true);
    candidatePTFares.push_back(candidatePTFare);

    prevalidatePaxTypeFare(_trx, _itin, *candidatePTFare);

    if (_trx.getRequest()->fareGroupRequested() && _fareMarket.validateFareGroup())
    {
      validateFareGroup(*candidatePTFare);
    }
  }

  // Put in FareMarket allPaxTypeFare if it is not SKIP
  for (const auto candidatePTFare : candidatePTFares)
  {
    if (candidatePTFare->foundCat1R3NoADT()) // this PaxType found matched R3

    {
      _fareMarket.allPaxTypeFare().push_back(candidatePTFare);
    }
  }

  if (_trx.getRequest()->fareGroupRequested() && _fareMarket.validateFareGroup())
  {
    validateFareGroup(paxTypeFare);
  }

  // original one for ADULT will be last in candidates
  candidatePTFares.push_back(&paxTypeFare);

  // put PaxTypeFare into cortege(s) that fit

  std::vector<PaxTypeBucket>& ptCorteges = _fareMarket.paxTypeCortege();
  bool usedAdultFare = false;

  for (PaxTypeBucket& paxTypeCortege : ptCorteges)
  {
    bool matchedNoAdult = false;

    for (PaxTypeFare* candidatePTFare : candidatePTFares)
    {
      const PaxTypeCode& candidatePaxType = candidatePTFare->fcasPaxType();
      if (candidatePaxType.empty() || candidatePaxType == ADULT)
      {

        if (!matchedNoAdult)
        {
          if (isFdTrx() &&
              candidatePTFare->actualPaxType()->paxType() !=
                  paxTypeCortege.actualPaxType().front()->paxType())
          {
            continue;
          }

          paxTypeCortege.paxTypeFare().push_back(candidatePTFare);
          usedAdultFare = true;
        }
        break; // ADULT is last in the candidatePTFares list, not
        // really necessary break, just for clear code reading
      }
      else
      {
        if (!candidatePTFare->foundCat1R3NoADT()) // never matched R3 psg
          continue;

        if (PaxTypeUtil::findMatch(paxTypeCortege.actualPaxType(), candidatePaxType))
        {
          if (isFdTrx() &&
              candidatePTFare->actualPaxType()->paxType() !=
                  paxTypeCortege.actualPaxType().front()->paxType())
          {
            continue;
          }
          paxTypeCortege.paxTypeFare().push_back(candidatePTFare);
          matchedNoAdult = true;
        }
      }
    }
  }

  if (!usedAdultFare)
  {
    // althought not in any ptCortege, keep it in fareMarket for cat25 but
    // set it as invalid
    paxTypeFare.setCategoryValid(RuleConst::ELIGIBILITY_RULE, false);
  }
}

//-------------------------------------------------------------------
// matchCurrency()
//   target - on input, desired currency; on output, converted amount

//   src1, src2 - pick the one w/ matching currency
//                if no match, convert to NUC, pick smallest non-zero
//                and convert to taget currency
//   return 0 : could not convert
//   return 1 : used src1
//   return 2 : used src2
//-------------------------------------------------------------------
char
FareController::matchCurrency(Money& target,
                              const bool isIntl,
                              const Money& src1,
                              const Money& src2)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "FC MATCH CURRENCY");
#endif

  try
  {
    // easy case when currency matches or nothing to convert
    if (target.code() == src1.code())
    {
      target.value() = src1.value();
      return 1;
    }

    if (target.code() == src2.code())
    {
      target.value() = src2.value();
      return 2;
    }

    if (src1.value() == 0.0)
    {
      target.value() = 0;
      return 1;
    }

    // src1 => _nuc;   src2 => nuc2

    _ccFacade.convert(_nuc, src1, _trx, isIntl);

    // see if src2 needed
    if (src2.value() != 0.0 && src1.code() != src2.code())
    {
      Money nuc2(0, NUC);
      _ccFacade.convert(nuc2, src2, _trx, isIntl);
      if (nuc2.value() < _nuc.value())

      {
        _ccFacade.convert(target, nuc2, _trx, isIntl);
        return 2;
      }
    }
    // if here, src1<src2 || src2 invalid, so use src1
    _ccFacade.convert(target, _nuc, _trx, isIntl);
  }
  catch (...) { return 0; }
  return 1;
}
//-------------------------------------------------------------------
// roundCurrency()
//-------------------------------------------------------------------

void
FareController::roundCurrency(MoneyAmount& amt, const CurrencyCode& cur, const bool isIntl)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "FC ROUND CURRENCY");
#endif

  Money temp(amt, cur);
  if (_ccFacade.round(temp, _trx, isIntl))

    amt = temp.value();
}

//-------------------------------------------------------------------
// sortPaxTypeFares()
//-------------------------------------------------------------------

bool
FareController::sortPaxTypeFares()
{
  // Sort the fares in paxTypeFare vector for each PaxTypeCortege

  const bool isCmdPricing = !fallback::compDiffCmdPricing(&_trx)
                            && _fareMarket.fbcUsage() == COMMAND_PRICE_FBC
                            && _fareMarket.serviceStatus().isSet(FareMarket::FareCollector);

  std::vector<PaxTypeBucket>::iterator ptc = _fareMarket.paxTypeCortege().begin();

  uint16_t paxTypeNum = 0;

  for (; ptc != _fareMarket.paxTypeCortege().end(); ptc++, paxTypeNum++)
  {
    PaxTypeFare::FareComparator fareComparator(
        *ptc, paxTypeNum, _trx.getOptions()->isZeroFareLogic(), isCmdPricing);
    std::sort(ptc->paxTypeFare().begin(), ptc->paxTypeFare().end(), fareComparator);
  }

  // Sort the fares in allPaxTypeFare vector

  sort(_fareMarket.allPaxTypeFare().begin(),
       _fareMarket.allPaxTypeFare().end(),
       PaxTypeFare::fareSort);

  return true;
}

void
FareController::prevalidatePaxTypeFare(PricingTrx& trx, Itin& itin, PaxTypeFare& ptf)
{
  if (!trx.getRequest()->isSFR())
    _prevalidationRuleController.validate(trx, itin, ptf);
}

//-------------------------------------------------------------------
// findFares()
//-------------------------------------------------------------------
void
FareController::findFares(std::vector<Fare*>& fares)
{
  findFares(_fareMarket.governingCarrier(), fares);
}

bool
FareController::findFaresESV(std::vector<Fare*>& fares)
{
  const CarrierCode& carrier = _fareMarket.governingCarrier();

  const LocCode& origin = _fareMarket.origin()->loc();
  const LocCode& destination = _fareMarket.destination()->loc();

  const LocCode& boardMultiCity = _fareMarket.boardMultiCity();
  const LocCode& offMultiCity = _fareMarket.offMultiCity();

  bool diffBoardPoint = ((!boardMultiCity.empty()) && (boardMultiCity != origin));
  bool diffOffPoint = ((!offMultiCity.empty()) && (offMultiCity != destination));

  if (!(_trx.getCollectBoundFares()))
  {
    LOG4CXX_ERROR(_logger,
                  "FareController::findFaresESV() - Bound fares turn off in configuration.");
    return false;
  }

  const std::vector<const FareInfo*>* origDestVec = nullptr;
  const std::vector<const FareInfo*>* boardDestVec = nullptr;
  const std::vector<const FareInfo*>* origOffVec = nullptr;
  const std::vector<const FareInfo*>* boardOffVec = nullptr;

  origDestVec =
      &(_trx.dataHandle().getBoundFaresByMarketCxr(origin, destination, carrier, _travelDate));

  if (diffBoardPoint)
  {
    boardDestVec = &(_trx.dataHandle().getBoundFaresByMarketCxr(
        boardMultiCity, destination, carrier, _travelDate));
  }

  if (diffOffPoint)
  {
    origOffVec =
        &(_trx.dataHandle().getBoundFaresByMarketCxr(origin, offMultiCity, carrier, _travelDate));
  }

  if (diffBoardPoint && diffOffPoint)
  {
    boardOffVec = &(_trx.dataHandle().getBoundFaresByMarketCxr(
        boardMultiCity, offMultiCity, carrier, _travelDate));
  }

  uint32_t vectorSize = 0;

  if (origDestVec)
  {
    vectorSize += origDestVec->size();
  }

  if (boardDestVec)
  {
    vectorSize += boardDestVec->size();
  }

  if (origOffVec)
  {
    vectorSize += origOffVec->size();
  }

  if (boardOffVec)
  {
    vectorSize += boardOffVec->size();
  }

  fares.reserve(vectorSize + 1);

  if (origDestVec && (!origDestVec->empty()))
  {
    addFaresToVectorESV(*origDestVec, origin, fares);
  }

  if (boardDestVec && !boardDestVec->empty())
  {
    addFaresToVectorESV(*boardDestVec, boardMultiCity, fares);
  }

  if (origOffVec && !origOffVec->empty())
  {
    addFaresToVectorESV(*origOffVec, origin, fares);
  }

  if (boardOffVec && !boardOffVec->empty())
  {
    addFaresToVectorESV(*boardOffVec, boardMultiCity, fares);
  }

  return true;
}

bool
FareController::addFaresToVectorESV(const std::vector<const FareInfo*>& fareInfoVec,
                                    const LocCode& origin,
                                    std::vector<Fare*>& fares)
{
  for (const FareInfo* fareInfo : fareInfoVec)
  {
    Fare* fare = createFare(fareInfo, origin, Fare::FS_PublishedFare);

    if (fare)
      fares.push_back(fare);
  }

  return true;
}

void
FareController::findFares(const CarrierCode& carrier, std::vector<Fare*>& fares)
{
  const LocCode& origin = _fareMarket.origin()->loc();
  const LocCode& destination = _fareMarket.destination()->loc();

  const LocCode& boardMultiCity = _fareMarket.boardMultiCity();
  const LocCode& offMultiCity = _fareMarket.offMultiCity();

  bool diffBoardPoint = (!boardMultiCity.empty()) && (boardMultiCity != origin);

  bool diffOffPoint = (!offMultiCity.empty()) && (offMultiCity != destination);

  try
  {
    findFares(carrier, origin, destination, fares);
    if (diffBoardPoint)
      findFares(carrier, boardMultiCity, destination, fares);
    if (diffOffPoint)
      findFares(carrier, origin, offMultiCity, fares);
    if (diffBoardPoint && diffOffPoint)
      findFares(carrier, boardMultiCity, offMultiCity, fares);
  }
  catch (ErrorResponseException& ex) { throw; }
}
//-------------------------------------------------------------------
// findFares()
//-------------------------------------------------------------------
void
FareController::findFares(const CarrierCode& carrier,
                          const LocCode& origin,
                          const LocCode& destination,
                          std::vector<Fare*>& fares)
{
  const std::vector<const FareInfo*>& allPublishedFares =
      findCityPairFares(origin, destination, carrier);

  if (fallback::reworkTrxAborter(&_trx))
    checkTrxAborted(_trx);
  else
    _trx.checkTrxAborted();

  // loop through all retrieved fareInfos and create Fare objects for each of them

  std::vector<const FareInfo*> filteredPublishedFares;

  const bool shouldFilterSMFOFares =
      fallback::createSMFOFaresForALlUsers(&_trx) ||
      (!fallback::dffOaFareCreation(&_trx) && !TrxUtil::isRequestFromAS(_trx));

  const std::vector<const FareInfo*>& publishedFares =
      shouldFilterSMFOFares
          ? filterByVendorForCat31AndDFF(allPublishedFares, filteredPublishedFares)
          : filterByVendorForCat31(allPublishedFares, filteredPublishedFares);

  bool sfaTag = false;
  std::vector<FareClassCode> fareClasses;
  sfaTag = sfaDiagRequested(_trx, fareClasses);
  const bool skipPCCVendor(_trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty()
                           && _trx.excTrxType() == PricingTrx::NOT_EXC_TRX);
  if (sfaTag)
  {
    for (const FareInfo* fareInfo : publishedFares)
    {
      uint16_t fareNum = 0;
      for (; fareNum != fareClasses.size(); ++fareNum)
      {
        if (fareInfo->fareClass() == fareClasses[fareNum])
          break;
      }
      if (fareNum >= fareClasses.size())
        continue;
      if (fareInfo->fareClass() != fareClasses[fareNum])
        continue;

      if(skipPCCVendor)
      {
         if(RuleUtil::isVendorPCC(fareInfo->vendor(), _trx))
           continue;
      }
      setOriginalFareAmountForRW(fareInfo);

      Fare* fare = createFare(fareInfo, origin, Fare::FS_PublishedFare);

      if (fare != nullptr)
        fares.push_back(fare);
    }
  }
  else
  {
    for (const FareInfo* fareInfo : publishedFares)
    {
      if(skipPCCVendor)
      {
         if(RuleUtil::isVendorPCC(fareInfo->vendor(), _trx))
           continue;
      }
      setOriginalFareAmountForRW(fareInfo);

      Fare* fare = createFare(fareInfo, origin, Fare::FS_PublishedFare);

      if (fare)
        fares.push_back(fare);
    }
  }
}

void
FareController::setOriginalFareAmountForRW(const FareInfo* fareInfo)
{
  if (UNLIKELY(_trx.getOptions()->isRtw()))
    const_cast<FareInfo*>(fareInfo)->resetToOriginalAmount();
}

const std::vector<const FareInfo*>&
FareController::findCityPairFares(const LocCode& origin,
                                  const LocCode& destination,
                                  const CarrierCode& carrier)
{
  // retrieve published fareInfo records for governing carrier
  const ShoppingTrx* shoppingTrx = dynamic_cast<ShoppingTrx*>(&_trx);
  if ((shoppingTrx != nullptr) && (shoppingTrx->isAltDates()))
  {
    DatePair dateRange = ShoppingAltDateUtil::getTravelDateRange(*shoppingTrx);
    const std::vector<const FareInfo*>& publishedFares = _trx.dataHandle().getFaresByMarketCxr(
        origin, destination, carrier, dateRange.first, dateRange.second);
    return publishedFares;
  }
  else if ((_trx.getTrxType() == PricingTrx::MIP_TRX) && (_trx.isAltDates()))
  {
    DatePair dateRange = ShoppingAltDateUtil::getMIPTravelDateRange(_trx);
    const std::vector<const FareInfo*>& publishedFares = _trx.dataHandle().getFaresByMarketCxr(
        origin, destination, carrier, dateRange.first, dateRange.second);
    return publishedFares;
  }
  else if (UNLIKELY(_trx.getCollectBoundFares()))
  {
    return _trx.dataHandle().getBoundFaresByMarketCxr(origin, destination, carrier, _travelDate);
  }
  else
  {
    const std::vector<const FareInfo*>& publishedFares =
        _trx.dataHandle().getFaresByMarketCxr(origin, destination, carrier, _travelDate);
    return publishedFares;
  }
}

Fare*
FareController::createFare(const FareInfo* fi,
                           const LocCode& origin,
                           const Fare::FareState initialStatus,
                           const ConstructedFareInfo* cfi)
{
  Fare* fare = nullptr;

  // first to see if fare is reversed to fareMarket

  bool reversedFare = (origin != fi->market1()) && (_fareMarket.boardMultiCity() != fi->market1());

  if (!_fareMarket.hasDuplicates())
  {
    if (FareUtil::postCheckOutboundFare(_fareMarket.removeOutboundFares(),
                                       reversedFare,
                                       fi->directionality()))
    {
      return nullptr;
    }
  }

  // For any US/CA Domestic (market1 to market2):
  // If a fare has footnote F, it is only good for the faremarket traveling from market1 to market2;
  // If a fare has footnote T, it is only good for the faremarket traveling from market2 to market1.

  Indicator domesticFootNote = BLANK;
  if (!isFdTrx() && _fareMarket.geoTravelType() == GeoTravelType::Domestic && fi->directionality() != BOTH &&
      fi->owrt() == ONE_WAY_MAY_BE_DOUBLED)
  {
    if ((fi->directionality() == FROM && reversedFare) ||
        (fi->directionality() == TO && !reversedFare))
    {
      return nullptr;
    }

    if (fi->directionality() == FROM)
    {
      domesticFootNote = 'F';
    }
    else

    {
      domesticFootNote = 'T';
    }

    FareInfo* fareInfo;
    fareInfo = fi->clone(_trx.dataHandle());
    fareInfo->directionality() = BOTH;
    fi = fareInfo;
  }

  // For simple trip, get rid of fare with wrong directionality
  if (boost::indeterminate(_matchFaresDirectionality))
    initMatchFaresDirectionality();

  if (_matchFaresDirectionality && fi->directionality() != BOTH)
  {
    Directionality fareDirection = fi->directionality();

    if (reversedFare)
      fareDirection = (fareDirection == TO) ? FROM : TO;

    if (_fareMarket.direction() == FMDirection::OUTBOUND && fareDirection == TO)
      return nullptr;

    if (fallback::fallbackSimpleTripCorrectionOWFare(&_trx))
    {
      // Allow outbound fares for inbound FareMarkets for Carnival SOL processing
      if (_fareMarket.direction() == FMDirection::INBOUND && fareDirection == FROM &&
        !(_trx.getTrxType() == PricingTrx::IS_TRX && _trx.getOptions()->isCarnivalSumOfLocal() &&
          (_trx.getOptions()->getAdditionalItinsRequestedForOWFares() > 0)))
      return fare;
    }
    else
    {
     // Allow outbound fares for inbound FareMarkets for Carnival SOL processing, or
     // simple trip where OW + OW is allowed
     if (_fareMarket.direction() == FMDirection::INBOUND && fareDirection == FROM)
     {
       if ( !(_trx.getTrxType() == PricingTrx::IS_TRX && _trx.getOptions()->isCarnivalSumOfLocal() && (_trx.getOptions()->getAdditionalItinsRequestedForOWFares() > 0))
              &&
            !(_itin.simpleTrip() && (fi->owrt() == ONE_WAY_MAY_BE_DOUBLED || fi->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)) )
         return nullptr;
     }
    }
  }

  // allocate memory for fare

  _trx.dataHandle().get(fare);

  // initialization
  // lint --e{413}
  fare->initialize(initialStatus, fi, _fareMarket, nullptr, cfi);

  if (reversedFare)
    fare->status().set(Fare::FS_ReversedDirection);

  if (UNLIKELY(_checkItinCalculationCurrency))
    fare->setCalcCurrForDomItinValid(_itin.calculationCurrency() == fare->currency());

  if (UNLIKELY(domesticFootNote != BLANK))
    fare->domesticFootNote() = domesticFootNote;

  // Carnival SOL outbound fares for inbound
  if (UNLIKELY(_trx.getTrxType() == PricingTrx::IS_TRX && _trx.getOptions()->isCarnivalSumOfLocal() &&
      (_trx.getOptions()->getAdditionalItinsRequestedForOWFares() > 0) &&
      (_fareMarket.direction() == FMDirection::INBOUND && fare->directionality() == FROM)))
  {
    fare->setOutboundFareForCarnivalInbound(true);
  }

  // find matching TariffCrossRef record

  if (LIKELY(fare != nullptr))
    if (!resolveTariffCrossRef(*fare))
      fare = nullptr;

  // free RAM if fare has not being created

  return fare;
}

bool
FareController::addFareToPaxTypeBucket(PaxTypeFare& ptFare)
{
  bool xoFares = false;

  // These get created for Cat25, they only need to be in allpaxtypefare
  if (UNLIKELY(ptFare.fareClassAppInfo() == nullptr || ptFare.isFareClassAppMissing() ||
      ptFare.fareClassAppSegInfo() == nullptr || ptFare.isFareClassAppSegMissing()))
    return true;

  if (UNLIKELY(_trx.getRequest()->fareGroupRequested() && _fareMarket.validateFareGroup()))
  {
    validateFareGroup(ptFare);
  }

  /// @todo Should we skip invalid fares?

  std::vector<PaxTypeBucket>& ptCorteges = _fareMarket.paxTypeCortege();
  bool isCmdPriced = isCmdPricedFM();

  for (PaxTypeBucket& ptCortege : ptCorteges)
  {
    if (UNLIKELY(!checkAge(ptFare, *ptCortege.requestedPaxType())))
      continue; // command pricing would not override age restriction

    if (find_if(ptCortege.actualPaxType().begin(),
                ptCortege.actualPaxType().end(),
                PaxTypeFinder(ptFare, xoFares, isCmdPriced)) != ptCortege.actualPaxType().end())
    {
      if (UNLIKELY(isFdTrx() &&
          ptFare.actualPaxType()->paxType() != ptCortege.actualPaxType().front()->paxType()))
      {
        continue;
      }

      ptCortege.paxTypeFare().push_back(&ptFare);

      //---------------------------------------------------
      // NEG Fare Check
      // Set the FareClassAppSeg to empty for NEG Fares
      // Unless..... NEG was requested OR it is a WEB Fare.
      //---------------------------------------------------
      if (ptFare.fcasPaxType() == NEG && !ptFare.isWebFare())
        if (UNLIKELY(!ptCortege.isPaxTypeInActualPaxType(NEG)))
          ptFare.fareClassAppSegInfo() = FareClassAppSegInfo::emptyFareClassAppSeg();
    }
  }

  return true;
}

bool
FareController::validateQualifiers(PaxTypeFare& ptFare,
                                   std::vector<CategoryRuleItemInfo>& seqQualifiers,
                                   const VendorCode& vendor,
                                   std::vector<CarrierCode>& inputValCxrList,
                                   std::vector<CarrierCode>& passedValCxrList,
                                   const CategoryRuleInfo* ruleInfo,
                                   DiagCollector* diagCollector,
                                   bool* const softPassFlag)
{
  if (UNLIKELY(seqQualifiers.empty()))
  {
    passedValCxrList = inputValCxrList;
    return true;
  }

  bool isCat15Here = isCat15Qualified(seqQualifiers);

  if(isCat15Here && !inputValCxrList.empty())
  {
    for (const CarrierCode cxr : inputValCxrList)
    {
      if(!validateQualifiers(
              ptFare, seqQualifiers, vendor, cxr, ruleInfo, diagCollector, softPassFlag))
        continue;
      passedValCxrList.push_back(cxr);
    }
    return !passedValCxrList.empty();
  }
  else
  {
    passedValCxrList = inputValCxrList;
    return validateQualifiers(
              ptFare, seqQualifiers, vendor, "", ruleInfo, diagCollector, softPassFlag);
  }
  return true;
}

bool
FareController::isCat15Qualified(std::vector<CategoryRuleItemInfo>& seqQualifiers)
{
  return std::any_of(seqQualifiers.begin(),
                     seqQualifiers.end(),
                     [](const CategoryRuleItemInfo& crii)
                     { return crii.itemcat() == RuleConst::SALE_RESTRICTIONS_RULE; });
}

bool
FareController::validateQualifiers(PaxTypeFare& ptFare,
                                   std::vector<CategoryRuleItemInfo>& seqQualifiers,
                                   const VendorCode& vendor,
                                   const CarrierCode& cxr,
                                   const CategoryRuleInfo* ruleInfo,
                                   DiagCollector* diagCollector,
                                   bool* const softPassFlag)
{
  Record3ReturnTypes lastSetValid = NOTPROCESSED;

  Diagnostic& trxDiag = _trx.diagnostic();

  for (const auto& catRuleItemInfo : seqQualifiers)
  {
    switch (catRuleItemInfo.relationalInd())
    {
    case CategoryRuleItemInfo::IF:
      // If there is a qualified C26/C27/C28 for C19/C25/C35, fail it for the pricing.
      if (UNLIKELY(!isFdTrx() && (catRuleItemInfo.itemcat() == 26 || catRuleItemInfo.itemcat() == 27 ||
                         catRuleItemInfo.itemcat() == 28)))
        return false;

      lastSetValid = validateQualifier(ptFare, catRuleItemInfo, vendor, cxr, ruleInfo);
      break;

    case CategoryRuleItemInfo::AND:
      // If the lastSetValid fail, there is no used trying to
      // test the AND condition
      if (lastSetValid == FAIL)
        return false;

      if (lastSetValid == SOFTPASS)
      {
        if (FAIL == validateQualifier(ptFare, catRuleItemInfo, vendor, cxr, ruleInfo))
          lastSetValid = FAIL; // otherwise keep the SOFTPASS result
      }
      else
        lastSetValid = validateQualifier(ptFare, catRuleItemInfo, vendor, cxr, ruleInfo);

      break;

    case CategoryRuleItemInfo::OR:
      // If this set already passed we can skip the rest of
      // the or conditions
      if (lastSetValid != PASS && lastSetValid != SOFTPASS)
      {
        lastSetValid = validateQualifier(ptFare, catRuleItemInfo, vendor, cxr, ruleInfo);
      }
      break;

    default:
      break;
    }

    if (UNLIKELY((trxDiag.diagnosticType() == Diagnostic225 &&
                  ruleInfo->categoryNumber() == RuleConst::FARE_BY_RULE) ||
                 (trxDiag.diagnosticType() == Diagnostic335 &&
                  ruleInfo->categoryNumber() == RuleConst::NEGOTIATED_RULE)))
    {
      if (diagCollector)
      {
        diagCollector->displayRelation(&catRuleItemInfo, lastSetValid);
      }
      else
      {
        DiagManager diag(_trx, trxDiag.diagnosticType());
        diag.collector().displayRelation(&catRuleItemInfo, lastSetValid);
      }
    }
  }

  if (lastSetValid == FAIL)
  {
    return false;
  }

  if (softPassFlag != nullptr)
  {
    if (lastSetValid == SOFTPASS)
      *softPassFlag = true;
    else
      *softPassFlag = false;
  }

  return true;
}

bool
FareController::validateQualifiers(PaxTypeFare& ptFare,
                                   std::vector<CategoryRuleItemInfo>& seqQualifiers,
                                   const VendorCode& vendor,
                                   const CategoryRuleInfo* ruleInfo,
                                   DiagCollector* diagCollector,
                                   bool* const softPassFlag)
{
  if (UNLIKELY(seqQualifiers.empty()))
    return true;

  Record3ReturnTypes lastSetValid = NOTPROCESSED;

  Diagnostic& trxDiag = _trx.diagnostic();

  for (const CategoryRuleItemInfo& catRuleItemInfo : seqQualifiers)
  {
    switch (catRuleItemInfo.relationalInd())
    {
    case CategoryRuleItemInfo::IF:
      // If there is a qualified C26/C27/C28 for C19/C25/C35, fail it for the pricing.
      if (UNLIKELY(!isFdTrx() && (catRuleItemInfo.itemcat() == 26 || catRuleItemInfo.itemcat() == 27 ||
                                  catRuleItemInfo.itemcat() == 28)))
        return false;

      lastSetValid = validateQualifier(ptFare, catRuleItemInfo, vendor, ruleInfo);
      break;

    case CategoryRuleItemInfo::AND:
      // If the last set failed, there is no used trying to
      // test the and condition
      if (lastSetValid == FAIL)
        return false;

      if (lastSetValid == SOFTPASS)
      {
        if (FAIL == validateQualifier(ptFare, catRuleItemInfo, vendor, ruleInfo))
          lastSetValid = FAIL; // otherwise keep the SOFTPASS result
      }
      else
        lastSetValid = validateQualifier(ptFare, catRuleItemInfo, vendor, ruleInfo);

      break;

    case CategoryRuleItemInfo::OR:
      // If this set already passed we can skip the rest of
      // the or conditions
      if (lastSetValid != PASS && lastSetValid != SOFTPASS)
      {
        lastSetValid = validateQualifier(ptFare, catRuleItemInfo, vendor, ruleInfo);
      }
      break;

    default:
      break;
    }

    if (UNLIKELY((trxDiag.diagnosticType() == Diagnostic225 &&
                  ruleInfo->categoryNumber() == RuleConst::FARE_BY_RULE) ||
                 (trxDiag.diagnosticType() == Diagnostic335 &&
                  ruleInfo->categoryNumber() == RuleConst::NEGOTIATED_RULE)))
    {
      if (diagCollector)
      {
        diagCollector->displayRelation(&catRuleItemInfo, lastSetValid);
      }
      else
      {
        DiagManager diag(_trx, trxDiag.diagnosticType());
        diag.collector().displayRelation(&catRuleItemInfo, lastSetValid);
      }
    }
  }

  if (lastSetValid == FAIL)
  {
    return false;
  }

  if (UNLIKELY(softPassFlag != nullptr))
  {
    if (lastSetValid == SOFTPASS)
      *softPassFlag = true;
    else
      *softPassFlag = false;
  }

  return true;
}


void
FareController::printEligibilityInfo(DiagCollector& diag,
                                     const PaxTypeFare& ptFare,
                                     const EligibilityInfo& eligibilityInfo,
                                     int categoryNumber) const
{
  diag << "FARE PAX TYPE : " << ptFare.fcasPaxType() << " MIN AGE : " << ptFare.fcasMinAge()
       << " MAX AGE : " << ptFare.fcasMaxAge() << std::endl
       << "R3 PAX TYPE   : " << eligibilityInfo.psgType()
       << " MIN AGE : " << eligibilityInfo.minAge() << " MAX AGE : " << eligibilityInfo.maxAge()
       << std::endl << "R3 ACCOUNT CODE/CORP ID : " << eligibilityInfo.acctCode() << std::endl
       << "QUALIFIED CAT1 TO CAT" << categoryNumber << std::endl;
}

Record3ReturnTypes
FareController::checkAccountCode(PaxTypeFare& ptFare,
                                 const EligibilityInfo& eligibilityInfo,
                                 Eligibility& eligibilityApp,
                                 int categoryNumber) const
{
  bool diagEnabled = _trx.diagnostic().diagnosticType() == Diagnostic301;
  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;

  if (UNLIKELY(diagEnabled))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(_trx);
    diagPtr->enable(Diagnostic301);
    printEligibilityInfo(*diagPtr, ptFare, eligibilityInfo, categoryNumber);
  }

  return eligibilityApp.checkAccountCode(
      &eligibilityInfo, ptFare, _trx, factory, false, diagPtr, diagEnabled);
}

Record3ReturnTypes
FareController::setIncompleteR3Rule(PaxTypeFare& ptFare, uint32_t itemcat) const
{
  if (ptFare.fareDisplayInfo())
  {
    // Update FareDisplayInfo object: Missing/Incomplete rule data
    ptFare.fareDisplayInfo()->setIncompleteR3Rule(itemcat);

    return NOTPROCESSED;
  }
  return FAIL;
}

Record3ReturnTypes
FareController::validateEligibilityQualifier(PaxTypeFare& ptFare,
                                             uint32_t itemcat,
                                             const CategoryRuleInfo* ruleInfo,
                                             const EligibilityInfo* eligibilityInfo,
                                             Eligibility& eligibilityApp)
{
  if (UNLIKELY(eligibilityInfo == nullptr))
  {
    if (isFdTrx())
      return setIncompleteR3Rule(ptFare, itemcat);

    return FAIL;
  }

  if (UNLIKELY((_trx.getTrxType() == PricingTrx::MIP_TRX) && (_trx.isFlexFare())))
  {
    std::shared_ptr<RuleValidationChancelor> chancelor(new RuleValidationChancelor());
    std::shared_ptr<CategoryValidationObserver> resultObserver;
    FareMarketDataAccess fareMarketDataAccess(_trx, &_itin, ptFare);

    chancelor->updateContextType(RuleValidationContext::FARE_MARKET);
    chancelor->updateContext(fareMarketDataAccess);
    chancelor->setPolicy(ELIGIBILITY_RULE, new FlexFaresValidationPolicyNoEligibility());

    resultObserver = std::shared_ptr<CategoryValidationObserver>(
        new CategoryValidationObserver(chancelor->getMutableMonitor()));

    eligibilityApp.setChancelor(chancelor);
  }

  if (ruleInfo->categoryNumber() == RuleConst::CHILDREN_DISCOUNT_RULE ||
      ruleInfo->categoryNumber() == RuleConst::TOUR_DISCOUNT_RULE ||
      ruleInfo->categoryNumber() == RuleConst::AGENTS_DISCOUNT_RULE ||
      ruleInfo->categoryNumber() == RuleConst::OTHER_DISCOUNT_RULE)
  {
    return checkAccountCode(ptFare, *eligibilityInfo, eligibilityApp, ruleInfo->categoryNumber());
  }

  if ((ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE ||
       ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
       ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD) &&
      (ptFare.fcasPaxType().empty() || ptFare.fcasPaxType() == ADULT))
  {

    if (LIKELY(ruleInfo->categoryNumber() == RuleConst::NEGOTIATED_RULE))
    {
      return checkAccountCode(ptFare, *eligibilityInfo, eligibilityApp, 35);
    }

    return NOTPROCESSED;
  }

  if (LIKELY(!isFdTrx()))
  {
      return eligibilityApp.validateFromFCO(
          _trx, eligibilityInfo, _itin, ptFare, ruleInfo, _fareMarket);
  }

  GlobalDirectionStorage fmGlobal(ptFare);

  FDEligibility fdEligibilityApp;
  return fdEligibilityApp.validate(_trx, _itin, ptFare, eligibilityInfo, _fareMarket, true, false);
}

Record3ReturnTypes
FareController::validateDayTimeQualifier(PaxTypeFare& ptFare,
                                         uint32_t itemcat,
                                         const CategoryRuleInfo* ruleInfo,
                                         const DayTimeAppInfo* dayTimeInfo,
                                         DayTimeApplication& dayTimeApp)
{
  if (UNLIKELY(isFdTrx()))
  {
    if (dayTimeInfo == nullptr)
      return setIncompleteR3Rule(ptFare, itemcat);

    GlobalDirectionStorage fmGlobal(ptFare);

    FDDayTimeApplication fdDayTimeApp;

    return fdDayTimeApp.validate(*_fdTrx, _itin, ptFare, dayTimeInfo, _fareMarket, true, false);
  }

  if (UNLIKELY(dayTimeInfo == nullptr))
    return FAIL;

  dayTimeApp.initialize(dayTimeInfo);

  return dayTimeApp.validate(_trx, _itin, ptFare, dayTimeInfo, _fareMarket);
}

Record3ReturnTypes
FareController::validateSeasonalQualifier(PaxTypeFare& ptFare,
                                          const CategoryRuleItemInfo& ruleItem,
                                          const CategoryRuleInfo* ruleInfo,
                                          const SeasonalAppl* seasonalInfo,
                                          SeasonalApplication& seasonalApp)
{
  if (UNLIKELY(isFdTrx()))
  {
    if (seasonalInfo == nullptr)
      return setIncompleteR3Rule(ptFare, ruleItem.itemcat());

    GlobalDirectionStorage fmGlobal(ptFare);

    FDSeasonalApplication fdSeasonalApp(&ruleItem);
    return fdSeasonalApp.validate(_trx, _itin, ptFare, seasonalInfo, _fareMarket, true, false);
  }

  if (UNLIKELY(seasonalInfo == nullptr))
    return FAIL;

  return seasonalApp.validate(_trx, _itin, ptFare, seasonalInfo, _fareMarket);
}

void
FareController::setQualifyFltAppRuleData(PaxTypeFare& ptFare, const CategoryRuleInfo* ruleInfo)
    const
{
  QualifyFltAppRuleData* data = _trx.dataHandle().create<QualifyFltAppRuleData>();
  data->categoryRuleInfo() = ruleInfo;

  ptFare.qualifyFltAppRuleDataMap()[ruleInfo->categoryNumber()] = data;
  ptFare.setCategoryProcessed(ruleInfo->categoryNumber());
}

Record3ReturnTypes
FareController::validateFlightApplicationQualifier(PaxTypeFare& ptFare,
                                                   uint32_t itemcat,
                                                   const VendorCode& vendor,
                                                   const CategoryRuleInfo* ruleInfo,
                                                   const FlightAppRule* flightInfo,
                                                   FlightApplication& flightApp)
{
  if (UNLIKELY(isFdTrx()))
  {
    if (flightInfo == nullptr)
      return setIncompleteR3Rule(ptFare, itemcat);
    return NOTPROCESSED;
  }

  if (UNLIKELY(flightInfo == nullptr))
    return FAIL;

  if (phase() == RuleItem::FCOPhase)
    return SOFTPASS;

  DiagManager diag(_trx, Diagnostic304);

  if (UNLIKELY(diag.isActive() && _trx.diagnostic().shouldDisplay(ptFare) &&
               diag.collector().parseFareMarket(_trx, *(ptFare.fareMarket()))))
  {
    if (ptFare.fareClass().empty())
    {
      diag << "----- FARE COLLECTION PHASE ----\n";
    }
    diag << "QUALIFIER FOR CAT " << ruleInfo->categoryNumber() << "\n";
  }

  flightApp.initialize(flightInfo, true, vendor, &_trx);

  return flightApp.process(ptFare, _trx);
}

Record3ReturnTypes
FareController::validateTravelRestrictionsQualifier(
    PaxTypeFare& ptFare,
    uint32_t itemcat,
    const CategoryRuleInfo* ruleInfo,
    const TravelRestriction* travelInfo,
    TravelRestrictionsObserverWrapper& travelAppWrapper)
{
  if (UNLIKELY(isFdTrx()))
  {
    if (travelInfo == nullptr)
      return setIncompleteR3Rule(ptFare, itemcat);

    GlobalDirectionStorage fmGlobal(ptFare);

    FDTravelRestrictions fdTravelRest;
    return fdTravelRest.validate(_trx, _itin, ptFare, travelInfo, _fareMarket, true);
  }

  if (UNLIKELY(travelInfo == nullptr))
    return FAIL;

  return travelAppWrapper.validate(_trx, _itin, ptFare, travelInfo, _fareMarket);
}

Record3ReturnTypes
FareController::validateSaleRestrictionsQualifier(PaxTypeFare& ptFare,
                                                  const CategoryRuleItemInfo& ruleItem,
                                                  const CategoryRuleInfo* ruleInfo,
                                                  const SalesRestriction* salesInfo,
                                                  SalesRestrictionRuleWrapper& salesApp,
                                                  const CarrierCode& cxr)
{
  bool isQualifier = ((ruleInfo->categoryNumber() < RuleConst::CHILDREN_DISCOUNT_RULE) ||
                      (ruleInfo->categoryNumber() > RuleConst::OTHER_DISCOUNT_RULE));

  bool isCat15Sec = true;

  if (UNLIKELY(isFdTrx()))
  {
    if (salesInfo == nullptr)
      return setIncompleteR3Rule(ptFare, ruleItem.itemcat());

    GlobalDirectionStorage fmGlobal(ptFare);

    FDSalesRestrictionRule fdsrr;
    return fdsrr.validate(
        _trx, _itin, nullptr, ptFare, *ruleInfo, &ruleItem, salesInfo, isQualifier, isCat15Sec, false);
  }

  if (UNLIKELY(salesInfo == nullptr))
    return FAIL;

  Record3ReturnTypes retval;

  FareMarketDataAccess rcda(_trx, &_itin, ptFare);
  rcda.setValidatingCxr(cxr);
  salesApp.setRuleDataAccess(&rcda);

  retval = salesApp.validate(
      _trx, _itin, nullptr, ptFare, *ruleInfo, &ruleItem, salesInfo, isQualifier, isCat15Sec, false);

  if (UNLIKELY(ruleInfo->categoryNumber() == RuleConst::FARE_BY_RULE && retval == SKIP))
    return FAIL;

  return retval;
}

Record3ReturnTypes
FareController::validateSaleRestrictionsQualifier(PaxTypeFare& ptFare,
                                                  const CategoryRuleItemInfo& ruleItem,
                                                  const CategoryRuleInfo* ruleInfo,
                                                  const SalesRestriction* salesInfo,
                                                  SalesRestrictionRuleWrapper& salesApp)
{
  bool isQualifier = ((ruleInfo->categoryNumber() < RuleConst::CHILDREN_DISCOUNT_RULE) ||
                      (ruleInfo->categoryNumber() > RuleConst::OTHER_DISCOUNT_RULE));

  bool isCat15Sec = true;

  if (isFdTrx())
  {
    if (salesInfo == nullptr)
      return setIncompleteR3Rule(ptFare, ruleItem.itemcat());

    GlobalDirectionStorage fmGlobal(ptFare);

    FDSalesRestrictionRule fdsrr;
    return fdsrr.validate(
        _trx, _itin, nullptr, ptFare, *ruleInfo, &ruleItem, salesInfo, isQualifier, isCat15Sec, false);
  }

  if (salesInfo == nullptr)
    return FAIL;

  Record3ReturnTypes retval = salesApp.validate(_trx,
                                                _itin,
                                                nullptr,
                                                ptFare,
                                                *ruleInfo,
                                                &ruleItem,
                                                salesInfo,
                                                isQualifier,
                                                isCat15Sec,
                                                false);

  if (ruleInfo->categoryNumber() == RuleConst::FARE_BY_RULE && retval == SKIP)
    return FAIL;

  return retval;
}

Record3ReturnTypes
FareController::validateQualifier(PaxTypeFare& ptFare,
                                  const CategoryRuleItemInfo& ruleItem,
                                  const VendorCode& vendor,
                                  const CarrierCode& cxr,
                                  const CategoryRuleInfo* ruleInfo)
{

  DataHandle& dh = _trx.dataHandle();

  Record3ReturnTypes result = NOTPROCESSED;

  switch (ruleItem.itemcat())
  {
  case RuleConst::ELIGIBILITY_RULE:
    result = validateEligibilityQualifier(ptFare,
                                          ruleItem.itemcat(),
                                          ruleInfo,
                                          dh.getEligibility(vendor, ruleItem.itemNo()),
                                          _eligibilityApplication);
    break;

  case RuleConst::DAY_TIME_RULE:
    result = validateDayTimeQualifier(ptFare,
                                      ruleItem.itemcat(),
                                      ruleInfo,
                                      dh.getDayTimeAppInfo(vendor, ruleItem.itemNo()),
                                      _dayTimeApplication);
    break;

  case RuleConst::SEASONAL_RULE:
    result = validateSeasonalQualifier(ptFare,
                                       ruleItem,
                                       ruleInfo,
                                       dh.getSeasonalAppl(vendor, ruleItem.itemNo()),
                                       _seasonalApplication);
    break;

  case RuleConst::FLIGHT_APPLICATION_RULE:
    if (_trx.isShopping())
    {
      setQualifyFltAppRuleData(ptFare, ruleInfo);
      return NOTPROCESSED;
    }
    result = validateFlightApplicationQualifier(ptFare,
                                                ruleItem.itemcat(),
                                                vendor,
                                                ruleInfo,
                                                dh.getFlightAppRule(vendor, ruleItem.itemNo()),
                                                _flightApplication);
    break;

  case RuleConst::TRAVEL_RESTRICTIONS_RULE:
    result = validateTravelRestrictionsQualifier(ptFare,
                                                 ruleItem.itemcat(),
                                                 ruleInfo,
                                                 dh.getTravelRestriction(vendor, ruleItem.itemNo()),
                                                 _travelApplication);
    break;

  case RuleConst::SALE_RESTRICTIONS_RULE:
    result = validateSaleRestrictionsQualifier(ptFare,
                                               ruleItem,
                                               ruleInfo,
                                               dh.getSalesRestriction(vendor, ruleItem.itemNo()),
                                               _salesRestrictionRule, cxr);
    break;

  default:
    break;
  }

  return result;
}

Record3ReturnTypes
FareController::validateQualifier(PaxTypeFare& ptFare,
                                  const CategoryRuleItemInfo& ruleItem,
                                  const VendorCode& vendor,
                                  const CategoryRuleInfo* ruleInfo)
{

  DataHandle& dh = _trx.dataHandle();

  Record3ReturnTypes result = NOTPROCESSED;

  switch (ruleItem.itemcat())
  {
  case RuleConst::ELIGIBILITY_RULE:
    result = validateEligibilityQualifier(ptFare,
                                          ruleItem.itemcat(),
                                          ruleInfo,
                                          dh.getEligibility(vendor, ruleItem.itemNo()),
                                          _eligibilityApplication);
    break;

  case RuleConst::DAY_TIME_RULE:
    result = validateDayTimeQualifier(ptFare,
                                      ruleItem.itemcat(),
                                      ruleInfo,
                                      dh.getDayTimeAppInfo(vendor, ruleItem.itemNo()),
                                      _dayTimeApplication);
    break;

  case RuleConst::SEASONAL_RULE:
    result = validateSeasonalQualifier(ptFare,
                                       ruleItem,
                                       ruleInfo,
                                       dh.getSeasonalAppl(vendor, ruleItem.itemNo()),
                                       _seasonalApplication);
    break;

  case RuleConst::FLIGHT_APPLICATION_RULE:
    if (_trx.isShopping())
    {
      setQualifyFltAppRuleData(ptFare, ruleInfo);
      return NOTPROCESSED;
    }
    result = validateFlightApplicationQualifier(ptFare,
                                                ruleItem.itemcat(),
                                                vendor,
                                                ruleInfo,
                                                dh.getFlightAppRule(vendor, ruleItem.itemNo()),
                                                _flightApplication);
    break;

  case RuleConst::TRAVEL_RESTRICTIONS_RULE:
    result = validateTravelRestrictionsQualifier(ptFare,
                                                 ruleItem.itemcat(),
                                                 ruleInfo,
                                                 dh.getTravelRestriction(vendor, ruleItem.itemNo()),
                                                 _travelApplication);
    break;

  case RuleConst::SALE_RESTRICTIONS_RULE:
    result = validateSaleRestrictionsQualifier(ptFare,
                                               ruleItem,
                                               ruleInfo,
                                               dh.getSalesRestriction(vendor, ruleItem.itemNo()),
                                               _salesRestrictionRule);
    break;

  default:
    break;
  }

  return result;
}

void
FareController::processRuleItemInfoSet(CategoryRuleItemInfoSet& ruleItemInfoSet,
                                       std::vector<CategoryRuleItemInfo>& segQual,
                                       std::vector<CategoryRuleItemInfo>::iterator& o,
                                       std::vector<CategoryRuleItemInfo>::iterator& p)
{

  // First find where our 'IF' record is
  for (; o != p; o++)
  {
    CategoryRuleItemInfo* catRuleItemInfo = &(*o);

    if (catRuleItemInfo->relationalInd() == CategoryRuleItemInfo::IF)
    {
      p = o;

      segQual.push_back(*catRuleItemInfo);

      ++o;

      // now keep looping and adding items to segQual as long as we have an OR or an AND
      for (; o != ruleItemInfoSet.end(); o++)
      {
        catRuleItemInfo = &(*o);

        if (LIKELY(catRuleItemInfo->relationalInd() == CategoryRuleItemInfo::OR ||
                   catRuleItemInfo->relationalInd() == CategoryRuleItemInfo::AND))
          segQual.push_back(*catRuleItemInfo);
        else
          break;
      }
      break;
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function FareByRuleController::getAllowedVendors
//
// Description: using vendor xref table, find allowed vendors
//              1st item always baseVendor
// @return: results: none - alters member
//
// </PRE>
// ----------------------------------------------------------
//
void
FareController::setAllowedVendors(const VendorCode& baseVend)

{
  // reuse old?
  if (!_allowedVend.empty())
  {
    if (LIKELY(*_allowedVend[0] == baseVend))
      return;
    else
      _allowedVend.clear();
  }

  switch (_trx.dataHandle().getVendorType(baseVend))
  {
  case 'P': // intentional fall-thru
  default:
    _allowedVend.push_back(&baseVend);
    break;
  }

  return;
}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function FareController::matchVendor
//
// Description: is OK to use this vendor?
//             must call selAllowedVendor() first
// @return: results: true - vendor is allowed
//
// </PRE>
// ----------------------------------------------------------
//
bool
FareController::isVendorAllowed(const VendorCode& candidateVend) const
{
  return std::any_of(_allowedVend.begin(),
                     _allowedVend.end(),
                     [candidateVend](const VendorCode* const vc)
                     { return candidateVend == *vc; });
}

bool
FareController::validateFareGroup(PaxTypeFare& ptFare)
{
  // Determine whether corporate id is required ?
  if (!ptFare.matchedCorpID() &&
      (ptFare.isFareByRule() && !(ptFare.getFbrRuleData()->fbrApp()->accountCode().empty())))
  {
    ptFare.setMatchedCorpID();
  }
  if (ptFare.actualPaxTypeItem().empty() == false)
  {
    ptFare.actualPaxTypeItem().clear();
    ptFare.setFailedFareGroup(false);
  }
  bool needPCCValidation = false;
  std::vector<PaxTypeBucket>& ptCorteges = _fareMarket.paxTypeCortege();
  std::vector<PaxTypeBucket>::iterator ptCortegesIt = ptCorteges.begin();
  std::vector<PaxTypeBucket>::iterator ptCortegesEnd = ptCorteges.end();
  std::vector<PaxType*>::iterator atPaxTypeIt;
  uint16_t paxIndex = 0;
  bool isCmdPriced = isCmdPricedFM();

  for (; ptCortegesIt != ptCortegesEnd; ++ptCortegesIt, ++paxIndex)
  {
    PaxTypeBucket& ptCortege = *ptCortegesIt;

    if (!checkAge(ptFare, *ptCortege.requestedPaxType()))
      atPaxTypeIt = ptCortege.actualPaxType().end();
    else
      atPaxTypeIt = find_if(ptCortege.actualPaxType().begin(),
                            ptCortege.actualPaxType().end(),
                            PaxTypeFinder(ptFare, false, isCmdPriced));

    uint16_t paxTypeNum = PaxTypeFare::PAXTYPE_FAIL;
    if (_trx.posPaxType()[paxIndex].empty()) // no fare group
    {
      paxTypeNum = PaxTypeFare::PAXTYPE_NO_MATCHED; // do not process fare group  for this pax type
    }
    else if (atPaxTypeIt != ptCortege.actualPaxType().end())
    {
      PosPaxType* posPaxType = dynamic_cast<PosPaxType*>(*atPaxTypeIt);
      if (posPaxType == nullptr)
        return false;
      //---------------------------------------------------------
      // if fare group number is 0 then keep this fare.
      // So we are able to create discounted fare from this fare.
      //---------------------------------------------------------
      if (posPaxType->fgNumber() != 0)
      {
        paxTypeNum = atPaxTypeIt - ptCortege.actualPaxType().begin();
        // validate fare group corp id if pax type fare matched corpid and fare group has no corp id
        if ((ptFare.matchedCorpID() != posPaxType->corpID().empty()) ||
            validateFareGroupCorpID(ptFare, atPaxTypeIt, ptCortege.actualPaxType()))
        {
          paxTypeNum = atPaxTypeIt - ptCortege.actualPaxType().begin(),
          posPaxType = dynamic_cast<PosPaxType*>(*atPaxTypeIt);
          if (posPaxType == nullptr)
            return false;

          if (!needPCCValidation && !posPaxType->pcc().empty())
          {
            needPCCValidation = true;
          }
        }
        else // no match
        {
          paxTypeNum = PaxTypeFare::PAXTYPE_FAIL;
        }
      }
      else
      {
        paxTypeNum = PaxTypeFare::PAXTYPE_NO_MATCHED;
      }
    }
    ptFare.actualPaxTypeItem().push_back(paxTypeNum);
  }

  // validate fare group pcc for all pax type
  if (needPCCValidation)

  {
    validateFareGroupPCC(ptFare);
  }
  // keep fare group number in actualPaxTypeItem.
  ptCortegesIt = ptCorteges.begin();
  std::vector<uint16_t>::iterator actualPaxTypeItemIt = ptFare.actualPaxTypeItem().begin();
  std::vector<uint16_t>::iterator actualPaxTypeItemItEnd = ptFare.actualPaxTypeItem().end();
  bool paxTypeMatch = false;
  bool basedDiscountedFareFound = false;
  for (paxIndex = 0; actualPaxTypeItemIt != actualPaxTypeItemItEnd && ptCortegesIt != ptCortegesEnd;
       ++actualPaxTypeItemIt, ++ptCortegesIt, ++paxIndex)
  {
    if (*actualPaxTypeItemIt == PaxTypeFare::PAXTYPE_FAIL)
    {
      *actualPaxTypeItemIt = 0;
    }
    else if (*actualPaxTypeItemIt == PaxTypeFare::PAXTYPE_NO_MATCHED)
    {
      if (_trx.posPaxType()[paxIndex].empty()) // no fare group
      {
        paxTypeMatch = true; // at least one match, fare is valid.
        *actualPaxTypeItemIt = PaxTypeFare::PAXTYPE_NO_FAREGROUP;
      }
      else
      {
        basedDiscountedFareFound = true;
        if (_trx.diagnostic().diagnosticType() != Diagnostic903)
        {
          *actualPaxTypeItemIt = 0; // fare is not valid for this pax type
        }
      }
    }
    else
    {
      paxTypeMatch = true; // at least one match, fare is valid.
      std::vector<PaxType*>::iterator atPaxTypeIt =
          (*ptCortegesIt).actualPaxType().begin() + *actualPaxTypeItemIt;
      PosPaxType* posPaxType = dynamic_cast<PosPaxType*>(*atPaxTypeIt);
      if (posPaxType == nullptr)
        return false;

      *actualPaxTypeItemIt = posPaxType->fgNumber();
    }
  }
  if (!paxTypeMatch)
  {
    ptFare.setFailedFareGroup();
    if (basedDiscountedFareFound)
    {
      ptFare.actualPaxTypeItem()[0] = PaxTypeFare::PAXTYPE_NO_MATCHED;
    }
  }
  return paxTypeMatch;
}

bool
FareController::validateFareGroupCorpID(PaxTypeFare& ptFare,
                                        std::vector<PaxType*>::iterator& atPaxTypeIt,
                                        std::vector<PaxType*>& atPaxTypeVec)
{
  // 1. set up the corp id.
  std::string reqCorpID = "";
  if (ptFare.matchedCorpID())
  {
    reqCorpID = _trx.getRequest()->corporateID();
  }
  // 2. if corp id is matched then pass fare group corp id validation.
  //    if corp id is not matched then step to next fare group.
  //       if pax type and pcc are not the same as previous one, fail fare group corp id validation.
  //       if the same and corp id is matched then pass fare group corp id validation.
  //    Fare groups will only maximum of 2 item that could have the same paxType and PCC.
  std::vector<PaxType*>::iterator atPaxTypeEnd = atPaxTypeVec.end();
  if (atPaxTypeVec.empty())
    return false;

  for (; atPaxTypeIt != atPaxTypeEnd - 1; ++atPaxTypeIt)

  {
    PosPaxType* posPaxType = dynamic_cast<PosPaxType*>(*atPaxTypeIt);
    if (posPaxType == nullptr)
      return false;

    if (posPaxType->corpID() != reqCorpID)
    {
      PosPaxType* nextPosPaxType = dynamic_cast<PosPaxType*>(*(atPaxTypeIt + 1));
      if (nextPosPaxType == nullptr)
        return false;
      // exit the loop once pax type has changed
      if (posPaxType->paxType() != nextPosPaxType->paxType())
        break;

      if (nextPosPaxType->corpID() == reqCorpID)
      {
        ++atPaxTypeIt;
        return true;
      }
    }
  }
  return false;
}

bool
FareController::validateFareGroupPCC(PaxTypeFare& ptFare)
{
  PaxTypeFareRuleData* paxTypeFareRuleData =
      ptFare.paxTypeFareRuleData(RuleConst::SALE_RESTRICTIONS_RULE);
  bool paxTypeMatched = false;
  std::vector<uint16_t>::iterator actualPaxTypeItemIt = ptFare.actualPaxTypeItem().begin();
  std::vector<uint16_t>::iterator actualPaxTypeItemItEnd = ptFare.actualPaxTypeItem().end();
  if (paxTypeFareRuleData == nullptr)
  {
    // no sales restriction
    // It can pass the negative item. Locate the 1st negative item in the list for the
    // same pax type and corp id.
    std::vector<PaxTypeBucket>& paxTypeCortege = ptFare.fareMarket()->paxTypeCortege();
    uint16_t paxTypeNum = 0;
    for (; actualPaxTypeItemIt != actualPaxTypeItemItEnd; ++actualPaxTypeItemIt, ++paxTypeNum)
    {
      bool matched = false;
      if (*actualPaxTypeItemIt == PaxTypeFare::PAXTYPE_FAIL ||
          *actualPaxTypeItemIt == PaxTypeFare::PAXTYPE_NO_MATCHED)
        continue; // nothing to be done

      std::vector<PaxType*>::iterator atPaxTypeItEnd =
          paxTypeCortege[paxTypeNum].actualPaxType().end();
      std::vector<PaxType*>::iterator atPaxTypeIt =
          paxTypeCortege[paxTypeNum].actualPaxType().begin() + *actualPaxTypeItemIt;
      PaxTypeCode& paxType = (*atPaxTypeIt)->paxType(); // keep original paxType
      while (atPaxTypeIt != atPaxTypeItEnd && paxType == (*atPaxTypeIt)->paxType())
      {
        PosPaxType* posPaxType = dynamic_cast<PosPaxType*>(*atPaxTypeIt);
        if (posPaxType == nullptr)
          return false;

        if (posPaxType->pcc().empty())
        {
          paxTypeMatched = true;
          break; // nothing to be done
        }
        // negative pcc and matched corp id whether we need it or not
        if (!posPaxType->positive() && ptFare.matchedCorpID() == !posPaxType->corpID().empty())
        {
          matched = true;
          paxTypeMatched = true;
          break;
        }
        ++atPaxTypeIt;
        ++(*actualPaxTypeItemIt);
      }
      if (!matched)
      {
        *actualPaxTypeItemIt = PaxTypeFare::PAXTYPE_FAIL;
      }
    }
  }
  else
  {
    if (_fareGroupSRValidationRuleController.validate(_trx, _itin, ptFare))
    {
      actualPaxTypeItemIt = ptFare.actualPaxTypeItem().begin();
      for (; actualPaxTypeItemIt != actualPaxTypeItemItEnd; ++actualPaxTypeItemIt)
      {
        if (*actualPaxTypeItemIt == PaxTypeFare::PAXTYPE_FAIL)
          continue; // nothing to be done

        paxTypeMatched = true;
      }
    }
  }
  return (paxTypeMatched);
}


void
FareController::findCandidatePaxTypes(std::vector<PaxType*>& candidatePaxTypes,
                                      const bool& careAboutAge)
{
  for (const PaxTypeBucket& paxTypeCortege : _fareMarket.paxTypeCortege())
  {
    for (PaxType* actualPaxType : paxTypeCortege.actualPaxType())
    {
      PaxTypeCode& paxTypeCode = actualPaxType->paxType();
      if (paxTypeCode == ADULT)
        continue; // ADULT is not going to be in list now

      if (!careAboutAge)
      {

        if (PaxTypeUtil::findMatch(candidatePaxTypes, paxTypeCode))
          continue; // No duplication
      }

      candidatePaxTypes.push_back(actualPaxType);
    }
  }
}

PaxTypeFare*
FareController::clonePTFare(const PaxTypeFare& originalPTFare, const PaxType& newPaxType)
{
  DataHandle& dataHandle = _trx.dataHandle();

  PaxTypeFare* newPTFare = originalPTFare.clone(dataHandle, true);

  if (!newPTFare)
    return nullptr;

  newPTFare->paxType() = const_cast<PaxType*>(&newPaxType);
  newPTFare->bookingCodeStatus() = PaxTypeFare::BKS_NOT_YET_PROCESSED;

  // need to clone Fare object so we do not share fare._ruleStatus with
  // originalPTFare
  Fare* newFare = originalPTFare.fare()->clone(dataHandle);
  newPTFare->setFare(newFare);

  // If faredisplay trx, create a FareDisplayInfo in the new paxtypefare.
  // Clone FareDisplayInfo object from the base paxtypefare so that
  // the new paxtypefare we do not share this with the base paxtypefare
  if (isFdTrx())
  {
    if (!FareDisplayUtil::initFareDisplayInfo(_fdTrx, *newPTFare))
    {
      LOG4CXX_ERROR(_logger, "FareController::clonePTFare - Unable to init FareDisplayInfo");
    }
    else
    {
      (const_cast<FareDisplayInfo*>(originalPTFare.fareDisplayInfo()))
          ->clone(newPTFare->fareDisplayInfo(), newPTFare);
    }
  }

  // From now on, we want all the categories validating new PaxTypeFare
  // see the "real" PaxType we resolved during cat1 validation, so
  // we would need to clone record1 and overwrite fcasPaxType
  FareClassAppSegInfo* fcAppSegInfo = originalPTFare.fareClassAppSegInfo()->clone(dataHandle);
  fcAppSegInfo->_paxType = newPaxType.paxType();

  newPTFare->fareClassAppSegInfo() = fcAppSegInfo;

  return newPTFare;
}

bool
FareController::createAllPTFs(std::vector<Fare*>& fares)
{
  if (_trx.isFootNotePrevalidationAllowed())
  {
    for (Fare* fare : fares)
    {
      if (!matchGlobalDirectionality(*fare, _fareMarket))
        continue;

      bool footNotesValid;

      if (commandPricingFare(fare->fareClass()))
        footNotesValid = true;
      else
      {
        PaxTypeFare ptf;
        ptf.initialize(fare, nullptr, &_fareMarket, _trx);

        footNotesValid = _footNotePrevalidationRuleController->validate(_trx, _itin, ptf);

        if (!footNotesValid)
        {
          try
          {
            _calcMoney.adjustPTF(ptf);
            _fareMarket.footNoteFailedFares().push_back(std::make_pair(fare, false));
          }
          catch (tse::ErrorResponseException& ex)
          {
            if (ex.code() != tse::ErrorResponseException::MISSING_NUC_RATE)
              throw;
          }
        }
      }

      if (footNotesValid)
      {
        std::vector<PaxTypeFare*> ptFares;

        if (resolveFareClassApp(*fare, ptFares) && !ptFares.empty())
          validateAndAddPaxTypeFares(ptFares);
      }
    }
  }
  else
  {
    for (Fare* fare : fares)
    {
      if (LIKELY(fare != nullptr))
      {
        if (UNLIKELY(!createPaxTypeFares(fare)))
          return false;
      }
    }
  }
  return true;
}

bool
FareController::createAllPaxTypeFaresESV(std::vector<Fare*>& fares)
{
  TSELatencyData metrics(_trx, "CREATE APP PAX TYPE FARES ESV");

  if (_fareMarket.paxTypeCortege().empty())
  {
    LOG4CXX_ERROR(_logger, "FareController::createAllPaxTypeFaresESV - Pax type cortege for "
                           "processed fare market is empty.");
    return false;
  }

  if (_trx.paxType().empty())
  {
    LOG4CXX_ERROR(
        _logger,
        "FareController::createAllPaxTypeFaresESV - Requested passenger type vector is empty.");
    return false;
  }

  if (_trx.paxType().size() != 1)
  {
    LOG4CXX_ERROR(_logger, "FareController::createAllPaxTypeFaresESV - ESV does not support more "
                           "than one passenger type.");
  }

  PaxType* reqPaxType = _trx.paxType()[0];

  PaxTypeBucket* ptCortege = nullptr;

  for (PaxTypeBucket& paxTypeCortege : _fareMarket.paxTypeCortege())
  {
    if (paxTypeCortege.requestedPaxType()->paxType() == reqPaxType->paxType())
    {
      ptCortege = &paxTypeCortege;
      break;
    }
  }

  if (nullptr == ptCortege)
  {
    LOG4CXX_ERROR(_logger, "FareController::createAllPaxTypeFaresESV - Pax type cortege for "
                           "requested passenger type is NULL.");
    return false;
  }

  ptCortege->paxTypeFare().reserve(fares.size());

  for (const Fare* fare : fares)
  {
    if (fare != nullptr)
    {
      createPaxTypeFareESV(fare, ptCortege);
    }
  }

  return true;
}

namespace
{

// A comparison of fares by the FareInfo. This is used
// in eliminateDuplicates() to group fares to reduce
// the number of comparisons.
  struct FareLessByFareInfo
  {

    bool operator()(const Fare* f1, const Fare* f2) const
    {
      const FareInfo& a = *f1->fareInfo();
      const FareInfo& b = *f2->fareInfo();

      if (a.fareTariff() < b.fareTariff())
      {
        return true;
      }
      else if (a.fareTariff() > b.fareTariff())
      {
        return false;
      }

      if (a.owrt() < b.owrt())
      {
        return true;
      }
      else if (a.owrt() > b.owrt())
      {
        return false;
      }

      if (UNLIKELY(a.globalDirection() < b.globalDirection()))
      {
        return true;
      }
      else if (UNLIKELY(a.globalDirection() > b.globalDirection()))
      {
        return false;
      }

      if (a.fareClass() < b.fareClass())
      {
        return true;
      }
      else if (a.fareClass() > b.fareClass())
      {
        return false;
      }

      if (UNLIKELY(a.vendor() < b.vendor()))
      {
        return true;
      }
      else if (a.vendor() > b.vendor())
      {
        return false;
      }

      if (UNLIKELY(a.carrier() < b.carrier()))
      {
        return true;
      }
      else if (UNLIKELY(a.carrier() > b.carrier()))
      {
        return false;
      }

      if (a.currency() < b.currency())
      {
        return true;
      }
      else if (a.currency() > b.currency())
      {
        return false;
      }

      if (a.ruleNumber() < b.ruleNumber())
      {
        return true;
      }
      else if (a.ruleNumber() > b.ruleNumber())
      {
        return false;
      }

      if (a.market1() < b.market1())
      {
        return true;
      }
      else if (a.market1() > b.market1())
      {
        return false;
      }

      if (a.market2() < b.market2())
      {
        return true;
      }
      else if (a.market2() > b.market2())
      {
        return false;
      }

      return false;
    }
  };
}

bool
FareController::isCFDInResponse(const std::vector<PaxTypeFare*>& ptFares,
                                const ConstructedFareInfo& cfi) const
{
  // iterate via vector of fares to see
  // whether we have equal fare or not.
  // if we do then apply special logic to avoid fare duplicates

  //    const LocCode& origin    = _fareMarket.origin()->loc();
  //    const LocCode& boardCity = _fareMarket.boardMultiCity();

  for (const PaxTypeFare* ptFare : ptFares)
  {
    if (UNLIKELY(ptFare == nullptr))
      continue;

    if (FareDup::isEqual(cfi, *(ptFare->fare())) == FareDup::EQUAL)
      return true;
  }

  return false;
}


void
FareController::eliminateDuplicates(std::vector<Fare*>& cxrConstructedFares,
                                    std::vector<Fare*>& cxrPublishedFares,
                                    std::vector<Fare*>& resultFares)
{
  Diag258Collector* diag258 = Diag258Collector::getDiag258(_trx, &_fareMarket);

  // put all the published fares into a multi set, grouping them
  // by their fare info. By doing this, we limit the number of
  // published fares each constructed fare has to be compared to
  typedef std::multiset<Fare*, FareLessByFareInfo> FareSet;
  typedef std::pair<FareSet::iterator, FareSet::iterator> FareSetItors;
  FareSet pubFares;
  for (Fare* fare : cxrPublishedFares)
  {
    if (UNLIKELY(fare == NULL))
      continue;

    if (UNLIKELY(fare->vendor() == ATPCO_VENDOR_CODE && fare->isCat23PublishedFail()))
      continue;

    pubFares.insert(fare);
  }

  const bool specifiedOverConstructed = (_fareMarket.governingCarrierPref()->applyspecoveraddon() == YES);

  // iterate over each constructed fare, and then find if there
  // are any duplicate published fares
  for (Fare* cfPtr : cxrConstructedFares)
  {
    bool createConstrFare = true;
    const ConstructedFareInfo& cfi = *(cfPtr->constructedFareInfo());

    // find all the published fares which have the same FareInfo
    // as the constructed fare
    for (FareSetItors pfItors = pubFares.equal_range(cfPtr); pfItors.first != pfItors.second;)
    {
      const Fare& pf = **pfItors.first;

      FareDup::ComparisonFailureCode cmpFailureCode = FareDup::isEqualSimple(cfi, pf);

      if (cmpFailureCode == FareDup::EQUAL)
      {
        // fares are equal. first to check specified over constructed
        // option from carrier preferences

        if (specifiedOverConstructed)
        {
          createConstrFare = false; // carrier prefers published fare
          // to constructed one, regardless
          // of price.
          if (UNLIKELY(diag258))
            diag258->writeDupDetail(pf, cfi, Diag258Collector::DRR_PUBLISHED_OVER_CONSTRUCTED);
          break;
        }

        // if we are here carrier doesn't care about fare type.
        // keep the cheapest fare. but before need to calculate
        // published fare NUC amount

        Money pfAmount(pf.fareAmount(), pf.currency());
        Money pfNuc("NUC");
        _ccFacade.convert(pfNuc, pfAmount, _trx);

        MoneyAmount cfAmount = cfi.constructedNucAmount();
        if (cfi.fareInfo().owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
          cfAmount = cfAmount / 2.0;

        if (cfAmount >= pfNuc.value())
        {
          if (UNLIKELY(diag258))
            diag258->writeDupDetail(
                pf, cfi, Diag258Collector::DRR_PUBLISHED_LESS_OR_EQ_FARE_AMOUNT);

          createConstrFare = false; // constructed fare is more
          break; // expensive. keep published fare.
        }
        else
        {
          if (UNLIKELY(diag258))
            diag258->writeDupDetail(pf, cfi, Diag258Collector::DRR_CONSTRUCTED_LESS_FARE_AMOUNT);

          // the published fare is more expensive, so remove it
          pubFares.erase(pfItors.first++);
        }
      }
      else
      {
        ++pfItors.first;
      }
    }

    if (createConstrFare)
      resultFares.push_back(cfPtr);
  }

  if (UNLIKELY(diag258))
  {
    diag258->writeDupRemovalFooter();
    Diag258Collector::reclaim(diag258);
  }

  // add all the published fares that remain to the results
  resultFares.resize(resultFares.size() + pubFares.size());
  std::copy(pubFares.begin(), pubFares.end(), resultFares.end() - pubFares.size());
}

bool
FareController::isInternationalFare(const Fare& fare) const
{
  return (fare.isInternational() || fare.isForeignDomestic());
}

VendorCode
FareController::getVendorForCat31() const
{
  const FareCompInfo* fc = _fareMarket.fareCompInfo();
  if (LIKELY(!fc || _trx.excTrxType() != PricingTrx::AR_EXC_TRX ||
      static_cast<RexPricingTrx&>(_trx).trxPhase() != RexPricingTrx::REPRICE_EXCITIN_PHASE))
    return Vendor::EMPTY;
  return (fc->hasVCTR() &&
          (fc->VCTR().vendor() == Vendor::ATPCO || fc->VCTR().vendor() == Vendor::SITA))
             ? fc->VCTR().vendor()
             : Vendor::EMPTY;
}

bool
FareController::checkAge(const PaxTypeFare& ptFare, const PaxType& paxType) const
{
  const uint16_t requestPaxAge = paxType.age();

  if (requestPaxAge == 0)
    return true;

  const uint16_t minAge = ptFare.fcasMinAge();
  if (minAge > 0)
  {
    if (UNLIKELY(minAge > requestPaxAge))
      return false;
  }
  const uint16_t maxAge = ptFare.fcasMaxAge();
  if (maxAge > 0)
  {
    if (maxAge < requestPaxAge)
      return false;
  }

  return true;
}

bool
FareController::sfaDiagRequested(PricingTrx& trx, std::vector<FareClassCode>& fareClasses)
{
  if (!trx.diagnostic().isActive())
    return false;

  const bool sfaEnabled = trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "SFA" ||
                          trx.diagnostic().diagParamMapItemPresent("SF");

  if (sfaEnabled)
  {
    int fareNum;
    std::string key;
    FareClassCode fareClassTemp;
    const uint16_t maxFareNumber = 24;
    for (uint16_t k = 0; k < maxFareNumber; k++)
    {
      fareNum = k + 1;
      std::stringstream fareNumstr;
      fareNumstr << fareNum;
      key = "F" + fareNumstr.str();
      // Get Fare Class from diagnostic request
      fareClassTemp = trx.diagnostic().diagParamMapItem(key);

      if (!fareClassTemp.empty())
      {
        fareClasses.push_back(fareClassTemp);
      }
      else
      {
        break;
      }
    }
    if (!fareClasses.empty())
    {
      return true;
    }
  }
  return false;
}

void
FareController::initMatchFaresDirectionality()
{
  _matchFaresDirectionality = _itin.simpleTrip();
}

bool
FareController::isCmdPricedFM()
{
  return (_trx.billing() && _trx.billing()->partitionID() == "WN")
             ? !_fareMarket.fareBasisCode().empty()
             : _trx.getOptions()->fbcSelected();
}

// ----------------------------------------------------------------------------
// @function bool FareController::prevalidateCat35If1
//
// Description:  This function prevalidates Cat 35 rule for Cat 25 Record 3 before
//               creating Cat 25/35 fare.
//               This may be used for stand alone Cat 35 fare (non-Cat 25) in future phase.
//               All Cat 35 rules must be filed with THEN 35 IF 1 in all Record 2 Cat 35(s)
//               to qualify for this prevalidation.
//
// @return bool
//        -false, if no possible Cat 35 rule
//        -true otherwise.
// ----------------------------------------------------------------------------
bool
FareController::prevalidateCat35If1(PaxTypeFare& ptFare,
                                    VendorCode& vendor,
                                    CarrierCode& cxr,
                                    TariffNumber& tariff,
                                    RuleNumber& ruleNumber)
{
  const std::vector<GeneralFareRuleInfo*>& allRec2Cat35 =
        _trx.dataHandle().getGeneralFareRule(vendor,
                                             cxr,
                                             tariff,
                                             ruleNumber,
                                             RuleConst::NEGOTIATED_RULE,
                                             _fareMarket.travelDate());

  if (allRec2Cat35.empty())
    return false; // No Record 2 Cat 35 - fail this fare/rule

  const uint32_t rec2Cat35SegCountLimit = TrxUtil::getRec2Cat35SegCountLimit(_trx);
  for (const auto fri : allRec2Cat35)
  {
    if (rec2Cat35SegCountLimit > 0 && fri)
    {
      const uint32_t r2Cat3Segcount = fri->segcount();
      if (r2Cat3Segcount > rec2Cat35SegCountLimit)
        return true; // Cat 35 rule not qualified for prevalidation - continue to process this fare/rule
    }
    for (CategoryRuleItemInfoSet* ruleItemInfoSet : fri->categoryRuleItemInfoSet())
    {
      if (UNLIKELY(ruleItemInfoSet == nullptr)) // Something wrong
        return true; // There can be a valid Cat 35 rule - continue to process this fare/rule
      bool ruleSetHasQualify = false;
      bool ruleSetHasCat1Qualify = false;
      for (const CategoryRuleItemInfo& catRuleItemInfo : (*ruleItemInfoSet))
      {
        if (catRuleItemInfo.relationalInd() == CategoryRuleItemInfo::ELSE) // ELSE 35
          return true; // Cat 35 rule not qualified for prevalidation - continue to process this fare/rule
        if (catRuleItemInfo.relationalInd() == CategoryRuleItemInfo::IF)
          ruleSetHasQualify = true;
        if (ruleSetHasQualify &&
            catRuleItemInfo.itemcat() == RuleConst::ELIGIBILITY_RULE)
        {
          const EligibilityInfo* cat1R3 =
                _trx.dataHandle().getEligibility(vendor, catRuleItemInfo.itemNo());
          if (!cat1R3)
            return true; // There can be a valid Cat 35 rule
          Eligibility eligibilityApplication;
          if (eligibilityApplication.checkAccountCode(cat1R3, ptFare, _trx, nullptr, false, nullptr, false) ==
              PASS)
            return true; // There can be a valid Cat 35 rule
          ruleSetHasCat1Qualify = true;
        }
      }
      if (!ruleSetHasCat1Qualify) // No IF CAT 1 for this set
        return true; // Cat 35 rule not qualified for prevalidation - continue to process this fare/rule
    }
  }
  return false; // no possible valid Cat 35 rule - fail this fare/rule
}

const std::vector<const FareInfo*>&
FareController::filterByVendorForCat31AndDFF(const std::vector<const FareInfo*>& all,
                                             std::vector<const FareInfo*>& filtered) const
{
  const VendorCode vendor = getVendorForCat31();

  if (LIKELY(vendor == Vendor::EMPTY))
  {
    alg::copy_if(all,
                 std::back_inserter(filtered),
                 [](const FareInfo* fareInfo)
                 { return (fareInfo->vendor() != Vendor::SMFO); });
  }
  else
  {
    alg::copy_if(all,
                 std::back_inserter(filtered),
                 [vendor](const FareInfo* fareInfo)
                 { return (fareInfo->vendor() == vendor); });
  }
  return filtered;
}
} // tse
