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

#include "Taxes/LegacyTaxes/TaxD8_01.h"

#include "Common/GoverningCarrier.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/AdjustTax.h"
#include "Taxes/LegacyTaxes/BaseTaxOnTaxCollector.h"
#include "Taxes/LegacyTaxes/CabinValidator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

namespace tse
{
Logger
TaxD8_01::_logger("atseintl.Taxes.TaxD8_01");

namespace
{

class TaxOnTaxCollector : public BaseTaxOnTaxCollector<TaxD8_01>
{
public:
  TaxOnTaxCollector(TaxD8_01& tax,
      CalculationDetails& details,
      std::vector<TaxItem*>& taxOnTaxItems,
      const std::list<TravelSeg*>& lstValidSeg,
      const Itin& itin,
      const DateTime& ticketingDT,
      const std::vector<TaxSpecConfigReg*>* tscv)
    : Base(tax)
    , _moneyAmount(0.0)
    , _details(details)
    , _taxOnTaxItems(taxOnTaxItems)
    , _lstValidSeg(lstValidSeg)
    , _itin(itin)
    , _filter(ticketingDT, tscv)
    {}

  MoneyAmount getMoneyAmount() const { return _moneyAmount; }

  void collect(const TaxResponse::TaxItemVector& taxItemVec, const TaxCode& taxCode)
  {
    for (TaxItem* taxItem : taxItemVec)
    {
      if (taxCode == taxItem->taxCode())
      {
        bool isValidItem = true;
        bool isYQorYR = taxCode.equalToConst("YQF") || taxCode.equalToConst("YQI") || taxCode.equalToConst("YRF") || taxCode.equalToConst("YRI");

        for (uint16_t index = taxItem->travelSegStartIndex();
            index <= taxItem->travelSegEndIndex(); index++)
        {
          const TravelSeg* seg = _itin.travelSeg()[index];
          if (_filter.isFilteredSegment(*seg, taxCode))
          {
            isValidItem = false;
            break;
          }

          if (isYQorYR &&
              std::find(_lstValidSeg.begin(), _lstValidSeg.end(), seg) == _lstValidSeg.end() )
          {
            isValidItem = false;
            break;
          }
        }

        if ( !isValidItem )
          continue;

        _details.taxableTaxes.push_back(std::make_pair(taxItem->taxCode(), taxItem->taxAmount()));
        _moneyAmount += taxItem->taxAmount();
        _taxOnTaxItems.push_back(taxItem);
      }
    }
  }

protected:
  MoneyAmount                  _moneyAmount;
  CalculationDetails&          _details;
  std::vector<TaxItem*>&       _taxOnTaxItems;
  const std::list<TravelSeg*>& _lstValidSeg;
  const Itin&                  _itin;
  const utc::TaxOnTaxFilterUtc _filter;
};

const char WPNCS_OFF = 'F';
const char* DIAG818_VERBOSE_SWITCH = "VR";

const NationCode MalaysiaCode("MY");

FareUsage*
locateFare(const FarePath& farePath, const TravelSeg* travelSegIn)
{
  for (const PricingUnit* pricingUnit : farePath.pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      for (const TravelSeg* travelSeg : fareUsage->travelSeg())
      {
        if (farePath.itin()->segmentOrder(travelSeg) ==
            farePath.itin()->segmentOrder(travelSegIn))
        {
          return fareUsage;
        }
      }
    }
  }

  return nullptr;
}

bool
isDomesticSegment(TravelSeg* travelSeg)
{
  if (!travelSeg->isAir())
    return false;

  return (travelSeg->origin()->nation() == MalaysiaCode &&
    travelSeg->destination()->nation() == MalaysiaCode);
}

} // namespace


