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

#include "Pricing/FarePathValidator.h"
#include "BookingCode/Cat31FareBookingCodeValidator.h"
#include "BookingCode/FareBookingCodeValidator.h"
#include "BookingCode/MixedClassController.h"
#include "Common/AltPricingUtil.h"
#include "Common/BookingCodeUtil.h"
#include "Common/BrandingUtil.h"
#include "Common/ClassOfService.h"
#include "Common/FallbackUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/ItinUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TSELatencyData.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "Common/ValidatingCxrUtil.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Diagnostic/DiagManager.h"
#include "Fares/FareByRuleValidator.h"
#include "Fares/FareTypeMatcher.h"
#include "Limitations/LimitationOnIndirectTravel.h"
#include "Pricing/Combinations.h"
#include "Pricing/MaximumPenaltyValidator.h"
#include "Pricing/NegotiatedFareCombinationValidator.h"
#include "Pricing/FarePathFactory.h"
#include "Pricing/FarePathUtils.h"
#include "Pricing/PaxFarePathFactory.h"
#include "Pricing/PaxFPFBaseData.h"
#include "Pricing/PricingUnitFactory.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/PUPath.h"
#include "RexPricing/RexPaxTypeFareValidator.h"
#include "Rules/FarePUResultContainer.h"
#include "Rules/PricingUnitRuleController.h"
#include "Rules/PricingUnitLogic.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"

namespace tse
{

static Logger
logger("atseintl.Pricing.FarePathValidator");

FarePathValidator::FarePathValidator(PaxFPFBaseData& paxFPFBaseData)
  : _trx(*paxFPFBaseData.trx()),
    _factoriesConfig(paxFPFBaseData.getFactoriesConfig()),
    _combinations(paxFPFBaseData.combinations()),
    _paxFPFBaseData(paxFPFBaseData)
{
}

bool
FarePathValidator::validateRexPaxTypeFare(DiagCollector& diag, FarePath& fpath)
{
  bool valid = true;
  if (UNLIKELY(_trx.excTrxType() == PricingTrx::AR_EXC_TRX &&
      static_cast<RexPricingTrx&>(_trx).trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE))
  {
    RexPaxTypeFareValidator ptfValidator(static_cast<RexPricingTrx&>(_trx), &diag);
    valid = ptfValidator.validate(fpath);
    if (static_cast<RexPricingTrx&>(_trx).applyReissueExchange())
    {
      std::string roeUse;
      if (fpath.useSecondRoeDate())
        roeUse = "SECOND ROE - " +
                 static_cast<RexPricingTrx&>(_trx)
                     .newItinSecondROEConversionDate()
                     .toIsoExtendedString();
      else
        roeUse = "FIRST ROE - " +
                 static_cast<RexPricingTrx&>(_trx).newItinROEConversionDate().toIsoExtendedString();
      diag << roeUse << std::endl;
    }
  }
  return valid;
}

bool
FarePathValidator::failFareRetailerCodeMismatch(const FarePath& farePath, DiagCollector& diag)
{
  bool rc = false;
  if (!RuleUtil::isFarePathValidForRetailerCode(_trx, farePath))
  {
    _results += "FAILED: FARE RETAILER RULE CODE SOURCE PCC MISMATCH";
    rc = true;
  }

  if (UNLIKELY(rc && (PricingTrx::IS_TRX == _trx.getTrxType())))
  {
    diag.enable(Diagnostic610);
    diag << "\n" << farePath << "\n" << _results << "\n";
    diag.printLine();
  }

  return rc;
}

bool
FarePathValidator::validate(FPPQItem& fppqItem, DiagCollector& diag, bool pricingAxess)
{
  _puPath = fppqItem.puPath();
  _itin = fppqItem.farePath()->itin();

  if (UNLIKELY(failMixedDateForRefund(*fppqItem.farePath())))
    return false;

  FarePath& fpath = *fppqItem.farePath();

  if (UNLIKELY(failFareRetailerCodeMismatch(fpath, diag)))
    return false;

  if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(_trx)))
  {
    if (fpath.rexStatus() != FarePath::REX_NOT_PROCESSED)
      return fpath.rexStatus() == FarePath::REX_PASSED;
  }
  _results.clear();
  bool mtStLinkAdded = false;

  exchangeReissueForQrex(fpath);
  ROEDateSetter roeDateSetter(_trx, fpath);
  roeDateSetter.reCalculateNucAmounts();

  if (UNLIKELY(farepathutils::putDiagMsg(_trx, fppqItem, diag, _paxFPFBaseData.fpCombTried())))
  {
    // if diag trx, Link from MainTrip to SideTrip is established
    //
    mtStLinkAdded = true;
  }
  bool valid = validateRexPaxTypeFare(diag, fpath);

  if (LIKELY(valid))
    valid = isValidFPathForValidatingCxr(fpath, diag);

  //-------------- check PREV FP Rule Validation result here -------------
  //
  if (valid)
  {
    std::vector<CarrierCode> failedValCxr;
    std::string failedCxrMsg;
    valid = checkFailedPUIdx(fppqItem, failedValCxr, failedCxrMsg);
    if (valid && !failedValCxr.empty())
    {
      if (!ValidatingCxrUtil::getValCxrSetDifference(
              fppqItem.farePath()->validatingCarriers(), failedValCxr, &_trx))
      {
        LOG4CXX_DEBUG(logger,
                      __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__
                               << " Should not happen:" << failedCxrMsg)
      }
    }

    if (!valid || !failedValCxr.empty())
    {
      diag.enable(&fpath, Diagnostic610, Diagnostic620);
      if (UNLIKELY(diag.isActive()))
      {
        if (DiagnosticUtil::showItinInMipDiag(_trx, fppqItem.farePath()->itin()->itinNum()))
        {
          diag << ("FAILED: PREV FP RULE-MIXCLASS VALIDATION" + failedCxrMsg) << " \n";
        }
      }
    }
  }

  //------------ check for at least one HARD_PASS for IBF here ----------
  if (UNLIKELY(_trx.getRequest()->isBrandedFaresRequest()))
  {
    if (valid)
    {
      BrandCode brandCode = _puPath->getBrandCode();
      if (brandCode == ANY_BRAND_LEG_PARITY)
      {
        valid = brandParityOnEachLegCheck(fpath);
        _results += (valid ? "" : "FAILED: IBF BRAND PARITY ON EACH LEG VALIDATION");
      }
      else if (brandCode != NO_BRAND)
      {
        valid = hardPassOnEachLegCheckIBF(fpath);
        _results += (valid ? "" : "FAILED: IBF HARD PASS ON EACH LEG VALIDATION");
      }
    }
  }
  //------------ check for at least one HARD_PASS for PBB here ----------
  if (_trx.isPbbRequest() == PBB_RQ_PROCESS_BRANDS)
  {
    if (valid)
    {
      valid = hardPassOnEachLegCheckPBB(fpath);
      _results += (valid ? "" : "FAILED: IBF HARD PASS ON EACH LEG VALIDATION");
    }
  }

  //------------ check FareTypePricing EndOnEnd and FT-Required ----------
  //
  if (UNLIKELY(valid && _trx.getOptions()->isFareFamilyType()))
  {
    FareTypeMatcher ftMatcher(_trx, fppqItem.farePath());
    valid = ftMatcher(fppqItem.farePath());
    if (!valid)
      _results += ftMatcher.getMessage();
  }

  //------------ check PREV EOE result -----------
  //
  if (valid)
  {
    valid = checkEOEFailedFare(fppqItem);
    _results += (valid ? "" : "FAILED: PREV EOE VALIDATION");
  }

  if (LIKELY(!mtStLinkAdded))
  {
    // PU-Business rules, combinability needs MainTrip, SideTrip info

    farepathutils::addMainTripSideTripLink(fppqItem);
  }

  //----------------- do PU-Business rules check here ----------------
  // check PU-Business rules which need FP-scope and
  // could not be done in PU-Scope
  //
  if (valid)
  {
    valid = checkFinalPUBusinessRule(fpath, diag);
    if (!valid)
      _results += "FPSCOPE-PU:F ";
  }

  //----------------- check Limitions of PricingUnit -------------
  if (valid && !pricingAxess)
  {
    TSELatencyData metrics(_trx, "PO LIMITATION");
    LimitationOnIndirectTravel limitation(_trx, *fpath.itin());
    valid = limitation.validateFarePath(fpath, diag);
  }

  //------------ Command Pricing Flags ----------
  //
  _shouldPassCmdPricing = false;
  bool isFareX = (_trx.getOptions() && _trx.getOptions()->fareX());

  if (valid)
  {
    if (UNLIKELY(isFareX))
    {
      _shouldPassCmdPricing = true;
    }
    else
    {
      for (const auto pu : fpath.pricingUnit())
      {
        if (UNLIKELY(pu->isCmdPricing()))
        {
          _shouldPassCmdPricing = true;
          break;
        }
      }
    }
  }

  //----------------- do EOE check here ----------------
  if (valid && !isFareX)
  {
    FareUsage* fakeFareUsage = nullptr;
    FareUsage* realFareUsage = nullptr;
    PaxTypeFare* realPTF = nullptr;
    uint16_t fakeFareIndex = 0;
    uint16_t fakePUIndex = 0;
    if (LIKELY(!_trx.getRequest()->originBasedRTPricing() ||
        (originBasedSpecialCond(fppqItem) &&
         (swapRealToFake(
             fppqItem, fakeFareUsage, realFareUsage, realPTF, fakeFareIndex, fakePUIndex)))))
    {
      valid = checkCombinability(fppqItem, diag);
    }
    if (UNLIKELY(fakeFareUsage && realFareUsage))
    {
      swapBackToFake(fppqItem, fakeFareUsage, realFareUsage, realPTF, fakeFareIndex, fakePUIndex);
    }

    _results += (valid ? "COMB:P " : "COMB:F ");
  }

  //----------------- do Cat25 Same Rule/Tariff check here ----------------
  if (valid && !isFareX)
  {
    valid = FareByRuleValidator::checkSameTariffRule(fpath);

    _results += (valid ? "" : "FBR-TARIFF:F ");
  }

  //----------------- do Negotiated Fare Combination check here ----------------
  if (valid && !_shouldPassCmdPricing)
  {
    NegotiatedFareCombinationValidator combinationValidator(_trx);

    valid = combinationValidator.validate(fpath);

    _results += (valid ? "NEG-COMB:P " : "NEG-COMB:F ");
  }

  return valid;
}

