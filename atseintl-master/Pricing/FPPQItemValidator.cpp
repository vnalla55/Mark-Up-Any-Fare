// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include "Pricing/FPPQItemValidator.h"

#include "BookingCode/Cat31FareBookingCodeValidator.h"
#include "BookingCode/FareBookingCodeValidator.h"
#include "BookingCode/MixedClassController.h"
#include "Common/AltPricingUtil.h"
#include "Common/ClassOfService.h"
#include "Common/Config/DynamicConfigurableFlag.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/ItinUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/SpanishLargeFamilyUtil.h"
#include "Common/SpanishResidentFaresEnhancementUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "Common/YQYR/YQYRCalculator.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DBAccess/FareCalcConfig.h"
#include "Diagnostic/Diag666Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Fares/FareCollectorOrchestrator.h"
#include "FreeBagService/BaggageCalculator.h"
#include "FreeBagService/FreeBagService.h"
#include "Limitations/LimitationOnIndirectTravel.h"
#include "MinFares/MinFareChecker.h"
#include "Pricing/BCMixedClassValidator.h"
#include "Pricing/Combinations.h"
#include "Pricing/FactoriesConfig.h"
#include "Pricing/FarePathFactoryFailedPricingUnits.h"
#include "Pricing/FarePathUtils.h"
#include "Pricing/FarePathValidator.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/JourneyValidator.h"
#include "Pricing/MaximumPenaltyValidator.h"
#include "Pricing/NegotiatedFareCombinationValidator.h"
#include "Pricing/PaxFPFBaseData.h"
#include "Pricing/PricingUnitFactory.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/PUPath.h"
#include "Pricing/RuleValidator.h"
#include "RexPricing/RefundSolutionValidator.h"
#include "RexPricing/RepriceSolutionValidator.h"
#include "Server/TseServer.h"
#include "Util/BranchPrediction.h"

#include <algorithm>