bool
TaxD8_01::locateDomesticSegments(TaxResponse& taxResponse,
                                 TaxLocIterator& locIt,
                                 const PricingTrx& trx)
{
  bool bTaxRequired = false;
  while (!locIt.isEnd())
  {
    bool bDomesticSeg = false;
    if (locIt.hasPrevious())
    {
      if (locIt.prevSeg()->isAir() && isDomesticSegment(locIt.prevSeg()))
      {
        _domesticTravelSegLst.push_back(locIt.prevSeg());
        bDomesticSeg = true;

        //store min-max segment with D8 tax
        uint16_t segmentOrder = taxResponse.farePath()->itin()->segmentOrder(locIt.prevSeg());
        _travelSegStartIndex =
            segmentOrder < _travelSegStartIndex ? segmentOrder : _travelSegStartIndex;
        _travelSegEndIndex =
            segmentOrder > _travelSegEndIndex ? segmentOrder : _travelSegEndIndex;
      }

      LOG4CXX_DEBUG(_logger, "TaxD8_01: Checking segment [segNo: " << locIt.prevSegNo() << "] "
                              << locIt.prevSeg()->origin()->loc() << "-"
                              << locIt.prevSeg()->destination()->loc() <<
                              (bDomesticSeg ? " DOMESTIC" : " INTERNATIONAL"));

      if (!bTaxRequired && locIt.hasNext())
      {
        if (!bDomesticSeg)
          bDomesticSeg = isDomesticSegment(locIt.nextSeg());

        //if prev or next is domestic (Malyasia) then check stopover condition
        if (bDomesticSeg)
        {
          bTaxRequired = locIt.isStop();
          if (bTaxRequired && LOG4CXX_UNLIKELY(IS_DEBUG_ENABLED(_logger)))
          {
            LOG4CXX_DEBUG(_logger,
                          "TaxD8_01: D8 should apply due to stopover between: "
                              << locIt.prevSeg()->origin()->loc() << "-"
                              << locIt.nextSeg()->destination()->loc()
                              << " segNo:" << locIt.prevSegNo());
          }

        } // if (bDomesticSeg)
      } //if (!bTaxRequired && locIt.hasNext())
    } //if (locIt.hasPrevious())
    locIt.next();
  } //while (!locIt.isEnd())

  return bTaxRequired;
}

void
TaxD8_01::removeConnectionSegmentsR5(Diagnostic& diag,
                                     std::list<TravelSeg*>& lstSeg,
                                     TaxLocIterator& locIt,
                                     const PricingTrx& trx) const
{
  //remove connection segments from begin
  locIt.toFront();

  //find first international
  while (locIt.hasNext() && isDomesticSegment(locIt.nextSeg()))
    locIt.next();

  while (locIt.hasNext() && !lstSeg.empty() )
  {
    //find first domestic segment
    while (locIt.hasNext() && !isDomesticSegment(locIt.nextSeg()))
      locIt.next();

    while (locIt.hasNext() && isDomesticSegment(locIt.nextSeg()))
    {
      if (locIt.isStop())
        break;

      if (diag.diagnosticType() == Diagnostic818)
      {
        std::ostringstream stream;
        stream << "CONNECTION SEGMENT FOUND: " << locIt.nextSeg()->origin()->loc()
               << locIt.nextSeg()->destination()->loc() << "" << std::endl;
        diag.insertDiagMsg(stream.str());
      }

      lstSeg.remove(locIt.nextSeg());
      locIt.next();
    }

    //find next international
    while (locIt.hasNext() && isDomesticSegment(locIt.nextSeg()))
      locIt.next();
  }

  //remove connection segments from end
  locIt.toBack();
  //find last international
  while (locIt.hasPrevious() && isDomesticSegment(locIt.prevSeg()))
    locIt.previous();

  while (locIt.hasPrevious() && !lstSeg.empty() )
  {
    //find last domestic segment
    while (locIt.hasPrevious() && !isDomesticSegment(locIt.prevSeg()))
      locIt.previous();

    while (locIt.hasPrevious() && isDomesticSegment(locIt.prevSeg()))
    {
      if (locIt.isStop())
        break;

      if (diag.diagnosticType() == Diagnostic818)
      {
        std::ostringstream stream;
        stream << "CONNECTION SEGMENT FOUND: " << locIt.prevSeg()->origin()->loc()
               << locIt.prevSeg()->destination()->loc() << "" << std::endl;
        diag.insertDiagMsg(stream.str());
      }

      lstSeg.remove(locIt.prevSeg());
      locIt.previous();
    }

    //find previous international
    while (locIt.hasPrevious() && isDomesticSegment(locIt.prevSeg()))
      locIt.previous();
  }
}