bool
FarePathValidator::originBasedSpecialCond(FPPQItem& fppqItem)
{
  FarePath& fpath = *fppqItem.farePath();
  if (fpath.pricingUnit().size() > 2)
  {
    return true;
  }
  std::vector<PricingUnit*>::iterator puIt = fpath.pricingUnit().begin();
  const std::vector<PricingUnit*>::iterator puItEnd = fpath.pricingUnit().end();
  bool foundRealFareAcrossPU = false;
  bool foundFirstRealFare = false;
  for (uint32_t puFactIdx = 0; puIt != puItEnd; ++puIt, ++puFactIdx)
  {
    if (((*puIt)->fareUsage().size() == 1) &&
        (*puIt)->fareUsage()[0]->paxTypeFare()->fareMarket()->useDummyFare())
    {
      continue;
    }
    PUPQItem::PUValidationStatus status = getPUScopeCat10Status(fppqItem, puFactIdx);
    if ((status != PUPQItem::PUValidationStatus::FAILED) &&
        (status != PUPQItem::PUValidationStatus::PASSED))
    {
      return true;
    }
    for (FareUsage* fareUsage : (*puIt)->fareUsage())
    {
      const PaxTypeFare* sourcePaxTypeFare = fareUsage->paxTypeFare();
      foundRealFareAcrossPU = !(sourcePaxTypeFare->fareMarket()->useDummyFare());
      if (foundRealFareAcrossPU)
      {
        if (foundFirstRealFare)
        {
          return true;
        }
        else
        {
          foundFirstRealFare = true;
          break;
        }
      }
    }
  }
  return false;
}

void
FarePathValidator::swapBackToFake(FPPQItem& fppqItem,
                                  FareUsage*& fakeFareUsage,
                                  FareUsage*& realFareUsage,
                                  PaxTypeFare*& realPTF,
                                  const uint16_t& fakeFareIndex,
                                  const uint16_t& fakePUIndex)
{
  FarePath& fpath = *fppqItem.farePath();
  fpath.pricingUnit()[fakePUIndex]->fareUsage()[fakeFareIndex] = fakeFareUsage;
  realFareUsage->paxTypeFare() = realPTF;
}