namespace tse
{
FALLBACK_DECL(smpValidateFPPQItem);
FALLBACK_DECL(conversionDateSSDSP1154);

namespace
{
DynamicConfigurableFlagOn
baggageBruteForce("PRICING_SVC", "BAGGAGE_IN_PQ_BRUTE_FORCE", false);
Logger
logger("atseintl.Pricing.FPPQItemValidator");
}

FPPQItemValidator::FPPQItemValidator(PaxFPFBaseData& pfpfBase,
                                     const std::vector<PricingUnitFactory*>& allPUF,
                                     FarePathFactoryFailedPricingUnits& failedPricingUnits,
                                     DiagCollector& diag)
  : _trx(*pfpfBase.trx()),
    _allPUF(allPUF),
    _factoriesConfig(pfpfBase.getFactoriesConfig()),
    _failedPricingUnits(failedPricingUnits),
    _diag(diag),
    _ruleController(&_trx),
    _combinations(pfpfBase.combinations())

{
}
void
FPPQItemValidator::wpaNoMatchHigherCabinValidation(bool& localValid,
                                                   FarePath& fpI,
                                                   std::string& localRes)
{
  AltPricingTrx* altTrx = dynamic_cast<AltPricingTrx*>(&_trx);
  if (UNLIKELY(_trx.getRequest()->isLowFareRequested() && altTrx &&
      (altTrx->altTrxType() == AltPricingTrx::WPA ||
       altTrx->altTrxType() == AltPricingTrx::WP_NOMATCH)))
  {
    localValid = wpaHigherCabin(fpI);
    localRes += (localValid ? "WPA:P " : "WPA:F ");
  }
}

void
FPPQItemValidator::checkIntlSurfaceTvlLimitation(FarePath& fpI)
{
  TSELatencyData metrics(_trx, "PO LIMITATION");
  LimitationOnIndirectTravel limitation(_trx, *fpI.itin());
  if (limitation.validateIntlSurfaceTravel(fpI))
  {
    fpI.intlSurfaceTvlLimit() = true;
    LOG4CXX_INFO(logger, "INTL SUURFACE TVL RESTRICTION APPLY")
  }
}

void
FPPQItemValidator::processMipAltDateSurcharges(FarePath& farePath)
{
  // The estimated amounts in the PUs are the lower bound amounts. We need
  // the correct estimated amount for farePath's actual VC now

  if (farePath.validatingCarriers().empty())
    return;

  CarrierCode valCxr = farePath.validatingCarriers().front();

  for (PricingUnit* pu : farePath.pricingUnit())
  {
    farePath.decreaseTotalNUCAmount(pu->taxAmount());
    pu->setTotalPuNucAmount(pu->getTotalPuNucAmount() - pu->taxAmount());
    pu->taxAmount() = 0;

    for (FareUsage* fareUsage : pu->fareUsage())
    {
      PaxTypeFare* paxTypeFare = fareUsage->paxTypeFare();
      const CxrPrecalculatedTaxes& taxes =
        paxTypeFare->fareMarket()->paxTypeCortege(farePath.paxType())->cxrPrecalculatedTaxes();

      MoneyAmount taxAmount = taxes.getTotalTaxAmountForCarrier(*paxTypeFare, valCxr);
      pu->taxAmount() += taxAmount;
      pu->setTotalPuNucAmount(pu->getTotalPuNucAmount() + taxAmount);
      farePath.increaseTotalNUCAmount(taxAmount);
    }
  }
}

void
FPPQItemValidator::validate(FPPQItem* fpPQI,
                            std::string& localRes,
                            bool& localValid,
                            bool& shouldPassCmdPricing,
                            bool pricingAxess,
                            const FPPQItem* topFppqItem,
                            const ValidationLevel validationLevel)
{
  FarePath& fpI = *fpPQI->farePath();
  RaiiRexFarePathPlusUps rfppu(_trx, fpI, localValid);
  std::shared_ptr<ExchangeUtil::RaiiProcessingDate> dateSetter;

  if (_trx.isRexBaseTrx())
  {
    RexBaseTrx& rexTrx = static_cast<RexBaseTrx&>(_trx);
    bool isFallback = fallback::conversionDateSSDSP1154(&rexTrx);
    dateSetter.reset(new ExchangeUtil::RaiiProcessingDate(rexTrx, fpI, isFallback));
  }

  _puPath = fpPQI->puPath();

  validateRules(localValid, shouldPassCmdPricing, fpPQI, localRes, validationLevel);

  bool isMipAltDates = (_trx.getTrxType() == PricingTrx::MIP_TRX) && _trx.isAltDates();
  if (isMipAltDates && localValid)
    processMipAltDateSurcharges(fpI);

  if (!isMipAltDates && (localValid || shouldPassCmdPricing))
    RuleUtil::getSurcharges(_trx, fpI);

  const bool isFareX = (_trx.getOptions() && _trx.getOptions()->fareX());

  validateJourneys(shouldPassCmdPricing, fpPQI, isFareX, localRes, localValid, pricingAxess, fpI);

  if (UNLIKELY(localValid && RexPricingTrx::isRexTrxAndNewItin(_trx)))
  {
    BCMixedClassValidator bcMixedClassValidator(_trx);
    localValid = bcMixedClassValidator.validate(localRes, fpI);
  }

  if (localValid)
    localValid = validForWQ(fpI);

  if (localValid && !pricingAxess)
  {
    wpaNoMatchHigherCabinValidation(localValid, fpI, localRes);
  }

  // We need to update FarePath's ISI before CT and MinFares check:
  if (localValid)
    PricingUtil::calculateFarePathISICode(_trx, fpI);

  circleTripsValidation(localValid, fpI, rfppu.plusUpsNeeded(), localRes);

  bool passByCmdPricing = false;
  if (UNLIKELY(!localValid && shouldPassCmdPricing))
  {
    passByCmdPricing = true;
    localValid = true;
  }

  if (localValid)
  {
    if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(_trx)))
    {
      localValid = isItemLessThanTopItem(
          topFppqItem, *fpPQI, fpI.plusUpFlag() && fpI.yqyrNUCAmount() > EPSILON);
      localRes += (localValid ? "REX LTT CHECK1:P " : "REX LTT CHECK1:F ");
      if (localValid)
      {
        localValid = validateBookingCodesCat31(fpI);
        localRes += (localValid ? "REX RBD CHECK1:P " : "REX RBD CHECK1:F ");
      }
      if (localValid)
      {
        TSELatencyData metrics(_trx, "PO CAT31 PERM VALIDATION");
        {
          if (_trx.getTrxType() == PricingTrx::MIP_TRX)
          {
            RepriceSolutionValidator rexSolutionValidator(static_cast<RexExchangeTrx&>(_trx), fpI);
            localValid = rexSolutionValidator.process();
          }
          else
          {
            RepriceSolutionValidator rexSolutionValidator(static_cast<RexPricingTrx&>(_trx), fpI);
            localValid = rexSolutionValidator.process();
          }
        }

        if (!localValid && _trx.getRequest()->isLowFareRequested() &&
            isSameFarePathValidForRebook(fpI))
        {
          resetToRebook(fpI, *fpPQI);
          if (_trx.getTrxType() == PricingTrx::MIP_TRX)
          {
            RepriceSolutionValidator rexSolutionValidator(static_cast<RexExchangeTrx&>(_trx), fpI);
            localValid = rexSolutionValidator.process();
          }
          else
          {
            RepriceSolutionValidator rexSolutionValidator(static_cast<RexPricingTrx&>(_trx), fpI);
            localValid = rexSolutionValidator.process();
          }
        }

        localRes += (localValid ? "REX SLV:P " : "REX SLV:F ");
      }

      if (localValid)
      {
        localValid = isItemLessThanTopItem(topFppqItem, *fpPQI, fpI.rexChangeFee() > EPSILON);
        localRes += (localValid ? "REX LTT CHECK2:P " : "REX LTT CHECK2:F ");
      }
    }

