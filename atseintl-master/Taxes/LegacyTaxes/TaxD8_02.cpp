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

#include "Taxes/LegacyTaxes/TaxD8_02.h"

#include "Common/FallbackUtil.h"
#include "Common/GoverningCarrier.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/Diagnostic.h"
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

#include <iomanip>

namespace tse
{
Logger
TaxD8_02::_logger("atseintl.Taxes.TaxD8_02");

namespace
{

class TaxOnTaxCollector : public BaseTaxOnTaxCollector<TaxD8_02>
{
public:
  TaxOnTaxCollector(TaxD8_02& tax,
      CalculationDetails& details,
      std::vector<TaxItem*>& taxOnTaxItems,
      const std::vector<TravelSeg*>& segsTaxableForYQYR,
      const Itin& itin,
      const DateTime& ticketingDT,
      const std::vector<TaxSpecConfigReg*>* tscv)
    : Base(tax),
    _moneyAmount(0.0),
    _details(details),
    _taxOnTaxItems(taxOnTaxItems),
    _segsTaxableForYQYR(segsTaxableForYQYR),
    _itin(itin),
    _filter(ticketingDT, tscv)
    {}

  MoneyAmount getMoneyAmount() const { return _moneyAmount; }

  bool
  isSegTaxableForYQYR(const TravelSeg* seg) const
  {
    return std::find(_segsTaxableForYQYR.begin(), _segsTaxableForYQYR.end(), seg) !=
          _segsTaxableForYQYR.end();
  }

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

          if (isYQorYR && !isSegTaxableForYQYR(seg))
          {
            isValidItem = false;
            break;
          }
        }

        if (!isValidItem)
          continue;

        _details.taxableTaxes.push_back(std::make_pair(taxItem->taxCode(), taxItem->taxAmount()));
        _moneyAmount += taxItem->taxAmount();
        _taxOnTaxItems.push_back(taxItem);
      }
    }
  }