bool
FarePathValidator::swapRealToFake(FPPQItem& fppqItem,
                                  FareUsage*& fakeFareUsage,
                                  FareUsage*& realFareUsage,
                                  PaxTypeFare*& realPTF,
                                  uint16_t& fakeFareIndex,
                                  uint16_t& fakePUIndex)
{
  FarePath& fpath = *fppqItem.farePath();
  std::vector<PricingUnit*>::iterator puIt = fpath.pricingUnit().begin();
  const std::vector<PricingUnit*>::iterator puItEnd = fpath.pricingUnit().end();
  for (uint32_t puFactIdx = 0; puIt != puItEnd; ++puIt, ++puFactIdx)
  {
    PricingUnit& prU = **puIt;
    std::vector<FareUsage*>::iterator it = prU.fareUsage().begin();
    const std::vector<FareUsage*>::iterator itEnd = prU.fareUsage().end();
    for (; it != itEnd; ++it)
    {
      PaxTypeFare* ptf = (*it)->paxTypeFare();
      if (ptf->fareMarket()->useDummyFare())
      {
        fakePUIndex = puFactIdx;
        fakeFareUsage = *it;
        fakeFareIndex = it - prU.fareUsage().begin();
      }
      else
      {
        realFareUsage = *it;
        realPTF = ptf;
      }
      if (fakeFareUsage && realPTF)
        break;
    }
    if (fakeFareUsage && realPTF)
      break;
  }
  if (fakeFareUsage && realPTF)
    fpath.pricingUnit()[fakePUIndex]->fareUsage()[fakeFareIndex] = realFareUsage;

  return true;
}

bool
FarePathValidator::checkFinalPUBusinessRule(FarePath& fpath, DiagCollector& diag)
{
  bool valid = checkContinuousNLSPFareLoop(fpath, diag);

  if (valid)
  {
    valid = checkNLSPIntlOJToOW(fpath, diag);
  }
  if (valid)
  {
    valid = checkSideTripRestriction(fpath.pricingUnit(), diag);
  }

  if (valid && _isEoeCombinabilityEnabled)
  {
    valid = checkTag1Tag3FareInABAItin(fpath, diag);
  }

  return valid;
}

bool
FarePathValidator::checkContinuousNLSPFareLoop(const PricingUnit::PUFareType fareType,
                                               std::vector<PricingUnit*>& puVect,
                                               const uint8_t stNumber,
                                               DiagCollector& diag)
{
  Itin& itinerary = *_itin;
  if (itinerary.geoTravelType() == GeoTravelType::Domestic || itinerary.geoTravelType() == GeoTravelType::Transborder)
  {
    // Domestic, Transborder Itin
    return true;
  }

  // First check if there is at least one Intl or F. Domestic OW PU
  //
  bool owPUexist = false;
  for (PricingUnit* pricingUnit : puVect)
  {
    PricingUnit& pu = *pricingUnit;

    if ((stNumber == 0) && pu.isSideTripPU())
      break; // main trip PU ended
    if (stNumber != pu.sideTripNumber())
      continue;

    if (pu.puType() == PricingUnit::Type::ONEWAY &&
        (pu.geoTravelType() == GeoTravelType::International || pu.geoTravelType() == GeoTravelType::ForeignDomestic) &&
        pu.puFareType() == fareType)
    {
      const PaxTypeFare& ptf = *pu.fareUsage().front()->paxTypeFare();
      const FareMarket& fm = *ptf.fareMarket();
      if (UNLIKELY(fm.origin()->nation() == JAPAN &&
          (fm.destination()->nation() == JAPAN || ptf.directionality() == FROM) &&
          fareType == PricingUnit::SP && ptf.isSpecial()))
      {
        // OW PU originated (OutBound, From) in Japan, with SP-Fare
        return true;
      }
      owPUexist = true;
      break;
    }
  }

  if (!owPUexist)
  {
    return true;
  }

  //-------- First find all the NL-Fare (SP-Fare) FareMarket and put
  //-------- them in the order of travel (These are out
  //-------- of order when we build PU)
  //
  std::map<uint16_t, FareMarket*> sortedFareMarket;

  for (PricingUnit* pricingUnitPtr : puVect)
  {
    PricingUnit& pu = *pricingUnitPtr;

    if ((stNumber == 0) && pu.isSideTripPU())
      break; // main trip PU ended
    if (stNumber != pu.sideTripNumber())
      continue;
    for (FareUsage* fareUsagePtr : pu.fareUsage())
    {
      FareUsage& fareUsage = *fareUsagePtr;
      if (fareUsage.hasSideTrip())
      {
        uint8_t stCount = fareUsage.sideTripPUs().back()->sideTripNumber();
        for (uint8_t stN = 1; stN <= stCount; ++stN)
        {
          if (!checkContinuousNLSPFareLoop(fareType, fareUsage.sideTripPUs(), stN, diag))
          {
            // we don't have side-trip out of the side-trip
            LOG4CXX_INFO(logger, "SideTrip Invalid: OW PU in Continuous NL/SP Fare")
            if (diag.isActive())
            {
              diag << " INVALID ST: OW-PU IN CONTINUOUS-TRAVEL WITH ";
              if (fareType == PricingUnit::NL)
                diag << "NL FARE\n";
              else
                diag << "SP FARE\n";
            }
            return false;
          }
        }
      }

      const bool spFare = fareUsage.paxTypeFare()->isSpecial();

      if (fareType == PricingUnit::NL && spFare)
      {
        continue;
      }
      else if (UNLIKELY(fareType == PricingUnit::SP && !spFare))
      {
        continue;
      }

      FareMarket* fm = fareUsage.paxTypeFare()->fareMarket();

      const uint16_t pos = itinerary.segmentOrder(fm->travelSeg().front());
      sortedFareMarket.insert(std::map<uint16_t, FareMarket*>::value_type(pos, fm));
    }
  }

  //-------- Now see if the FareMarket create a continuous  travel ---------
  //
  if (sortedFareMarket.size() > 1)
  {
    if (createsLoop(sortedFareMarket))
    {
      if (diag.isActive())
      {
        diag << " INVALID: OW-PU IN CONTINUOUS-TRAVEL WITH ";
        if (fareType == PricingUnit::NL)
          diag << "NL FARE\n";
        else
          diag << "SP FARE\n";
      }
      return false;
    }
  }
  return true;
}

