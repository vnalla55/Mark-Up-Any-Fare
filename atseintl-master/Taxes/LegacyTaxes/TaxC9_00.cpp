//---------------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include <algorithm>
#include "Common/TrxUtil.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/AdjustTax.h"
#include "Taxes/LegacyTaxes/TaxC9_00.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"

namespace tse
{

namespace
{
  const NationCode BahamasCode("BS");
  const char* DIAG818_VERBOSE_SWITCH = "VR";
  const char* DOMESTIC_FARE_BASIS = "SDOM";
  const char* ZED_ACCOUNT = "ZED";
}

bool
TaxC9_00::isDomesticSegment(const TravelSeg& travelSeg) const
{
  if (!travelSeg.isAir())
    return false;

  return (travelSeg.origin()->nation() == BahamasCode &&
    travelSeg.destination()->nation() == BahamasCode);
}

void
TaxC9_00::taxCreate(PricingTrx& trx,
                    TaxResponse& taxResponse,
                    TaxCodeReg& taxCodeReg,
                    uint16_t travelSegStartIndex,
                    uint16_t travelSegEndIndex)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic818)
      trx.diagnostic().insertDiagMsg("***\n***C9 SPN - START PROCESSING***\n***\n");

  bool isDAEntry = false;
  bool isDPEntry = false;
  if (TrxUtil::newDiscountLogic(trx))
  {
    isDAEntry = trx.getRequest()->isDAEntryNew();
    isDPEntry = trx.getRequest()->isDPEntryNew();
  }
  else
  {
    isDAEntry = trx.getRequest()->isDAEntry();
    isDPEntry = trx.getRequest()->isDPEntry();
  }

  const bool bZedFare = trx.getRequest()->accountCode() == ZED_ACCOUNT;

  bool isDiscount = isDPEntry ||
                    isDAEntry ||
                    trx.getRequest()->isTktDesignatorEntry() ||
                    trx.getRequest()->isSpecifiedTktDesignatorEntry() ||
                    bZedFare;

  if (isDiscount && trx.diagnostic().diagnosticType() == Diagnostic818)
  {
    if (isDPEntry)
      trx.diagnostic().insertDiagMsg("DP ENTRY - YES\n");
    if (isDAEntry)
      trx.diagnostic().insertDiagMsg("DA ENTRY - YES\n");
    if (trx.getRequest()->isTktDesignatorEntry())
      trx.diagnostic().insertDiagMsg("TICKET DESIGNATOR ENTRY - YES\n");
    if (trx.getRequest()->isSpecifiedTktDesignatorEntry())
      trx.diagnostic().insertDiagMsg("SPECIFIED TICKET DESIGNATOR ENTRY - YES\n");
    if (bZedFare)
      trx.diagnostic().insertDiagMsg("ZED FARE - YES\n");
  }

  Tax::taxCreate(trx, taxResponse, taxCodeReg, travelSegStartIndex, travelSegEndIndex);

  if (!isDiscount && taxCodeReg.travelType()=='D')
  {
    if (trx.diagnostic().diagnosticType() == Diagnostic818)
    {
      trx.diagnostic().insertDiagMsg("DOMESTIC AND NO DISCOUNT - CALCULATED BY COMMON PROCESS\n");
      trx.diagnostic().insertDiagMsg("***\n***C9 SPN - END PROCESSING***\n***\n");
    }

    return;
  }

  _taxSplitDetails.setFareSumAmount(0.0);

  _failCode = TaxDiagnostic::NONE;
  _taxAmount = _taxableFare = _taxableBaseFare = 0;
  _calculationDetails.fareInPaymentCurrencyAmount = 0;
  _calculationDetails.fareInCalculationCurrencyAmount = 0;
  _calculationDetails.fareInBaseFareCurrencyAmount = 0;

  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
  locIt->setSkipHidden(true);
  locIt->toSegmentNo(0);
  locIt->setStopHours(48);

  std::list<TravelSeg*> domesticTravelSegLst;
  bool isOnFare = true;
  if (taxCodeReg.travelType() == 'D') //domestic
  {
    locateAllDomesticSegments(trx.diagnostic(), *taxResponse.farePath()->itin(),
      *locIt, domesticTravelSegLst);
  }
  else
  {
    isOnFare = locateAllDomesticSegments(trx.diagnostic(), *taxResponse.farePath()->itin(),
        *locIt, domesticTravelSegLst);
  }

  if (!isOnFare)
  {
    if (trx.diagnostic().diagnosticType() == Diagnostic818)
      trx.diagnostic().insertDiagMsg("C9 APPLIES ONLY ON TAXES WITHOUT FAREBASE\n");
  }
  else
  {
    if (trx.diagnostic().diagnosticType() == Diagnostic818)
    {
      std::ostringstream stream;
      stream << "FAREBASE WILL BE CALCULATED FOR " << domesticTravelSegLst.size() <<
          " DOMESTIC SEGMENTS" << std::endl;

      trx.diagnostic().insertDiagMsg(stream.str());
    }

    calculateTaxFareAmount(trx, taxResponse, taxCodeReg, domesticTravelSegLst, isDiscount);
  }

  _taxSplitDetails.setFareSumAmount(_taxableFare);

  _calculationDetails.fareInPaymentCurrencyAmount = _taxableFare;

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
    trx.diagnostic().insertDiagMsg("***\n***C9 SPN - END PROCESSING***\n***\n");
}