protected:
  MoneyAmount                    _moneyAmount;
  CalculationDetails&            _details;
  std::vector<TaxItem*>&         _taxOnTaxItems;
  const std::vector<TravelSeg*>& _segsTaxableForYQYR;
  const Itin&                    _itin;
  const utc::TaxOnTaxFilterUtc   _filter;
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
isEconomy(const TravelSeg* travelSegIn, const FarePath& farePath)
{
  return locateFare(farePath, travelSegIn)->paxTypeFare()->cabin().isEconomyClass();
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
TaxD8_02::isInList(TravelSeg* travelSeg, const TaxResponse& taxResponse) const
{
  const auto& segmentOrder = taxResponse.farePath()->itin()->segmentOrder(travelSeg);
  for (const auto& listSeg : _domesticTravelSegLst)
  {
    if (taxResponse.farePath()->itin()->segmentOrder(listSeg) == segmentOrder)
      return true;
  }
  return false;
}

void
TaxD8_02::locateDomesticSegments(TaxResponse& taxResponse,
                                 TaxLocIterator& locIt,
                                 const PricingTrx& trx)
{
  bool isTaxRequired = false;
  while (!locIt.isEnd())
  {
    bool isSegDomestic = false;
    if (locIt.hasPrevious())
    {
      if (locIt.prevSeg()->isAir() && isDomesticSegment(locIt.prevSeg()))
      {
        _domesticTravelSegLst.push_back(locIt.prevSeg());
        isSegDomestic = true;

        //store min-max segment with D8 tax
        uint16_t segmentOrder = taxResponse.farePath()->itin()->segmentOrder(locIt.prevSeg());
        _travelSegStartIndex =
            segmentOrder < _travelSegStartIndex ? segmentOrder : _travelSegStartIndex;
        _travelSegEndIndex =
            segmentOrder > _travelSegEndIndex ? segmentOrder : _travelSegEndIndex;
      }

      LOG4CXX_DEBUG(_logger, "TaxD8_02: Checking segment [segNo: " << locIt.prevSegNo() << "] "
                              << locIt.prevSeg()->origin()->loc() << "-"
                              << locIt.prevSeg()->destination()->loc() <<
                              (isSegDomestic ? " DOMESTIC" : " INTERNATIONAL"));

      if (!isTaxRequired && locIt.hasNext())
      {
        if (!isSegDomestic)
          isSegDomestic = isDomesticSegment(locIt.nextSeg());

        //if prev or next is domestic (Malyasia) then check stopover condition
        if (isSegDomestic)
        {
          isTaxRequired = locIt.isStop();
          if (isTaxRequired)
          {
            LOG4CXX_DEBUG(_logger,
                          "TaxD8_02: D8 should apply due to stopover between: "
                              << locIt.prevSeg()->origin()->loc() << "-"
                              << locIt.nextSeg()->destination()->loc()
                              << " segNo:" << locIt.prevSegNo());
          }

        }
      }
    }
    locIt.next();
  }
}

void
TaxD8_02::removeConnectionSegmentsR5(DiagManager& diag,
                                     TaxLocIterator& locIt,
                                     const PricingTrx& trx)
{
  //remove connection segments from begin
  locIt.toFront();

  //find first international
  while (locIt.hasNext() && isDomesticSegment(locIt.nextSeg()))
    locIt.next();

  while (locIt.hasNext() && !_domesticTravelSegLst.empty() )
  {
    //find first domestic segment
    while (locIt.hasNext() && !isDomesticSegment(locIt.nextSeg()))
      locIt.next();

    while (locIt.hasNext() && isDomesticSegment(locIt.nextSeg()))
    {
      if (locIt.isStop())
        break;

      diag << "CONNECTION SEGMENT FOUND: " << locIt.nextSeg()->origin()->loc()
           << locIt.nextSeg()->destination()->loc() << std::endl;

      _domesticTravelSegLst.remove(locIt.nextSeg());
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

  while (locIt.hasPrevious() && !_domesticTravelSegLst.empty() )
  {
    //find last domestic segment
    while (locIt.hasPrevious() && !isDomesticSegment(locIt.prevSeg()))
      locIt.previous();

    while (locIt.hasPrevious() && isDomesticSegment(locIt.prevSeg()))
    {
      if (locIt.isStop())
        break;

      diag << "CONNECTION SEGMENT FOUND: " << locIt.prevSeg()->origin()->loc()
           << locIt.prevSeg()->destination()->loc() << std::endl;

      _domesticTravelSegLst.remove(locIt.prevSeg());
      locIt.previous();
    }

    //find previous international
    while (locIt.hasPrevious() && isDomesticSegment(locIt.prevSeg()))
      locIt.previous();
  }
}

bool
TaxD8_02::validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  _domesticTravelSegLst.clear();
  _segsTaxableForYQYR.clear();

  return Tax::validateItin(trx, taxResponse, taxCodeReg);
}

bool
TaxD8_02::isRuralExempted(TravelSeg* travelSeg,
                          PricingTrx& trx,
                          TaxResponse& taxResponse,
                          TaxCodeReg& taxCodeReg) const
{
  DiagManager diag(trx, Diagnostic818);

  if (!CabinValidator().validateCabinRestriction(trx, taxResponse, taxCodeReg, travelSeg) &&
      isEconomy(travelSeg, *(taxResponse.farePath())))
  {
    diag << "CABIN FILTER FOUND: " << travelSeg->origin()->loc()
         << travelSeg->destination()->loc() << std::endl;
    return true;
  }
  return false;
}

void
TaxD8_02::taxCreate(PricingTrx& trx,
                    TaxResponse& taxResponse,
                    TaxCodeReg& taxCodeReg,
                    uint16_t travelSegStartIndex,
                    uint16_t travelSegEndIndex)
{
  DiagManager diag(trx, Diagnostic818);
  diag << "***\n***D8 START SPN PROCESSING***\n***" << std::endl;

  Tax::taxCreate(trx, taxResponse, taxCodeReg, travelSegStartIndex, travelSegEndIndex);
  LOG4CXX_DEBUG(_logger, "TaxD8_02::taxCreate");
  LOG4CXX_DEBUG(_logger, "TaxD8_02: payment currency " << _paymentCurrency);

  _taxSplitDetails.setFareSumAmount(0.0);
  _failCode = TaxDiagnostic::NONE;
  _taxAmount = _taxableFare = _taxableBaseFare = 0;

  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
  locIt->setSkipHidden(true);
  locIt->toSegmentNo(0);
  locIt->setStopHours(24);

  locateDomesticSegments(taxResponse, *locIt, trx);
  removeConnectionSegmentsR5(diag, *locIt, trx);

  if (_domesticTravelSegLst.empty())
    diag << "D8 APPLIES ONLY ON TAXES WITHOUT FAREBASE" << std::endl;
  else
    diag << "FAREBASE WILL BE CALCULATED FOR " << _domesticTravelSegLst.size() <<
      " DOMESTIC SEGMENTS" << std::endl;
  LOG4CXX_DEBUG(_logger, "TaxD8_02: Calculate TaxFare amount");

  calculateTaxFareAmount(trx, taxResponse, taxCodeReg);

  diag << "***\n***D8 END SPN PROCESSING***\n***" << std::endl;
}

void
TaxD8_02::calculateTaxFareAmount(PricingTrx& trx,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg)
{
  DiagManager diag(trx, Diagnostic818);
  diag << std::fixed;

  for (const PricingUnit* pricingUnit : taxResponse.farePath()->pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      std::vector<TravelSeg*> internationalSegsBefore, domesticSegs, internationalSegsAfter;
      uint32_t milesDomestic = 0;
      uint32_t milesTotal = 0;
      bool areAllDomExempted = true;

      MoneyAmount taxableFare = fareUsage->totalFareAmount();
      diag << "  CHECKING FAREBASIS: " << fareUsage->paxTypeFare()->createFareBasis(trx)
           << " TOTAL AMOUNT: " << std::setprecision(2) << taxableFare << std::endl;

      for (TravelSeg* fareSegment : fareUsage->travelSeg())
      {
        if (!(fareSegment->isAir()))
          continue;

        uint32_t miles = taxUtil::calculateMiles(trx, taxResponse,
                         *(fareSegment->origin()), *(fareSegment->destination()), fareUsage->travelSeg());
        diag << std::flush;

        diag << std::to_string(miles) << " MILES" << std::endl;
        milesTotal += miles;
        if (isInList(fareSegment, taxResponse))
        {
          milesDomestic += miles;
          domesticSegs.push_back(fareSegment);
          if (!isRuralExempted(fareSegment, trx, taxResponse, taxCodeReg))
            areAllDomExempted = false;
        }
        else
        {
          (milesDomestic == 0) ? internationalSegsBefore.push_back(fareSegment) :
                                 internationalSegsAfter.push_back(fareSegment);
        }
      }

      if (domesticSegs.empty())
      {
        diag << "NO TRULY DOMESTIC SEGS - SKIPPING FARE USAGE" << std::endl;
        continue;
      }

      if (areAllDomExempted)
      {
        if (utc::shouldExemptIfAllRural(trx, taxSpecConfig()))
        {
          diag << "ALL SEGMENTS IN FARE USAGE EXEMPTED\n"
               << "SKIPPING FARE USAGE" << std::endl;
          continue;
        }
      }

      setSegsTaxableForYQYR(domesticSegs);
      if (milesDomestic == milesTotal)
      {
        diag << "FULLY DOMESTIC FARE - TAKE TOTAL AMOUNT\n"
             << "TAXABLE AMOUNT: " << std::setprecision(4) << taxableFare << std::endl;

        LOG4CXX_DEBUG(_logger, "TaxD8_02: Whole domestic fare found: " << taxableFare);

      }
      else //alike or proreated method
      {
        diag << "MILES DOMESTIC: " << std::to_string(milesDomestic)<< std::endl;
        diag << "MILES TOTAL: " << std::to_string(milesTotal)<< std::endl;
        const PaxTypeFare* internationalFareBefore = nullptr;
        const PaxTypeFare* internationalFareAfter = nullptr;

        if (!internationalSegsBefore.empty())
          internationalFareBefore = getLikeFare(trx, fareUsage, internationalSegsBefore,
                                                taxResponse.paxTypeCode(), *taxResponse.farePath()->itin());
        if (!internationalSegsAfter.empty())
          internationalFareAfter = getLikeFare(trx, fareUsage, internationalSegsAfter,
                                               taxResponse.paxTypeCode(), *taxResponse.farePath()->itin());

        //a like fare(s)
        if ((internationalSegsBefore.empty() || internationalFareBefore != nullptr) &&
            (internationalSegsAfter.empty() || internationalFareAfter != nullptr))
        {
          taxableFare = aLikeMethod(trx, taxableFare, internationalFareBefore, internationalFareAfter, fareUsage);
        }
        else
        {
          taxableFare = prorateMethod(trx, taxableFare, milesTotal, milesDomestic);
        }
      }

      applyPartialAmount(trx, taxResponse, taxCodeReg, taxableFare, domesticSegs.front());
    }
  }

  _taxSplitDetails.setFareSumAmount(_taxableFare);
}

MoneyAmount
TaxD8_02::prorateMethod(PricingTrx& trx, MoneyAmount taxableFare, uint32_t milesTotal,
                        uint32_t milesDomestic) const
{
  DiagManager diag(trx, Diagnostic818);
  diag << std::fixed << std::setprecision(4);

  double ratio = milesTotal >= 0 ? (double)milesDomestic/milesTotal : 1.0;

  diag << "PRORATED METHOD\n"
       << "TOTAL MILES: " << milesTotal << " DOMESTIC MILES: " << milesDomestic << " RATIO: " << ratio << "\n"
       << "TOTAL AMOUNT: " << taxableFare << " TAXABLE AMOUNT: " << taxableFare * ratio << std::endl;

  return taxableFare * ratio;
}

MoneyAmount
TaxD8_02::subtractInternationalQSurcharges(PricingTrx& trx,
                                           MoneyAmount taxableFare,
                                           const FareUsage* fareUsage) const
{
  DiagManager diag(trx, Diagnostic818);
  DiagManager diagVerbose(trx, Diagnostic818, DIAG818_VERBOSE_SWITCH);

  diag << "SEARCHING INTERNATIONAL SURCHARGES" << std::endl;

  for (const SurchargeData* sur : fareUsage->surchargeData())
  {
    const Loc* loc1 = trx.dataHandle().getLoc(sur->fcBrdCity(), trx.getRequest()->ticketingDT());
    const Loc* loc2 = trx.dataHandle().getLoc(sur->fcOffCity(), trx.getRequest()->ticketingDT());

    bool isInternational = loc1->nation() != MalaysiaCode || loc2->nation() != MalaysiaCode;
    if (isInternational)
      taxableFare -= sur->amountNuc();

    diag << loc1->loc() << "(" << loc1->nation() << ")"
         << "-" << loc2->loc() << "(" << loc2->nation() << ")"
         << " NUC-" << sur->amountNuc()
         << " COUNT-" << sur->itinItemCount()
         << " " << sur->surchargeDesc() << std::flush;

    diagVerbose << " APPL-" << sur->surchargeAppl()
         << " TYPE-" << sur->surchargeType()
         << " TRAVPORT-" << sur->travelPortion() << std::flush;

    if (isInternational)
      diag << " - INTERNATIONAL, SUBTRACT AMOUNT";
    else
      diag << " - DOMESTIC, SKIP";
    diag << std::endl;
  }

  return taxableFare;
}

MoneyAmount
TaxD8_02::aLikeMethod(PricingTrx& trx, MoneyAmount taxableFare, const PaxTypeFare* paxFare1,
                      const PaxTypeFare* paxFare2, const FareUsage* fareUsage) const
{
  DiagManager diag(trx, Diagnostic818);
  diag << std::fixed << std::setprecision(2);

  if (paxFare1)
  {
    diag << "SUBTRACT A LIKE FARE AMOUNT: " << paxFare1->totalFareAmount() << std::endl;
    taxableFare -= paxFare1->totalFareAmount();
  }

  if (paxFare2)
  {
    diag << "SUBTRACT A LIKE FARE AMOUNT: " << paxFare2->totalFareAmount() << std::endl;
    taxableFare -= paxFare2->totalFareAmount();
  }

  if (taxableFare > 0)
    taxableFare = subtractInternationalQSurcharges(trx, taxableFare, fareUsage);

  if (taxableFare < 0)
    taxableFare = 0;

  diag << "TAXABLE AMOUNT: " << std::setprecision(4) << taxableFare << std::endl;

  return taxableFare;
}

PricingTrx*
TaxD8_02::getFareMarkets(PricingTrx& trx,
                         const FareUsage* fareUsage,
                         std::vector<TravelSeg*>& tvlSeg,
                         const PaxTypeCode& paxTypeCode,
                         std::vector<FareMarket*>& fareMarkets) const
{
  DiagManager diag(trx, Diagnostic818);
  PricingTrx* trxTmp = &trx;

  TrxUtil::getFareMarket(trx, tvlSeg, fareUsage->paxTypeFare()->retrievalDate(), fareMarkets);
  if (fareMarkets.empty())
  {
    if (!TrxUtil::isPricingTaxRequest(&trx))
    {
      diag << "REPRICING IS NEEDED" << std::endl;

      trxTmp = getRepricingTrx(trx, tvlSeg, WPNCS_OFF, paxTypeCode, false);

      if (!trxTmp)
      {
        diag << "UNABLE TO REPRICE - END A LIKE FARE SEARCHING" << std::endl;
        return trxTmp;
      }

      TrxUtil::getFareMarket(*trxTmp, tvlSeg, fareUsage->paxTypeFare()->retrievalDate(),
                             fareMarkets);
    }
  }

  return trxTmp;
}

const PaxTypeFare*
TaxD8_02::getLikeFare(PricingTrx& trx,
                      const FareUsage* fareUsage,
                      std::vector<TravelSeg*>& tvlSeg,
                      const PaxTypeCode& paxTypeCode,
                      const Itin& itin) const
{
  DiagManager diag(trx, Diagnostic818);
  DiagManager diagVerbose(trx, Diagnostic818, DIAG818_VERBOSE_SWITCH);

  std::string fareBasisOrg = fareUsage->paxTypeFare()->createFareBasis(trx, false);

  diag << "SEARCHING A LIKE FARE FOR " << tvlSeg.size() << " SEGMENTS "
       << tvlSeg.front()->origin()->loc() << tvlSeg.back()->destination()->loc() << std::endl;

  diag << "ORIGINAL BASIS-" << fareBasisOrg
       << " FTYPE-" << fareUsage->paxTypeFare()->fcaFareType()
       << " FRULE-" << fareUsage->paxTypeFare()->ruleNumber()
       << std::endl;

  diagVerbose << " REVERSE-" << fareUsage->paxTypeFare()->isReversed()
              << " RT-" << fareUsage->paxTypeFare()->isRoundTrip()
              << " OUTBOUND-" << fareUsage->isOutbound()
              << " DIRECTION-" << fareUsage->paxTypeFare()->directionality()
              << " CMDPRICING-" << fareUsage->paxTypeFare()->isCmdPricing()
              << std::endl;

  std::vector<FareMarket*> retFareMarket;
  PricingTrx* trxTmp = getFareMarkets(trx, fareUsage, tvlSeg, paxTypeCode, retFareMarket);

  if (retFareMarket.empty())
  {
    diag << "FARE MARKET WAS NOT FOUND - END A LIKE FARE SEARCHING" << std::endl;
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

      diag << "BASIS-" << fareBasis
           << " TYPE-" << fare->fcaFareType()
           << " RULE-" << fare->ruleNumber()
           << " AMOUNT-" << fare->totalFareAmount() << std::flush;

      diagVerbose << " DIRECTION-" << fare->directionality()
                  << " REVERSE-" << fare->isReversed()
                  << " RT-" << fare->isRoundTrip() << std::flush;

      Directionality dir = fare->isRoundTrip() && fareUsage->isInbound() ? TO : FROM;

      //APM-766, fare has different direction than pricing unit
      //finally we have two OUTBOUND pricing units, but fareusage is INBOUND
      if (fareUsage->isInbound() && fareUsage->paxTypeFare()->directionality() == FROM)
        dir = FROM;

      if (fare->directionality() != dir)
      {
        diag << " - WRONG DIR" << std::endl;
        continue;
      }

      if (fareBasis == fareBasisOrg)
      {
        diag << " - SAME BASIS" << std::endl;

        //for command pricing, the same farebasis is not enough
        if (fareUsage->paxTypeFare()->isCmdPricing())
        {
          if (fare->ruleNumber() != fareUsage->paxTypeFare()->ruleNumber())
          {
            diag << "   COMMAND PRICING - DIFF RULES" << std::endl;
            continue;
          }
        }

        alikefare = fare;
        break;
      }

      if (alikefare)
      {
        diag << " - DIFF BASIS" << std::endl;
        continue;
      }

      if (fare->fcaFareType() != fareUsage->paxTypeFare()->fcaFareType())
      {
        diag << " - DIFF TYPES" << std::endl;
        continue;
      }

      if (fare->ruleNumber() != fareUsage->paxTypeFare()->ruleNumber())
      {
        diag << " - DIFF RULES" << std::endl;
        continue;
      }

      bool bBookingCode = true;
      for (TravelSeg* seg : tvlSeg)
      {
         BookingCode bk1 = getBookingCode(itin, *fare,
             fare->fareMarket()->travelSeg(), seg);
         BookingCode bk2 = getBookingCode(itin, *fareUsage->paxTypeFare(),
             fareUsage->travelSeg(), seg);

         if (bk1 != bk2)
         {
           bBookingCode = false;
           break;
         }
      }

      if (!bBookingCode)
      {
        diag << " - DIFF BOOKING CODE" << std::endl;
        continue;
      }

      diag << " - FARE MATCH" << std::endl;
      diag << "POTENTIALLY A LIKE FARE BUT LOOKING FOR THE SAME FARE BASIS" << std::endl;

      if (!alikefare)
        alikefare = fare;
    }

    if (alikefare)
      break;
  }

  if (alikefare)
    diag << "A LIKE FARE FOUND" << std::endl;

  return alikefare;
}