bool
FarePathValidator::createsLoop(std::map<uint16_t, FareMarket*>& sortedFareMarket)
{
  std::map<uint16_t, FareMarket*>::iterator it = sortedFareMarket.begin();
  const std::map<uint16_t, FareMarket*>::iterator itEnd = sortedFareMarket.end();
  FareMarket* fm = it->second;
  // LOG4CXX_INFO(logger, it->first << " " << fm->boardMultiCity()
  //                                << "--" << fm->offMultiCity())

  const Loc& orig = *(fm->origin());
  const LocCode& origCity = fm->boardMultiCity();
  const Loc* dest = fm->destination();
  LocCode destCity = fm->offMultiCity();
  ++it;
  for (; it != itEnd; ++it)
  {
    fm = it->second;
    // LOG4CXX_INFO(logger, it->first << " " << fm->boardMultiCity()
    //                                << "--" << fm->offMultiCity())
    if (LocUtil::isSamePoint(*dest, destCity, *fm->origin(), fm->boardMultiCity()))
    {
      dest = fm->destination();
      destCity = fm->offMultiCity();
      // LOG4CXX_INFO(logger, "Continuous ")
    }
    else
    {
      // LOG4CXX_INFO(logger, "Not Continuous ")
      return false;
    }
  }

  if (LocUtil::isSamePoint(orig, origCity, *fm->destination(), fm->offMultiCity()))
  {
    // LOG4CXX_INFO(logger, "Continuous ")
    return true;
  }

  return false;
}

bool
FarePathValidator::checkNLSPIntlOJToOW(const FarePath& fpath, DiagCollector& diag)
{
  std::vector<PricingUnit*>::const_iterator it(fpath.pricingUnit().begin());
  const std::vector<PricingUnit*>::const_iterator itEnd(fpath.pricingUnit().end());
  for (; it != itEnd; ++it)
  {
    const PricingUnit* pu(*it);
    if (!pu->intlOJToOW().empty() && PricingUnit::NL == pu->puFareType())
    {
      std::vector<PricingUnit*>::const_iterator it2(it + 1);
      for (; it2 != itEnd; ++it2)
      {
        const PricingUnit* pu2(*it2);
        if (!pu2->intlOJToOW().empty() && PricingUnit::NL == pu2->puFareType())
        {
          std::set<PU*> intersection;
          const std::set<PU*>& first(pu->intlOJToOW());
          const std::set<PU*>& second(pu2->intlOJToOW());
          std::set_intersection(first.begin(),
                                first.end(),
                                second.begin(),
                                second.end(),
                                std::inserter(intersection, intersection.begin()));
          if (!intersection.empty())
          {
            // std::cerr << __FUNCTION__ << std::endl;
            if (diag.isActive())
            {
              diag << " INVALID: BOTH OW-PU from OJ WITH NL FARE\n";
            }
            return false;
          }
        }
      }
    }
  }
  return true;
}

bool
FarePathValidator::checkSideTripRestriction(std::vector<PricingUnit*>& puVect, DiagCollector& diag)
{
  Itin& itinerary = *_itin;
  if (itinerary.geoTravelType() == GeoTravelType::Domestic || itinerary.geoTravelType() == GeoTravelType::Transborder)
  {
    // Domestic, Transborder Itin
    return true;
  }

  std::vector<PricingUnit*>::iterator i = puVect.begin();
  const std::vector<PricingUnit*>::iterator iEnd = puVect.end();
  for (; i != iEnd; ++i)
  {
    PricingUnit& pu = *(*i);

    if (pu.isSideTripPU())
    {
      if (UNLIKELY(checkExtFareInSideTripPU(pu, diag) == false))
      {
        return false;
      }
      continue;
    }

    if (pu.hasSideTrip() == false)
    {
      continue;
    }

    const Loc& itinOrigin = *itinerary.travelSeg().front()->origin();
    std::vector<FareUsage*>::iterator j = pu.fareUsage().begin();
    const std::vector<FareUsage*>::iterator jEnd = pu.fareUsage().end();
    for (; j != jEnd; ++j)
    {
      FareUsage& fareUsage = *(*j);
      if (fareUsage.hasSideTrip())
      {
        if (UNLIKELY((pu.puType() == PricingUnit::Type::ONEWAY && pu.puFareType() == PricingUnit::NL) &&
            !checkToOrViaCountryOfCommencement(itinOrigin, fareUsage)))
        {
          LOG4CXX_INFO(logger, "SideTrip Invalid: OW-ST PU To or Via country of commencement")
          if (diag.isActive())
            diag << " INVALID ST: TO OR VIA COUNTRY OF COMMENCEMENT";
          return false;
        }
      }
    }
  }

  return true;
}

bool
FarePathValidator::checkExtFareInSideTripPU(const PricingUnit& pu, DiagCollector& diag)
{
  std::vector<FareUsage*>::const_iterator it = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator itEnd = pu.fareUsage().end();
  for (; it != itEnd; ++it)
  {
    const PaxTypeFare& paxTypeFare = *(*it)->paxTypeFare();
    if (UNLIKELY((paxTypeFare.vendor() == SITA_VENDOR_CODE) && (paxTypeFare.tcrRuleTariff() == Tariff_8) &&
        (paxTypeFare.ruleNumber() == Rule_T225 || paxTypeFare.ruleNumber() == Rule_T226 ||
         paxTypeFare.ruleNumber() == Rule_T227)))
    {
      if (diag.isActive())
        diag << "FAILED: EXT FARE NOT ALLOWED FOR SIDE TRIP\n";
      return false;
    }
  }

  return true;
}
bool
FarePathValidator::checkToOrViaCountryOfCommencement(const Loc& itinOrigin,
                                                     const FareUsage& mtFareUsage)
{
  std::vector<PricingUnit*>::const_iterator it = mtFareUsage.sideTripPUs().begin();
  const std::vector<PricingUnit*>::const_iterator itEnd = mtFareUsage.sideTripPUs().end();
  for (; it != itEnd; ++it)
  {
    PricingUnit& pu = *(*it);

    if (pu.puType() != PricingUnit::Type::ONEWAY)
    {
      continue;
    }

    if (UNLIKELY(LocUtil::isWithinSameCountry(_itin->geoTravelType(),
                                     _puPath->itinWithinScandinavia(),
                                     itinOrigin,
                                     *pu.travelSeg().front()->origin())))
    {
      // This side trip started from the country of Itin origin
      // not started from another country
      continue;
    }

    std::vector<TravelSeg*>::iterator j = pu.travelSeg().begin();
    const std::vector<TravelSeg*>::iterator jE = pu.travelSeg().end();
    for (; j != jE; ++j)
    {
      const TravelSeg* tvlSeg = *j;

      if (LocUtil::isWithinSameCountry(_itin->geoTravelType(),
                                       _puPath->itinWithinScandinavia(),
                                       itinOrigin,
                                       *tvlSeg->origin()) ||
          LocUtil::isWithinSameCountry(_itin->geoTravelType(),
                                       _puPath->itinWithinScandinavia(),
                                       itinOrigin,
                                       *tvlSeg->destination()))
      {
        // To or Via Country of Commencement of Transportation
        return false;
      }
    }
  }

  return true;
}