void
TaxC9_00::calculateTaxFareAmount(PricingTrx& trx,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg,
                                 std::list<TravelSeg*> lstSeg,
                                 bool isDiscount)
{
  for (std::list<TravelSeg*>::iterator it = lstSeg.begin();
       it != lstSeg.end() && !lstSeg.empty();)
  {
    MoneyAmount taxableFare = 0;

    FareUsage* fareUsage = locateFare(taxResponse.farePath(), (*it));
    if (fareUsage)
    {
      taxableFare = fareUsage->totalFareAmount();

      if (trx.diagnostic().diagnosticType() == Diagnostic818)
      {
        std::ostringstream stream;
        stream.setf(std::ios::fixed, std::ios::floatfield);
        stream.precision(2);
        stream << "***" << std::endl << "SEGMENT: "
            << (*it)->origin()->loc() << (*it)->destination()->loc()
            << " IN FAREBASIS: " << fareUsage->paxTypeFare()->createFareBasis(trx)
            << " TOTAL AMOUNT: " << taxableFare << std::endl;
        trx.diagnostic().insertDiagMsg(stream.str());
      }

      bool isDomesticFare = true;
      std::vector<TravelSeg*> tvlSeg;
      for(TravelSeg* seg : fareUsage->travelSeg())
      {
        if ( !seg->isAir())
          continue;

        if (it != lstSeg.end() &&
            (taxResponse.farePath()->itin()->segmentOrder(seg)
            == taxResponse.farePath()->itin()->segmentOrder(*it)))
        {
          ++it;
        }

        if (isDomesticSegment(*seg))
          tvlSeg.push_back(seg);
        else
          isDomesticFare = false;
      }

      if(isDomesticFare)
      {
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
        {
          std::ostringstream stream;
          stream.setf(std::ios::fixed, std::ios::floatfield);
          stream.precision(4);
          stream << "FULLY DOMESTIC FARE - TAXABLE AMOUNT: " << taxableFare <<std::endl;
          trx.diagnostic().insertDiagMsg(stream.str());
        }

        if (taxableFare>0)
        {
          const PaxTypeFare* sdomFare = nullptr;
          if (isDiscount && tvlSeg.size()==1)
          {
            sdomFare = getDomesticFare(trx, *tvlSeg.front(), fareUsage, taxResponse.paxTypeCode());
            if (sdomFare && sdomFare->createFareBasis(trx, false) != DOMESTIC_FARE_BASIS)
              sdomFare = nullptr;
          }
          applyPartialAmount(trx, taxResponse, taxCodeReg, taxableFare, tvlSeg.front(), sdomFare);
        }
      }
      else
      {
        applyPartialAmountForSegments(trx, taxResponse, taxCodeReg,
            tvlSeg, fareUsage, taxResponse.paxTypeCode());
      }

      lstSeg.erase(lstSeg.begin(), it);
      it = lstSeg.begin();
    }
    else
    {
      lstSeg.erase(it);
      it = lstSeg.begin();
    }
  } //for

  _taxSplitDetails.setFareSumAmount(_taxableFare);
}