    else if (UNLIKELY(_trx.excTrxType() == PricingTrx::AF_EXC_TRX &&
             static_cast<RefundPricingTrx&>(_trx).trxPhase() == RexBaseTrx::PRICE_NEWITIN_PHASE))
      localValid = callRefundValidation(localRes, fpI);
  }

  // --------------- check MinFare -----------------
  if (localValid && rfppu.plusUpsNeeded())
    localValid = checkMinFare(fpI, localRes);

  if (!localValid)
    shouldPassCmdPricing = false;

  if (UNLIKELY(localValid && RexPricingTrx::isRexTrxAndNewItin(_trx)))
  {
    localValid = isItemLessThanTopItem(
        topFppqItem, *fpPQI, fpI.plusUpFlag() && fpI.yqyrNUCAmount() > EPSILON);
    localRes += (localValid ? "REX LTT CHECK3:P " : "REX LTT CHECK3:F ");
  }

  if (localValid)
  {
    localValid = calcBaggageCharge(fpI);
    localRes += localValid ? "BAG:P " : "BAG:F ";
  }

  if (localValid)
    fpI.setYqyrNUCAmount(getYQYRs(fpI, validationLevel));

  if (localValid || shouldPassCmdPricing)
    checkIntlSurfaceTvlLimitation(fpI);

  displayFarePathDiagnostics(localRes, fpPQI, fpI, localValid);

  if (UNLIKELY(shouldPassCmdPricing && passByCmdPricing))
  {
    fpI.pricingUnit().front()->setCmdPrcFailedFlag(
        RuleConst::COMBINABILITY_RULE); // may not accurate where, not matter
    _diag.enable(&fpI, Diagnostic610, Diagnostic620, Diagnostic690);
    if (_diag.isActive())
    {
      if (DiagnosticUtil::showItinInMipDiag(_trx, fpI.itin()->itinNum()))
      {
        _diag << " -- FP COMBINABILITY PASS BY COMMAND PRICING" << std::endl;
      }
    }

    localValid = true;
  }

  // We look for first winner and current solution is as Booked
  if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(_trx) && _trx.getRequest()->isLowFareRequested() &&
      !fpI.rebookClassesExists()))
  {
    fpI.rexStatus() = (localValid ? FarePath::REX_PASSED : FarePath::REX_FAILED);
  }

  if (!fallback::smpValidateFPPQItem(&_trx) && localValid)
  {
    MaximumPenaltyValidator penaltyValidator(_trx);

    localValid = penaltyValidator.validateFarePath(fpI).first;
  }

  if (localValid && _puPath->isSRFApplicable())
  {
    SRFEUtil::applySpanishResidentDiscount(_trx, fpI, *_puPath, false);

    DiagManager diag666(_trx, DiagnosticTypes::Diagnostic666);
    if (diag666.isActive())
    {
      Diag666Collector& dc = static_cast<Diag666Collector&>(diag666.collector());
      dc.printFarePathSolutions(fpI, _trx, *_puPath);
    }
  }
}

void
FPPQItemValidator::validateRules(bool& localValid,
                                 bool& shouldPassCmdPricing,
                                 FPPQItem* fpPQI,
                                 std::string& localRes,
                                 ValidationLevel validationLevel)
{
  if (localValid || shouldPassCmdPricing)
  {
    // We don't want to reuse results for similar itins - they may not be applicable
    const bool validatePULevel = !_factoriesConfig.puScopeValidationEnabled() &&
                                 validationLevel == ValidationLevel::NORMAL_VALIDATION;
    RuleValidator rulevalidator(_trx, _allPUF, _ruleController, _failedPricingUnits, _diag);
    const bool ruleValid = rulevalidator.validate(*fpPQI, validatePULevel);
    localRes += (ruleValid ? "RULE:P " : "RULE:F ");
    if (!ruleValid)
    {
      localValid = false;
      shouldPassCmdPricing = false; // if RuleController did not let this
      // cmd priced, it means that the failure was not cmd pricable
    }
  }
}