bool
TaxD8_01::validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  _domesticTravelSegLst.clear();

  return Tax::validateItin(trx, taxResponse, taxCodeReg);
}

void
TaxD8_01::taxCreate(PricingTrx& trx,
                    TaxResponse& taxResponse,
                    TaxCodeReg& taxCodeReg,
                    uint16_t travelSegStartIndex,
                    uint16_t travelSegEndIndex)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic818)
    trx.diagnostic().insertDiagMsg("***\n***D8 START SPN PROCESSING***\n***\n");

  Tax::taxCreate(trx, taxResponse, taxCodeReg, travelSegStartIndex, travelSegEndIndex);

  _taxSplitDetails.setFareSumAmount(0.0);

  LOG4CXX_DEBUG(_logger, "TaxD8_01::taxCreate");
  LOG4CXX_DEBUG(_logger, "TaxD8_01: payment currency " << _paymentCurrency);
  _failCode = TaxDiagnostic::NONE;
  _taxAmount = _taxableFare = _taxableBaseFare = 0;

  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));

  locIt->setSkipHidden(true);
  locIt->toSegmentNo(0);
  locIt->setStopHours(24);

  if (!locateDomesticSegments(taxResponse, *locIt, trx))
  {
    if (trx.diagnostic().diagnosticType() == Diagnostic818)
      trx.diagnostic().insertDiagMsg("D8 APPLIES ONLY ON TAXES WITHOUT FAREBASE\n");

    _domesticTravelSegLst.clear();

    LOG4CXX_DEBUG(_logger, "TaxD8_01: TaxFare amount is not required");
  }
  else
  {
    removeConnectionSegmentsR5(trx.diagnostic(), _domesticTravelSegLst, *locIt, trx);

    if (trx.diagnostic().diagnosticType() == Diagnostic818)
    {
      std::ostringstream stream;

      if (_domesticTravelSegLst.empty())
        stream << "D8 APPLIES ONLY ON TAXES WITHOUT FAREBASE" << std::endl;
      else
        stream << "FAREBASE WILL BE CALCULATED FOR " << _domesticTravelSegLst.size() <<
          " DOMESTIC SEGMENTS" << std::endl;

      trx.diagnostic().insertDiagMsg(stream.str());
    }

    LOG4CXX_DEBUG(_logger, "TaxD8_01: Calculate TaxFare amount");

    calculateTaxFareAmount(trx, taxResponse, taxCodeReg, _domesticTravelSegLst);
  }

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
    trx.diagnostic().insertDiagMsg("***\n***D8 END SPN PROCESSING***\n***\n");
}

