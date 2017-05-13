//---------------------------------------------------------------------------
//  Copyright Sabre 2012
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/CurrencyConversionFacade.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/GoverningCarrier.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/Customer.h"
#include "DBAccess/TaxCodeReg.h"
#include "Rules/RuleConst.h"
#include "Taxes/Common/PartialTaxableFare.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/ProrateCalculator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxUS1_01.h"

namespace tse
{

namespace
{
Logger logger("atseintl.Taxes.TaxUS1_01");
}

bool
TaxUS1_01::validateItin(PricingTrx& trx,
                        TaxResponse& taxResponse,
                        TaxCodeReg& taxCodeReg)
{
  _latestEndIndex = -1;
  return Tax::validateItin(trx, taxResponse, taxCodeReg);
}

bool
TaxUS1_01::validateTripTypes(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex)
{
  Itin& itin = *(taxResponse.farePath()->itin());
  _soldInUS = taxUtil::soldInUS(trx);
  _openJaw = false;
  if (UNLIKELY(startIndex == 0 && locateOpenJaw(trx, taxResponse) && _soldInUS))
  {
    _latestEndIndex = itin.travelSeg().size() - 1;
    _openJaw = true;
    if (trx.diagnostic().diagnosticType() == Diagnostic818)
      trx.diagnostic().insertDiagMsg("OPEN JAW DETECTED\n");
    return true;
  }
  if (!_fareBreaksFound && trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX &&
      trx.getTrxType() != PricingTrx::IS_TRX)
  {
    taxUtil::findFareBreaks(_fareBreaks, *(taxResponse.farePath()));
    _fareBreaksFound = true;
  }
  if (!_allUSScanned)
  {
    _allUS = isAllUS(*(taxResponse.farePath()));
    _allUSScanned = true;
  }

  bool fallbackUSSurface = false;
  if (taxCodeReg.specialProcessNo() == 1101 || itin.validatingCarrier() != "WN")
    fallbackUSSurface = true;

  return findApplicableSegs(trx, taxResponse, startIndex, endIndex, fallbackUSSurface);
}

TravelSeg*
TaxUS1_01::previousAirSeg(std::vector<TravelSeg*>::const_iterator segI,
                          const std::vector<TravelSeg*>& tsVector)
{
  while (segI != tsVector.begin())
  {
    --segI;
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*segI);
    if (airSeg != nullptr)
      return *segI;
  }
  return nullptr;
}

TravelSeg*
TaxUS1_01::nextAirSeg(std::vector<TravelSeg*>::const_iterator segI,
                      const std::vector<TravelSeg*>& tsVector)
{
  ++segI;
  while (segI != tsVector.end())
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*segI);
    if (airSeg != nullptr)
      return *segI;
    ++segI;
  }
  return nullptr;
}

bool
TaxUS1_01::isUS(const Loc* loc)
{
  if (UNLIKELY(loc == nullptr))
    return false;
  return (taxUtil::checkLocCategory(*loc) != taxUtil::OTHER ||
          (_soldInUS && taxUtil::isBufferZone(*loc)));
}

bool
TaxUS1_01::isAllUS(FarePath& farePath)
{
  TaxLocIterator* locIt = getLocIterator(farePath);

  locIt->toFront();
  while (!locIt->isEnd())
  {
    if (!(isUS(locIt->loc())))
      return false;
    locIt->next();
  }
  return true;
}