void
FPPQItemValidator::validateJourneys(bool& shouldPassCmdPricing,
                                    FPPQItem* fpPQI,
                                    const bool isFareX,
                                    std::string& localRes,
                                    bool& localValid,
                                    bool pricingAxess,
                                    FarePath& fpI)
{
  const bool journeyLogic = checkJourney();
  JourneyValidator journeyValidator(_trx, _diag);
  //------------------- Local and Flow Journey checks -------------
  if (localValid && !isFareX && !pricingAxess && journeyLogic)
  {
    localValid = journeyValidator.validateJourney(fpI, localRes);
  }

  RaiiRexFarePathPlusUps rfppu(_trx, fpI, localValid);

  if (localValid && !isFareX && !pricingAxess && rfppu.plusUpsNeeded() && !_trx.fxCnException())
  {
    TSELatencyData metrics(_trx, "PO BKGCODE MIXED CLASS");

    if (_trx.getTrxType() == PricingTrx::MIP_TRX && _trx.itin().size() > 1)
      localValid = validateBookingCodes(fpI);

    if (LIKELY(localValid))
    {
      MixedClassController mxc(_trx);

      uint32_t puFactIdx = 0;

      for (auto* pu : fpI.pricingUnit())
      {
        if (!mxc.validate(fpI, *pu))
        {
          localValid = false;
          saveFPScopeFailedPUIdx(*fpPQI, puFactIdx, mxc, localRes);
          break;
        }
        ++puFactIdx;
      }
    }

    if (localValid)
    {
      localValid = AltPricingUtil::validateFarePathBookingCode(_trx, fpI);

      if (UNLIKELY(_trx.excTrxType() == PricingTrx::PORT_EXC_TRX))
        applyDiffOverride(fpI);
    }

    localRes += (localValid ? "MIXCLASS:P " : "MIXCLASS:F ");

    if (!localValid)
      shouldPassCmdPricing = false;
  }

  //------------- Local and Flow Journey checks after Mixed class -------------
  if (localValid && !isFareX && !pricingAxess)
  {
    if (journeyLogic)
      localValid = journeyValidator.processJourneyAfterDifferential(fpI, localRes);

    if (localValid && journeyValidator.checkMarriedConnection())
    {
      localValid = journeyValidator.processMarriedConnection(fpI);
      localRes += (localValid ? "MARRIED CNX:P " : "MARRIED CNX:F ");
    }
  }
}

namespace
{
bool
isPreviousValidationCorrect(std::vector<std::vector<ClassOfService*>*>& cosVec,
                            std::vector<PaxTypeFare::SegmentStatus>& segStatus,
                            PricingTrx& trx,
                            const PaxTypeFare& paxTypeFare)
{
  if (cosVec.size() != segStatus.size())
    return false;

  const uint16_t numSeatsRequired = PaxTypeUtil::numSeatsForFare(trx, paxTypeFare);
  for (size_t index = 0; index < cosVec.size(); ++index)
  {
    const ClassOfService* cos =
        ShoppingUtil::getCOS(*cosVec[index], segStatus[index]._bkgCodeReBook);
    if (cos != nullptr && cos->numSeats() >= numSeatsRequired)
      return true;
  }

  return false;
}
}

bool
FPPQItemValidator::validateBookingCodes(FarePath& fpath)
{
  for (PricingUnit* pricingUnit : fpath.pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      FareMarket* mkt = fareUsage->paxTypeFare()->fareMarket();
      if (mkt->getMergedAvailability())
      {
        mkt->classOfServiceVec().clear();
        ShoppingUtil::getFMCOSBasedOnAvailBreak(_trx, fpath.itin(), mkt);

        if (isPreviousValidationCorrect(mkt->classOfServiceVec(),
                                        fareUsage->segmentStatus(),
                                        _trx,
                                        *fareUsage->paxTypeFare()))
          continue;
        for (PaxTypeFare::SegmentStatus& segSt : fareUsage->segmentStatus())
          segSt._bkgCodeSegStatus = PaxTypeFare::BKSS_NOT_YET_PROCESSED;

        FareBookingCodeValidator fbcv(_trx, *mkt, fpath.itin());

        if (!fbcv.validate(*fareUsage, &fpath))
          return false;
      }
    }
  }
  return true;
}