bool
FarePathValidator::checkTag1Tag3FareInABAItin(const FarePath& fpath, DiagCollector& diag) const
{
  if (!_puPath->abaTripWithOWPU())
    return true;

  const PricingUnit& pu1 = *fpath.pricingUnit().front();
  const PricingUnit& pu2 = *fpath.pricingUnit().back();

  if (isAnyTag2(pu1, pu2))
    return true;

  if (isTag1Tag1(pu1, pu2))
  {
    if (pu1.geoTravelType() != GeoTravelType::International && pu1.geoTravelType() != GeoTravelType::ForeignDomestic)
    {
      // T1 + T1 are allowed in Domestic and Transboarder
      return true;
    }

    // Fail T1 + T1 for International and ForeignDomestic A-B-A with OW + OW
    if (diag.isActive())
    {
      diag << " FAILED: TAG1-TAG1 COMBINATION\n";
    }
    return false;
  }

  if (isTag3Tag3(pu1, pu2))
  {
    if (diag.isActive())
    {
      diag << " FAILED: TAG3-TAG3 COMBINATION\n";
    }
    return false;
  }

  // To get here, we have T1 + T3
  if (pu1.puFareType() == pu2.puFareType())
  {
    // all NL or SP combination
    if (diag.isActive())
    {
      diag << "FAILED: TAG1-TAG3 All NL OR SP \n";
    }
    return false;
  }

  if (!checkTag1Tag3CarrierPreference(pu1, pu2))
  {
    if (diag.isActive())
    {
      diag << "FAILED: TAG1-TAG3 CXR PREF \n";
    }
    return false;
  }
  return true;
}

bool
FarePathValidator::checkTag1Tag3CarrierPreference(const PricingUnit& prU1, const PricingUnit& prU2)
    const
{
  const CarrierPreference* cxrPref1 =
      prU1.fareUsage().front()->paxTypeFare()->fareMarket()->governingCarrierPref();

  if (cxrPref1 &&
      (cxrPref1->noApplycombtag1and3() == 'Y' || cxrPref1->noApplycombtag1and3() == 'y'))
  {
    return false;
  }

  const CarrierPreference* cxrPref2 =
      prU2.fareUsage().front()->paxTypeFare()->fareMarket()->governingCarrierPref();

  if (cxrPref2 &&
      (cxrPref2->noApplycombtag1and3() == 'Y' || cxrPref2->noApplycombtag1and3() == 'y'))
  {
    return false;
  }

  return true;
}

bool
FarePathValidator::isAnyTag2(const PricingUnit& pu1, const PricingUnit& pu2) const
{
  const PaxTypeFare* pu1Ptf = pu1.fareUsage().front()->paxTypeFare();
  const PaxTypeFare* pu2Ptf = pu2.fareUsage().front()->paxTypeFare();

  if (pu1Ptf->isRoundTrip() || pu2Ptf->isRoundTrip())
    return true;

  return false;
}
bool
FarePathValidator::checkContinuousNLSPFareLoop(FarePath& fpath, DiagCollector& diag)
{
  if (bypassForGvtPaxType(fpath))
  {
    return true;
  }

  if (_puPath->isIntlCTJourneyWithOWPU())
  {
    return checkPreCalculatedLoop(fpath, diag);
  }

  bool valid = checkContinuousNLSPFareLoop(PricingUnit::NL,
                                           fpath.pricingUnit(),
                                           0, // 0 for MainTrip
                                           diag);
  return valid;
}

bool
FarePathValidator::checkPreCalculatedLoop(const FarePath& fpath, DiagCollector& diag)
{
  bool normalFareExists = false;
  bool specialFareExists = false;
  std::vector<PricingUnit*>::const_iterator it = fpath.pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator itEnd = fpath.pricingUnit().end();
  for (; it != itEnd; ++it)
  {
    if ((*it)->puFareType() == PricingUnit::NL)
      normalFareExists = true;
    else
      specialFareExists = true;

    if (normalFareExists && specialFareExists)
      return true;
  }
  if (!specialFareExists)
  {
    // all are NL Fare
    if (UNLIKELY(diag.isActive()))
    {
      diag << " INVALID: OW-PU IN CONTINUOUS-TRAVEL WITH NL FARE\n";
    }
    return false;
  }

  return true;
}

bool
FarePathValidator::bypassForGvtPaxType(const FarePath& fpath)
{
  if (UNLIKELY(fpath.paxType()->paxType() == GVT || fpath.paxType()->paxType() == CTZ ||
      fpath.paxType()->paxType() == GDP || fpath.paxType()->paxType() == GST ||
      fpath.paxType()->paxType() == NAT || fpath.paxType()->paxType() == GCF ||
      fpath.paxType()->paxType() == GCT || fpath.paxType()->paxType() == GEX ||
      fpath.paxType()->paxType() == GV1))
  {
    std::vector<PricingUnit*>::const_iterator i = fpath.pricingUnit().begin();
    std::vector<PricingUnit*>::const_iterator iEnd = fpath.pricingUnit().end();
    for (; i != iEnd; ++i)
    {
      std::vector<FareUsage*>::const_iterator j = (*i)->fareUsage().begin();
      std::vector<FareUsage*>::const_iterator jEnd = (*i)->fareUsage().end();
      for (; j != jEnd; ++j)
      {
        const PaxTypeCode& farePaxType = (*j)->paxTypeFare()->fcasPaxType();
        if (farePaxType == GVT || farePaxType == CTZ || farePaxType == GDP || farePaxType == GST ||
            farePaxType == NAT || farePaxType == GCF || farePaxType == GCT || farePaxType == GEX ||
            farePaxType == GV1)
        {
          return true;
        }
      }
    }
  }

  return false;
}