bool
TaxUS1_01::findApplicableSegs(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              int16_t startIndex,
                              uint16_t& endIndex,
                              bool fallbackUSSurface)
{
  if (!_allUS && !_soldInUS)
    return false;

  if (startIndex <= _latestEndIndex)
    return false;

  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));

  locIt->setStopHours(12);
  locIt->toSegmentNo(startIndex);

  bool startsFromHidden = false;

  if (!isUS(locIt->loc()))
  {
    locIt->next();

    if (!(locIt->isHidden() && isUS(locIt->loc())))
      return false;

    startsFromHidden = true;
  }

  if (!locIt->isNextSegAirSeg())
    return false;

  const Loc* locFrom = nullptr;

  if (fallbackUSSurface)
  {
    if (locIt->hasPrevious())
    {
      locIt->previous();
      if (!isUS(locIt->loc()))
        locFrom = locIt->loc();
      locIt->next();
    }
  }
  else
  {
    if (locIt->hasPrevious())
    {
      locIt->previous();
      if (!isUS(locIt->loc()))
      {
        locFrom = locIt->loc();
      }
      else if (!locIt->isNextSegAirSeg() && locIt->hasPrevious())
      {
        locIt->previous();
        if (!isUS(locIt->loc()))
          locFrom = locIt->loc();
        locIt->next();
      }
      locIt->next();
    }
  }

  uint32_t maxStopTpm = 0;
  uint16_t maxStopI = 0;
  uint32_t maxFBTpm = 0;
  uint16_t maxFBI = 0;
  uint16_t maxI = 0;
  bool isTA = false;

  std::vector<TravelSeg*> milesTravelSegs;
  while (locIt->hasNext() && isUS(locIt->loc()) && (!fallbackUSSurface || locIt->isNextSegAirSeg()))
  {
    if (locFrom)
    {
      milesTravelSegs.push_back(locIt->prevSeg());
      if (locIt->isStop() || locIt->isFareBreak())
      {
        const uint32_t tpm =
            calculateMiles(trx, taxResponse, milesTravelSegs, *locFrom, *(locIt->loc()));
        if (locIt->isStop() && tpm > maxStopTpm)
        {
          maxStopTpm = tpm;
          maxStopI = locIt->prevSegNo();
          isTA = true;
        }
        if (locIt->isFareBreak() && tpm > maxFBTpm)
        {
          maxFBTpm = tpm;
          maxFBI = locIt->prevSegNo();
          isTA = true;
        }
      }
    }
    locIt->next();
  }

  if (maxStopTpm == 0)
    maxI = maxFBI;
  else
    maxI = maxStopI;

  const Loc* locTo = nullptr;

  if (fallbackUSSurface)
  {
    if (!isUS(locIt->loc()))
    {
      locTo = locIt->loc();

      if (LIKELY(locIt->hasPrevious()))
        locIt->previous();
    }
  }
  else
  {
    if (!isUS(locIt->loc()) && locIt->hasPrevious() && locIt->isPrevSegAirSeg())
      locTo = locIt->loc();

    locIt->toSegmentNo(startIndex);
    if (startsFromHidden)
      locIt->next();

    while (locIt->hasNext() && isUS(locIt->loc()) && locIt->isNextSegAirSeg())
      locIt->next();

    if (!isUS(locIt->loc()) && locIt->hasPrevious())
      locIt->previous();
  }

  if (isTA && locTo)
  {

    if ((locFrom->area() != locTo->area()) || (locFrom->subarea() != locTo->subarea()) ||
        (LocUtil::isMexico(*locFrom) && !(LocUtil::isMexico(*locTo))) ||
        (!(LocUtil::isMexico(*locFrom)) && LocUtil::isMexico(*locTo)))
    {
      endIndex = locIt->prevSegNo();
      _latestEndIndex = endIndex;

      if (startIndex <= endIndex)
        return bool(maxStopTpm);

      return false;
    }

    if (maxI < startIndex)
    {
      endIndex = startIndex;
      _latestEndIndex = endIndex;
      locIt->toSegmentNo(startIndex);
      locIt->next();
      while (locIt->hasNext() && isUS(locIt->loc()))
      {
        if (locIt->isStop())
          return true;
        locIt->next();
      }
      return false;
    }

    locIt->toSegmentNo(maxI);
    endIndex = maxI;
    _latestEndIndex = endIndex;

    while (locIt->hasPrevious() && isUS(locIt->loc()))
    {
      if (locIt->isStop())
        return true;
      locIt->previous();
    }
    return false;
  }
  else
  {
    endIndex = locIt->prevSegNo();
  }
  _latestEndIndex = startIndex;
  if (startIndex <= endIndex)
  {
    _latestEndIndex = endIndex;

    if (_allUS)
    {
      if (startIndex == 0 && _fareBreaks.size() == 2)
      {
        std::map<uint16_t, FareUsage*>::const_iterator fareBreaksBeginI = _fareBreaks.begin();
        if (fareBreaksBeginI->second->totalFareAmount() ==
            (++fareBreaksBeginI)->second->totalFareAmount())
        {
          _latestEndIndex = _fareBreaks.begin()->first;
          endIndex = _latestEndIndex;
        }
      }
      return true;
    }

    if (locTo)
    {
      locIt->toSegmentNo(startIndex);
      locIt->next();
      while (locIt->hasNext() && isUS(locIt->loc()))
      {
        if (locIt->isStop())
          return true;
        locIt->next();
      }
    }
    else if (locFrom)
    {
      locIt->toSegmentNo(endIndex);
      while (locIt->hasPrevious() && isUS(locIt->loc()))
      {
        if (locIt->isStop())
          return true;

        locIt->previous();
      }
    }
    else
    {
      return true;
    }
    return false;
  }
  return false;
}