void
FPPQItemValidator::saveFPScopeFailedPUIdx(FPPQItem& fppqItem,
                                          const uint16_t puFactIdx,
                                          const MixedClassController& mcc,
                                          std::string& res)
{
  if (_trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    const PaxTypeFare* ptf = mcc.paxTfare();
    if (ptf && (mcc.failReason() == MixedClassController::FAIL_DIFF))
    {
      PricingUnitFactory* puf = _allPUF[puFactIdx];
      puf->failedFareSet().insert(ptf);
      res += "\nFARE ";
      res += ptf->fareClass().c_str();
      res += " FAILED MIXED CLASS VALIDATION\n";
    }
  }
  farepathutils::setFailedPricingUnits(puFactIdx, fppqItem, _failedPricingUnits);
}

bool
FPPQItemValidator::checkJourney()
{
  if (UNLIKELY(_trx.excTrxType() == PricingTrx::AR_EXC_TRX &&
      static_cast<const RexPricingTrx*>(&_trx)->trxPhase() != RexPricingTrx::PRICE_NEWITIN_PHASE))
    return false;

  if (UNLIKELY(!(_trx.getRequest()->isLowFareRequested())))
  {
    if (_trx.excTrxType() != PricingTrx::AR_EXC_TRX)
      return false;
  }

  if (!(_trx.getOptions()->applyJourneyLogic()))
    return false;

  if (_trx.getRequest()->isSFR())
  {
    return false;
  }

  if (UNLIKELY(_trx.getTrxType() == PricingTrx::PRICING_TRX))
  {
    return (_trx.getOptions()->journeyActivatedForPricing());
  }
  else if (LIKELY(_trx.getTrxType() == PricingTrx::MIP_TRX))
  {
    return (_trx.getOptions()->journeyActivatedForShopping());
  }

  return false;
}
void
FPPQItemValidator::applyDiffOverride(FarePath& fp)
{
  ExchangePricingTrx& excTrx = static_cast<ExchangePricingTrx&>(_trx);

  if (excTrx.exchangeOverrides().differentialOverride().empty())
    return;

  fp.plusUpFlag() = true;

  for (const auto* const differentialOverride : excTrx.exchangeOverrides().differentialOverride())
  {
    fp.plusUpAmount() += differentialOverride->amount();
    fp.increaseTotalNUCAmount(differentialOverride->amount());
  }
}

bool
FPPQItemValidator::validForWQ(FarePath& fpath)
{
  NoPNRPricingTrx* noPNRPricingTrx = dynamic_cast<NoPNRPricingTrx*>(&_trx);
  if (UNLIKELY(noPNRPricingTrx && noPNRPricingTrx->isNoMatch()))
  {
    const std::vector<PricingUnit*>& pricingUnits = fpath.pricingUnit();
    std::vector<PricingUnit*>::const_iterator iterPU = pricingUnits.begin();

    PaxTypeFare* paxTypeFare = ((*iterPU)->fareUsage()[0])->paxTypeFare();
    NoPNRPricingTrx::FareTypes::FTGroup group =
        noPNRPricingTrx->fareTypes().getFareTypeGroup(paxTypeFare->fcaFareType());
    ++iterPU;

    while (iterPU != pricingUnits.end())
    {
      paxTypeFare = ((*iterPU)->fareUsage()[0])->paxTypeFare();
      if (group != noPNRPricingTrx->fareTypes().getFareTypeGroup(paxTypeFare->fcaFareType()))
        return false;
      ++iterPU;
    }
  }
  return true;
}

bool
FPPQItemValidator::wpaHigherCabin(FarePath& fpath)
{
  const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(_trx);
  if (fcConfig == nullptr)
    return true;

  if (fcConfig->wpaNoMatchHigherCabinFare() != '2' || dynamic_cast<NoPNRPricingTrx*>(&_trx))
    return true;

  Itin& itinerary = *(fpath.itin());

  if (fpath.itin() != nullptr)
  {
    if (!ItinUtil::allFlightsBookedInSameCabin(itinerary))
      return true;
  }

  std::vector<CabinType> differentCabins;
  FareUsage* fu = nullptr;
  uint16_t tvlSegIndex = 0;

  for (TravelSeg* travelSeg : itinerary.travelSeg())
  {
    if (travelSeg == nullptr)
      continue;

    if (dynamic_cast<AirSeg*>(travelSeg) == nullptr)
      continue;

    fu = farepathutils::getFareUsage(fpath, travelSeg, tvlSegIndex);
    if (fu == nullptr)
      continue;

    if (tvlSegIndex > fu->travelSeg().size() - 1)
      continue;

    const PaxTypeFare::SegmentStatus& segStat = fu->segmentStatus()[tvlSegIndex];

    differentCabins.push_back(cabin(fu, travelSeg, segStat));
  }

  uint16_t sizeDifferentCabins = differentCabins.size();

  if (sizeDifferentCabins < 2)
    return true;

  for (uint16_t iDiffCabin = 0; iDiffCabin < sizeDifferentCabins; iDiffCabin++)
  {
    if (differentCabins[iDiffCabin] != differentCabins[0])
      return false;
  }

  return true;
}