void
TaxC9_00::applyPartialAmountForSegments(PricingTrx& trx,
                                        TaxResponse& taxResponse,
                                        TaxCodeReg& taxCodeReg,
                                        const std::vector<TravelSeg*>& tvlSeg,
                                        const FareUsage* fareUsage,
                                        const PaxTypeCode& paxTypeCode)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic818)
    trx.diagnostic().insertDiagMsg("THROUGH FARE - LOOKING FOR DOMESTIC FARES\n");

  for (TravelSeg* seg : tvlSeg)
  {
    const PaxTypeFare* fare = getDomesticFare(trx, *seg, fareUsage, paxTypeCode);

    if (!fare)
      continue;

    if (fare->totalFareAmount()>0)
      applyPartialAmount(trx, taxResponse, taxCodeReg, fare->totalFareAmount(), seg);
  }
}

const PaxTypeFare*
TaxC9_00::getDomesticFare(PricingTrx& trx,
                          TravelSeg& seg,
                          const FareUsage* fareUsage,
                          const PaxTypeCode& paxTypeCode) const
{
  if (trx.diagnostic().diagnosticType() == Diagnostic818)
  {
    std::ostringstream stream;
    stream << "SEARCHING DOMESTIC FARE FOR SEGMENT "
      << seg.origin()->loc() << seg.destination()->loc() << std::endl;
    trx.diagnostic().insertDiagMsg(stream.str());
  }

  std::vector<FareMarket*> retFareMarket;
  PricingTrx* trxTmp = getFareMarkets(trx, fareUsage->paxTypeFare()->retrievalDate(),
      seg, paxTypeCode, retFareMarket);

  if (retFareMarket.empty())
  {
    if (trx.diagnostic().diagnosticType() == Diagnostic818)
      trx.diagnostic().insertDiagMsg("FARE MARKET WAS NOT FOUND - NO POSSIBILTY TO FIND DOMESTIC FARE\n");
    return nullptr;
  }

  const PaxTypeFare* domesticFare = nullptr;
  for (FareMarket* fareMarket: retFareMarket)
  {
    const std::vector<PaxTypeFare*>* paxTypeFare = taxUtil::locatePaxTypeFare(fareMarket, paxTypeCode);
    if (!paxTypeFare)
      continue;

    for (const PaxTypeFare* fare: *paxTypeFare)
    {
      std::string fareBasis = fare->createFareBasis(trxTmp, false);

      if (!fare->isValid() && fareBasis != DOMESTIC_FARE_BASIS)
        continue;

      if (trx.diagnostic().diagnosticType() == Diagnostic818)
      {
        std::ostringstream stream;
        stream << "BASIS-" << fareBasis
          << " TYPE-" << fare->fcaFareType()
          << " RULE-" << fare->ruleNumber()
          << " AMOUNT-" << fare->totalFareAmount();

        if ( trx.diagnostic().diagParamMapItemPresent(DIAG818_VERBOSE_SWITCH) )
          stream << " DIRECTION-" << fare->directionality()
                 << " REVERSE-" << fare->isReversed()
                 << " RT-" << fare->isRoundTrip();

        trx.diagnostic().insertDiagMsg(stream.str());
      }

      Directionality dir = fareUsage->isInbound() ? TO : FROM;

      if (fareUsage->isInbound() && fareUsage->paxTypeFare()->directionality() == FROM)
        dir = FROM;

      if (fare->directionality() != dir)
      {
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
          trx.diagnostic().insertDiagMsg(" - WRONG DIR\n");

        continue;
      }

      if (fareBasis == DOMESTIC_FARE_BASIS)
      {
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
          trx.diagnostic().insertDiagMsg(" - SDOM FARE\n");

        domesticFare = fare;
        break;
      }

      if (domesticFare)
      {
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
          trx.diagnostic().insertDiagMsg(" - SKIP\n");

        continue;
      }

      if (!fare->cabin().isEconomyClass())
      {
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
          trx.diagnostic().insertDiagMsg(" - NOT ECONOMY CLASS\n");

        continue;
      }

      if (trx.diagnostic().diagnosticType() == Diagnostic818)
      {
        trx.diagnostic().insertDiagMsg(" - FARE MATCH\n");
        trx.diagnostic().insertDiagMsg("POTENTIALLY CHEAPEST FARE BUT LOOKING FOR SDOM FARE\n");
      }

      if (!domesticFare)
        domesticFare = fare;

    } //for (const PaxTypeFare* fare: *paxTypeFare)

    if (domesticFare)
      break;

  } //for (FareMarket* fareMarket: retFareMarket)

  if (domesticFare && trx.diagnostic().diagnosticType() == Diagnostic818)
    trx.diagnostic().insertDiagMsg("DOMESTIC FARE FOUND\n");

  return domesticFare;
}