MoneyAmount
TaxUS1_01::calculatePartAmount(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t startIndex,
                               uint16_t endIndex,
                               uint16_t fareBreakEnd,
                               FareUsage& fareUsage)
{
  Itin& itin = *(taxResponse.farePath()->itin());
  uint16_t fareBreakStart = itin.segmentOrder(fareUsage.travelSeg().front()) - 1;
  MoneyAmount totalFareAmount = fareUsage.totalFareAmount();
  convertCurrency(totalFareAmount, trx, taxResponse, taxCodeReg);
  if (startIndex == fareBreakStart && endIndex == fareBreakEnd &&
      isUS((itin.travelSeg()[startIndex])->origin()) &&
      isUS((itin.travelSeg()[endIndex])->destination()))
  {
    getNetAmountForLCT(trx, &fareUsage, taxResponse, totalFareAmount);
    return totalFareAmount;
  }
  else
  {
    if (trx.diagnostic().diagnosticType() == Diagnostic818)
    {
      std::ostringstream stream;
      stream.setf(std::ios::fixed, std::ios::floatfield);
      stream.precision(2);
      stream << "TOTAL FARE:" << totalFareAmount << std::endl;
      MoneyAmount totalNetAmount;
      getNetAmountForLCT(trx, &fareUsage, taxResponse, totalNetAmount);
      stream << "TOTAL NET FARE:" << totalNetAmount << std::endl;
      trx.diagnostic().insertDiagMsg(stream.str());
    }
    MoneyAmount taxableAmount = totalFareAmount;
    const Loc* loc1 = itin.travelSeg()[startIndex]->origin();
    const Loc* loc2 = itin.travelSeg()[endIndex]->destination();
    if ((isUS(loc1) && isUS(loc2)) || _openJaw)
    {
      std::vector<TravelSeg*> travelSegsEx;
      travelSegsEx.insert(travelSegsEx.begin(),
                          itin.travelSeg().begin() + fareBreakStart,
                          itin.travelSeg().begin() + startIndex);
      if (substractPartialFare(trx,
                               taxResponse,
                               taxCodeReg,
                               totalFareAmount,
                               taxableAmount,
                               fareUsage,
                               travelSegsEx))
      {
        travelSegsEx.clear();
        travelSegsEx.insert(travelSegsEx.begin(),
                            itin.travelSeg().begin() + endIndex + 1,
                            itin.travelSeg().begin() + fareBreakEnd + 1);
        if (substractPartialFare(trx,
                                 taxResponse,
                                 taxCodeReg,
                                 totalFareAmount,
                                 taxableAmount,
                                 fareUsage,
                                 travelSegsEx))
        {
          return taxableAmount;
        }
      }
    }
    if (!_openJaw)
    {
      if (!isUS(loc1))
      {
        TravelSeg* travelSeg = itin.travelSeg()[startIndex];
        std::vector<const Loc*>::const_iterator hidStopsI;
        for (hidStopsI = travelSeg->hiddenStops().begin();
             hidStopsI != travelSeg->hiddenStops().end();
             ++hidStopsI)
        {
          if (isUS(*hidStopsI))
          {
            loc1 = *hidStopsI;
            break;
          }
        }
      }
      if (!isUS(loc2))
      {
        TravelSeg* travelSeg = itin.travelSeg()[endIndex];
        std::vector<const Loc*>::reverse_iterator hidStopsI;
        for (hidStopsI = travelSeg->hiddenStops().rbegin();
             hidStopsI != travelSeg->hiddenStops().rend();
             ++hidStopsI)
        {
          if (isUS(*hidStopsI))
          {
            loc2 = *hidStopsI;
            break;
          }
        }
      }
    }
    std::vector<TravelSeg*> travelSegs;
    travelSegs.insert(travelSegs.begin(),
                      itin.travelSeg().begin() + fareBreakStart,
                      itin.travelSeg().begin() + fareBreakEnd + 1);
    const Loc* locf1 = travelSegs.front()->origin();
    const Loc* locf2 = travelSegs.back()->destination();
    uint32_t thruMiles;
    uint32_t localMiles = calculateMiles(trx, taxResponse, travelSegs, *loc1, *loc2);
    if (localMiles > 0)
    {
      thruMiles = calculateMiles(trx, taxResponse, travelSegs, *locf1, *locf2);
      if (thruMiles > 0)
      {
        if (localMiles < thruMiles)
        {
          getNetAmountForLCT(trx, &fareUsage, taxResponse, totalFareAmount);
          taxableAmount = totalFareAmount * localMiles / thruMiles;
        }
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
        {
          std::ostringstream stream;
          stream << "MILES: " << loc1->loc() << loc2->loc() << "/" << locf1->loc() << locf2->loc();
          stream << " " << localMiles << "/" << thruMiles << std::endl;
          trx.diagnostic().insertDiagMsg(stream.str());
        }
        return taxableAmount;
      }
    }
    return totalFareAmount;
  }
}