CabinType
FPPQItemValidator::cabin(const FareUsage* fu,
                         const TravelSeg* tvlSeg,
                         const PaxTypeFare::SegmentStatus& fuSegStat)
{
  const DifferentialData* diff = farepathutils::differentialData(fu, tvlSeg);

  if (diff != nullptr)
  {
    const PaxTypeFare::SegmentStatus& diffSegStat = farepathutils::diffSegStatus(diff, tvlSeg);
    if (diffSegStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
        !(diffSegStat._bkgCodeReBook.empty()))
    {
      return diffSegStat._reBookCabin;
    }
    else
    {
      return tvlSeg->bookedCabin();
    }
  }

  if (fuSegStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
      !(fuSegStat._bkgCodeReBook.empty()))
  {
    return fuSegStat._reBookCabin;
  }

  return tvlSeg->bookedCabin();
}

void
FPPQItemValidator::circleTripsValidation(bool& localValid,
                                         FarePath& fpI,
                                         bool plusUpsNeeded,
                                         std::string& localRes)
{
  if (localValid && plusUpsNeeded)
  {
    TSELatencyData metrics(_trx, "PO RT TO CT CONVERSION");
    bool fCorrected = false;

    for (const auto pricingUnit : fpI.pricingUnit())
    {
      localValid = ensureCircleTripCorrectlyTyped(fpI, *pricingUnit, fCorrected);
    }

    if (fCorrected)
    {
      _diag.enable(&fpI, Diagnostic610);
      if (_diag.isActive())
      {
        if(DiagnosticUtil::showItinInMipDiag(_trx, fpI.itin()->itinNum()))
        {
          _diag << " ------------------  HIP APPLIED  ----------------" << std::endl;
          farepathutils::printFarePathToDiag(_trx, _diag, fpI, _puPath);
        }
      }
    }
    localRes += (localValid ? "REV103:P " : "REV103:F ");
  }
}