bool
FarePathValidator::brandParityOnEachLegCheck(FarePath& farePath)
{
  ShoppingUtil::ParityBrandPerLegMap parityBrandPerLegMap =
    ShoppingUtil::createParityBrandPerLegMap(_trx, farePath);
  // this map is empty if parity per leg requirement is not met for all the legs.
  if (parityBrandPerLegMap.empty())
    return false;

  // If soft passes are used then one fare may be valid in more than one brand
  // In the response we need to display brand that meets parity per leg requirement
  // For that reason we store "leg parity" brand in fareUsage
  ShoppingUtil::saveParityBrandsToFareUsages(farePath, _trx, parityBrandPerLegMap);
  return true;
}

// Checks if there's at least one hard pass on each (non-soldout) leg
bool
FarePathValidator::hardPassOnEachLegCheckIBF(FarePath& farePath)
{
  BrandCode brandCode = _puPath->getBrandCode();

  if (brandCode == NO_BRAND)
    return true;

  farePath.setBrandCode(brandCode);

  for (auto& tSegs : _itin->itinLegs())
  {
    uint16_t originalLegId = tSegs.front()->legId();
    // skipping validation on fixed legs
    if (_trx.isContextShopping() && _trx.getFixedLegs().at(originalLegId))
      continue;

    if (_trx.getRequest()->isChangeSoldoutBrand())
    {
      // We don't want to return itineraries that are soldout on every leg ( total soldout )
      // At least one leg has to have a desired brand
      if (!(_itin->getIbfAvailabilityTracker().atLeastOneLegValidForBrand(brandCode)))
        return false;

      IbfErrorMessage ibfErrorMessage;
      if (_trx.getRequest()->isUseCbsForNoFares())
      {
        ibfErrorMessage =
          _itin->getIbfAvailabilityTracker().getStatusForLegHardPassFix(brandCode, originalLegId);
      }
      else
      {
        ibfErrorMessage =
          _itin->getIbfAvailabilityTracker().getStatusForLeg(brandCode, originalLegId);
      }

      // We don't want to do cross cabin check for soldout legs
      // Those legs may have brands changed ( in order to be able to price the whole itin)
      // and hence cross cabin check would never pass
      if (ibfErrorMessage == IbfErrorMessage::IBF_EM_NOT_OFFERED ||
          ibfErrorMessage == IbfErrorMessage::IBF_EM_NOT_AVAILABLE ||
          ibfErrorMessage == IbfErrorMessage::IBF_EM_NO_FARE_FILED)
        continue;
    }

    if (!atLeastOneHardPass(tSegs, farePath, brandCode))
      return false;
  }

  return true; // All legs pass the check
}

// Checks if there's at least one hard pass on each (non-soldout) leg
bool
FarePathValidator::hardPassOnEachLegCheckPBB(FarePath& farePath)
{
  if (BrandingUtil::getBrandRetrievalMode(_trx) == BrandRetrievalMode::PER_FARE_COMPONENT)
    return true;

  for (auto& tSegs : _itin->itinLegs())
  {
    BrandCode brandCode = tSegs.front()->getBrandCode();

    if (brandCode == NO_BRAND || brandCode.empty())
      continue;

    if (!atLeastOneHardPass(tSegs, farePath, brandCode))
      return false;
  }

  return true; // All legs pass the check
}

bool
FarePathValidator::atLeastOneHardPass(TravelSegPtrVec& tSegs,
                                      FarePath& farePath,
                                      const BrandCode& brandCode)
{
  uint16_t legSize = tSegs.size();
  uint16_t legSegmentIndex = 0;

  do
  {
    TravelSeg* tSeg = tSegs[legSegmentIndex];

    if (tSeg->segmentType() == Arunk)
    {
      if (tSegs.size() == 1)
        return true;

      ++legSegmentIndex;
    }
    else
    {
      uint16_t fuTvlSegIndex;

      FareUsage* fu = farepathutils::getFareUsage(farePath, tSeg, fuTvlSegIndex);
      if (!fu)
        ++legSegmentIndex;
      else
      {
        if (fu->paxTypeFare()->isValidForBrand(_trx, &brandCode, true) ||
            (_trx.isExchangeTrx() && _trx.getRequest()->isBrandedFaresRequest() &&
             fu->paxTypeFare()->isFromFlownOrNotShoppedFM()))
        {
          // keep fares and fares from flown FM are always valid
          return true;
        }

        legSegmentIndex += (fu->travelSeg().size() - fuTvlSegIndex);
      }
    }
  } while (legSegmentIndex < legSize);

  return false;
}

namespace
{
struct HasDifferentRetrievalFlag
    : public std::binary_function<const FareUsage*, FareMarket::FareRetrievalFlags, bool>
{
  bool operator()(const FareUsage* fu, FareMarket::FareRetrievalFlags flag) const
  {
    return fu->paxTypeFare()->retrievalFlag() != flag;
  }
};
}

bool
FarePathValidator::failMixedDateForRefund(const FarePath& farePath) const
{
  if (UNLIKELY(_trx.excTrxType() == PricingTrx::AF_EXC_TRX &&
      static_cast<RefundPricingTrx&>(_trx).trxPhase() == RexBaseTrx::PRICE_NEWITIN_PHASE))
  {
    FareMarket::FareRetrievalFlags firstFlag =
        farePath.pricingUnit().front()->fareUsage().front()->paxTypeFare()->retrievalFlag();

    std::vector<PricingUnit*>::const_iterator puIter = farePath.pricingUnit().begin();
    for (; puIter != farePath.pricingUnit().end(); ++puIter)
    {
      if (std::find_if((**puIter).fareUsage().begin(),
                       (**puIter).fareUsage().end(),
                       std::bind2nd(HasDifferentRetrievalFlag(), firstFlag)) !=
          (**puIter).fareUsage().end())
        return true;
    }
  }
  return false;
}
void
FarePathValidator::exchangeReissueForQrex(FarePath& fp)
{
  if (UNLIKELY(_trx.excTrxType() == PricingTrx::AR_EXC_TRX ||
      _trx.excTrxType() == PricingTrx::PORT_EXC_TRX || _trx.excTrxType() == PricingTrx::EXC_IS_TRX))
  {
    BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(&_trx);
    if (excTrx && excTrx->applyReissueExchange())
    {
      fp.exchangeReissue() = fp.itin()->exchangeReissue();
    }
  }
  return;
}