bool
TaxUS1_01::doReprice(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     const PaxTypeFare* paxTypeFare,
                     const std::vector<TravelSeg*>& travelSegsEx,
                     const std::set<CarrierCode>& govCxrs,
                     PartialTaxableFare& partialFareLocator)
{
  if (!TrxUtil::isPricingTaxRequest(&trx))
   {
     RepricingTrx* reTrx =
         getRepricingTrx(trx,
                         travelSegsEx,
                         paxTypeFare->directionality() == TO ? FMDirection::INBOUND
                                                                         : FMDirection::UNKNOWN);
     if (!reTrx)
     {
       LOG4CXX_DEBUG(logger, "TaxUS1_00: no repricing trx");
       return false;
     }
     LOG4CXX_DEBUG(logger, "TaxUS1_00: repricing trx is ready");

     const FareMarket* fareMarket = TrxUtil::getFareMarket(
         *reTrx, *(govCxrs.begin()), travelSegsEx, paxTypeFare->retrievalDate());

     if (!fareMarket)
     {
       LOG4CXX_DEBUG(logger, "TaxUS1_00: repricing - TrxUtil::getFareMarket failed");
       return false;
     }

     if (!partialFareLocator.appliedFare(*reTrx, *(taxResponse.farePath()), fareMarket))
     {
       LOG4CXX_DEBUG(logger, "TaxUS1_00: repricing - partialFareLocator.appliedFare failed");
       return false;
     }

     LOG4CXX_DEBUG(logger, "TaxUS1_00: applied repriced fare");
   }
   else
     return false;

  return true;
}

bool
TaxUS1_01::substractPartialFare(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg,
                                const MoneyAmount& totalFareAmount,
                                MoneyAmount& taxableAmount,
                                FareUsage& fareUsage,
                                const std::vector<TravelSeg*>& travelSegsEx)
{
  if (travelSegsEx.size() == 0)
    return true;
  PartialTaxableFare partialFareLocator;
  partialFareLocator.fareUsage() = &fareUsage;
  partialFareLocator.thruTotalFare() = totalFareAmount;
  partialFareLocator.taxablePartialFare() = taxableAmount;
  partialFareLocator.paymentCurrency() = _paymentCurrency;
  std::set<CarrierCode> govCxrs;
  GoverningCarrier govCxrSel(&trx);
  govCxrSel.getGoverningCarrier(fareUsage.travelSeg(), govCxrs);
  const FareMarket* fareMarket = TrxUtil::getFareMarket(
      trx, *(govCxrs.begin()), travelSegsEx, fareUsage.paxTypeFare()->retrievalDate());
  if (!fareMarket)
  {
    if (!doReprice(trx, taxResponse, fareUsage.paxTypeFare(), travelSegsEx, govCxrs, partialFareLocator))
      return false;
  }
  else
  {
    if (!partialFareLocator.appliedFare(trx, *(taxResponse.farePath()), fareMarket))
    {
      if (partialFareLocator.needsReprice())
      {
        if (!doReprice(trx, taxResponse, fareUsage.paxTypeFare(), travelSegsEx, govCxrs, partialFareLocator))
          return false;
      }
      else
        return false;
    }
  }
  taxableAmount = partialFareLocator.taxablePartialFare();
  return true;
}