bool
FPPQItemValidator::ensureCircleTripCorrectlyTyped(FarePath& fpath,
                                                  PricingUnit& prU,
                                                  bool& fCorrected)
{
  bool ret = true;

  // IF the Pricing Unit is set to a "Round Trip" make sure the
  //    components are both "Round Trip" and have the same "HIP"
  //    ammounts.  If they do not, flag it as a "Circle Trip"
  if (prU.puType() == PricingUnit::Type::ROUNDTRIP)
  {
    // Check the number of segments
    const int l_SegCount = prU.fareUsage().size();

    if (LIKELY(l_SegCount == 2))
    {
      if (!PricingUnitLogic::shouldConvertRTtoCT(_trx, prU, fpath))
        return true;

      {
        // Change the type to a "Circle Trip"  and
        // "recombine" the Pricing Unit
        prU.puType() = PricingUnit::Type::CIRCLETRIP;

        // need to recombine
        TSELatencyData hipMetrics(_trx, "PO RT TO CT RECOMBINATION");

        FareUsage* failedFareUsage;
        FareUsage* failedTargetFareUsage;

        _diag.enable(&prU, Diagnostic605);
        if (_diag.isActive())
        {
          _diag << " HIP APPLIED: CONVERTED RT-PU TO CT-PU \n";
          _diag << " REQUESTED PAXTYPE: " << fpath.paxType()->paxType();
          if (fpath.paxType()->age() > 0)
            _diag << " AGE: " << fpath.paxType()->age();
          _diag << "   NUC AMOUNT: " << prU.getTotalPuNucAmount() << std::endl;
          _diag << prU << std::endl;
        }

        if (_combinations->process(
                prU, failedFareUsage, failedTargetFareUsage, _diag, fpath.itin()) != CVR_PASSED)
        {
          LOG4CXX_INFO(logger, "PricingUnit: COMBINATION VALIDATION FAILED")
          ret = false;
        }
        else
        {
          fCorrected = true;
        }
        _diag.printLine();

        if (ret)
          LOG4CXX_INFO(logger, "PricingUnit: COMBINATION VALIDATION PASSED")
      }
    }
  }

  return ret;
}
void
FPPQItemValidator::displayFarePathDiagnostics(std::string& localRes,
                                              FPPQItem* fpPQI,
                                              FarePath& fpI,
                                              bool& localValid)
{
  if (localValid)
  {
    _diag.enable(&fpI, Diagnostic660, Diagnostic690);
    if (UNLIKELY(_diag.isActive()))
    {
      _diag.printLine();
      _diag << fpI;
      _diag << "PASSED FARE PATH" << farepathutils::displayValCxr(fpI.validatingCarriers())
            << std::endl;
      if (fpPQI->priorityStatus().farePriority() > DEFAULT_PRIORITY)
        _diag << "FAREPATH PRIORITY: LOW\n";
    }
  }

  _diag.enable(&fpI, Diagnostic610, Diagnostic620);
  if (UNLIKELY(_diag.isActive()))
  {
    if(DiagnosticUtil::showItinInMipDiag(_trx, fpI.itin()->itinNum()))
    {
      if (localValid)
      {
        if (fpI.plusUpFlag())
        {
          _diag << "PLUS-UP ADDED" << farepathutils::displayValCxr(fpI.validatingCarriers())
                << "   AMOUNT: " << fpI.getTotalNUCAmount() << std::endl;
        }
        _diag << "PASSED FARE PATH" << farepathutils::displayValCxr(fpI.validatingCarriers())
              << std::endl;
        if (fpPQI->priorityStatus().farePriority() > DEFAULT_PRIORITY)
          _diag << "FAREPATH PRIORITY: LOW" << farepathutils::displayValCxr(fpI.validatingCarriers())
                << std::endl;
      }
      else
      {
        _diag << localRes << std::endl;
        _diag << "FAILED FARE PATH" << farepathutils::displayValCxr(fpI.validatingCarriers())
              << std::endl;
      }
    }
  }

  if (UNLIKELY(_diag.diagnosticType() == Diagnostic556 && farepathutils::journeyDiag(_trx, fpI, _diag)))
  {
    _diag.enable(Diagnostic556);
    if (_diag.isActive())
    {
      _diag << "------------------------------------------------------------ \n";
      if (fpI.plusUpFlag())
        _diag << "PLUS-UP ADDED \n";

      if (localValid)
      {
        _diag << localRes << " \nPASSED FARE PATH \n";
      }
      else
      {
        _diag << localRes << " \n";
        _diag << "FAILED FARE PATH \n";
      }
      _diag << "*************** END  JOURNEY  diag ************************ \n";
      _diag.flushMsg();
    }
  }
}

MoneyAmount
FPPQItemValidator::getYQYRs(FarePath& farePath, ValidationLevel level)
{
  if (TrxUtil::isTotalPriceEnabled(_trx) && level == ValidationLevel::NORMAL_VALIDATION)
    _yqyrCalc = farePath.itin()->yqyrLBFactory()->getYQYRPreCalc(_puPath->fareMarketPath(),
                                                                 farePath.paxType());

  farePath.setYqyrCalculator(_yqyrCalc);

  if (!_yqyrCalc)
    return 0.0;

  const TSELatencyData m(_trx, "YQYR");
  const std::vector<CarrierCode>& validatingCarriers = farePath.validatingCarriers();

  if (validatingCarriers.empty())
    return _yqyrCalc->chargeFarePath(farePath, farePath.itin()->validatingCarrier());

  MoneyAmount minCharge(_yqyrCalc->chargeFarePath(farePath, validatingCarriers.front()));
  for (auto cxrIt = validatingCarriers.cbegin() + 1; cxrIt != validatingCarriers.cend(); ++cxrIt)
    minCharge = std::min(_yqyrCalc->chargeFarePath(farePath, *cxrIt), minCharge);

  return minCharge;
}

bool
FPPQItemValidator::checkMinFare(FarePath& fpath, std::string& _results) const
{
  TSELatencyData metrics(_trx, "PO MINFARE PROCESS");
  bool ret = true;
  if (!fpath.minFareCheckDone())
  {
    MinFareChecker minFareChecker;
    ret = minFareChecker.process(_trx, fpath);
  }

  _results += (ret ? "MIN-FARE:P " : "MIN-FARE:F ");
  return ret;
}
bool
FPPQItemValidator::isItemLessThanTopItem(const FPPQItem* topFppqItem,
                                         const FPPQItem& fppqItem,
                                         bool needValidate)
{
  if (!needValidate || !topFppqItem || !topFppqItem->farePath()->processed() ||
      fppqItem.farePath()->rebookClassesExists() != topFppqItem->farePath()->rebookClassesExists())
  {
    return true;
  }

  if (_factoriesConfig.searchAlwaysLowToHigh())
  {
    FPPQItem::GreaterLowToHigh<FPPQItem::GreaterFare> greater;
    return greater(topFppqItem, &fppqItem);
  }

  FPPQItem::Greater<FPPQItem::GreaterFare> greater;
  return greater(topFppqItem, &fppqItem);
}