bool
FarePathValidator::isValidFPathForValidatingCxr(FarePath& farePath, DiagCollector& diag)
{
  if (!_trx.isValidatingCxrGsaApplicable())
    return true;

  if (ValidatingCxrUtil::isValidFPathForValidatingCxr(farePath))
  {
    if (UNLIKELY(diag.isActive() && !farePath.validatingCarriers().empty()))
      diag << farepathutils::displayValCxr(farePath.validatingCarriers()) << std::endl;

    return true;
  }

  _results += "FAILED: NO COMMON VALIDATING CXR";
  return false;
}

bool
FarePathValidator::checkFailedPUIdx(FPPQItem& fppqItem,
                                    std::vector<CarrierCode>& failedValCxr,
                                    std::string& failedCxrMsg)
{
  if (fppqItem.ignorePUIndices())
  {
    return true;
  }

  const PUPath& puPath = *fppqItem.puPath();
  if (puPath.totalPU() == 1)
  {
    return true;
  }

  // TSELatencyData metrics( _trx, "CHECK FAILED PU IDX RES" );

  if (_failedPricingUnits->empty())
  {
    return true;
  }

  std::vector<uint32_t>::const_iterator puIt = fppqItem.puIndices().begin();
  std::vector<uint32_t>::const_iterator puItEnd = fppqItem.puIndices().end();
  for (uint32_t puFactIdx = 0; puIt != puItEnd; ++puIt, ++puFactIdx)
  {
    const std::vector<CarrierCode>& valCxrList = fppqItem.farePath()->validatingCarriers();
    if (valCxrList.empty())
    {
      if (_failedPricingUnits->isFailed(puFactIdx, *puIt))
      {
        return false;
      }
    }
    else
    {
      if (_failedPricingUnits->isFailed(puFactIdx, valCxrList, failedValCxr, *puIt))
      {
        // failed for all cxr
        return false;
      }
      else
      {
        if (!failedValCxr.empty())
        {
          // failed for some cxr but not all
          failedCxrMsg = " FOR" + farepathutils::displayValCxr(failedValCxr);
        }
      }
    }
  }
  return true;
}

void
FarePathValidator::recalculatePriority(FPPQItem& fppqItem)
{
  fppqItem.clearPriority();
  for (PUPQItem* puPQItem : fppqItem.pupqItemVect())
  {
    farepathutils::setPriority(_trx, *puPQItem, fppqItem, _itin);
  }
}

bool
FarePathValidator::checkPULevelCombinability(FPPQItem& fppqItem, DiagCollector& diag)
{
  FarePath& fpath = *fppqItem.farePath();
  std::vector<PricingUnit*>::iterator puIt = fpath.pricingUnit().begin();
  const std::vector<PricingUnit*>::iterator puItEnd = fpath.pricingUnit().end();

  for (uint32_t puFactIdx = 0; puIt != puItEnd; ++puIt, ++puFactIdx)
  {
    PUPQItem::PUValidationStatus status = getPUScopeCat10Status(fppqItem, puFactIdx);
    if (status == PUPQItem::PUValidationStatus::FAILED)
    {
      return false;
    }
    else if (status == PUPQItem::PUValidationStatus::PASSED)
    {
      continue;
    }

    PricingUnit& prU = *(*puIt);
    FareUsage* failedFareUsage = nullptr;
    FareUsage* failedTargetFareUsage = nullptr;

    if (_combinations->process(prU, failedFareUsage, failedTargetFareUsage, diag, fpath.itin()) ==
        CVR_PASSED)

      setPUScopeCat10Status(fppqItem, puFactIdx, PUPQItem::PUValidationStatus::PASSED);

    else
    {
      // set the command pricing failed flag
      if (prU.isCmdPricing())
      {
        setPUScopeCat10Status(fppqItem, puFactIdx, PUPQItem::PUValidationStatus::PASSED);
        prU.setCmdPrcFailedFlag(RuleConst::COMBINABILITY_RULE);
        if (diag.isActive())
          diag << " -- PU COMBINABILITY PASS BY COMMAND PRICING" << std::endl;
      }
      else
      {
        setPUScopeCat10Status(fppqItem, puFactIdx, PUPQItem::PUValidationStatus::FAILED);
        if (failedFareUsage != nullptr)
        {
          saveFailedFare(puFactIdx, failedFareUsage, failedTargetFareUsage);
        }
        return false;
      }
    }
  }

  return true;
}

void
FarePathValidator::saveFailedFare(const uint16_t puFactIdx,
                                  FareUsage* failedFareUsage1,
                                  FareUsage* failedFareUsage2)
{
  if (_allPUF == nullptr)
    return;
  PricingUnitFactory* puf = (*_allPUF)[puFactIdx];
  puf->saveCat10FailedFare(failedFareUsage1, failedFareUsage2);
}

bool
FarePathValidator::checkCombinability(FPPQItem& fppqItem, DiagCollector& diag)
{
  bool valid = true;
  if (UNLIKELY(!_isEoeCombinabilityEnabled))
  {
    return true;
  }
  if (LIKELY(!_factoriesConfig.puScopeValidationEnabled()))
  {
    valid = checkPULevelCombinability(fppqItem, diag);
  }

  if (valid)
  {
    if (fppqItem.eoeValidationStatus() == FPPQItem::EOEValidationStatus::EOE_UNKNOWN)
    {
      valid = checkEOECombinability(fppqItem, diag);
    }
    else
    {
      valid = fppqItem.eoeValidationStatus() == FPPQItem::EOEValidationStatus::EOE_PASSED ? true
                                                                                          : false;
    }
  }

  return valid;
}