bool
TaxUS1_01::getNetAmountForLCT(PricingTrx& trx,
                              FareUsage* fareUsage,
                              TaxResponse& taxResponse,
                              MoneyAmount& netAmount)
{
  bool netAmountKnown = false;

  if (!trx.getRequest())
    return false;
  const Agent* agent = trx.getRequest()->ticketingAgent();

  if (!agent)
    return false;

  if (!agent->agentTJR())
    return false;

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
  {
    std::ostringstream stream;
    stream << "VALID FOR NET FARE CALCULATION:" << agent->agentTJR()->pricingApplTag3()
           << std::endl;
    trx.diagnostic().insertDiagMsg(stream.str());
  }

  if (agent->agentTJR()->pricingApplTag3() != 'Y')
    return false;

  if (!fareUsage->paxTypeFare())
    return false;

  PaxTypeFare& ptFare = *(fareUsage->paxTypeFare());
  bool IsTypeLCT = ptFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF &&
                   (ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE ||
                    ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
                    ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD); // ||
  // ptFare.carrier() == INDUSTRY_CARRIER;

  const NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;
  if (fareUsage->paxTypeFare()->isNegotiated())
  {
    negPaxTypeFare = fareUsage->paxTypeFare()->getNegRuleData();

    if (negPaxTypeFare && negPaxTypeFare->nucNetAmount() > 0)
    {
      netAmountKnown = true;
    }
  }

  if ( // trx.getOptions()->isCat35Net() &&
      IsTypeLCT && netAmountKnown)
  {
    // const CollectedNegFareData* cNegFareData = taxResponse.farePath()->collectedNegFareData();
    netAmount = negPaxTypeFare->netAmount();

    if (negPaxTypeFare->baseFare() && negPaxTypeFare->baseFare()->fare() &&
        negPaxTypeFare->baseFare()->fare()->currency() != _paymentCurrency)
    {
      Money sourceMoney(netAmount, negPaxTypeFare->baseFare()->fare()->currency());
      Money targetMoney(_paymentCurrency);
      CurrencyConversionFacade ccFacade;

      if (ccFacade.convert(targetMoney, sourceMoney, trx, false))
      {
        netAmount = targetMoney.value();

        if (trx.diagnostic().diagnosticType() == Diagnostic818)
        {
          std::ostringstream stream;
          stream << "CONVERT " << sourceMoney.value() << sourceMoney.code().c_str() << " TO "
                 << targetMoney.value() << targetMoney.code().c_str() << std::endl;
          trx.diagnostic().insertDiagMsg(stream.str());
        }
      }
    }
  }
  return netAmountKnown;
}

void
TaxUS1_01::taxCreate(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     uint16_t travelSegStartIndex,
                     uint16_t travelSegEndIndex)
{
  _paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();

  if (!trx.getOptions()->currencyOverride().empty())
  {
    _paymentCurrency = trx.getOptions()->currencyOverride();
  }

  Money money(_paymentCurrency);

  _paymentCurrencyNoDec = money.noDec(trx.ticketingDate());
  _taxAmount = 0.0;
  _taxableFare = 0.0;
  _travelSegStartIndex = travelSegStartIndex;
  _travelSegEndIndex = travelSegEndIndex;

  Itin& itin = *(taxResponse.farePath()->itin());

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
    trx.diagnostic().insertDiagMsg("*** US1 TAX ITEM DETAILS ***\n");

  MoneyAmount taxableAmount;
  uint16_t startIndex;
  uint16_t endIndex;
  for (startIndex = _travelSegStartIndex; startIndex <= _travelSegEndIndex;
       startIndex = endIndex + 1)
  {
    for (endIndex = uint16_t(startIndex);
         endIndex < _travelSegEndIndex && _fareBreaks.find(endIndex) == _fareBreaks.end();
         ++endIndex)
      ;

    std::map<uint16_t, FareUsage*>::const_iterator fbI;
    uint16_t fareBreakIndex;
    for (fareBreakIndex = endIndex; fareBreakIndex < itin.travelSeg().size(); ++fareBreakIndex)
    {
      fbI = _fareBreaks.find(fareBreakIndex);
      if (fbI != _fareBreaks.end())
      {
        FareUsage* fareUsage = fbI->second;
        if (itin.segmentOrder(fareUsage->travelSeg().front()) - 1 <= startIndex)
          break;

        fbI = _fareBreaks.end();
      }
    }
    if (fbI != _fareBreaks.end())
    {
      taxableAmount = calculatePartAmount(
          trx, taxResponse, taxCodeReg, startIndex, endIndex, fareBreakIndex, *(fbI->second));
    }
    else
    {
      taxableAmount = taxResponse.farePath()->getTotalNUCAmount();
    }

    checkEquipment(taxableAmount, startIndex, endIndex, trx, itin);

    _specialPercentage = calculatePercentage(trx, taxResponse, taxCodeReg, startIndex, endIndex);
    if (trx.getOptions() && trx.getOptions()->isRtw())
    {
      ProrateCalculator calculator(trx, itin.travelDate(), itin.travelSeg());
      taxableAmount = calculator.getProratedAmount(startIndex, endIndex, taxableAmount);
    }
    _taxableFare += taxableAmount;
    _taxAmount += taxableAmount * _specialPercentage;
    if (trx.diagnostic().diagnosticType() == Diagnostic818)
    {
      std::ostringstream stream;
      stream.setf(std::ios::fixed, std::ios::floatfield);
      stream.precision(2);
      stream << "SEGS:" << startIndex + 1 << "-" << endIndex + 1 << " "
             << "TAXABLE:";
      stream << taxableAmount << " PERCENT:" << _specialPercentage * 100;
      stream << " TAX:" << taxableAmount* _specialPercentage << std::endl << std::endl;
      trx.diagnostic().insertDiagMsg(stream.str());
    }
  }
}

