//-------------------------------------------------------------------
//  Copyright Sabre 2011
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

#include "FreeBagService/AllowanceDataStrategy.h"
#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "Diagnostic/Diag852Collector.h"
#include "FreeBagService/AllowanceUtil.h"
#include "FreeBagService/BaggageAllowanceValidator.h"
#include "FreeBagService/BaggageOcValidationAdapter.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "ServiceFees/OCFees.h"


namespace tse
{

const SubCodeInfo*
AllowanceDataStrategy::retrieveS5Record(BaggageTravel* baggageTravel, bool isUsDot) const
{
  baggageTravel->_carrierTravelSeg = isUsDot ? *baggageTravel->_MSSJourney : *baggageTravel->_MSS;

  const SubCodeInfo* s5;
  const bool isWq = dynamic_cast<NoPNRPricingTrx*>(&_trx);
  const bool isOpenSeg = baggageTravel->_carrierTravelSeg->segmentType() == Open;
  const bool useMarketingCxr = isUsDot || isOpenSeg || isWq;

  const CarrierCode mssCarrier =
      useMarketingCxr
          ? dynamic_cast<const AirSeg*>(baggageTravel->_carrierTravelSeg)->marketingCarrierCode()
          : dynamic_cast<const AirSeg*>(baggageTravel->_carrierTravelSeg)->operatingCarrierCode();

  if ((s5 = retrieveS5Record(mssCarrier)))
    return s5;

  baggageTravel->_carrierTravelSeg = isUsDot ? baggageTravel->itin()->firstTravelSeg()
                                             : *baggageTravel->getTravelSegBegin();

  const CarrierCode fciCarrier =
      useMarketingCxr
          ? dynamic_cast<const AirSeg*>(baggageTravel->_carrierTravelSeg)->marketingCarrierCode()
          : dynamic_cast<const AirSeg*>(baggageTravel->_carrierTravelSeg)->operatingCarrierCode();

  if (mssCarrier != fciCarrier)
  {
    s5 = retrieveS5Record(fciCarrier);
  }

  if (!s5)
    baggageTravel->_carrierTravelSeg = isUsDot ? *baggageTravel->_MSSJourney : *baggageTravel->_MSS;

  return s5;
}

const SubCodeInfo*
AllowanceDataStrategy::retrieveS5Record(const CarrierCode& carrier) const
{
  const SubCodeInfo* s5 = retrieveS5Record(ATPCO_VENDOR_CODE, carrier);
  if (!s5)
    s5 = retrieveS5Record(MERCH_MANAGER_VENDOR_CODE, carrier);

  return s5;
}

const SubCodeInfo*
AllowanceDataStrategy::retrieveS5Record(const VendorCode& vendor, const CarrierCode& carrier) const
{
  for (const SubCodeInfo* subCodeInfo : DataStrategyBase::retrieveS5Records(vendor, carrier))
    if (checkServiceType(subCodeInfo))
      return subCodeInfo;

  return nullptr;
}

bool
AllowanceDataStrategy::checkServiceType(const SubCodeInfo* codeInfo) const
{
  return codeInfo->serviceSubTypeCode().equalToConst("0DF") && codeInfo->fltTktMerchInd() == BAGGAGE_ALLOWANCE &&
         codeInfo->concur() == 'X' && (codeInfo->rfiCode() == 'C' || codeInfo->rfiCode() == ' ') &&
         codeInfo->ssimCode() == ' ' && codeInfo->ssrCode().empty() && codeInfo->emdType() == '4' &&
         codeInfo->bookingInd().empty();
}

void
AllowanceDataStrategy::processBaggageTravel(BaggageTravel* baggageTravel,
                                            const BaggageTravelInfo& bagInfo,
                                            const CheckedPoint& furthestCheckedPoint,
                                            BaggageTripType btt,
                                            Diag852Collector* dc) const
{
  if (LIKELY(!(isAllowanceCarrierOverridden() && isChargesCarrierOverridden())))
  {
    if (btt.isWhollyWithinUsOrCa())
      processBaggageTravelWhollyWithinUsOrCa(baggageTravel, bagInfo, furthestCheckedPoint, dc);
    else if (btt.isUsDot())
      processBaggageTravelDotTable(baggageTravel, bagInfo, furthestCheckedPoint, btt, dc);
    else
    {
      MatchS7Context cxt(bagInfo, furthestCheckedPoint, btt, dc);
      processBaggageTravelNoDotTable(*baggageTravel, cxt);
    }
  }
  if (isAllowanceCarrierOverridden())
  {
    processBaggageTravelForAllowanceCarrierOverridden(
        baggageTravel, bagInfo, furthestCheckedPoint, btt.isUsDot(), dc);
  }
}

AllowanceDataStrategy::AllowanceStepResult
AllowanceDataStrategy::matchAllowance(BaggageTravel& bagTravel,
                                      const MatchS7Context& cxt,
                                      const AirSeg& as,
                                      const uint32_t flags) const
{
  bagTravel._carrierTravelSeg = &as;
  bagTravel._processCharges = false;
  bagTravel._allowanceCxr.clear();
  bagTravel._defer = false;
  bagTravel._allowance = nullptr;

  const CarrierCode cxr = (flags & USE_MARKETING) ? as.carrier() : as.operatingCarrierCode();
  const SubCodeInfo* s5 = retrieveS5Record(cxr);

  if (!s5)
    return S5_FAIL;

  bagTravel._processCharges = true;
  bagTravel._allowanceCxr = s5->carrier();
  bagTravel._defer = (flags & IS_DEFERRED);

  OCFees& allowance = matchS7(&bagTravel,
                              cxt.bagInfo,
                              s5,
                              cxt.furthestCp,
                              cxt.bagTripType.isUsDot(),
                              cxt.dc,
                              flags & IS_DEFERRED);
  const OptionalServicesInfo* s7 = allowance.optFee();

  if (!s7)
    return S7_FAIL;

  if (AllowanceUtil::isDefer(*s7))
    return S7_DEFER;

  bagTravel._allowance = &allowance;
  return S7_PASS;
}

void
AllowanceDataStrategy::processBaggageTravelNoDotTable(BaggageTravel& bagTravel, const MatchS7Context& cxt) const
{
  if (!TrxUtil::isIataReso302MandateActivated(_trx))
  {
    processBaggageTravelNoDotTableOld(
        &bagTravel, cxt.bagInfo, cxt.furthestCp, cxt.bagTripType.isUsDot(), cxt.dc);
    return;
  }

  const AirSeg* mss = (**bagTravel._MSS).toAirSeg();
  const AirSeg* first = (**bagTravel.getTravelSegBegin()).toAirSeg();

  if (!mss || !first)
    return;

  CarrierCode msCxrUsed = mss->carrier();
  AllowanceStepResult status = matchAllowance(bagTravel, cxt, *mss, USE_MARKETING);

  if (status == S7_DEFER)
  {
    msCxrUsed = mss->operatingCarrierCode();
    status = matchAllowance(bagTravel, cxt, *mss, IS_DEFERRED);
  }

  if (LIKELY(status != S5_FAIL || msCxrUsed == first->carrier())) // don't process the same carrier twice
    return;

  status = matchAllowance(bagTravel, cxt, *first, USE_MARKETING);

  if (status == S7_DEFER)
    matchAllowance(bagTravel, cxt, *first, IS_DEFERRED);
}

void
AllowanceDataStrategy::processBaggageTravelNoDotTableOld(BaggageTravel* baggageTravel,
                                                         const BaggageTravelInfo& bagInfo,
                                                         const CheckedPoint& furthestCheckedPoint,
                                                         bool isUsDot,
                                                         Diag852Collector* dc) const
{
  const SubCodeInfo* s5 = retrieveS5Record(baggageTravel, isUsDot);

  if (!s5)
  {
    baggageTravel->_processCharges = false;
    return;
  }
  baggageTravel->_allowanceCxr = s5->carrier();

  OCFees& allowance = matchS7(baggageTravel, bagInfo, s5, furthestCheckedPoint, isUsDot, dc);
  const OptionalServicesInfo* s7 = allowance.optFee();

  if (!s7)
    return;

  bool applyAllowance = false;

  if (s7->notAvailNoChargeInd() == BaggageAllowanceValidator::DEFER_BAGGAGE_RULES_FOR_MC)
  {
    s5 = retrieveS5Record(
        s7->vendor(),
        dynamic_cast<const AirSeg*>(baggageTravel->_carrierTravelSeg)->marketingCarrierCode());
    if (s5)
    {
      baggageTravel->_allowanceCxr = s5->carrier();
      baggageTravel->_defer = true;
      allowance = matchS7(baggageTravel, bagInfo, s5, furthestCheckedPoint, isUsDot, dc, true);
      s7 = allowance.optFee();

      if (s7)
        applyAllowance = true;
      else
        return;
    }
    else if (baggageTravel->_carrierTravelSeg == *(baggageTravel->getTravelSegBegin()))
    {
      baggageTravel->_processCharges = false;
      return;
    }
  }
  else
    applyAllowance = true;

  if (!applyAllowance && baggageTravel->_carrierTravelSeg != *(baggageTravel->getTravelSegBegin()))
  {
    baggageTravel->_carrierTravelSeg = *(baggageTravel->getTravelSegBegin());

    CarrierCode carrier =
        isUsDot
            ? dynamic_cast<const AirSeg*>(baggageTravel->_carrierTravelSeg)->marketingCarrierCode()
            : dynamic_cast<const AirSeg*>(baggageTravel->_carrierTravelSeg)->operatingCarrierCode();

    s5 = retrieveS5Record(carrier);

    if (s5)
    {
      baggageTravel->_allowanceCxr = s5->carrier();
      allowance = matchS7(baggageTravel, bagInfo, s5, furthestCheckedPoint, isUsDot, dc, true);
      s7 = allowance.optFee();

      if (s7)
      {
        if (s7->notAvailNoChargeInd() == BaggageAllowanceValidator::DEFER_BAGGAGE_RULES_FOR_MC)
        {
          s5 = retrieveS5Record(s7->vendor(),
                                dynamic_cast<const AirSeg*>(baggageTravel->_carrierTravelSeg)
                                    ->marketingCarrierCode());
          if (s5)
          {
            baggageTravel->_allowanceCxr = s5->carrier();
            baggageTravel->_defer = true;
            allowance =
                matchS7(baggageTravel, bagInfo, s5, furthestCheckedPoint, isUsDot, dc, true);
            s7 = allowance.optFee();

            if (s7)
              applyAllowance = true;
          }
          else
          {
            baggageTravel->_processCharges = false;
            return;
          }
        }
        else
          applyAllowance = true;
      }
    }
    else
    {
      baggageTravel->_processCharges = false;
      return;
    }
  }

  if (applyAllowance)
  {
    baggageTravel->_allowance = &allowance;
  }
}

void
AllowanceDataStrategy::processBaggageTravelDotTable(BaggageTravel* baggageTravel,
                                                    const BaggageTravelInfo& bagInfo,
                                                    const CheckedPoint& furthestCP,
                                                    BaggageTripType btt,
                                                    Diag852Collector* dc) const
{
  const std::vector<TravelSeg*>& travelSegs = baggageTravel->itin()->travelSeg();
  const AllowanceUtil::DotTableChecker carrierTablesChecker(_trx, btt);
  TravelSegPtrVecCI foundIt =
      std::find_if(travelSegs.begin(), travelSegs.end(), carrierTablesChecker);

  if (foundIt != travelSegs.end())
  {
    baggageTravel->_carrierTravelSeg = *foundIt;
    const CarrierCode fciCarrier = (*foundIt)->toAirSeg()->marketingCarrierCode();
    const SubCodeInfo* s5 = retrieveS5Record(fciCarrier);

    if (s5)
    {
      OCFees& allowance = matchS7(baggageTravel, bagInfo, s5, furthestCP, btt.isUsDot(), dc);
      baggageTravel->_allowance = &allowance;
      const OptionalServicesInfo* s7 = allowance.optFee();

      if (s7)
      {
        if (s7->notAvailNoChargeInd() != BaggageAllowanceValidator::DEFER_BAGGAGE_RULES_FOR_MC)
          return;

        if (carrierTablesChecker(*baggageTravel->_MSSJourney) &&
            findJourneyMscAllowance(*baggageTravel, bagInfo, furthestCP, btt, dc))
        {
          return;
        }

        OCFees& nextS7 =
            matchS7(baggageTravel, bagInfo, s5, furthestCP, btt.isUsDot(), s7->seqNo(), dc);
        baggageTravel->_allowance = &nextS7;

        if (nextS7.optFee())
          return;
      }

      // allowance UNKNOWN, process charges
      baggageTravel->_allowanceCxr = fciCarrier;
      return;
    }
  }
  // allowance UNKNOWN, don't process charges
  if (isAllowanceCarrierOverridden() || !isChargesCarrierOverridden())
    baggageTravel->_processCharges = false;
}

bool
AllowanceDataStrategy::findJourneyMscAllowance(BaggageTravel& baggageTravel,
                                               const BaggageTravelInfo& bagInfo,
                                               const CheckedPoint& furthestCP,
                                               BaggageTripType btt,
                                               Diag852Collector* dc) const
{
  const CarrierCode journeyMsc = (*baggageTravel._MSSJourney)->toAirSeg()->marketingCarrierCode();
  const SubCodeInfo* mscS5 = retrieveS5Record(journeyMsc);

  if (!mscS5)
    return false;

  baggageTravel._carrierTravelSeg = *(baggageTravel._MSSJourney);
  OCFees& mscAllowance = matchS7(&baggageTravel, bagInfo, mscS5, furthestCP, btt.isUsDot(), 0, dc);

  if (mscAllowance.optFee())
  {
    baggageTravel._allowance = &mscAllowance;
    return true;
  }

  return false;
}

void
AllowanceDataStrategy::processBaggageTravelWhollyWithinUsOrCa(
    BaggageTravel* baggageTravel,
    const BaggageTravelInfo& bagInfo,
    const CheckedPoint& furthestCheckedPoint,
    Diag852Collector* dc) const
{
  const CarrierCode fciCarrier =
      dynamic_cast<const AirSeg*>(baggageTravel->itin()->firstTravelSeg())
          ->marketingCarrierCode();

  const SubCodeInfo* fciS5 = retrieveS5Record(fciCarrier);
  if (fciS5)
  {
    baggageTravel->_carrierTravelSeg = baggageTravel->itin()->firstTravelSeg();
    OCFees& fciAllowance =
        matchS7(baggageTravel, bagInfo, fciS5, furthestCheckedPoint, true, 0, dc);
    if (fciAllowance.optFee())
    {
      baggageTravel->_allowance = &fciAllowance;
    }
    // else -> allowance UNKNOWN, process charges
  }
  else
  {
    // allowance UNKNOWN, don't process charges
    if (isAllowanceCarrierOverridden() || !isChargesCarrierOverridden())
      baggageTravel->_processCharges = false;
  }
}

void
AllowanceDataStrategy::processBaggageTravelForAllowanceCarrierOverridden(
    BaggageTravel* baggageTravel,
    const BaggageTravelInfo& bagInfo,
    const CheckedPoint& furthestCheckedPoint,
    bool isUsDot,
    Diag852Collector* dc) const
{
  if (baggageTravel->_allowanceCxr.empty() && baggageTravel->_allowance)
    baggageTravel->_allowanceCxr = baggageTravel->_allowance->subCodeInfo()->carrier();

  baggageTravel->_allowance = nullptr;

  if (shouldDisplayDiagnostic(baggageTravel, bagInfo, dc))
    dc->printBaggageHeader(_trx, true);

  const SubCodeInfo* s5 = retrieveS5Record(getAllowanceCarrierOverridden());
  if (!s5)
    return;

  OCFees& allowance =
      matchS7(baggageTravel, bagInfo, s5, furthestCheckedPoint, isUsDot, dc, false, true);
  const OptionalServicesInfo* s7 = allowance.optFee();

  if (s7 && !AllowanceUtil::isDefer(*s7))
    baggageTravel->_allowance = &allowance;
}

OCFees&
AllowanceDataStrategy::matchS7(BaggageTravel* baggageTravel,
                               const BaggageTravelInfo& bagInfo,
                               const SubCodeInfo* s5,
                               const CheckedPoint& furthestCheckedPoint,
                               bool isUsDot,
                               Diag852Collector* dc,
                               bool defer,
                               bool allowanceCarrierOverridden) const
{
  bool displayDiag = shouldDisplayDiagnostic(baggageTravel, bagInfo, dc);

  if (displayDiag)
    printS7ProcessingContext(
        baggageTravel, bagInfo, s5, isUsDot, dc, defer, allowanceCarrierOverridden);

  return BaggageOcValidationAdapter::matchS7AllowanceRecord(*s5,
                                                            *baggageTravel,
                                                            furthestCheckedPoint,
                                                            (displayDiag ? dc : nullptr),
                                                            allowanceCarrierOverridden);
}

OCFees&
AllowanceDataStrategy::matchS7(BaggageTravel* baggageTravel,
                               const BaggageTravelInfo& bagInfo,
                               const SubCodeInfo* s5,
                               const CheckedPoint& furthestCheckedPoint,
                               bool isUsDot,
                               uint32_t lastSequence,
                               Diag852Collector* dc,
                               bool defer,
                               bool allowanceCarrierOverridden) const
{
  bool displayDiag = shouldDisplayDiagnostic(baggageTravel, bagInfo, dc);

  if (displayDiag)
    printS7ProcessingContext(
        baggageTravel, bagInfo, s5, isUsDot, dc, defer, allowanceCarrierOverridden);

  return BaggageOcValidationAdapter::matchS7FastForwardAllowanceRecord(
      *s5, *baggageTravel, furthestCheckedPoint, lastSequence, (displayDiag ? dc : nullptr));
}

bool
AllowanceDataStrategy::shouldDisplayDiagnostic(BaggageTravel* baggageTravel,
                                               const BaggageTravelInfo& bagInfo,
                                               const Diag852Collector* dc) const
{
  return checkFareLineAndCheckedPortion(baggageTravel, bagInfo, dc) &&
         !dc->hasDisplayChargesOption() && !dc->isDisplayCarryOnAllowanceOption() &&
         !dc->isDisplayCarryOnChargesOption() && !dc->isDisplayEmbargoesOption();
}

} // tse