bool
FPPQItemValidator::validateBookingCodesCat31(FarePath& farePath) const
{
  DiagManager diagManager(_trx, Diagnostic411);
  DiagCollector* diag = diagManager.isActive() ? &diagManager.collector() : nullptr;

  if (diag)
  {
    diag->lineSkip(0);
    (*diag) << farePath;
    diag->lineSkip(1);
  }

  for (PricingUnit* pricingUnit : farePath.pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      if (fareUsage->bookingCodeStatus().isSet(PaxTypeFare::BKS_NEED_FARE_PATH))
      {
        FareBookingCodeValidator(_trx, *fareUsage->paxTypeFare()->fareMarket(), farePath.itin())
            .validate(*fareUsage, &farePath);

        if (!fareUsage->bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS))
        {
          return false;
        }
      }
    }
  }
  return true;
}
namespace
{
bool
existsLocalFareMarketForWP(const std::vector<PaxTypeFare::SegmentStatus>& segmentStatus)
{
  return std::any_of(
      segmentStatus.cbegin(),
      segmentStatus.cend(),
      [](const PaxTypeFare::SegmentStatus& ss)
      { return ss._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REX_WP_LOCALMARKET_VALIDATION); });
}
}

void
FPPQItemValidator::resetToRebook(FarePath& fp, FPPQItem& fppqItem) const
{
  fp.rebookClassesExists() = true;

  for (PricingUnit* pricingUnit : fp.pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      PaxTypeFare& paxTypeFare = *fareUsage->paxTypeFare();
      if (paxTypeFare.bkgCodeTypeForRex() == PaxTypeFare::BKSS_PASS &&
          existsLocalFareMarketForWP(paxTypeFare.segmentStatus()))
        paxTypeFare.bkgCodeTypeForRex() = PaxTypeFare::BKSS_REBOOKED;
    }
  }

  for (PUPQItem* pupqItemPtr : fppqItem.pupqItemVect())
  {
    PUPQItem& pupqItem = *pupqItemPtr;
    pupqItem.rebookClassesExists() = pupqItem.pricingUnit()->isRebookedClassesStatus();
  }
}

bool
FPPQItemValidator::isSameFarePathValidForRebook(FarePath& fp) const
{
  for (const PricingUnit* pricingUnit : fp.pricingUnit())
  {
    for (const FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      const PaxTypeFare& paxTypeFare = *fareUsage->paxTypeFare();
      if (paxTypeFare.bkgCodeTypeForRex() == PaxTypeFare::BKSS_PASS &&
          existsLocalFareMarketForWP(paxTypeFare.segmentStatus()))
        return true;
    }
  }
  return false;
}

bool
FPPQItemValidator::callRefundValidation(std::string& results, FarePath& fpath)
{
  TSELatencyData metrics(_trx, "PO CAT33 VALIDATION");

  RefundSolutionValidator refundSolutionValidator(static_cast<RefundPricingTrx&>(_trx), fpath);
  const bool valid = refundSolutionValidator.process();
  results += valid ? "REFUND:P " : "REFUND:F ";
  return valid;
}

inline bool
FPPQItemValidator::calcBaggageCharge(FarePath& fp)
{
  if (_savedBaggageCharge.processed())
  {
    fp.setBagChargeNUCAmount(_savedBaggageCharge.amount());
    return _savedBaggageCharge.valid();
  }

  if (_bagCalculator)
  {
    TSELatencyData metrics(_trx, "BAG IN PQ");

    if (_bagCalculator->chargeFarePath(fp))
      _savedBaggageCharge.setAmount(fp.bagChargeNUCAmount());
    else
      _savedBaggageCharge.setInvalid();
    return _savedBaggageCharge.valid();
  }

  // The part below is for reference only and will be removed when the project is completed
  if (UNLIKELY(baggageBruteForce.isValid(&_trx)))
  {
    TSELatencyData metrics(_trx, "BAG IN PQ");

    FreeBagService* fbs =
        static_cast<FreeBagService*>(TseServer::getInstance()->service("FREE_BAG_SVC"));
    TSE_ASSERT(fbs);
    if (fbs->processBagInPqBruteForce(_trx, fp))
      _savedBaggageCharge.setAmount(fp.bagChargeNUCAmount());
    else
      _savedBaggageCharge.setInvalid();
    return _savedBaggageCharge.valid();
  }

  return true;
}

} // tse namespace