void
TaxUS1_01::checkEquipment(MoneyAmount& taxableAmount,
                          const uint16_t startIndex,
                          const uint16_t endIndex,
                          PricingTrx& trx,
                          const Itin& itin)
{
  bool isValid = false;
  MoneyAmount reduceBy = 0.00;
  TravelSeg* travelSeg;
  uint16_t i;
  for (i = startIndex; i <= endIndex; ++i)
  {
    travelSeg = itin.travelSeg()[i];
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);
    if (!airSeg)
      continue;
    if (airSeg->equipmentType() == TGV || airSeg->equipmentType() == ICE ||
        airSeg->equipmentType() == BOAT || airSeg->equipmentType() == LMO)
      continue;
    if (airSeg->equipmentType() == BUS || airSeg->equipmentType() == TRAIN)
    {
      if (airSeg->carrier().equalToConst("CO"))
      {
        if (((airSeg->origin()->loc() == "EWR") && (airSeg->destination()->loc() == "ABE")) ||
            ((airSeg->origin()->loc() == "ABE") && (airSeg->destination()->loc() == "EWR")))
        {
          reduceBy += 29.00;
        }
        if (((airSeg->origin()->loc() == "EWR") && (airSeg->destination()->loc() == "ZVE")) ||
            ((airSeg->origin()->loc() == "ZVE") && (airSeg->destination()->loc() == "EWR")))
        {
          reduceBy += 35.00;
        }
        if (((airSeg->origin()->loc() == "EWR") && (airSeg->destination()->loc() == "ZFV")) ||
            ((airSeg->origin()->loc() == "ZFV") && (airSeg->destination()->loc() == "EWR")))
        {
          reduceBy += 46.00;
        }
        if (((airSeg->origin()->loc() == "EWR") && (airSeg->destination()->loc() == "ZTF")) ||
            ((airSeg->origin()->loc() == "ZTF") && (airSeg->destination()->loc() == "EWR")))
        {
          reduceBy += 33.00;
        }
        if (((airSeg->origin()->loc() == "EWR") && (airSeg->destination()->loc() == "ZWI")) ||
            ((airSeg->origin()->loc() == "ZWI") && (airSeg->destination()->loc() == "EWR")))
        {
          reduceBy += 52.00;
        }
      }
      continue;
    }
    isValid = true;
  }

  if (isValid)
  {
    if (reduceBy > 0.0)
    {
      taxableAmount -= reduceBy;
      if (trx.diagnostic().diagnosticType() == Diagnostic818)
      {
        std::ostringstream stream;
        stream.setf(std::ios::fixed, std::ios::floatfield);
        stream.precision(2);
        stream << "BUS/TRAIN SECTOR FOUND - FARE AMT REDUCED BY " << reduceBy << std::endl;
        trx.diagnostic().insertDiagMsg(stream.str());
      }
    }
  }
  else
  {
    taxableAmount = 0;
  }
}