bool
FarePathValidator::checkEOECombinability(FPPQItem& fppqItem,
                                         FareUsage*& failedSourceFareUsage,
                                         FareUsage*& failedTargetFareUsage,
                                         DiagCollector& diag)
{
  FarePath& fpath = *fppqItem.farePath();
  if (fpath.pricingUnit().size() == 1)
  {
    return true;
  }

  farepathutils::copyPUPathEOEInfo(fpath, fppqItem.puPath());

  if (_combinations->process(fpath,
                             _paxFPFBaseData.fpCombTried(),
                             failedSourceFareUsage,
                             failedTargetFareUsage,
                             diag) != CVR_PASSED)
  {
    if (failedSourceFareUsage != nullptr && failedTargetFareUsage != nullptr)
    {
      saveEOEFailedFare(failedSourceFareUsage->paxTypeFare(), failedTargetFareUsage->paxTypeFare());
    }
    return false;
  }

  return true;
}

void
FarePathValidator::saveEOEFailedFare(const PaxTypeFare* paxTypeFare1,
                                     const PaxTypeFare* paxTypeFare2)
{
  typedef std::map<const PaxTypeFare*, std::set<const PaxTypeFare*>> EOEFailedMMap;

  if (UNLIKELY(_factoriesConfig.eoeTuningEnabled() == false))
  {
    return;
  }

  if (_puPath->totalPU() == 1 || _puPath->totalFC() <= 2)
  {
    // No EOE needed
    return;
  }

  // TSELatencyData metrics( *_trx, "SAVE EOE COMB RES" );

  std::map<const PaxTypeFare*, std::set<const PaxTypeFare*>>::iterator it =
      _eoeFailedFare->find(paxTypeFare1);
  if (it != _eoeFailedFare->end())
  {
    it->second.insert(paxTypeFare2);
  }
  else
  {
    std::set<const PaxTypeFare*> newSet;
    newSet.insert(paxTypeFare2);
    _eoeFailedFare->insert(EOEFailedMMap::value_type(paxTypeFare1, newSet));
  }

  return;
}

bool
FarePathValidator::checkEOEFailedFare(FPPQItem& fppqItem,
                                      FareUsage*& failedSourceFareUsage,
                                      FareUsage*& failedEOETargetFareUsage)
{
  if (UNLIKELY(_factoriesConfig.eoeTuningEnabled() == false))
  {
    return true;
  }

  if (fppqItem.puPath()->totalPU() == 1 || fppqItem.puPath()->totalFC() <= 2)
  {
    // No EOE needed
    return true;
  }

  // TSELatencyData metrics( _trx, "CHECK EOE COMB RES" );

  if (_eoeFailedFare->empty())
  {
    return true;
  }

  std::vector<PricingUnit*>& puVect = fppqItem.farePath()->pricingUnit();

  std::vector<PricingUnit*>::iterator puIt = puVect.begin();
  const std::vector<PricingUnit*>::iterator puItEnd = puVect.end();
  const std::vector<PricingUnit*>::iterator puItLast = puVect.end() - 1;
  for (; puIt != puItLast; ++puIt)
  {
    std::vector<FareUsage*>::iterator fuIt = (*puIt)->fareUsage().begin();
    const std::vector<FareUsage*>::iterator fuItEnd = (*puIt)->fareUsage().end();
    for (; fuIt != fuItEnd; ++fuIt)
    {
      const PaxTypeFare* sourcePaxTypeFare = (*fuIt)->paxTypeFare();
      typedef std::map<const PaxTypeFare*, std::set<const PaxTypeFare*>> EOEFailedMMap;
      const EOEFailedMMap::iterator fSetIt = _eoeFailedFare->find(sourcePaxTypeFare);
      if (fSetIt == _eoeFailedFare->end())
      {
        continue;
      }

      std::set<const PaxTypeFare*>& failedSet = fSetIt->second;

      std::vector<PricingUnit*>::iterator puTarget = puIt + 1;
      for (; puTarget != puItEnd; ++puTarget)
      {
        std::vector<FareUsage*>::iterator fuTarget = (*puTarget)->fareUsage().begin();
        const std::vector<FareUsage*>::iterator fuTargetEnd = (*puTarget)->fareUsage().end();
        for (; fuTarget != fuTargetEnd; ++fuTarget)
        {
          PaxTypeFare* targetPaxTypeFare = (*fuTarget)->paxTypeFare();
          if (failedSet.count(targetPaxTypeFare) != 0)
          {
            // TSELatencyData metrics( _trx, "EOE RES MATCH FOUND" );
            failedSourceFareUsage = *fuIt;
            failedEOETargetFareUsage = *fuTarget;
            return false; // EOE pair found
          }
        }
      }
    }
  }

  return true;
}

bool
FarePathValidator::penaltyValidation(FarePath& farePath)
{
  bool valid;
  std::string failReason;
  MaximumPenaltyValidator penaltyValidator(_trx);

  std::tie(valid, failReason) = penaltyValidator.validateFarePath(farePath);

  _results += (valid ? "MAX-PEN:P " : "MAX-PEN:F(" + failReason + ") " );

  return valid;
}

void
FarePathValidator::setPUScopeCat10Status(const FPPQItem& fppqItem,
                                         const uint16_t puFactIdx,
                                         PUPQItem::PUValidationStatus status)
{
  if (_allPUF == nullptr)
    return;
  const PUPath& puPath = *fppqItem.puPath();
  if (puPath.totalPU() == 1)
    return;
  PricingUnitFactory* puf = (*_allPUF)[puFactIdx];

  uint32_t puIdx = fppqItem.puIndices()[puFactIdx];
  puf->setPUScopeCat10Status(puIdx, status);
}

PUPQItem::PUValidationStatus
FarePathValidator::getPUScopeCat10Status(const FPPQItem& fppqItem, const uint16_t puFactIdx)
{
  if (UNLIKELY(_allPUF == nullptr))
    return PUPQItem::PUValidationStatus::UNKNOWN;
  PricingUnitFactory* puf = (*_allPUF)[puFactIdx];
  uint32_t puIdx = fppqItem.puIndices()[puFactIdx];
  return puf->getPUScopeCat10Status(puIdx);
}
} // tse namespace