void
TaxD8_01::calculateTaxFareAmount(PricingTrx& trx,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg,
                                 std::list<TravelSeg*> lstSeg)
{
  for (std::list<TravelSeg*>::iterator it = lstSeg.begin();
       it != lstSeg.end() && !lstSeg.empty();)
  {
    MoneyAmount taxableFare = 0;

    FareUsage* fareUsage = locateFare(*taxResponse.farePath(), (*it));
    if (fareUsage)
    {
      taxableFare = fareUsage->totalFareAmount();

      if (trx.diagnostic().diagnosticType() == Diagnostic818)
      {
        std::ostringstream stream;
        stream.setf(std::ios::fixed, std::ios::floatfield);
        stream.precision(2);
        stream << "SEGMENT: " << (*it)->origin()->loc() << (*it)->destination()->loc()
            << " IN FAREBASIS: " << fareUsage->paxTypeFare()->createFareBasis(trx)
            << " TOTAL AMOUNT: " << taxableFare << std::endl;
        trx.diagnostic().insertDiagMsg(stream.str());
      }

      std::vector<TravelSeg*> tvlSeg1, tvlSeg2;
      uint32_t unMilesDomestic = 0;
      uint32_t unMilesTotal = 0;

      //we are looking for interanational and domestic parts of fare:
      //tvlSeg1 - tvlSeg1 - DOM - DOM - DOM - tvlSeg2 - tvlSeg2
      //additionally milages are calculated
      for(std::vector<TravelSeg*>::const_iterator itFare = fareUsage->travelSeg().begin();
        itFare!=fareUsage->travelSeg().end(); itFare++)
      {
        if ( !(*itFare)->isAir())
          continue;

        uint32_t unMiles = taxUtil::calculateMiles(trx, taxResponse,
                    *(*itFare)->origin(), *(*itFare)->destination(), fareUsage->travelSeg());

        if (trx.diagnostic().diagnosticType() == Diagnostic818)
        {
          trx.diagnostic().insertDiagMsg(std::to_string(unMiles));
          trx.diagnostic().insertDiagMsg("MILES\n");
        }

        unMilesTotal += unMiles;

        if (it != lstSeg.end() &&
            (taxResponse.farePath()->itin()->segmentOrder(*itFare)
            == taxResponse.farePath()->itin()->segmentOrder(*it)))
        {
          unMilesDomestic += unMiles;
          ++it;
        }
        else //international parts, add segment to proper vector
        {
          unMilesDomestic==0 ? tvlSeg1.push_back(*itFare) : tvlSeg2.push_back(*itFare);
        }
      }

      if (unMilesDomestic == unMilesTotal)
      {
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
        {
          trx.diagnostic().insertDiagMsg("FULLY DOMESTIC FARE - TAKE TOTAL AMOUNT\n");
          std::ostringstream stream;
          stream.setf(std::ios::fixed, std::ios::floatfield);
          stream.precision(4);
          stream << "TAXABLE AMOUNT: " << taxableFare <<std::endl;
          trx.diagnostic().insertDiagMsg(stream.str());
        }

        LOG4CXX_DEBUG(_logger, "TaxD8_01: Whole domestic fare found: "
                               << taxableFare << " Seqno: " << (*it)->pnrSegment() << " "
                               << (*it)->origin()->loc() << "-" << (*it)->destination()->loc());

      }
      else //alike or proreated method
      {
        const PaxTypeFare* paxFare1 = nullptr;
        const PaxTypeFare* paxFare2 = nullptr;

        if (!tvlSeg1.empty())
          paxFare1 = getLikeFare(trx, fareUsage, tvlSeg1, taxResponse.paxTypeCode(),
                                 *taxResponse.farePath()->itin());
        if (!tvlSeg2.empty())
          paxFare2 = getLikeFare(trx, fareUsage, tvlSeg2, taxResponse.paxTypeCode(),
                                 *taxResponse.farePath()->itin());

        //a like fare(s)
        if ((tvlSeg1.empty() || paxFare1) && (tvlSeg2.empty() || paxFare2))
        {
          taxableFare = aLikeMethod(trx, taxableFare, paxFare1, paxFare2, fareUsage);
        }
        else
        {
          taxableFare = prorateMethod(trx, taxableFare, unMilesTotal, unMilesDomestic);
        }
      }

      applyPartialAmount(trx, taxResponse, taxCodeReg, taxableFare, lstSeg.front());
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

MoneyAmount
TaxD8_01::prorateMethod(PricingTrx& trx, MoneyAmount taxableFare, uint32_t unMilesTotal,
                        uint32_t unMilesDomestic) const
{
  double ratio = unMilesTotal >= 0 ? (double)unMilesDomestic/unMilesTotal : 1.0;

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
  {
    trx.diagnostic().insertDiagMsg("PRORATED METHOD\n");

    std::ostringstream stream;
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(4);

    stream << "TOTAL MILES: " << unMilesTotal << " DOMESTIC MILES: "
           << unMilesDomestic << " RATIO: " << ratio << std::endl;
    trx.diagnostic().insertDiagMsg(stream.str());

    stream.str("");
    stream << "TOTAL AMOUNT: " << taxableFare << " TAXABLE AMOUNT: "
           << taxableFare * ratio << std::endl;
    trx.diagnostic().insertDiagMsg(stream.str());
  }

  return taxableFare * ratio;
}

MoneyAmount
TaxD8_01::subtractInternationalQSurcharges(PricingTrx& trx,
                                           MoneyAmount taxableFare,
                                           const FareUsage* fareUsage) const
{
  if (trx.diagnostic().diagnosticType() == Diagnostic818)
    trx.diagnostic().insertDiagMsg("SEARCHING INTERNATIONAL SURCHARGES\n");

  for (const SurchargeData* sur : fareUsage->surchargeData())
  {
    const Loc* loc1 = trx.dataHandle().getLoc(sur->fcBrdCity(), trx.getRequest()->ticketingDT());
    const Loc* loc2 = trx.dataHandle().getLoc(sur->fcOffCity(), trx.getRequest()->ticketingDT());

    bool isInternational = loc1->nation() != MalaysiaCode || loc2->nation() != MalaysiaCode;
    if (isInternational)
      taxableFare -= sur->amountNuc();

    if (trx.diagnostic().diagnosticType() == Diagnostic818)
    {
      std::ostringstream stream;
      stream << loc1->loc() << "(" << loc1->nation() << ")"
        << "-" << loc2->loc() << "(" << loc2->nation() << ")"
        << " NUC-" << sur->amountNuc()
        << " COUNT-" << sur->itinItemCount()
        << " " << sur->surchargeDesc();

      if ( trx.diagnostic().diagParamMapItemPresent(DIAG818_VERBOSE_SWITCH) )
      {
        stream << " APPL-" << sur->surchargeAppl()
          << " TYPE-" << sur->surchargeType()
          << " TRAVPORT-" << sur->travelPortion();
      }

      if (isInternational)
        stream << " - INTERNATIONAL, SUBTRACT AMOUNT";
      else
        stream << " - DOMESTIC, SKIP";


      stream << std::endl;
      trx.diagnostic().insertDiagMsg(stream.str());
    }
  }

  return taxableFare;
}

MoneyAmount
TaxD8_01::aLikeMethod(PricingTrx& trx, MoneyAmount taxableFare, const PaxTypeFare* paxFare1,
                      const PaxTypeFare* paxFare2, const FareUsage* fareUsage) const
{
  if (paxFare1)
  {
    if (trx.diagnostic().diagnosticType() == Diagnostic818)
    {
      std::ostringstream stream;
      stream.setf(std::ios::fixed, std::ios::floatfield);
      stream.precision(2);
      stream << "SUBTRACT A LIKE FARE AMOUNT: " << paxFare1->totalFareAmount() << std::endl;
      trx.diagnostic().insertDiagMsg(stream.str());
    }
    taxableFare -=  paxFare1->totalFareAmount();
  }

  if (paxFare2)
  {
    if (trx.diagnostic().diagnosticType() == Diagnostic818)
    {
      std::ostringstream stream;
      stream.setf(std::ios::fixed, std::ios::floatfield);
      stream.precision(2);
      stream << "SUBTRACT A LIKE FARE AMOUNT: " << paxFare2->totalFareAmount() << std::endl;
      trx.diagnostic().insertDiagMsg(stream.str());
    }
    taxableFare -=  paxFare2->totalFareAmount();
  }

  if (taxableFare > 0)
    taxableFare = subtractInternationalQSurcharges(trx, taxableFare, fareUsage);

  if (taxableFare < 0)
    taxableFare = 0;

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
  {
    std::ostringstream stream;
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(4);
    stream << "TAXABLE AMOUNT: " << taxableFare << std::endl;
    trx.diagnostic().insertDiagMsg(stream.str());
  }

  return taxableFare;
}

PricingTrx*
TaxD8_01::getFareMarkets(PricingTrx& trx,
                         const FareUsage* fareUsage,
                         std::vector<TravelSeg*>& tvlSeg,
                         const PaxTypeCode& paxTypeCode,
                         std::vector<FareMarket*>& vFareMarkets) const
{
  PricingTrx* trxTmp = &trx;

  TrxUtil::getFareMarket(trx, tvlSeg, fareUsage->paxTypeFare()->retrievalDate(), vFareMarkets);
  if (vFareMarkets.empty())
  {
    if (!TrxUtil::isPricingTaxRequest(&trx))
    {
      if (trx.diagnostic().diagnosticType() == Diagnostic818)
        trx.diagnostic().insertDiagMsg("REPRICING IS NEEDED\n");

      trxTmp = getRepricingTrx(trx, tvlSeg, WPNCS_OFF, paxTypeCode, false);

      if (!trxTmp)
      {
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
          trx.diagnostic().insertDiagMsg("UNABLE TO REPRICE - END A LIKE FARE SEARCHING\n");
        return trxTmp;
      }

      TrxUtil::getFareMarket( *trxTmp, tvlSeg, fareUsage->paxTypeFare()->retrievalDate(),
          vFareMarkets);
    }
  } //if (retFareMarket.empty())

  return trxTmp;
}

const PaxTypeFare*
TaxD8_01::getLikeFare(PricingTrx& trx,
                      const FareUsage* fareUsage,
                      std::vector<TravelSeg*>& tvlSeg,
                      const PaxTypeCode& paxTypeCode,
                      const Itin& itin) const
{
  std::string fareBasisOrg = fareUsage->paxTypeFare()->createFareBasis(trx, false);

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
  {
    std::ostringstream stream;
    stream << "SEARCHING A LIKE FARE FOR " << tvlSeg.size() << " SEGMENTS "
      << tvlSeg.front()->origin()->loc() << tvlSeg.back()->destination()->loc() << std::endl;
    trx.diagnostic().insertDiagMsg(stream.str());

    stream.str("");
    stream << "ORIGINAL BASIS-" << fareBasisOrg
      << " FTYPE-" << fareUsage->paxTypeFare()->fcaFareType()
      << " FRULE-" << fareUsage->paxTypeFare()->ruleNumber()
      << std::endl;

    if ( trx.diagnostic().diagParamMapItemPresent(DIAG818_VERBOSE_SWITCH) )
      stream << " REVERSE-" << fareUsage->paxTypeFare()->isReversed()
             << " RT-" << fareUsage->paxTypeFare()->isRoundTrip()
             << " OUTBOUND-" << fareUsage->isOutbound()
             << " DIRECTION-" << fareUsage->paxTypeFare()->directionality()
             << " CMDPRICING-" << fareUsage->paxTypeFare()->isCmdPricing()
             << std::endl;

    trx.diagnostic().insertDiagMsg(stream.str());
  }

  std::vector<FareMarket*> retFareMarket;
  PricingTrx* trxTmp = getFareMarkets(trx, fareUsage, tvlSeg, paxTypeCode, retFareMarket);

  if (retFareMarket.empty())
  {
    if (trx.diagnostic().diagnosticType() == Diagnostic818)
      trx.diagnostic().insertDiagMsg("FARE MARKET WAS NOT FOUND - END A LIKE FARE SEARCHING\n");
    return nullptr;
  }

  const PaxTypeFare* alikefare = nullptr;
  for (FareMarket* fareMarket : retFareMarket)
  {
    const std::vector<PaxTypeFare*>* paxTypeFare = taxUtil::locatePaxTypeFare(fareMarket, paxTypeCode);
    if (!paxTypeFare)
      continue;

    for (const PaxTypeFare* fare : *paxTypeFare)
    {
      if (!fare->isValid())
        continue;

      std::string fareBasis = fare->createFareBasis(trxTmp, false);

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

      Directionality dir = fare->isRoundTrip() && fareUsage->isInbound() ? TO : FROM;

      //APM-766, fare has different direction than pricing unit
      //finally we have two OUTBOUND pricing units, but fareusage is INBOUND
      if (fareUsage->isInbound() && fareUsage->paxTypeFare()->directionality() == FROM)
        dir = FROM;

      if (fare->directionality()!=dir)
      {
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
          trx.diagnostic().insertDiagMsg(" - WRONG DIR\n");
         continue;
      }

      if (fareBasis == fareBasisOrg)
      {
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
          trx.diagnostic().insertDiagMsg(" - SAME BASIS\n");

        //for command pricing, the same farebasis is not enough
        if (fareUsage->paxTypeFare()->isCmdPricing())
        {
          if (fare->ruleNumber() != fareUsage->paxTypeFare()->ruleNumber())
          {
            if (trx.diagnostic().diagnosticType() == Diagnostic818)
              trx.diagnostic().insertDiagMsg("   COMMAND PRICING - DIFF RULES\n");

            continue;
          }
        }

        alikefare = fare;
        break;
      }

      if (alikefare)
      {
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
          trx.diagnostic().insertDiagMsg(" - DIFF BASIS\n");

        continue;
      }

      if (fare->fcaFareType() != fareUsage->paxTypeFare()->fcaFareType())
      {
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
          trx.diagnostic().insertDiagMsg(" - DIFF TYPES\n");

        continue;
      }

      if (fare->ruleNumber() != fareUsage->paxTypeFare()->ruleNumber())
      {
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
          trx.diagnostic().insertDiagMsg(" - DIFF RULES\n");

        continue;
      }

      bool bBookingCode = true;
      for (TravelSeg* seg : tvlSeg)
      {
         BookingCode bk1 = getBookingCode(itin, *fare,
             fare->fareMarket()->travelSeg(), seg);
         BookingCode bk2 = getBookingCode(itin, *fareUsage->paxTypeFare(),
             fareUsage->travelSeg(), seg);

         if (bk1!=bk2)
         {
           bBookingCode = false;
           break;
         }
      }

      if (!bBookingCode)
      {
        if (trx.diagnostic().diagnosticType() == Diagnostic818)
          trx.diagnostic().insertDiagMsg(" - DIFF BOOKING CODE\n");

        continue;
      }

      if (trx.diagnostic().diagnosticType() == Diagnostic818)
      {
        trx.diagnostic().insertDiagMsg(" - FARE MATCH\n");
        trx.diagnostic().insertDiagMsg("POTENTIALLY A LIKE FARE BUT LOOKING FOR THE SAME FARE BASIS\n");
      }

      if (!alikefare)
        alikefare = fare;

    } // for (const PaxTypeFare* fare : *paxTypeFare)

    if (alikefare)
      break;

  } // for (FareMarket* fareMarket : retFareMarket)

  if (alikefare && trx.diagnostic().diagnosticType() == Diagnostic818)
    trx.diagnostic().insertDiagMsg("A LIKE FARE FOUND\n");

  return alikefare;
}

BookingCode
TaxD8_01::getBookingCode(const Itin& itin,
                         const PaxTypeFare& fare,
                         const std::vector<TravelSeg*>& segs,
                         const TravelSeg* travelSegment) const
{
  size_t i = 0;
  int segOrder = itin.segmentOrder(travelSegment);
  for (const TravelSeg* fuTravelSegment : segs)
  {
    if (itin.segmentOrder(fuTravelSegment) == segOrder)
    {
      if (i < fare.segmentStatus().size())
      {
        const PaxTypeFare::SegmentStatus& segmentStatus = fare.segmentStatus()[i];
        if (!segmentStatus._bkgCodeReBook.empty() &&
            segmentStatus._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
        {
          return segmentStatus._bkgCodeReBook;
        }
      }
    }
    ++i;
  }

  return travelSegment->getBookingCode();
}

void
TaxD8_01::applyPartialAmount(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             MoneyAmount taxableFare,
                             const TravelSeg* travelSeg)
{
  std::ostringstream stream;

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
  {
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(4);
    stream << "APPLY PARTIAL FARE: " << taxableFare << " /*";
  }

  taxableFare = taxUtil::convertCurrency(trx, taxableFare, _paymentCurrency,
      taxResponse.farePath()->calculationCurrency(),
      taxResponse.farePath()->baseFareCurrency(),
      CurrencyConversionRequest::FARES,
      taxResponse.farePath()->itin()->useInternationalRounding());

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
    stream << taxableFare << _paymentCurrency << "*/ ";

  Percent discPercent = trx.getRequest()->discountPercentage(travelSeg->segmentOrder());
  if (discPercent >= 0 && discPercent <= 100)
  {
    LOG4CXX_DEBUG(_logger,
                  "TaxD8_01: discount " << discPercent << " on taxable fare: " << taxableFare);
    taxableFare *= (1.0 - discPercent / 100.0);
  }

  MoneyAmount taxAmount = taxableFare * taxCodeReg.taxAmt();
  LOG4CXX_DEBUG(_logger, "TaxD8_01: tax amount: " << taxableFare << " converted: " << taxAmount);

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

RepricingTrx*
TaxD8_01::getRepricingTrx(PricingTrx& trx,
                          std::vector<TravelSeg*>& tvlSeg,
                          Indicator wpncsFlagIndicator,
                          const PaxTypeCode& extraPaxType,
                          const bool privateFareCheck) const
{
  try
  {
    return TrxUtil::reprice(trx,
                            tvlSeg,
                            FMDirection::UNKNOWN,
                            false,
                            nullptr,
                            nullptr,
                            extraPaxType,
                            false,
                            false,
                            wpncsFlagIndicator,
                            ' ',
                            false,
                            privateFareCheck);
  }
  catch (const ErrorResponseException& ex)
  {
    LOG4CXX_ERROR(_logger, "TaxD8_01: Exception during repricing with "
                           << ex.code() << " - " << ex.message());
  }
  catch (...)
  {
    LOG4CXX_ERROR(_logger, "TaxD8_01: Unknown exception during repricing");
  }

  return nullptr;
}

void
TaxD8_01::applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  TaxOnTaxCollector collector(*this, _calculationDetails, _taxOnTaxItems,
      _domesticTravelSegLst, *(taxResponse.farePath()->itin()),
      trx.getRequest()->ticketingDT(), taxSpecConfig());

  _applyFeeOnTax = true;
  _calculationDetails.isTaxOnTax = true;

  const bool taxShoppingRequest = TrxUtil::isShoppingTaxRequest(&trx)
    && taxResponse.farePath() && !taxResponse.farePath()->getExternalTaxes().empty();

  for (const TaxCode& taxCode : taxCodeReg.taxOnTaxCode())
  {
    collector.collect(taxResponse.taxItemVector(), taxCode);
    if (taxShoppingRequest)
      collector.collect(taxResponse.farePath()->getExternalTaxes(), taxCode);
  }

  _taxableFare += collector.getMoneyAmount();
  _taxAmount += collector.getMoneyAmount() * taxCodeReg.taxAmt();

  _calculationDetails.taxableTaxSumAmount = collector.getMoneyAmount();

  _taxSplitDetails.setUseTaxableTaxSumAmount(true);

  _mixedTax = true;
}

}; //namespace tse