MoneyAmount
TaxUS1_01::calculatePercentage(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t startIndex,
                               uint16_t endIndex)
{
  Itin& itin = *(taxResponse.farePath()->itin());
  const TravelSeg* travelSeg;
  const Loc* loc1;
  LocCode locCode1;
  const Loc* loc2;
  LocCode locCode2;
  loc1 = itin.travelSeg()[startIndex]->origin();
  locCode1 = itin.travelSeg()[startIndex]->boardMultiCity();
  if ((!isUS(loc1)) && (!_openJaw))
  {
    std::vector<const Loc*>::const_iterator hidStopsI;
    travelSeg = itin.travelSeg()[startIndex];
    for (hidStopsI = travelSeg->hiddenStops().begin(); hidStopsI != travelSeg->hiddenStops().end();
         ++hidStopsI)
      if (isUS(*hidStopsI))
      {
        loc1 = *hidStopsI;
        locCode1 = (*hidStopsI)->city();
        break;
      }
  }
  if (trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX && trx.getTrxType() != PricingTrx::IS_TRX)
  {
    loc2 = itin.travelSeg()[endIndex]->destination();
    locCode2 = itin.travelSeg()[endIndex]->offMultiCity();
  }
  else
  {
    loc2 = itin.travelSeg()[startIndex]->destination();
    locCode2 = itin.travelSeg()[startIndex]->offMultiCity();
  }
  if ((!isUS(loc2)) && (!_openJaw))
  {
    std::vector<const Loc*>::const_reverse_iterator hidStopsI;
    travelSeg = itin.travelSeg()[endIndex];
    for (hidStopsI = travelSeg->hiddenStops().rbegin();
         hidStopsI != travelSeg->hiddenStops().rend();
         ++hidStopsI)
      if (isUS(*hidStopsI))
      {
        loc2 = *hidStopsI;
        locCode2 = (*hidStopsI)->city();
        break;
      }
  }

  if (!(LocUtil::isHawaii(*loc1) || LocUtil::isHawaii(*loc2) || LocUtil::isAlaska(*loc1) ||
        LocUtil::isAlaska(*loc2)))
    return taxCodeReg.taxAmt();

  if ((LocUtil::isHawaii(*loc1) && LocUtil::isHawaii(*loc2)) ||
      (LocUtil::isAlaska(*loc1) && LocUtil::isAlaska(*loc2)))
    return taxCodeReg.taxAmt();

  if (LocUtil::isHawaii(*loc1) && !LocUtil::isHawaii(*loc2))
    return taxUtil::locateHiFactor(trx, locCode2);

  if (!LocUtil::isHawaii(*loc1) && LocUtil::isHawaii(*loc2))
    return taxUtil::locateHiFactor(trx, locCode1);

  if (LocUtil::isAlaska(*loc1) && !LocUtil::isAlaska(*loc2))
    return taxUtil::locateAkFactor(trx, loc1, locCode2);

  return taxUtil::locateAkFactor(trx, loc2, locCode1);
}

void
TaxUS1_01::convertCurrency(MoneyAmount& taxableAmount,
                           PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg)
{
  CurrencyConversionFacade ccFacade;
  Money targetMoney(_paymentCurrency);
  targetMoney.value() = 0;
  _paymentCurrencyNoDec = targetMoney.noDec(trx.ticketingDate());

  if (taxResponse.farePath()->calculationCurrency() != taxResponse.farePath()->baseFareCurrency())
  {
    Money targetMoneyOrigination(taxResponse.farePath()->baseFareCurrency());
    targetMoneyOrigination.value() = 0;
    Money sourceMoneyCalculation(taxableAmount, taxResponse.farePath()->calculationCurrency());
    if (!ccFacade.convert(targetMoneyOrigination,
                          sourceMoneyCalculation,
                          trx,
                          taxResponse.farePath()->itin()->useInternationalRounding()))
    {
      LOG4CXX_WARN(logger, "Currency Convertion Collection *** Tax::taxCreate ***");
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
    }
    taxableAmount = targetMoneyOrigination.value();
  }

  if (taxResponse.farePath()->baseFareCurrency() != _paymentCurrency)
  {
    Money sourceMoney(taxableAmount, taxResponse.farePath()->baseFareCurrency());

    if (!ccFacade.convert(targetMoney, sourceMoney, trx, false))
    {
      LOG4CXX_WARN(logger, "Currency Convertion Collection *** Tax::taxCreate ***");
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
    }
    taxableAmount = targetMoney.value();
  }
}

uint32_t
TaxUS1_01::calculateMiles(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          std::vector<TravelSeg*>& travelSegs,
                          const Loc& origin,
                          const Loc& destination)
{
  GlobalDirection gd = GlobalDirection::XX;
  DateTime travelDate = taxResponse.farePath()->itin()->travelDate();
  GlobalDirectionFinderV2Adapter::getGlobalDirection(&trx, travelDate, travelSegs, gd);

  if (gd == GlobalDirection::XX)
  {
    LOG4CXX_DEBUG(logger, "GlobalDirection Not Located");
    return 0;
  }

  return LocUtil::getTPM(
      origin, destination, gd, trx.getRequest()->ticketingDT(), trx.dataHandle());
}