PricingTrx*
TaxC9_00::getFareMarkets(PricingTrx& trx,
                         const DateTime& dateFare,
                         TravelSeg& seg,
                         const PaxTypeCode& paxTypeCode,
                         std::vector<FareMarket*>& vFareMarkets) const
{
  PricingTrx* trxTmp = &trx;

  std::vector<TravelSeg*> tvlSeg = {&seg};
  TrxUtil::getFareMarket(trx, tvlSeg, dateFare, vFareMarkets);
  if (vFareMarkets.empty())
  {
    if (!TrxUtil::isPricingTaxRequest(&trx))
    {
      if (trx.diagnostic().diagnosticType() == Diagnostic818)
        trx.diagnostic().insertDiagMsg("REPRICING IS NEEDED\n");

      try
      {
        trxTmp = TrxUtil::reprice(trx,
          tvlSeg,
          FMDirection::UNKNOWN,
          false,
          nullptr,
          nullptr,
          paxTypeCode,
          false,
          false,
          'F', //WPNCS_OFF
          ' ',
          false,
          false); //private fare
      }
      catch (...)
      {
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
          trx.diagnostic().insertDiagMsg("UNABLE TO REPRICE - INTERNAL ERROR\n");
      }

      if (!trxTmp)
      {
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
          trx.diagnostic().insertDiagMsg("UNABLE TO REPRICE - END FARE SEARCHING\n");
        return trxTmp;
      }

      TrxUtil::getFareMarket( *trxTmp, tvlSeg, dateFare, vFareMarkets);
    }
  } //if (retFareMarket.empty())

  return trxTmp;
}

bool
TaxC9_00::locateAllDomesticSegments(Diagnostic& diag,
                                 const Itin& itin,
                                 TaxLocIterator& locIt,
                                 std::list<TravelSeg*>& lstSeg)
{
  bool isStopOver = false;
  while (!locIt.isEnd())
  {
    if ((locIt.hasPrevious() || locIt.numOfSegs() == 1) && locIt.hasNext())
    {
      if (locIt.hasPrevious() && locIt.prevSeg()->isAir() && isDomesticSegment(*locIt.prevSeg()))
      {
        if (lstSeg.empty() || (lstSeg.back() != locIt.prevSeg()))
        {
          lstSeg.push_back(locIt.prevSeg());

          if (!isStopOver && locIt.isStop())
          {
            isStopOver = true;
            if (diag.diagnosticType() == Diagnostic818)
            {
              std::ostringstream stream;
              stream << "STOPOVER FOUND - AFTER SEGMENT: " << locIt.prevSeg()->origin()->loc()
                << locIt.prevSeg()->destination()->loc() << "" << std::endl;
              diag.insertDiagMsg(stream.str());
            }
          }

          //store min-max segment with D8 tax
          uint16_t segmentOrder = itin.segmentOrder(locIt.prevSeg());
          _travelSegStartIndex = std::min(segmentOrder, _travelSegStartIndex);
          _travelSegEndIndex = std::max(segmentOrder,  _travelSegEndIndex);
        }
      }
      if (locIt.nextSeg()->isAir() && isDomesticSegment(*locIt.nextSeg()))
      {
        lstSeg.push_back(locIt.nextSeg());

        if (!isStopOver && locIt.isStop())
        {
          isStopOver = true;

          if (diag.diagnosticType() == Diagnostic818)
          {
            std::ostringstream stream;
            stream << "STOPOVER FOUND - BEFORE SEGMENT: " << locIt.nextSeg()->origin()->loc()
               << locIt.nextSeg()->destination()->loc() << "" << std::endl;
            diag.insertDiagMsg(stream.str());
          }
        }

        //store min-max segment with D8 tax
        uint16_t segmentOrder = itin.segmentOrder(locIt.nextSeg());
        _travelSegStartIndex = std::min(segmentOrder, _travelSegStartIndex);
        _travelSegEndIndex = std::max(segmentOrder,  _travelSegEndIndex);
      }
    }
    locIt.next();
  }

  return isStopOver;
}