BookingCode
TaxD8_02::getBookingCode(const Itin& itin,
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
TaxD8_02::applyPartialAmount(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             MoneyAmount taxableFare,
                             const TravelSeg* travelSeg)
{
  DiagManager diag(trx, Diagnostic818);

  diag << std::fixed << "APPLY FARE VALUE: "
       << std::setprecision(4) << taxableFare << " /*";

  taxableFare = taxUtil::convertCurrency(trx, taxableFare, _paymentCurrency,
      taxResponse.farePath()->calculationCurrency(),
      taxResponse.farePath()->baseFareCurrency(),
      CurrencyConversionRequest::FARES,
      taxResponse.farePath()->itin()->useInternationalRounding());

  diag << taxableFare << _paymentCurrency << "*/ ";

  Percent discPercent = trx.getRequest()->discountPercentage(travelSeg->segmentOrder());
  if (discPercent >= 0 && discPercent <= 100)
  {
    LOG4CXX_DEBUG(_logger,
                  "TaxD8_02: discount " << discPercent << " on taxable fare: " << taxableFare);
    taxableFare *= (1.0 - discPercent / 100.0);
  }

  MoneyAmount taxAmount = taxableFare * taxCodeReg.taxAmt();
  LOG4CXX_DEBUG(_logger, "TaxD8_02: tax amount: " << taxableFare << " converted: " << taxAmount);

  taxAmount = AdjustTax::applyAdjust(
      trx, taxResponse, taxAmount, _paymentCurrency, taxCodeReg, _calculationDetails);

  MoneyAmount keepTaxableFare = _taxableFare;
  MoneyAmount keepTaxAmount = _taxAmount;

  _taxableFare = taxableFare;
  _taxAmount = taxAmount;

  doTaxRound(trx, taxCodeReg);

  diag << " TAX: " << _taxAmount << std::endl;

  _taxableFare += keepTaxableFare;
  _taxAmount += keepTaxAmount;

  _thruTotalFare = _taxableFare;
}

RepricingTrx*
TaxD8_02::getRepricingTrx(PricingTrx& trx,
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
    LOG4CXX_ERROR(_logger, "TaxD8_02: Exception during repricing with "
                           << ex.code() << " - " << ex.message());
  }
  catch (...)
  {
    LOG4CXX_ERROR(_logger, "TaxD8_02: Unknown exception during repricing");
  }

  return nullptr;
}

void
TaxD8_02::applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  TaxOnTaxCollector collector(*this, _calculationDetails, _taxOnTaxItems,
      _segsTaxableForYQYR, *(taxResponse.farePath()->itin()),
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