bool
TaxUS1_01::locateOpenJaw(PricingTrx& trx, TaxResponse& taxResponse)
{
  if (trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX || trx.getTrxType() == PricingTrx::IS_TRX)
    return false;

  Itin* itin = taxResponse.farePath()->itin();

  TravelSeg* travelSegBack = itin->travelSeg().back();
  TravelSeg* travelSegFront = itin->travelSeg().front();

  if (!LocUtil::isUS(*(travelSegFront->origin())) ||
      LocUtil::isUSTerritoryOnly(*(travelSegFront->origin())))
    return false;

  if (!LocUtil::isUS(*(travelSegBack->destination())) ||
      LocUtil::isUSTerritoryOnly(*(travelSegBack->destination())))
    return false;

  if (travelSegFront->origin()->loc() == travelSegBack->destination()->loc())
    return false;

  bool intPoint = false;
  std::vector<TravelSeg*>::iterator travelSegI = itin->travelSeg().begin();
  for (; travelSegI != itin->travelSeg().end(); travelSegI++)
  {
    if (!isUS((*travelSegI)->origin()) || !isUS((*travelSegI)->destination()))
      intPoint = true;

    if (((*travelSegI)->origin()->area() != IATA_AREA1) ||
        ((*travelSegI)->origin()->subarea() == IATA_SUB_AREA_14()))
    {
      return false;
    }

    if (((*travelSegI)->destination()->area() != IATA_AREA1) ||
        ((*travelSegI)->destination()->subarea() == IATA_SUB_AREA_14()))
    {
      return false;
    }
  }
  if (!intPoint)
    return false;

  uint32_t startToEndMiles = calculateMiles(trx,
                                            taxResponse,
                                            itin->travelSeg(),
                                            *(travelSegFront->origin()),
                                            *(travelSegBack->destination()));

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
  {
    std::ostringstream stream;
    stream << "TRY OPEN JAW:\n";
    stream << travelSegFront->origin()->loc() << travelSegBack->destination()->loc();
    stream << " " << startToEndMiles << std::endl;
    trx.diagnostic().insertDiagMsg(stream.str());
  }

  if ((!locRestrictionValidator().fareBreaksFound()))
    locRestrictionValidator().findFareBreaks(*(taxResponse.farePath()));
  locRestrictionValidator().findFarthestPoint(trx, *(taxResponse.farePath()->itin()), 0);
  TravelSeg* travelSegFar = itin->travelSeg()[locRestrictionValidator().getFarthestSegIndex()];

  uint32_t sideMiles = calculateMiles(trx,
                                      taxResponse,
                                      itin->travelSeg(),
                                      *(itin->travelSeg().front()->origin()),
                                      *(travelSegFar->destination()));

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
  {
    std::ostringstream stream;
    stream << travelSegFront->origin()->loc() << travelSegFar->destination()->loc();
    stream << " " << sideMiles << std::endl;
    trx.diagnostic().insertDiagMsg(stream.str());
  }

  if (sideMiles < startToEndMiles && sideMiles != 0)
    return true;

  sideMiles = calculateMiles(trx,
                             taxResponse,
                             itin->travelSeg(),
                             *(travelSegFar->destination()),
                             *(travelSegBack->destination()));

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
  {
    std::ostringstream stream;
    stream << travelSegFar->destination()->loc() << travelSegBack->destination()->loc();
    stream << " " << sideMiles << std::endl;
    trx.diagnostic().insertDiagMsg(stream.str());
  }

  if (sideMiles < startToEndMiles && sideMiles != 0)
    return true;

  return false;
}

RepricingTrx*
TaxUS1_01::getRepricingTrx(PricingTrx& trx,
                           const std::vector<TravelSeg*>& tvlSeg,
                           FMDirection fmDirectionOverride)
{
  RepricingTrx* rpTrx = nullptr;

  try { rpTrx = TrxUtil::reprice(trx, tvlSeg, fmDirectionOverride, false, nullptr, nullptr, "", true, true); }
  catch (const ErrorResponseException& ex)
  {
    LOG4CXX_ERROR(logger,
                  "TaxUS1_00: Exception during repricing with " << ex.code() << " - "
                                                                << ex.message());
    return nullptr;
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, "TaxUS1_00: Unknown exception during repricing");
    return nullptr;
  }

  //  rpTrx->redirectedDiagnostic() = &trx.diagnostic();

  return rpTrx;
}

void
TaxUS1_01::applyTaxOnTax (PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  if (! taxUtil::doUsTaxesApplyOnYQYR(trx, *(taxResponse.farePath())))
    return;

  Tax::applyTaxOnTax(trx, taxResponse, taxCodeReg);
}

}