FareUsage*
TaxC9_00::locateFare(const FarePath* farePath, const TravelSeg* travelSegIn) const
{
  for(const PricingUnit* pricingUnit: farePath->pricingUnit())
    for(FareUsage* fareUsage: pricingUnit->fareUsage())
      for(const TravelSeg* travelSeg: fareUsage->travelSeg())
        if (farePath->itin()->segmentOrder(travelSeg) ==
            farePath->itin()->segmentOrder(travelSegIn))
        {
          return fareUsage;
        }

  return nullptr;
}

void
TaxC9_00::applyPartialAmount(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             MoneyAmount taxableFare,
                             const TravelSeg* travelSeg,
                             const PaxTypeFare* sdomFare/*=nullptr*/)
{
  std::ostringstream stream;

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
  {
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(4);
    stream << "APPLY PARTIAL FARE AMOUNT: " << taxableFare << " /*";
  }

  taxableFare = taxUtil::convertCurrency(trx, taxableFare, _paymentCurrency,
      taxResponse.farePath()->calculationCurrency(),
      taxResponse.farePath()->baseFareCurrency(),
      CurrencyConversionRequest::FARES,
      taxResponse.farePath()->itin()->useInternationalRounding());

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
    stream << taxableFare << _paymentCurrency << "*/ ";

  if (sdomFare)
  {
    MoneyAmount taxableSdomFare = taxUtil::convertCurrency(trx, sdomFare->totalFareAmount(), _paymentCurrency,
        taxResponse.farePath()->calculationCurrency(),
        taxResponse.farePath()->baseFareCurrency(),
        CurrencyConversionRequest::FARES,
        taxResponse.farePath()->itin()->useInternationalRounding());

    if (taxableSdomFare > taxableFare)
    {
      if (trx.diagnostic().diagnosticType() == Diagnostic818)
        stream << std::endl << "TAKE SDOM FARE " << taxableSdomFare << _paymentCurrency
          << " BECAUSE IS HIGHER -";
      taxableFare = taxableSdomFare;
    }
  }

  MoneyAmount taxAmount = taxableFare * taxCodeReg.taxAmt();

  taxAmount = AdjustTax::applyAdjust(
      trx, taxResponse, taxAmount, _paymentCurrency, taxCodeReg, _calculationDetails);

  MoneyAmount keepTaxableFare = _taxableFare;
  MoneyAmount keepTaxAmount = _taxAmount;

  _taxableFare = taxableFare;
  _taxAmount = taxAmount;

  doTaxRound(trx, taxCodeReg);

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
    stream << " TAX: " << _taxAmount << std::endl;

  _taxableFare += keepTaxableFare;
  _taxAmount += keepTaxAmount;

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
    trx.diagnostic().insertDiagMsg(stream.str());

  _thruTotalFare = _taxableFare;
}

}; //namespace tse
