//----------------------------------------------------------------------------
//  File:        Diag852Collector.C
//
//  Description: Diagnostic 852 formatter
//
//  Updates:
//          02/28/04 - DVD - Intitial Development
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
//----------------------------------------------------------------------------
#include "Diagnostic/Diag852Collector.h"

#include "Common/Assert.h"
#include "Common/BaggageStringFormatter.h"
#include "Common/BaggageTripType.h"
#include "Common/FreeBaggageUtil.h"
#include "Common/TseConsts.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PrecalcBaggageData.h"
#include "DBAccess/EmdInterlineAgreementInfo.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/SubCodeInfo.h"
#include "FreeBagService/AncillaryResultProcessorUtils.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "ServiceFees/OCFees.h"
#include "Util/IteratorRange.h"

#include <boost/lexical_cast.hpp>

#include <iomanip>
#include <iostream>

namespace tse
{
FALLBACK_DECL(fallbackAddSegmentsNumbersToCheckedPortionSectionInDiagnostic852);

const std::string Diag852Collector::CODE_SHARE = "CS";
const std::string Diag852Collector::FARE_LINE = "FL";
const std::string Diag852Collector::CHECKED_PORTION = "CP";
const std::string Diag852Collector::DISPLAY_DETAIL = "DD";
const std::string Diag852Collector::BTA = "BT";
const std::string Diag852Collector::DISPLAY_CHARGES = "DC";
const std::string Diag852Collector::SUB_TYPE_CODE = "ST";
const std::string Diag852Collector::CARRY_ON_ALLOWANCE = "CA";
const std::string Diag852Collector::CARRY_ON_CHARGES = "CC";
const std::string Diag852Collector::EMBARGOES = "EE";
const std::string Diag852Collector::FEE_ASSIGNMENT = "FA";
const std::string Diag852Collector::FREQ_FLYER = "FF";
const std::string Diag852Collector::PREVALIDATION = "PR";
const std::string Diag852Collector::PQ = "PQ";

const std::string Diag852Collector::BTA_FIELD_STATUS_PASSED = "PASS";
const std::string Diag852Collector::BTA_FIELD_STATUS_FAILED = "FAIL";

namespace
{
const std::string BAG_OCCURENCE_TEXT[MAX_BAG_PIECES] = {"1ST ", "2ND ", "3RD ", "4TH "};

struct IsOpenSegment
{
  bool operator()(const TravelSeg* travelSeg) const { return travelSeg->segmentType() == Open; }
};

inline std::set<PaxTypeFare*>
getBaggageTravelFares(const std::vector<TravelSeg*>::const_iterator segBegin,
                      const std::vector<TravelSeg*>::const_iterator segEnd,
                      const Ts2ss& ts2ss)
{
  std::set<PaxTypeFare*> result;

  for (TravelSeg* ts : makeIteratorRange(segBegin, segEnd))
  {
    const auto tsI = ts2ss.find(ts);

    if (tsI != ts2ss.end())
      result.insert(tsI->second.second);
  }

  return result;
}
} // anonymous ns

Diag852Collector::Diag852ParsedParams::Diag852ParsedParams(const Diag852Collector& diag)
  : _diagType(BASIC),
    _fareLine(0),
    _checkedPortion(0),
    _subTypeCode(""),
    _initialised(false),
    _parent(diag)
{
}

Diag852Collector::DiagType
Diag852Collector::Diag852ParsedParams::type() const
{
  initialiseParams();
  return _diagType;
}

uint32_t
Diag852Collector::Diag852ParsedParams::fareLine() const
{
  initialiseParams();
  return _fareLine;
}

uint32_t
Diag852Collector::Diag852ParsedParams::checkedPortion() const
{
  initialiseParams();
  return _checkedPortion;
}

const std::string&
Diag852Collector::Diag852ParsedParams::subTypeCode() const
{
  initialiseParams();
  return _subTypeCode;
}

void
Diag852Collector::Diag852ParsedParams::initialiseParams() const
{
  if (_initialised)
    return;

  if (_parent.rootDiag()->diagParamMapItemPresent(Diagnostic::BAG_FB))
    _diagType = FBACTIVE;
  else if (_parent.rootDiag()->diagParamMapItemPresent(CODE_SHARE))
    _diagType = CSACTIVE;
  else if (_parent.rootDiag()->diagParamMapItemPresent(FREQ_FLYER) &&
           TrxUtil::isFrequentFlyerTierActive(*_parent.trx()))
    _diagType = FFACTIVE;
  else if (_parent.rootDiag()->diagParamMapItemPresent(FARE_LINE))
  {
    try
    {
      _fareLine = boost::lexical_cast<uint32_t>(_parent.rootDiag()->diagParamMapItem(FARE_LINE));
    }
    catch (boost::bad_lexical_cast&)
    {
      // nothing to clean up, _fareLine set to 0 by default
    }

    if (_parent.rootDiag()->diagParamMapItemPresent(CHECKED_PORTION))
    {
      try
      {
        _checkedPortion =
            boost::lexical_cast<uint32_t>(_parent.rootDiag()->diagParamMapItem(CHECKED_PORTION));
      }
      catch (boost::bad_lexical_cast&)
      {
        // nothing to clean up, _checkedPortion set to 0 by default
      }

      if (_parent.hasDisplayChargesOption() || _parent.isDisplayCarryOnChargesOption() ||
          _parent.isDisplayEmbargoesOption())
      {
        if (_parent.rootDiag()->diagParamMapItemPresent(SUB_TYPE_CODE))
        {
          _subTypeCode = _parent.rootDiag()->diagParamMapItem(SUB_TYPE_CODE);
          _diagType = STACTIVE;
          if (_parent.rootDiag()->diagParamMapItemPresent(FEE_ASSIGNMENT))
          {
            _diagType = FAACTIVE;
          }
        }
        else
          _diagType =
              (_parent.isDisplayCarryOnChargesOption() || _parent.isDisplayEmbargoesOption())
                  ? CCACTIVE
                  : DCACTIVE;
      }
      else if (_parent.rootDiag()->diagParamMapItemPresent(CARRY_ON_ALLOWANCE))
      {
        _diagType = CAACTIVE;
      }
      else
        _diagType = CPACTIVE;
    }
    else
      _diagType = FLACTIVE;
  }
  else if (_parent.rootDiag()->diagParamMapItemPresent(PREVALIDATION))
    _diagType = PRACTIVE;
  else if (_parent.rootDiag()->diagParamMapItemPresent(PQ))
    _diagType = PQACTIVE; // Baggage calculation in FarePathFactory
  else
    _diagType = BASIC;

  _initialised = true;
}

void
Diag852Collector::S7PrinterBase::printS7RecordValidationFooter(const OptionalServicesInfo& info)
{
}

void
Diag852Collector::S7PrinterBase::printS7ProcessingHeader(const PricingTrx& trx,
                                                         const BaggageTravel* baggageTravel)
{
  _diag << "------------------S7 RECORD DATA PROCESSING-------------------\n";
  const FarePath& farePath = *baggageTravel->farePath();
  static_cast<DiagCollector&>(_diag) << farePath;
  _diag << "--------------------------------------------------------------\n";
}

void
Diag852Collector::S7PrinterBase::printPTC(const PricingTrx& trx, const BaggageTravel* baggageTravel)
{
  const PaxType& paxType = *baggageTravel->paxType();

  _diag << "PTC:" << paxType.paxType();
  _diag << " TRAVEL DATE: "
        << (*(baggageTravel->getTravelSegBegin()))->departureDT().dateToString(DDMMMYY, "");
  _diag << " RETRIEVAL DATE: " << trx.ticketingDate().dateToString(DDMMMYY, "");
}

bool
Diag852Collector::S7PrinterBase::shouldPrintFreqFlyer(const PricingTrx& trx) const
{
  const BaggageTrx* bTrx = dynamic_cast<const BaggageTrx*>(&trx);
  if (bTrx)
    return true;
  else
  {
    const AncillaryPricingTrx* aTrx = dynamic_cast<const AncillaryPricingTrx*>(&trx);
    if (aTrx &&
        (aTrx->billing()->actionCode() == "WP*BG" || aTrx->billing()->actionCode() == "WPBG*"))
      return true;

    return false;
  }
}

void
Diag852Collector::S7PrinterBase::printFreqFlyerStatus(
    const std::vector<PaxType::FreqFlyerTierWithCarrier*>& ff, const CarrierCode cxr)
{
  _diag << "\nFREQUENT FLYER: ";

  for (const PaxType::FreqFlyerTierWithCarrier* ffsd : ff)
  {
    const uint16_t ffstatus = ffsd->freqFlyerTierLevel();
    if ((cxr.empty() || (ffsd->cxr() == cxr)) && ffsd->isFromPnr())
      static_cast<DiagCollector&>(_diag) << ffstatus << " ";
  }
}

void
Diag852Collector::S7PrinterBase::printCorporateId(const PricingTrx& trx)
{
  _diag << "\nCORPORATE ID:";

  if (trx.getRequest()->corpIdVec().empty())
    _diag << " " << trx.getRequest()->corporateID();
  else
    for (const std::string& corpId : trx.getRequest()->corpIdVec())
      _diag << " " << corpId;
}

void
Diag852Collector::S7PrinterBase::printAccountCode(const PricingTrx& trx)
{
  _diag << "\nACCOUNT CODE:";

  if (trx.getRequest()->accCodeVec().empty())
    _diag << " " << trx.getRequest()->accountCode();
  else
    for (const std::string& accCode : trx.getRequest()->accCodeVec())
      _diag << " " << accCode;
}

void
Diag852Collector::S7PrinterBase::printInputDesignator(const PricingTrx& trx)
{
  _diag << "\nTKT DESIG INPUT: ";
  typedef const std::pair<int16_t, TktDesignator> TktDes;
  for (TktDes& tktDes : trx.getRequest()->tktDesignator())
    _diag << " " << tktDes.second;
}

void
Diag852Collector::S7PrinterBase::printOutputDesignator(const PricingTrx& trx,
                                                       const BaggageTravel* baggageTravel)
{
  printOutputDesignator(trx,
                        baggageTravel->getTravelSegBegin(),
                        baggageTravel->getTravelSegEnd(),
                        *baggageTravel->farePath());
}

void
Diag852Collector::S7PrinterBase::printOutputDesignator(
    const PricingTrx& trx,
    const std::vector<TravelSeg*>::const_iterator& segB,
    const std::vector<TravelSeg*>::const_iterator& segE,
    const FarePath& farePath)
{
  std::set<TktDesignator> tktDesignators;
  _diag << "\nTKT DESIG OUTPUT:";
  for (PricingUnit* pu : farePath.pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      bool isSegmentProcessed(false);
      for (TravelSeg* ts : fu->travelSeg())
      {
        for (TravelSeg* bts : makeIteratorRange(segB, segE))
        {
          if (bts->segmentOrder() == ts->segmentOrder())
            isSegmentProcessed = true;
        }
      }

      if (isSegmentProcessed)
      {
        TktDesignator tktD =
            trx.getRequest()->specifiedTktDesignator(fu->travelSeg().back()->segmentOrder());
        if (!tktD.empty())
          tktDesignators.insert(tktD); // tkt des from DP
        else
        {
          std::string fareBasis = fu->paxTypeFare()->createFareBasis(const_cast<PricingTrx*>(&trx));
          std::string::size_type tktDesPos = fareBasis.find("/");
          if (tktDesPos != std::string::npos)
          {
            TktDesignator designator(fareBasis.substr(tktDesPos + 1).c_str());
            tktDesignators.insert(designator); // tkt des from cat
          }
        }
      }
    }
  }
  for (TktDesignator tktDes : tktDesignators)
    _diag << " " << tktDes;

  _diag.flushMsg();
}

void
Diag852Collector::S7PrinterBase::printTourCode(const BaggageTravel* baggageTravel)
{
  _diag << "\nTOUR CODE: ";
  const CollectedNegFareData* negFareData = baggageTravel->farePath()->collectedNegFareData();
  bool cat35 = negFareData && negFareData->indicatorCat35();
  if (cat35)
  {
    const std::string& cat35TourCode = negFareData->tourCode();
    if (!cat35TourCode.empty())
      _diag << cat35TourCode;
  }
  else
  {
    const std::string& cat27TourCode = baggageTravel->farePath()->cat27TourCode();
    if (!cat27TourCode.empty())
      _diag << cat27TourCode;
  }
}

void
Diag852Collector::S7PrinterBase::printDayOfWeek(const BaggageTravel* baggageTravel)
{
  _diag << "\nDAY OF WEEK: " << (*(baggageTravel->getTravelSegBegin()))->departureDT().dayOfWeek()
        << " START/STOP TIME: "
        << (*(baggageTravel->getTravelSegBegin()))->departureDT().dateToString(YYYYMMDD, "-")
        << " ";

  if ((*(baggageTravel->getTravelSegBegin()))->segmentType() == Open)
    _diag << "00-00-00";
  else
    _diag << (*(baggageTravel->getTravelSegBegin()))->departureDT().timeToString(HHMMSS, "-");
}

void
Diag852Collector::S7PrinterBase::printEquipment(const BaggageTravel* baggageTravel)
{
  _diag << "\nEQUIPMENT:";
  for (TravelSegPtrVecCI tsIt = baggageTravel->getTravelSegBegin();
       tsIt != baggageTravel->getTravelSegEnd();
       ++tsIt)
  {
    const TravelSeg* tvlSeg = *tsIt;

    if (!tvlSeg->hiddenStops().empty())
    {
      for (const EquipmentType& equipType : tvlSeg->equipmentTypes())
        _diag << " " << equipType;
    }
    else
      _diag << " " << tvlSeg->equipmentType();
  }
  _diag << "\n";
}

void
Diag852Collector::S7PrinterNoDdinfo::printS7Processing(const PricingTrx& trx,
                                                       const BaggageTravel* baggageTravel)
{
  printS7ProcessingHeader(trx, baggageTravel);
  _diag.flushMsg();
}

void
Diag852Collector::S7PrinterNoDdinfo::printTravelInfo(const BaggageTravel* baggageTravel,
                                                     const Ts2ss& ts2ss)
{
}

void
Diag852Collector::S7PrinterNoDdinfo::printS7DetailInfo(const OptionalServicesInfo* info,
                                                       const PricingTrx& trx)
{
  _diag.Diag877Collector::printS7DetailInfo(info, trx);
  printS7BaggageInfo(info);
}

void
Diag852Collector::S7PrinterNoDdinfo::printS7BaggageInfo(const OptionalServicesInfo* info)
{
  DiagCollector& dc = _diag;

  const char fillChar = dc.fill('0');
  dc << std::setiosflags(std::ios::right);

  dc << "           FREE BAGGAGE PCS : ";
  if (info->freeBaggagePcs() >= 0)
    dc << std::setw(2) << info->freeBaggagePcs();
  dc << "\n";
  dc << "BAGGAGE OCCURRENCE FIRST PC : ";
  if (info->baggageOccurrenceFirstPc() >= 0)
    dc << std::setw(2) << info->baggageOccurrenceFirstPc();
  dc << "\n";
  dc << " BAGGAGE OCCURRENCE LAST PC : ";
  if (info->baggageOccurrenceLastPc() >= 0)
    dc << std::setw(2) << info->baggageOccurrenceLastPc();
  dc << "\n";
  dc << "             BAGGAGE WEIGHT : ";
  if (info->baggageWeight() >= 0)
    dc << std::setw(2) << info->baggageWeight();
  dc << "\n";
  dc << "        BAGGAGE WEIGHT UNIT : " << info->baggageWeightUnit() << "\n";

  dc << std::resetiosflags(std::ios::right);
  dc.fill(fillChar);
}

void
Diag852Collector::S7PrinterDEPRECATED::printS7Processing(const PricingTrx& trx,
                                                         const BaggageTravel* baggageTravel)
{
  printS7ProcessingHeader(trx, baggageTravel);
  printPTC(trx, baggageTravel);
  if (shouldPrintFreqFlyer(trx))
    printFreqFlyerStatus(baggageTravel->paxType()->freqFlyerTierWithCarrier(), "AA");
  printCorporateId(trx);
  printAccountCode(trx);
  printInputDesignator(trx);
  printOutputDesignator(trx, baggageTravel);

  _diag.flushMsg();
}

void
Diag852Collector::S7PrinterDEPRECATED::printTravelInfo(const BaggageTravel* bt, const Ts2ss& ts2ss)
{
  Diag852Collector& dc = _diag;

  const std::set<PaxTypeFare*> ptfs =
      getBaggageTravelFares(bt->getTravelSegBegin(), bt->getTravelSegEnd(), ts2ss);

  AirSeg* mss = dynamic_cast<AirSeg*>(*bt->_MSS);
  const auto ts2ssI = ts2ss.find(mss);
  const PaxTypeFare::SegmentStatus* stat = (ts2ssI != ts2ss.end()) ? ts2ssI->second.first : nullptr;

  dc << "\nTOUR CODE: ";
  const CollectedNegFareData* negFareData = bt->farePath()->collectedNegFareData();
  bool cat35 = negFareData && negFareData->indicatorCat35();
  if (cat35)
  {
    const std::string& cat35TourCode = negFareData->tourCode();
    if (!cat35TourCode.empty())
      dc << cat35TourCode;
  }
  else
  {
    const std::string& cat27TourCode = bt->farePath()->cat27TourCode();
    if (!cat27TourCode.empty())
      dc << cat27TourCode;
  }
  dc << "\nCABIN: MSS: " << mss->origin()->loc() << " " << mss->destination()->loc()
     << " M:" << mss->marketingCarrierCode()
     << " RBD: " << ((stat && stat->_bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
                         ? stat->_bkgCodeReBook
                         : mss->getBookingCode()) << " "
     << ((stat && stat->_bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
          stat->_reBookCabin.isValidCabin())
             ? stat->_reBookCabin
             : mss->bookedCabin()).printName();
  dc << "\nRBD:";
  for (TravelSeg* ts : makeIteratorRange(bt->getTravelSegBegin(), bt->getTravelSegEnd()))
  {
    const auto ts2ssI = ts2ss.find(ts);
    const PaxTypeFare::SegmentStatus* stat =
        (ts2ssI != ts2ss.end()) ? ts2ssI->second.first : nullptr;
    const BookingCode& bookingCode =
        ((stat && stat->_bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
             ? stat->_bkgCodeReBook
             : ts->getBookingCode());
    dc << " " << bookingCode;
  }
  dc << "\nFARE CLASS:";
  for (const PaxTypeFare* ptf : ptfs)
    dc << " " << ptf->fareClass();

  dc << "\nTARIFF:";
  for (const PaxTypeFare* ptf : ptfs)
    dc << " " << ptf->tcrRuleTariff();

  dc << "\nRULE:";
  for (const PaxTypeFare* ptf : ptfs)
    dc << " " << ptf->ruleNumber();

  dc << "\nCAT19/22/25/35:";
  for (const PaxTypeFare* ptf : ptfs)
  {
    if (ptf->isNegotiated() || ptf->isFareByRule() ||
        (ptf->isDiscounted() && (ptf->discountInfo().category() == DiscountInfo::CHILD ||
                                 ptf->discountInfo().category() == DiscountInfo::OTHERS)))
    {
      dc << " " << ptf->fareClass();
      if (ptf->isDiscounted())
      {
        if (dynamic_cast<BaggageTrx*>(bt->_trx))
        {
          if (ptf->discountInfo().category() == DiscountInfo::CHILD)
            dc << "/19/22";
        }
        else
        {
          if (ptf->discountInfo().category() == DiscountInfo::CHILD)
            dc << "/19";
          else if (ptf->discountInfo().category() == DiscountInfo::OTHERS)
            dc << "/22";
        }
      }
      if (ptf->isFareByRule())
        dc << "/25";
      if (ptf->isNegotiated())
        dc << "/35";
      dc << "\n";
    }
  }

  printDayOfWeek(bt);

  dc << "\nFLIGHT APPLICATION:\n";
  for (TravelSeg* ts : makeIteratorRange(bt->getTravelSegBegin(), bt->getTravelSegEnd()))
  {
    const AirSeg* airSeg = ts->toAirSeg();
    if (!airSeg)
      continue;
    dc << " " << airSeg->origin()->loc();
    dc << " " << airSeg->destination()->loc();
    dc << " M:" << airSeg->marketingCarrierCode();
    dc << " O:" << airSeg->operatingCarrierCode();
    dc << " " << airSeg->flightNumber() << "\n";
  }
  printEquipment(bt);

  AirSeg* mssInJourney = dynamic_cast<AirSeg*>(*bt->_MSSJourney);
  if (mssInJourney && bt->itin()->getBaggageTripType().isUsDot())
  {
    dc << "JOUNREY MSS: " << mssInJourney->origAirport() << " " << mssInJourney->destAirport()
       << " M:" << mssInJourney->marketingCarrierCode()
       << " O:" << mssInJourney->operatingCarrierCode() << "\n";
  }
  dc.flushMsg();
}

void
Diag852Collector::S7PrinterDdinfo::printS7Processing(const PricingTrx& trx,
                                                     const BaggageTravel* baggageTravel)
{
  printS7ProcessingHeader(trx, baggageTravel);
  printPTC(trx, baggageTravel);
  if (shouldPrintFreqFlyer(trx))
    printFreqFlyerStatus(baggageTravel->paxType()->freqFlyerTierWithCarrier(), "AA");
  printCorporateId(trx);
  printAccountCode(trx);
  printInputDesignator(trx);
  printTourCode(baggageTravel);
  printDayOfWeek(baggageTravel);
  printEquipment(baggageTravel);

  _diag.flushMsg();
}

void
Diag852Collector::S7PrinterDdinfo::printTravelInfo(const BaggageTravel* baggageTravel,
                                                   const Ts2ss& ts2ss)
{
  const bool isBaggageTrx = dynamic_cast<BaggageTrx*>(baggageTravel->_trx) != nullptr;
  const Itin* itin = baggageTravel->itin();

  _diag << "---------------- CHECKED PORTION DETAILS ---------------------";
  printTravelInfoForAllSectors(baggageTravel->getTravelSegBegin(),
                               baggageTravel->getTravelSegEnd(),
                               baggageTravel->_MSS,
                               isBaggageTrx,
                               baggageTravel,
                               ts2ss);

  _diag << "---------------- JOURNEY DETAILS -----------------------------";
  printTravelInfoForAllSectors(itin->travelSeg().begin(),
                               itin->travelSeg().end(),
                               baggageTravel->_MSSJourney,
                               isBaggageTrx,
                               baggageTravel,
                               ts2ss);
}

void
Diag852Collector::S7PrinterDdinfo::printBTAInfo(const OptionalServicesInfo* info, bool allignRight)
{
  DiagCollector& dc = _diag;
  dc << (allignRight ? "    BTA : " : "BTA : ") << info->baggageTravelApplication() << "\n";
}

void
Diag852Collector::S7PrinterDdinfo::printTravelInfoForAllSectors(
    std::vector<TravelSeg*>::const_iterator itBegin,
    std::vector<TravelSeg*>::const_iterator itEnd,
    std::vector<TravelSeg*>::const_iterator itMostSignificant,
    const bool isBaggageTrx,
    const BaggageTravel* baggageTravel,
    const Ts2ss& ts2ss)
{
  const std::set<PaxTypeFare*> ptfs = getBaggageTravelFares(itBegin, itEnd, ts2ss);

  DiagCollector& dc = _diag;
  dc << "\nCABIN: ";
  for (std::vector<TravelSeg*>::const_iterator tsIt = itBegin; tsIt != itEnd; ++tsIt)
  {
    dc << (*tsIt == *itMostSignificant ? "MSS: " : "     ");
    printCabinInfo(*tsIt, ts2ss);
    if (tsIt != itEnd - 1)
      dc << "\n       ";
  }

  dc << "\nRBD:";
  for (TravelSeg* ts : makeIteratorRange(itBegin, itEnd))
    printRBDInfo(ts, ts2ss);

  dc << "\nFARE BASIS:";
  for (const PaxTypeFare* ptf : ptfs)
    dc << " " << ptf->createFareBasis(baggageTravel->_trx);

  printOutputDesignator(*baggageTravel->_trx, itBegin, itEnd, *baggageTravel->farePath());

  dc << "\nTARIFF:";
  for (const PaxTypeFare* ptf : ptfs)
    dc << " " << ptf->tcrRuleTariff();

  dc << "\nRULE:";
  for (const PaxTypeFare* ptf : ptfs)
    dc << " " << ptf->ruleNumber();

  dc << "\nCAT19/22/25/35:";
  for (const PaxTypeFare* ptf : ptfs)
    printCATInfo(ptf, isBaggageTrx);

  dc << "\nFLIGHT APPLICATION:\n";
  for (TravelSegPtrVecCI tsIt = itBegin; tsIt != itEnd; ++tsIt)
    printFlightAppInfo(*tsIt);

  dc << "\n";
  dc.flushMsg();
}

void
Diag852Collector::S7PrinterDdinfo::printS7RecordValidationFooter(const OptionalServicesInfo& info)
{
  if (info.serviceFeesResBkgDesigTblItemNo() != 0 ||
      info.serviceFeesCxrResultingFclTblItemNo() != 0 ||
      info.resultServiceFeesTktDesigTblItemNo() != 0 || info.carrierFltTblItemNo() != 0)
  {
    _diag << "* USE BTA QUALIFIER FOR DETAILED PROCESSING\n\n";
  }
}

void
Diag852Collector::S7PrinterDdinfo::printS7DetailInfo(const OptionalServicesInfo* info,
                                                     const PricingTrx& trx)
{
  Diag877Collector::S7DetailsDiagBuilder diagBuilder(_diag, info);
  diagBuilder.buildFullInfo("", true);

  printBTAInfo(info, true);

  printS7BaggageInfo(info);
}

void
Diag852Collector::S7PrinterDdinfo::printCabinInfo(TravelSeg* seg, const Ts2ss& ts2ss)
{
  if (!seg->isAir())
    return;
  DiagCollector& dc = _diag;
  const auto ts2ssI = ts2ss.find(seg);
  const PaxTypeFare::SegmentStatus* stat = (ts2ssI != ts2ss.end()) ? ts2ssI->second.first : nullptr;

  AirSeg* airSeg = static_cast<AirSeg*>(seg);
  dc << airSeg->origAirport() << " " << airSeg->destAirport()
     << " M:" << airSeg->marketingCarrierCode() << " O:" << airSeg->operatingCarrierCode()
     << " RBD: " << ((stat && stat->_bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
                         ? stat->_bkgCodeReBook
                         : seg->getBookingCode()) << " "
     << ((stat && stat->_bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
          stat->_reBookCabin.isValidCabin())
             ? stat->_reBookCabin
             : seg->bookedCabin()).printName();
}

void
Diag852Collector::S7PrinterDdinfo::printRBDInfo(TravelSeg* seg, const Ts2ss& ts2ss)
{
  DiagCollector& dc = _diag;

  const auto ts2ssI = ts2ss.find(seg);
  const PaxTypeFare::SegmentStatus* stat = (ts2ssI != ts2ss.end()) ? ts2ssI->second.first : nullptr;
  const BookingCode& bookingCode =
      ((stat && stat->_bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED)) ? stat->_bkgCodeReBook
                                                                           : seg->getBookingCode());
  dc << " " << bookingCode;
}

void
Diag852Collector::S7PrinterDdinfo::printCATInfo(const PaxTypeFare* ptf, bool isBaggageTrx)
{
  DiagCollector& dc = _diag;

  if (ptf->isNegotiated() || ptf->isFareByRule() ||
      (ptf->isDiscounted() && (ptf->discountInfo().category() == DiscountInfo::CHILD ||
                               ptf->discountInfo().category() == DiscountInfo::OTHERS)))
  {
    dc << " " << ptf->fareClass();
    if (ptf->isDiscounted())
    {
      if (isBaggageTrx)
      {
        if (ptf->discountInfo().category() == DiscountInfo::CHILD)
          dc << "/19/22";
      }
      else
      {
        if (ptf->discountInfo().category() == DiscountInfo::CHILD)
          dc << "/19";
        else if (ptf->discountInfo().category() == DiscountInfo::OTHERS)
          dc << "/22";
      }
    }
    if (ptf->isFareByRule())
      dc << "/25";
    if (ptf->isNegotiated())
      dc << "/35";
    dc << "\n";
  }
}

void
Diag852Collector::S7PrinterDdinfo::printFlightAppInfo(TravelSeg* seg)
{
  DiagCollector& dc = _diag;

  if (seg->isAir())
  {
    const AirSeg* airSeg = static_cast<AirSeg*>(seg);
    dc << " " << airSeg->origin()->loc();
    dc << " " << airSeg->destination()->loc();
    dc << " M:" << airSeg->marketingCarrierCode();
    dc << " O:" << airSeg->operatingCarrierCode();
    dc << " " << airSeg->flightNumber() << "\n";
  }
}

void
Diag852Collector::S7PrinterBTA::printS7RecordValidationFooter(const OptionalServicesInfo& info)
{
}

void
Diag852Collector::S7PrinterBTA::printS7DetailInfo(const OptionalServicesInfo* info,
                                                  const PricingTrx& trx)
{
  Diag877Collector::S7DetailsDiagBuilder diagBuilder(_diag, info);
  diagBuilder.addHeader("BTA ");

  printBTAInfo(info, false);

  diagBuilder.addCabin(false)
      .addRBD198(false, false)
      .addT171(false)
      .addT173(false)
      .addRuleTariff(false)
      .addRule()
      .addT186(false);
}

void
Diag852Collector::printBaggageHeader(const PricingTrx& trx, bool isCarrierOverridden)
{
  if (diagType() == FAACTIVE || diagType() == PRACTIVE)
    return;

  Diag852Collector& dc = *this;

  const AncRequest* req = dynamic_cast<const AncRequest*>(trx.getRequest());
  bool showCxrOverride = req && (req->majorSchemaVersion() >= 2);

  if (hasDisplayChargesOption())
    if (showCxrOverride && !req->carrierOverriddenForBaggageCharges().empty())
      dc << "-----------------BAGGAGE CHARGES OVERRIDE---------------------\n";
    else
      dc << "---------------------BAGGAGE CHARGES--------------------------\n";
  else
  {
    if (showCxrOverride && !isCarrierOverridden &&
        !req->carrierOverriddenForBaggageAllowance().empty() &&
        !req->carrierOverriddenForBaggageCharges().empty())
      return;
    else if (!isCarrierOverridden)
      dc << "--------------------BAGGAGE ALLOWANCE-------------------------\n";
    else
      dc << "-----------------BAGGAGE ALLOWANCE OVERRIDE-------------------\n";
  }
}

void
Diag852Collector::printCarryOnBaggageHeader(bool processingCarryOn, bool processingEmbargo)
{
  if (diagType() == Diag852Collector::BASIC ||
      (processingCarryOn &&
       (isDisplayCarryOnAllowanceOption() || isDisplayCarryOnChargesOption())) ||
      (processingEmbargo && isDisplayEmbargoesOption()))
  {
    if (diagType() == FAACTIVE)
      return;

    Diag852Collector& dc = *this;

    if (isDisplayCarryOnAllowanceOption())
      dc << "-----------------CARRY ON BAGGAGE ALLOWANCE-------------------\n";
    else if (isDisplayCarryOnChargesOption())
      dc << "-----------------CARRY ON BAGGAGE CHARGES---------------------\n";
    else if (isDisplayEmbargoesOption())
      dc << "-----------------------BAGGAGE EMBARGO------------------------\n";
    else
      dc << "--------CARRY ON BAGGAGE ALLOWANCE AND CHARGES/EMBARGO--------\n";
  }
}

void
Diag852Collector::printItinAnalysisResults(const PricingTrx& trx,
                                           const CheckedPointVector& checkedPoints,
                                           const CheckedPoint& furthestCheckedPoint,
                                           const TravelSeg* furthestTicketedPoint,
                                           const Itin& itin,
                                           bool retransitPointsExist)
{
  const BaggageTripType baggageTripType = itin.getBaggageTripType();
  Diag852Collector& dc = *this;

  if (diagType() == BASIC)
  {
    dc << "--------------------ITINERARY ANALYSIS------------------------\n";
    if (trx.getTrxType() == PricingTrx::MIP_TRX)
      dc << "ITIN NUM : " << itin.itinNum() << "\n";
    printCheckedPoints(checkedPoints, furthestTicketedPoint);
    printFurthestCheckedPoint(checkedPoints, furthestCheckedPoint);
    printOriginAndDestination(itin, baggageTripType.isUsDot());

    dc << "JOURNEY TYPE : " << baggageTripType.getJourneyName(itin) << "\n";
    dc << "TKT DATE : " << trx.ticketingDate().dateToString(YYYYMMDD, "-") << "\n";

    if (!baggageTripType.isUsDot())
    {
      for (TravelSeg* travelSeg : itin.travelSeg())
      {
        if (travelSeg->isNonAirTransportation())
        {
          dc << "NON AIR TRANSPORTATION\n";
          break;
        }
      }
    }

    if (std::find_if(itin.travelSeg().begin(), itin.travelSeg().end(), IsOpenSegment()) !=
        itin.travelSeg().end())
    {
      dc << "OPEN SEGMENTS\n";
    }
  }

  if (retransitPointsExist)
    dc << "RETRANSIT POINTS\n";

  if (!isDisplayCarryOnAllowanceOption() && !isDisplayCarryOnChargesOption() &&
      !isDisplayEmbargoesOption())
    printBaggageHeader(trx);

  dc.flushMsg();
}


void
Diag852Collector::printInfoAboutUnknownBaggageCharges(PricingTrx& trx)
{
  if (!hasDisplayChargesOption())
    return;

  const auto requestedBagCharges = trx.getBaggagePolicy().getRequestedBagPieces();
  Diag852Collector& dc = *this;
  bool isAnyItinFailed = false;
  for (const Itin* itin : trx.itin())
  {
    if (itin->errResponseCode() == ErrorResponseException::UNKNOWN_BAGGAGE_CHARGES)
    {
      isAnyItinFailed = true;
      printHeader("FAILED ITINERARY: " + std::to_string(itin->itinNum()), '-', 62);

      for (const FarePath* farePath : itin->farePath())
      {
        for (const BaggageTravel* bt : farePath->baggageTravels())
        {
          dc << "BAGGAGE TRAVEL: ";

          if (fallback::fallbackAddSegmentsNumbersToCheckedPortionSectionInDiagnostic852(_trx))
            BaggageStringFormatter::old_printBaggageTravelSegments(*bt, dc);
          else
            dc << BaggageStringFormatter::printBaggageTravelSegmentsWithNumbering(*bt);

          dc << "\n";
          const auto freePieces = FreeBaggageUtil::calcFirstChargedPiece(bt->_allowance);
          auto allPieces = freePieces;
          for (auto i = freePieces; i < requestedBagCharges; ++i)
            if (bt->_charges[i])
              ++allPieces;

          dc << "FREE BAGGAGE PIECES: " << freePieces << std::endl;
          dc << "ALL BAGGAGE PIECES: " << allPieces << std::endl;
          dc << std::to_string(requestedBagCharges - allPieces) << " BAGGAGE CHARGES UNKNOWN" << std::endl;
        }
      }
    }
  }
  if (isAnyItinFailed)
    dc << "--------------------------------------------------------------" << std::endl;
}

void
Diag852Collector::printCheckedPoint(const CheckedPoint& checkedPoint)
{
  if (diagType() == FAACTIVE)
    return;

  Diag852Collector& dc = *this;

  const Loc* loc = (checkedPoint.second == CP_AT_ORIGIN) ? (*(checkedPoint.first))->origin()
                                                         : (*(checkedPoint.first))->destination();
  dc << loc->loc();
}

void
Diag852Collector::printCheckedPoints(const CheckedPointVector& checkedPoints,
                                     const TravelSeg* furthestTicketedPoint)
{
  if (diagType() == FAACTIVE)
    return;

  Diag852Collector& dc = *this;

  dc << "CHECKED POINTS :";

  for (const CheckedPoint& checkedPoint : checkedPoints)
  {
    // Skip furthest ticketed point. Print is as last checked point
    if (furthestTicketedPoint && (*(checkedPoint.first)) == furthestTicketedPoint)
      continue;

    dc << " ";

    printCheckedPoint(checkedPoint);
  }

  if (furthestTicketedPoint)
    dc << " " << furthestTicketedPoint->destination()->loc() << "*";

  dc << "\n";
}

void
Diag852Collector::printFurthestCheckedPoint(const CheckedPointVector& cp,
                                            const CheckedPoint& furthestCheckedPoint)
{
  if (cp.empty() || diagType() == FAACTIVE)
    return;

  Diag852Collector& dc = *this;

  const Loc* furthestCheckedPointLoc = (furthestCheckedPoint.second == CP_AT_ORIGIN)
                                           ? (*(furthestCheckedPoint.first))->origin()
                                           : (*(furthestCheckedPoint.first))->destination();

  dc << "FURTHEST CHECKED POINT : " << furthestCheckedPointLoc->loc() << "\n";
}

void
Diag852Collector::printOriginAndDestination(const Itin& itin, bool isUSDOT)
{
  if (diagType() == FAACTIVE)
    return;

  Diag852Collector& dc = *this;
  const std::vector<TravelSeg*>& travelSegs = itin.travelSeg();
  std::vector<TravelSeg*>::const_iterator origI;
  std::vector<TravelSeg*>::const_reverse_iterator destI;

  if (!isUSDOT)
  {
    origI = std::find_if(travelSegs.begin(),
                         travelSegs.end(),
                         std::not1(std::mem_fun(&TravelSeg::isNonAirTransportation)));
    destI = std::find_if(travelSegs.rbegin(),
                         travelSegs.rend(),
                         std::not1(std::mem_fun(&TravelSeg::isNonAirTransportation)));
  }
  else
  {
    origI = travelSegs.begin();
    destI = travelSegs.rbegin();
  }

  if (origI == travelSegs.end())
  {
    dc << "NO AIR TRANSPORTATION\n";
  }
  else
  {
    dc << "ORIGIN : " << (*origI)->origin()->loc() << "\n";
    dc << "DESTINATION : " << (*destI)->destination()->loc() << "\n";
  }
}

void
Diag852Collector::printMileage(const LocCode& origin,
                               const LocCode& destination,
                               const std::vector<Mileage*>& mil,
                               const boost::container::flat_set<GlobalDirection>& gdirs,
                               Indicator mileageType)
{
  Diag852Collector& dc = *this;

  std::string gdStr;

  dc << (mileageType == MPM ? "MPM" : "TPM") << " ANALYSIS FOR CITY PAIR " << origin << ' '
     << destination << " GD-";

  if (gdirs.empty())
    dc << "XX";

  for (const GlobalDirection gd : gdirs)
  {
    globalDirectionToStr(gdStr, gd);
    dc << gdStr << " ";
  }

  dc << "\n";

  if (mil.empty())
  {
    dc << "NO DATA\n";
    if (mileageType == MPM)
      dc << "GREAT CIRCLE MILEAGE WILL BE USED\n";
  }

  for (const Mileage* mileage : mil)
  {
    globalDirectionToStr(gdStr, mileage->globaldir());
    dc << gdStr << std::setw(20) << mileage->mileage();

    if (mileageType == 'M')
      dc << '/' << TseUtil::getTPMFromMPM(mileage->mileage());

    if (gdirs.count(mileage->globaldir()))
      dc << " GLOBAL DIRECTION MATCHED";
    dc << "\n";
  }
  dc << "--------------------------------------------------------------\n";
}

void
Diag852Collector::printBaggageTravels(const std::vector<const BaggageTravel*>& baggageTravels,
                                      bool isUsDot)
{
  if (diagType() != BASIC)
    return;

  for (uint32_t i = 0; i < baggageTravels.size(); ++i)
    printBaggageTravel(baggageTravels[i], isUsDot, i + 1);
  flushMsg();
}

void
Diag852Collector::printCarryOnBaggageTravels(
    const std::vector<const BaggageTravel*>& baggageTravels)
{
  if (diagType() != BASIC)
    return;

  Diag852Collector& dc = *this;

  if (baggageTravels.empty())
    dc << "CARRY ON/EMBARGO IS NOT PROCESSED FOR OPEN/SURFACE SEGMENT\n";

  for (uint32_t i = 0; i < baggageTravels.size(); ++i)
    printCarryOnBaggageTravel(baggageTravels[i], i);

  flushMsg();
}

void
Diag852Collector::printCarryOnBaggageTravel(const BaggageTravel* baggageTravel, uint32_t index)
{
  if (diagType() == FAACTIVE)
    return;

  Diag852Collector& dc = *this;

  dc << "--------------------------------------------------------------\n";
  const char fillChar = dc.fill('0');
  dc << setiosflags(std::ios::right) << std::setw(2) << index + 1
     << std::resetiosflags(std::ios::right) << " CHECKED PORTION : ";
  dc.fill(fillChar);

  dc << (*baggageTravel->_MSS)->origin()->loc() << " " << (*baggageTravel->_MSS)->pnrSegment() << " - "
     << (*baggageTravel->_MSS)->destination()->loc() << " " << (*baggageTravel->_MSS)->pnrSegment() << "\n";

  dc << "MSC MARKETING : \n";
  dc << "MSC OPERATING : " << dynamic_cast<AirSeg*>(*baggageTravel->_MSS)->operatingCarrierCode()
     << "\n";
  dc << "FCIC MARKETING : \n";
  dc << "FCIC OPERATING : \n";
}

void
Diag852Collector::printS7ProcessingCarryOnContext(const PricingTrx& trx,
                                                  const BaggageTravel* baggageTravel,
                                                  uint32_t index,
                                                  const SubCodeInfo* s5)
{
  if (diagType() == FAACTIVE)
    return;

  printCarryOnBaggageTravel(baggageTravel, index);
  printS5Record(s5, false, false);
  printS7Processing(trx, baggageTravel);
}

void
Diag852Collector::printBaggageTravel(const BaggageTravel* baggageTravel,
                                     bool isUsDot,
                                     uint32_t index,
                                     bool defer)
{
  if (diagType() == FAACTIVE)
    return;

  Diag852Collector& dc = *this;

  dc << "--------------------------------------------------------------\n";
  const char fillChar = dc.fill('0');
  dc << setiosflags(std::ios::right) << std::setw(2) << index << resetiosflags(std::ios::right)
     << " CHECKED PORTION : ";
  dc.fill(fillChar);

  if (fallback::fallbackAddSegmentsNumbersToCheckedPortionSectionInDiagnostic852(_trx))
    BaggageStringFormatter::old_printBaggageTravelSegments(*baggageTravel, dc);
  else
    dc << BaggageStringFormatter::printBaggageTravelSegmentsWithNumbering(*baggageTravel);

  dc << "\n";

  printCheckedSegments(baggageTravel, isUsDot);
  dc << "\n";

  BaggageStringFormatter::printBtCarriers(*baggageTravel, isUsDot, defer, dc);
}

void
Diag852Collector::printS5Record(const SubCodeInfo* s5, bool isMarketingCxr, bool isCarrierOverride)
{
  if (diagType() == FAACTIVE)
    return;

  Diag852Collector& dc = *this;

  dc << "------------------S5 RECORD DATA PROCESSING-------------------\n";
  if (isAvEMDIA())
    dc << "V CXR O/M SERVICE IND GRP SUBGRP SUBCODE EMDTYPE\n";
  else
    dc << "V CXR O/M SERVICE IND GRP SUBGRP SUBCODE\n";
  dc << std::setiosflags(std::ios::left);
  dc << std::setw(1) << (s5->vendor() == ATPCO_VENDOR_CODE ? "A" : "M") << std::setw(1) << " ";
  dc << std::setw(3) << s5->carrier() << std::setw(1) << " ";
  dc << std::setw(1) << (isCarrierOverride ? EMPTY_STRING() : (isMarketingCxr ? "M" : "O"))
     << std::setw(3) << " ";
  dc << std::setw(2) << s5->serviceTypeCode() << std::setw(6) << " ";
  dc << std::setw(1) << s5->industryCarrierInd() << std::setw(3) << " ";
  dc << std::setw(3) << s5->serviceGroup() << std::setw(1) << " ";
  dc << std::setw(3) << s5->serviceSubGroup() << std::setw(4) << " ";
  dc << std::setw(3) << s5->serviceSubTypeCode();

  if (isAvEMDIA())
    dc << std::setw(5) << " " << s5->emdType();

  dc << "\n";
  dc << std::resetiosflags(std::ios::left);
}

void
Diag852Collector::printS7ProcessingContext(const PricingTrx& trx,
                                           const BaggageTravel* baggageTravel,
                                           bool isUsDot,
                                           uint32_t index,
                                           bool isMarketingCxr,
                                           const SubCodeInfo* s5,
                                           bool defer,
                                           bool isCarrierOverride)
{
  if (diagType() != CAACTIVE && diagType() != FAACTIVE)
  {
    printBaggageTravel(baggageTravel, isUsDot, index, defer);
    printS5Record(s5, isMarketingCxr, isCarrierOverride);
    printS7Processing(trx, baggageTravel);
  }
}

void
Diag852Collector::printS7Processing(const PricingTrx& trx, const BaggageTravel* baggageTravel)
{
  if ((diagType() != DCACTIVE) && (diagType() != CCACTIVE) && (diagType() != FAACTIVE))
  {
    s7Printer(trx)->printS7Processing(trx, baggageTravel);
  }
}

void
Diag852Collector::printTravelInfo(const BaggageTravel* baggageTravel, const Ts2ss& ts2ss)
{
  if (diagType() == FAACTIVE)
    return;

  s7Printer(*baggageTravel->_trx)->printTravelInfo(baggageTravel, ts2ss);
}

void
Diag852Collector::printS7CommonHeader()
{
  if (diagType() == FAACTIVE)
    return;

  Diag852Collector& dc = *this;

  if (hasDisplayChargesOption() || isDisplayCarryOnChargesOption())
    dc << "V SCODE SERVICE   SEQ NUM  PAX CHARGE     STATUS\n";
  else if (isDisplayEmbargoesOption())
    dc << "V SCODE SERVICE   SEQ NUM                 STATUS\n";
  else
    dc << "V SCODE SERVICE   SEQ NUM  PAX PC/WEIGHT  STATUS\n";
}

void
Diag852Collector::printS7ChargesHeader(uint32_t bagNo)
{
  Diag852Collector& dc = *this;
  dc << "-----------------------";
  dc << BAG_OCCURENCE_TEXT[bagNo] << "CHECKED BAG------------------------\n";
  printS7CommonHeader();
}

void
Diag852Collector::displayAmount(const OCFees& ocFees)
{
  if (diagType() == FAACTIVE)
    return;

  if (hasDisplayChargesOption() || isDisplayCarryOnChargesOption())
  {
    Diag877Collector::displayAmount(ocFees);
  }
  else
  {
    Diag852Collector& dc = *this;

    const OptionalServicesInfo* s7 = ocFees.optFee();
    bool displayPieces = s7->freeBaggagePcs() >= 0;
    bool displayWeight = s7->baggageWeight() > 0;

    dc << std::setw((displayPieces && displayWeight) ? 1 : 4) << " ";
    if (displayPieces)
    {
      appendAmount(s7->freeBaggagePcs(), "PC");
    }
    if (displayWeight)
    {
      if (displayPieces)
      {
        dc << "/";
      }
      appendAmount(s7->baggageWeight(), (std::string() += s7->baggageWeightUnit()));
    }
    dc << std::setw((displayWeight && displayPieces) ? 2 : 4) << " ";
  }
}

void
Diag852Collector::appendAmount(const int32_t quantity, const std::string& unit)
{
  if (diagType() == FAACTIVE)
    return;

  Diag852Collector& dc = *this;
  const char fillChar = dc.fill('0');
  dc << std::setiosflags(std::ios::right) << std::setw(2) << quantity
     << std::resetiosflags(std::ios::right);
  dc.fill(fillChar);
  dc << std::setiosflags(std::ios::left) << std::setw(2) << unit
     << std::resetiosflags(std::ios::left);
}

void
Diag852Collector::printS7DetailInfo(const OptionalServicesInfo* info, const PricingTrx& trx)
{
  if (!isActive() || diagType() == FAACTIVE)
    return;

  s7Printer(trx)->printS7DetailInfo(info, trx);
}

bool
Diag852Collector::hasDisplayChargesOption() const
{
  return rootDiag()->diagParamMapItemPresent(DISPLAY_CHARGES);
}

bool
Diag852Collector::isDisplayCarryOnAllowanceOption() const
{
  return rootDiag()->diagParamMapItemPresent(CARRY_ON_ALLOWANCE);
}

bool
Diag852Collector::isDisplayCarryOnChargesOption() const
{
  return rootDiag()->diagParamMapItemPresent(CARRY_ON_CHARGES);
}

bool
Diag852Collector::isDisplayEmbargoesOption() const
{
  return rootDiag()->diagParamMapItemPresent(EMBARGOES);
}

void
Diag852Collector::printChargesHeader()
{
  if (!_active || diagType() == FAACTIVE)
    return;

  Diag852Collector& dc = *this;
  dc << "\n-----------------------BAGGAGE CHARGES------------------------\n";
  dc << "BAG V SCODE SEQ NUM  PAX CHARGE     OCC FIRST  OCC LAST\n";
}

void
Diag852Collector::printCharge(const BaggageCharge* baggageCharge, uint32_t occurence)
{
  if (!_active || diagType() == FAACTIVE)
    return;

  Diag852Collector& dc = *this;
  dc << BAG_OCCURENCE_TEXT[occurence];

  if (!baggageCharge || baggageCharge->feeCurrency().empty())
  {
    dc << "\n";
    return;
  }

  const OptionalServicesInfo* chargesS7 = baggageCharge->optFee();

  dc << std::resetiosflags(std::ios::right);
  dc << std::setiosflags(std::ios::left);
  dc << std::setw(1) << (chargesS7->vendor() == ATPCO_VENDOR_CODE ? "A" : "M") << std::setw(1)
     << " ";
  dc << std::setw(3) << chargesS7->serviceSubTypeCode() << std::setw(3) << " ";
  dc << std::setw(7) << chargesS7->seqNo() << std::setw(2) << " ";
  dc << std::setw(3) << chargesS7->psgType() << std::setw(1) << " ";
  if (chargesS7->notAvailNoChargeInd() == 'X')
    dc << std::setw(10) << "NOTAVAIL" << std::setw(1) << " ";
  else
    dc << std::setw(10) << Money(baggageCharge->feeAmount(), baggageCharge->feeCurrency())
                               .toString() << std::setw(1) << " ";
  if (chargesS7->baggageOccurrenceFirstPc() >= 0)
    dc << std::setw(5) << chargesS7->baggageOccurrenceFirstPc();
  else
    dc << std::setw(5) << "BLANK";
  dc << std::setw(6) << " ";
  if (chargesS7->baggageOccurrenceLastPc() >= 0)
    dc << std::setw(5) << chargesS7->baggageOccurrenceLastPc();
  else
    dc << std::setw(5) << "BLANK";
  dc << "\n";
  dc << std::resetiosflags(std::ios::left);
}

void
Diag852Collector::displayFFMileageAppl(Indicator ind)
{
  if (!_active || diagType() == FAACTIVE)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << ind;
  switch (ind)
  {
  case '1':
    dc << " - PER ONE WAY";
    break;
  case '2':
    dc << " - PER ROUND TRIP";
    break;
  case '3':
    dc << " - PER CHECKED PORTION";
    break;
  case '4':
    dc << " - PER BAGGAGE TRAVEL";
    break;
  case '5':
    dc << " - PER TICKET";
    break;
  default:
    break;
  }
}

void
Diag852Collector::printTariffCarriers(const PricingTrx& trx, const Itin& itin)
{
  if (!isActive() || diagType() != BASIC)
    return;

  const BaggageTripType btt = itin.getBaggageTripType();
  const bool usDotCarrierApplicable =
      TrxUtil::isBaggageCTAMandateActivated(trx) ? btt.usDotCarrierApplicable() : btt.isUsDot();
  const bool ctaCarrierApplicable = btt.ctaCarrierApplicable();

  if (!usDotCarrierApplicable && !ctaCarrierApplicable)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "--------------------------------------------------------------\n";

  std::set<CarrierCode> uniqueCarriers;
  for (const TravelSeg* travelSeg : itin.travelSeg())
  {
    const AirSeg* airSeg = travelSeg->toAirSeg();
    if (airSeg)
      uniqueCarriers.insert(airSeg->marketingCarrierCode());
  }

  for (const CarrierCode carrier : uniqueCarriers)
  {
    if (usDotCarrierApplicable)
    {
      dc << std::setiosflags(std::ios::left) << std::setw(3) << carrier
         << ": US TARIFF: " << (trx.dataHandle().isCarrierUSDot(carrier) ? "YES" : "NO") << "\n";
    }

    if (ctaCarrierApplicable)
    {
      dc << std::setiosflags(std::ios::left) << std::setw(3) << carrier
         << ": CTA TARIFF: " << (trx.dataHandle().isCarrierCTA(carrier) ? "YES" : "NO") << "\n";
    }
  }

  dc.flushMsg();
}

bool
Diag852Collector::printTable196ForCarryOnDetailSetup(uint32_t checkedPortion,
                                                     const OptionalServicesInfo* s7,
                                                     const FarePath& farePath,
                                                     const PricingTrx& trx)
{
  const bool isCarryOnAllowanceInfoDiag =
      (hasDisplayChargesOption() ? false : isDisplayCarryOnAllowanceOption() ||
                                               (!isDisplayCarryOnChargesOption() &&
                                                !isDisplayEmbargoesOption()));

  printTable196DetailSetup(checkedPortion, s7, farePath, trx, isCarryOnAllowanceInfoDiag);

  return _displayT196;
}

void
Diag852Collector::printTable196ForBaggageDetailSetup(uint32_t checkedPortion,
                                                     const OptionalServicesInfo* s7,
                                                     const FarePath& farePath,
                                                     const PricingTrx& trx)
{
  if (diagType() == FAACTIVE)
    return;

  const bool isBaggageAllowanceInfoDiag =
      ((isDisplayCarryOnAllowanceOption() || isDisplayCarryOnChargesOption() ||
        isDisplayEmbargoesOption())
           ? false
           : !hasDisplayChargesOption());

  printTable196DetailSetup(checkedPortion, s7, farePath, trx, isBaggageAllowanceInfoDiag);
}

void
Diag852Collector::printTable196DetailSetup(uint32_t checkedPortion,
                                           const OptionalServicesInfo* s7,
                                           const FarePath& farePath,
                                           const PricingTrx& trx,
                                           bool isAllowanceInfoDiag)
{
  if (diagType() == FAACTIVE)
    return;

  _displayT196 = false;

  if (!isAllowanceInfoDiag)
    return;

  if (!s7)
    return;

  if (isDDInfo())
  {
    uint32_t seqNo = 0;
    if (rootDiag()->diagParamMapItemPresent(Diagnostic::SEQ_NUMBER))
    {
      seqNo = (uint32_t)atoi(rootDiag()->diagParamMapItem(Diagnostic::SEQ_NUMBER).c_str());
      if (seqNo == s7->seqNo() && this->checkedPortion() == checkedPortion)
      {
        const PaxType* paxType = farePath.paxType();
        if (!paxType)
          return;

        uint32_t inputOrder = paxType->inputOrder() + 1;
        if (inputOrder == fareLine())
          _displayT196 = true;
      }
    }
  }
}

void
Diag852Collector::printTable196DetailHeader(const uint32_t itemno,
                                            const std::vector<std::string>& t196)
{
  if (diagType() == FAACTIVE)
    return;

  if (!_displayT196)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "*- - - - - - - - - - - - - - - - - - - - - - - - - -* " << std::endl;
  dc << "* TEXT TABLE 196  ITEM NO : " << itemno << "   DETAIL INFO    *" << std::endl;
  dc << "*- - - - - - - - - - - - - - - - - - - - - - - - - -* " << std::endl;
  dc << " SEQ        TEXT " << std::endl << std::endl;

  uint32_t index = 1;
  for (const std::string& t196Record : t196)
  {
    dc << " " << std::setw(3) << index << "       " << t196Record << std::endl;
    index++;
  }

  dc << "*- - - - - - - - - - - - - - - - - - - - - - - - - -* " << std::endl;
  dc << "SBC  DESC1 DESC2 GRP SBGRP CXR" << std::endl << std::endl;

  dc.flushMsg();
}

void
Diag852Collector::printTable196Detail(const SubCodeInfo* subCodeInfo)
{
  if (diagType() == FAACTIVE)
    return;

  if (!_displayT196)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << std::setw(3) << subCodeInfo->serviceSubTypeCode() << "  " << std::setw(2)
     << subCodeInfo->description1() << "    " << std::setw(2) << subCodeInfo->description2()
     << "    " << std::setw(2) << subCodeInfo->serviceGroup() << "  " << std::setw(2)
     << subCodeInfo->serviceSubGroup() << "    " << subCodeInfo->carrier();
  dc.flushMsg();
}

void
Diag852Collector::printTable196Detail(bool selected)
{
  if (diagType() == FAACTIVE)
    return;

  if (!_displayT196)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  if (selected)
    dc << "*";
  else
    dc << std::endl;
  dc.flushMsg();
}

void
Diag852Collector::printTable196DetailEnd(const SubCodeInfo* subCodeInfo)
{
  if (diagType() == FAACTIVE)
    return;

  if (!_displayT196 || !subCodeInfo)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  std::string attributes;
  dc << "*- - - - - - - - - - - - - - - - - - - - - - - - - -* " << std::endl;

  const ServicesDescription* svcDesc =
      _trx->dataHandle().getServicesDescription(subCodeInfo->description1());
  if (svcDesc)
  {
    attributes = svcDesc->description();
    dc << subCodeInfo->description1() << "- " << attributes << std::endl;

    svcDesc = _trx->dataHandle().getServicesDescription(subCodeInfo->description2());
    if (svcDesc)
    {
      attributes = svcDesc->description();
      dc << subCodeInfo->description2() << "- " << attributes << std::endl;
    }
  }
  dc.flushMsg();
}

bool
Diag852Collector::checkFl(const Itin& itin) const
{
  bool printHeader = true;
  if (diagType() >= FLACTIVE)
  {
    uint32_t currentItinNum = itin.getItinOrderNum();
    if ((fareLine() + 1) != currentItinNum)
      printHeader = false;
  }
  return printHeader;
}

bool
Diag852Collector::isDDInfo() const
{
  return rootDiag()->diagParamMapItem(DISPLAY_DETAIL) == "INFO";
}

bool
Diag852Collector::isAvEMDIA() const
{
  return rootDiag()->diagParamMapItem(Diagnostic::ALL_VALID) == "EMDIA";
}

void
Diag852Collector::printFareInfo(const PaxTypeFare* paxTypeFare, PricingTrx& pricingTrx)
{

  if (!paxTypeFare || diagType() == FAACTIVE)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::right, std::ios::adjustfield);
  static constexpr int LENGTH = 21;

  dc << std::setw(LENGTH) << "FROM : " << paxTypeFare->origin() << "-" << paxTypeFare->destination()
     << "\n" << std::setw(LENGTH)
     << "FARE BASIS CODE : " << paxTypeFare->createFareBasis(&pricingTrx) << "\n"
     << std::setw(LENGTH) << "CXR : " << paxTypeFare->carrier() << "\n" << std::setw(LENGTH)
     << "BASE FARE AMOUNT : " << paxTypeFare->fareAmount() << "\n" << std::setw(LENGTH)
     << "BASE FARE CURRENCY : " << paxTypeFare->currency() << "\n" << std::setw(LENGTH)
     << "FARE TYPE : " << paxTypeFare->fcaFareType() << "\n" << std::setw(LENGTH)
     << "OW/RT : " << (paxTypeFare->owrt() == ONE_WAY_MAY_BE_DOUBLED ||
                               paxTypeFare->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED
                           ? "OW"
                           : "RT") << "\n"
     << "--------------------------------------------------------------\n";
}

void
Diag852Collector::printFareSelectionInfo(const FareMarket& fareMarket,
                                         const CarrierCode& carrierCode,
                                         PricingTrx& pricingTrx,
                                         const PaxTypeFare* selectedFare)
{
  if (_active && (diagType() == FAACTIVE))
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc.setf(std::ios::right, std::ios::adjustfield);
    dc << "------------------HIGHEST FARE SELECTION----------------------\n"
       << std::setw(CARRIER_LENGTH) << "CRX" << std::setw(FARE_BASIS_LENGTH) << "FARE"
       << std::setw(STATUS_LENGTH) << "STATUS" << std::setw(AMOUNT_LENGTH) << "AMOUNT"
       << std::setw(CURRENCY_LENGTH) << "CURRENCY\n";

    for (const PaxTypeFare* paxTypeFare :
         makeIteratorRange(fareMarket.allPaxTypeFare().begin(), fareMarket.allPaxTypeFare().end()))
    {
      if (!paxTypeFare->isRoundTrip())
        printFareCheckInfo(paxTypeFare->carrier(),
                           getFareCheckStatus(*paxTypeFare, carrierCode),
                           paxTypeFare->createFareBasis(&pricingTrx),
                           paxTypeFare->fareAmount(),
                           paxTypeFare->currency(),
                           selectedFare == paxTypeFare);
    }
  }
}

void
Diag852Collector::printFareCheckInfo(const CarrierCode& fareCarrier,
                                     const std::string& fareCheckStatus,
                                     const std::string& paxFareBasis,
                                     MoneyAmount moneyAmount,
                                     const CurrencyCode& currency,
                                     bool isSelected)
{
  if (_active && (diagType() == FAACTIVE))
  {
    DiagCollector& dc = (DiagCollector&)*this;

    std::streamsize oldPrecision = dc.precision(FARE_CHECK_INFO_PRECISION);

    dc << std::setw(CARRIER_LENGTH) << fareCarrier << std::setw(FARE_BASIS_LENGTH) << paxFareBasis
       << std::setw(STATUS_LENGTH) << fareCheckStatus << std::setw(AMOUNT_LENGTH) << moneyAmount
       << std::setw(CURRENCY_LENGTH) << currency << std::setw(SELECTION_MARK_LENGTH)
       << (isSelected ? "*" : " ") << "\n";

    dc.precision(oldPrecision);
  }
}

std::string
Diag852Collector::getFareCheckStatus(const PaxTypeFare& paxTypeFare, const CarrierCode& carrier)
    const
{
  if (paxTypeFare.nucFareAmount() == 0)
    return "BSR NOT FOUND";
  else if (paxTypeFare.carrier() != carrier && paxTypeFare.carrier() != INDUSTRY_CARRIER)
    return "DIFFERENT CARRIER";
  else if (paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
    return "PRIVATE TARIFF";
  else if (!paxTypeFare.isPublished())
    return "NOT PUBLISHED";
  else if (!paxTypeFare.isNormal())
    return "NOT NORMAL";
  else if (paxTypeFare.fcaFareType()[0] != E_TYPE)
    return "NOT E TYPE";
  else if (paxTypeFare.directionality() != FROM)
    return "DIRECTIONALITY";
  else if (!paxTypeFare.paxType()->paxType().empty() && paxTypeFare.paxType()->paxType() != ADULT)
    return "NOT ADULT";
  else
    return "PASS";
}

void
Diag852Collector::printS7OptionalServiceStatus(StatusS7Validation status)
{
  if (!_active || diagType() == FAACTIVE)
    return;

  Diag877Collector::printS7OptionalServiceStatus(status);
}

void
Diag852Collector::printS7RecordValidationFooter(const OptionalServicesInfo& info,
                                                const PricingTrx& trx)
{
  if (!isActive() || diagType() == FAACTIVE)
    return;

  s7Printer(trx)->printS7RecordValidationFooter(info);
}

Diag852Collector::S7PrinterBase*
Diag852Collector::s7Printer(const PricingTrx& trx)
{
  if (!TrxUtil::isBaggageBTAActivated(trx))
    return &_dataHandle.safe_create<S7PrinterDEPRECATED>(*this);

  if (isBTAContextOn())
    return &_dataHandle.safe_create<S7PrinterBTA>(*this);
  else if (isDDInfo())
    return &_dataHandle.safe_create<S7PrinterDdinfo>(*this);
  else
    return &_dataHandle.safe_create<S7PrinterNoDdinfo>(*this);
}

bool
Diag852Collector::shouldCollectInRequestedContext(Diag877Collector::ProcessingContext context) const
{
  if (!_active)
    return false;

  const PricingTrx* trx = dynamic_cast<const PricingTrx*>(_trx);

  TSE_ASSERT(trx);

  if (TrxUtil::isBaggageBTAActivated(*trx))
    return validContextForBTA(context);

  return isDDInfo();
}

bool
Diag852Collector::validContextForBTA(Diag877Collector::ProcessingContext context) const
{
  switch (context)
  {
  case PROCESSING_RBD:
  case PROCESSING_T171:
  case PROCESSING_OUTPUT_TICKET_DESIG:
  case PROCESSING_T186:
    return isBTAContextOn();

  case PROCESSING_ACCOUNT_CODES:
  case PROCESSING_INPUT_TICKET_DESIG:
  case PROCESSING_T170:
  case PROCESSING_T183:
    return !isBTAContextOn() && isDDInfo();

  default:
    return false;
  }
}

void
Diag852Collector::printBTASegmentHeader(const TravelSeg& segment)
{
  if (!isBTAContextOn())
    return;

  (*this) << "-------------------------SECTOR " << segment.origAirport() << "-"
          << segment.destAirport() << "----------------------------\n";
}

void
Diag852Collector::printBTASegmentFooter(bool segmentValidationPassed, const TravelSeg& segment)
{
  if (!isBTAContextOn())
    return;

  (*this) << "                                           S7 STATUS : "
          << (segmentValidationPassed ? BTA_FIELD_STATUS_PASSED : BTA_FIELD_STATUS_FAILED) << " "
          << segment.origAirport() << "-" << segment.destAirport() << "\n";
}

bool
Diag852Collector::isBTAContextOn() const
{
  return isDDInfo() && rootDiag()->diagParamMapItem(BTA) == "A";
}

void
Diag852Collector::printBTAFieldStatusCabin(bool matched)
{
  if (!isBTAContextOn())
    return;

  (*this) << "CABIN : " << (matched ? BTA_FIELD_STATUS_PASSED : BTA_FIELD_STATUS_FAILED) << "\n";
}

void
Diag852Collector::printBTAFieldStatusRBD(bool matched)
{
  if (!isBTAContextOn())
    return;

  (*this) << "RBD T198 : " << (matched ? BTA_FIELD_STATUS_PASSED : BTA_FIELD_STATUS_FAILED) << "\n";
}

void
Diag852Collector::printBTAStatusTableT171(bool matched)
{
  if (!isBTAContextOn())
    return;

  (*this) << "CXR/RESULT FARE CLASS T171 : " << (matched ? BTA_FIELD_STATUS_PASSED
                                                         : BTA_FIELD_STATUS_FAILED) << "\n";
}

void
Diag852Collector::printBTAStatusOutputTicketDesignator(bool matched)
{
  if (!isBTAContextOn())
    return;

  (*this) << "FARE TICKT DESIGNATOR T173 : " << (matched ? BTA_FIELD_STATUS_PASSED
                                                         : BTA_FIELD_STATUS_FAILED) << "\n";
}

void
Diag852Collector::printBTAFieldStatusRule(bool matched)
{
  if (!isBTAContextOn())
    return;

  (*this) << "RULE : " << (matched ? BTA_FIELD_STATUS_PASSED : BTA_FIELD_STATUS_FAILED) << "\n";
}

void
Diag852Collector::printBTAFieldStatusRuleTariff(bool matched)
{
  if (!isBTAContextOn())
    return;

  (*this) << "RULE TARIFF : " << (matched ? BTA_FIELD_STATUS_PASSED : BTA_FIELD_STATUS_FAILED)
          << "\n";
}

void
Diag852Collector::printBTAFieldStatusFareInd(bool matched)
{
  if (!isBTAContextOn())
    return;

  (*this) << "FARE IND : " << (matched ? BTA_FIELD_STATUS_PASSED : BTA_FIELD_STATUS_FAILED) << "\n";
}

void
Diag852Collector::printBTAFieldStatusCarrierFlightApplT186(bool matched)
{
  if (!isBTAContextOn())
    return;

  (*this) << "CXR/FLT T186 : " << (matched ? BTA_FIELD_STATUS_PASSED : BTA_FIELD_STATUS_FAILED)
          << "\n";
}

void
Diag852Collector::printCheckedSegments(const BaggageTravel* baggageTravel, bool isUsDot)
{
  if (!hasDisplayChargesOption() || !baggageTravel->_trx->activationFlags().isAB240())
    return;

  TravelSegPtrVecCI segI = baggageTravel->getTravelSegBegin();
  TravelSegPtrVecCI segIE = baggageTravel->getTravelSegEnd();

  (*this) << "CHECKED SEGMENTS : ";

  if (!isUsDot)
  {
    segI = std::find_if(segI, segIE, CheckPortionOfTravelIndicator('T'));
    segIE = std::find_if_not(segI, segIE, CheckPortionOfTravelIndicator('T'));
  }
  if (segI != segIE)
  {
    if (fallback::fallbackAddSegmentsNumbersToCheckedPortionSectionInDiagnostic852(_trx))
      BaggageStringFormatter::old_printBaggageTravelSegments(segI, segIE, *this);
    else
      (*this) << BaggageStringFormatter::printBaggageTravelSegmentsWithNumbering(segI, segIE);
  }
  else
  {
    (*this) << "NO CHECKED SEGMENTS FOR CHECKED PORTION\n";
  }
}

std::string
Diag852Collector::getCarrierListAsString(const std::set<tse::CarrierCode>& carriers, const std::string& separator) const
{
  std::string carrierList;
  int counter = 0;
  for(std::set<tse::CarrierCode>::const_iterator it = carriers.begin(); it != carriers.end(); ++it)
  {
    if(it != carriers.begin())
      carrierList += separator;
    if(++counter % 14 == 0)
       carrierList += "\n                      ";
    carrierList += *it;
  }
  return carrierList;
}
void
Diag852Collector::printDoNotReadA03Table()
{
  if (diagType() != FFACTIVE)
    return;

  *this << "DO NOT READ A03 FOR " << _owningCarrierWithA03Status.first;
  *this << "\n";
}

void
Diag852Collector::printReadTable(const std::vector<PaxType::FreqFlyerTierWithCarrier*>& data)
{
  if (diagType() != FFACTIVE)
    return;

  *this << "READ A03 TABLE FOR " << _owningCarrierWithA03Status.first << " AGAINST CARRIERS ";
  for (const PaxType::FreqFlyerTierWithCarrier* ffFata : data)
  {
    if (ffFata->cxr() != _owningCarrierWithA03Status.first)
    {
      *this << ffFata->cxr() << " ";
    }
  }
  *this << "\n";
}
void
Diag852Collector::printDeterminedFFStatus(const uint16_t level)
{
  if (diagType() != FFACTIVE)
    return;

  *this << "TIER LEVEL FOR " << _owningCarrierWithA03Status.first << ": ";
  if (level == FF_LEVEL_NOT_DETERMINED)
    *this << "NOT DETERMINED";
  else
    *this << std::to_string(level);
  *this << "\n";
}

void
Diag852Collector::printFFStatuses(const CarrierCode partnerCarrier,
                                  const uint16_t partnerlevel,
                                  const uint16_t determinedLevel)
{
  const CarrierCode owningCarrier = _owningCarrierWithA03Status.first;
  if (diagType() != FFACTIVE)
    return;

  if (owningCarrier == partnerCarrier)
    *this << "SPECIFIED " << partnerCarrier << " TIER " << partnerlevel << std::endl;
  else
  {
    *this << partnerCarrier << " TIER " << partnerlevel << " - ";

    if (determinedLevel == FF_LEVEL_NOT_DETERMINED)
    {
      if (_owningCarrierWithA03Status.second)
        *this << "NO " << owningCarrier << " TIER" << std::endl;
      else
        *this << "NO " << owningCarrier << " A03" << std::endl;
    }
    else
      *this << owningCarrier << " TIER " << determinedLevel << std::endl;
  }
}

void
Diag852Collector::printDetailInterlineEmdProcessingS5Info(const tse::NationCode& nation,
                                                          const tse::CrsCode& gds,
                                                          const tse::CarrierCode& validatingCarrier,
                                                          const std::set<tse::CarrierCode>& marketingCarriers,
                                                          const std::set<tse::CarrierCode>& operatingCarriers)
{
  if (!_active || !isDDInfo() || _params.subTypeCode().empty())
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "--------- INTERLINE EMD AGREEMENT PROCESSING DETAILS ---------\n"
     << " NATION             : " << nation << "\n"
     << " GDS                : " << gds << "\n"
     << " VALIDATING CXR     : " << validatingCarrier << "\n"
     << " MARKETING  CXR     : " << getCarrierListAsString(marketingCarriers, " ") << "\n"
     << " OPERATING  CXR     : " << getCarrierListAsString(operatingCarriers, " ") << "\n";
}

void
Diag852Collector::printNoInterlineDataFoundInfo()
{
  if (!_active || !isDDInfo() || _params.subTypeCode().empty())
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "--------- NO INTERLINE EMD AGREEMENT DATA FOUND ---------\n";
}

void
Diag852Collector::printDetailInterlineEmdAgreementInfo(const std::vector<EmdInterlineAgreementInfo*>& eiaList,
                                                       const tse::CarrierCode& validatingCarrier) const
{
  if (!_active || !isAvEMDIA() || !isDDInfo() || _params.subTypeCode().empty())
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " ALLOWED    CXR     : ";
  // Validating carrier is allowed by default, since the carrier always has EMD agreement with itself
  dc << std::setw(3) << validatingCarrier;
  std::vector<EmdInterlineAgreementInfo*>::const_iterator it = eiaList.begin();
  std::vector<EmdInterlineAgreementInfo*>::const_iterator itE = eiaList.end();

  const int maxCarriersPerLine = 13;
  for (int counter = 1; it != itE; ++it)
  {
    if (counter == maxCarriersPerLine)
    {
      counter = 0;
      dc << "\n                      ";
    }
    dc << std::setw(3) << (*it)->getParticipatingCarrier();
    ++counter;
  }
  dc << "\n";
}

void
Diag852Collector::printEmdValidationResult(bool emdValidationResult, AncRequestPath rp,
                                           const BaggageTravel* baggageTravel,
                                           const BaggageTravelInfo& bagInfo) const
{
  if (checkedPortion() != bagInfo.bagIndex() + 1)
    return;

  const ChargeVector& charges = baggageTravel->_chargeVector;
  ChargeVector::const_iterator it = find_if(charges.begin(), charges.end(),
    [this](const BaggageCharge* charge)
    {
     return charge->subCodeInfo() != nullptr
           && charge->subCodeInfo()->serviceSubTypeCode() == subTypeCode();
    }
  );
  if(it != charges.end())
  {
    DiagCollector& dc = (DiagCollector&)*this;

    const BaggageCharge* charge = *it;
    if(charge->subCodeInfo() == nullptr ||
       (charge->subCodeInfo()->emdType() != '2' &&
        charge->subCodeInfo()->emdType() != '3'))
      return;

    if (rp == AncRequestPath::AncCheckInPath)
    {
      if(emdValidationResult)
        dc << " S5 STATUS          : I-EMD PASS\n";
      else
        dc << " S5 STATUS          : I-EMD FAILED/SOFT PASS\n";
    }
    else if (rp == AncRequestPath::AncReservationPath)
    {
      if (emdValidationResult)
      {
        dc << " S5 STATUS          : I-EMD PASS\n";
      }
      else
      {
        bool is1stOr2ndBag  = find_if(charges.begin(), charges.end(),
                                      [this, baggageTravel](const BaggageCharge* charge)
                                      {
                                        return charge->subCodeInfo() != nullptr &&
                                               charge->subCodeInfo()->serviceSubTypeCode() == subTypeCode() &&
                                               baggageTravel->isBaggageCharge1stOr2ndBag(charge);
                                      }
                                     ) != charges.end();

        if (is1stOr2ndBag)
          dc << " S5 STATUS          : I-EMD AGREEMENT NOT CHECKED - SDO/FDO\n";
        else
          dc << " S5 STATUS          : I-EMD FAILED \n";
      }
    }
  }
}

void
Diag852Collector::printFarePathBaggageCharge(const FarePath& fp, MoneyAmount lbound)
{
  if (diagType() != PQACTIVE)
    return;

  printHeader("BAGGAGE CALCULATOR RESULTS");
  DiagCollector::operator<<(fp);
  *this << " BAGGAGE LOWER BOUND: " << lbound << "\n\n";
  *this << " BAGGAGE CHARGE AMOUNT: " << fp.bagChargeNUCAmount() << "\n";
}

void
Diag852Collector::printBaggageTravelCharges(const BaggageTravel& bt)
{
  if (diagType() != PQACTIVE)
    return;

  std::ostringstream& dc = *this;

  dc << "\n ";

  if (fallback::fallbackAddSegmentsNumbersToCheckedPortionSectionInDiagnostic852(_trx))
    BaggageStringFormatter::old_printBaggageTravelSegments(bt.getTravelSegBegin(), bt.getTravelSegEnd(), dc);
  else
    dc << BaggageStringFormatter::printBaggageTravelSegmentsWithNumbering(bt);

  dc << "\n";

  if (bt._allowance && bt._allowance->optFee())
  {
    dc << std::left << std::setw(20) << "   FREE PIECES: "
       << std::left << std::setw(12) << bt._allowance->optFee()->freeBaggagePcs();
    printSimpleS7Id(*bt._allowance->optFee());
    dc << "\n";
  }

  for (uint32_t bagNo = 0; bagNo < MAX_BAG_PIECES; ++bagNo)
  {
    const BaggageCharge* bc = bt._charges[bagNo];
    if (!bc || !bc->optFee())
      continue;
    std::ostringstream ossAmt;
    ossAmt << bc->feeAmount() << bc->feeCurrency();
    dc << std::left << std::setw(20) << ("   " + std::to_string(bagNo + 1) + " PC: ")
       << std::left << std::setw(12) << ossAmt.str();
    printSimpleS7Id(*bc->optFee());
    dc << "\n";
  }
}

void
Diag852Collector::printSimpleS7Id(const OptionalServicesInfo& s7)
{
  *this << s7.carrier() << " " << s7.serviceSubTypeCode() << " " << s7.seqNo();
}

Diag852Collector&
Diag852Collector::operator<<(const Header)
{
  if (diagType() == PRACTIVE)
  {
    Prevalidation prevalidation(*this);
    return prevalidation << Header();
  }
  else
    return *this;
}

Diag852Collector&
Diag852Collector::operator<<(const Itin& itin)
{
  if (diagType() == PRACTIVE)
  {
    Prevalidation prevalidation(*this);
    return prevalidation << itin;
  }
  else
    return (Diag852Collector&) DiagCollector::operator<<(itin);
}

Diag852Collector&
Diag852Collector::Prevalidation::operator<<(const Header)
{
  _dc.printHeader("DIAGNOSTIC 852");
  _dc.printHeader("PR: BAGGAGE PREVALIDATION");
  return _dc;
}

void
Diag852Collector::Prevalidation::printBaggageWeightOrPcs(const OptionalServicesInfo& info)
{
  std::ostringstream os;
  if (info.freeBaggagePcs() >= 0)
  {
    os << info.freeBaggagePcs() << "PC";
    if (info.freeBaggagePcs() > 1)
      os << "S";
   if (info.baggageWeight() > 0)
    os << "/";
  }
  if (info.baggageWeight() > 0)
    os << info.baggageWeight() << info.baggageWeightUnit();
  _dc << std::setw(14) << os.str();
}


void
Diag852Collector::Prevalidation::printSoftPassStatus(const OCFees& ocFees)
{
  if (ocFees.bagSoftPass().isNull())
    return;

  bool atLeastOne = false;
  if (ocFees.bagSoftPass().isSet(OCFees::BAG_SP_RULETARIFF))
  {
    _dc << "RULETARIFF";
    atLeastOne = true;
  }
  if (ocFees.bagSoftPass().isSet(OCFees::BAG_SP_TOURCODE))
  {
    if (atLeastOne)
      _dc << "\n" << std::string(44, ' ');
    _dc << "TOURCODE";
    atLeastOne = true;
  }
  if (ocFees.bagSoftPass().isSet(OCFees::BAG_SP_BTA_FARE_CHECKS))
  {
    if (atLeastOne)
      _dc << "\n" << std::string(44, ' ');
    _dc << "BTA FARE CHECKS";
  }
}

void
Diag852Collector::Prevalidation::printAllowanceInfo(const OptionalServicesInfo& info,
                                                    const OCFees& ocFees)
{
  _dc.displayVendor(info.vendor(), false);

  _dc << info.serviceSubTypeCode() << "  ";
  _dc.displayFltTktMerchInd(info.fltTktMerchInd());
  _dc << " ";
  _dc << std::setw(8) << info.seqNo();
  _dc << info.notAvailNoChargeInd() << "   ";
  printBaggageWeightOrPcs(info);
  printSoftPassStatus(ocFees);

  _dc << "\n";
}

void
Diag852Collector::Prevalidation::printChargeInfo(const OptionalServicesInfo* info,
                                                 const BaggageCharge& bc,
                                                 const CarrierCode carrierCode)
{
  _dc.setf(std::ios::left, std::ios::adjustfield);

  _dc.displayVendor(info->vendor(), false);

  _dc << info->serviceSubTypeCode() << "  ";
  _dc.displayFltTktMerchInd(info->fltTktMerchInd());
  _dc << " ";
  _dc << std::setw(8) << info->seqNo();
  _dc << std::setw(5) << bc.matchedBags().to_string();

  std::ostringstream amtOss;
  amtOss << bc.feeAmount() << bc.feeCurrency();
  _dc << std::setw(9) << amtOss.str();

  _dc << std::setw(4) << carrierCode;
  printSoftPassStatus(bc);

  _dc << "\n";
}

void
Diag852Collector::Prevalidation::printItineraryInfo(const Itin& itin)
{
  const BaggageTripType baggageTripType = itin.getBaggageTripType();

  _dc.printHeader("ITINERARY ANALYSIS");

  PricingTrx *pricingTrx = dynamic_cast<PricingTrx*>(_dc.trx());
  if (pricingTrx && pricingTrx->getTrxType() == PricingTrx::MIP_TRX)
    _dc << "ITIN NUM : " << itin.itinNum() << "\n";

  _dc.printOriginAndDestination(itin, baggageTripType.isUsDot());

  _dc << "JOURNEY TYPE : " << baggageTripType.getJourneyName(itin) << "\n";
  _dc << "TKT DATE : " << _dc.trx()->ticketingDate().dateToString(YYYYMMDD, "-") << "\n";
}

void
Diag852Collector::Prevalidation::
printBaggageAllowances(const boost::container::flat_map<PrecalcBaggage::CxrPair,
                       PrecalcBaggage::AllowanceRecords>& baggageAllowance)
{
  for (const auto& cxrPairData : baggageAllowance)
  {
    _dc.printHeader(std::string("ALLOWANCES: ") + cxrPairData.first.allowanceCxr());
    _dc << "ALLOWANCE CXR: " << cxrPairData.first.allowanceCxr() << "\n";
    _dc << "DEFER TARGET: " << cxrPairData.first.deferCxr() << "\n";

    const PrecalcBaggage::AllowanceRecords& records = cxrPairData.second;
    _dc << "S5 0DF RECORD: " << (records.s5Found ? "FOUND" : "NOT FOUND") << "\n\n";

    _dc.setf(std::ios::left, std::ios::adjustfield);
    _dc << std::setw(2) << "V";
    _dc << std::setw(6) << "SCODE";
    _dc << std::setw(10) << "SERVICE";
    _dc << std::setw(8) << "SEQ NUM ";
    _dc << std::setw(4) << "DFR";
    _dc << std::setw(14) << "PC/WEIGHT";
    _dc << std::setw(7) << "SOFT PASS STATUS";
    _dc << "\n";

    for (const OCFees* oc : records.s7s)
      printAllowanceInfo(*oc->optFee(), *oc);

    _dc << "\n";
  }
}

void
Diag852Collector::Prevalidation::
printBaggageCharges(const boost::container::flat_map<CarrierCode,
                    PrecalcBaggage::ChargeRecords>& baggageCharges)
{

  for (const auto& cxrRecords : baggageCharges)
  {
    _dc.printHeader(std::string("CHARGES: ") + cxrRecords.first);
    _dc.setf(std::ios::left, std::ios::adjustfield);
    _dc << std::setw(2) << "V";
    _dc << std::setw(6) << "SCODE";
    _dc << std::setw(10) << "SERVICE";
    _dc << std::setw(8) << "SEQ NUM ";
    _dc << std::setw(5) << "OCC";
    _dc << std::setw(9) << "AMOUNT";
    _dc << std::setw(4) << "CXR";
    _dc << std::setw(7) << "SOFT PASS STATUS";
    _dc << "\n";

    const PrecalcBaggage::ChargeRecords& records = cxrRecords.second;

    for (const auto& s5Records : records.s7s)
      for (const BaggageCharge* bc : s5Records.second)
        printChargeInfo(bc->optFee(), *bc, cxrRecords.first);
  }
}

Diag852Collector&
Diag852Collector::Prevalidation::operator<<(const Itin& itin)
{
  _dc.setf(std::ios::left, std::ios::adjustfield);

  printItineraryInfo(itin);

  const PrecalcBaggage::ItinData& itinData = *itin.getPrecalcBagData();
  uint16_t paxCounter = 0;
  for (const auto& paxDataPair : itinData.paxData)
  {
    ++paxCounter;
    const PrecalcBaggage::PaxData& paxData = paxDataPair.second;
    std::ostringstream paxHeaderString;
    paxHeaderString << "PASSENGER " << paxCounter
                    << "/" << itinData.paxData.size()
                    << ": " << paxDataPair.first->paxType();
    _dc.printHeader(paxHeaderString.str());
    uint16_t btCounter = 0;
    for (const PrecalcBaggage::BagTravelData& btData : paxData.bagTravelData)
    {
      _dc.printHeader(std::string("CHECKED PORTION"));
      const char fillChar = _dc.fill('0');
      _dc << setiosflags(std::ios::right) << std::setw(2)
          << ++btCounter << resetiosflags(std::ios::right)
         << " CHECKED PORTION : ";
      _dc.fill(fillChar);

      if (fallback::fallbackAddSegmentsNumbersToCheckedPortionSectionInDiagnostic852(_dc._trx))
        BaggageStringFormatter::old_printBaggageTravelSegments(*btData.bagTravel, _dc);
      else
        _dc << BaggageStringFormatter::printBaggageTravelSegmentsWithNumbering(*btData.bagTravel);

      _dc << "\n";

      const AirSeg* fcis = btData.bagTravel->getMsSeg(itin.getBaggageTripType().isUsDot())->toAirSeg();
      const AirSeg* mss = btData.bagTravel->getFciSeg(itin.getBaggageTripType().isUsDot())->toAirSeg();

      _dc << "MSS MARKETING: " << mss->carrier() << "\n";
      _dc << "MSS OPERATING: " << mss->operatingCarrierCode() << "\n";
      _dc << "FCIS MARKETING: " << fcis->carrier() << "\n";
      _dc << "FCIS OPERATING: " << fcis->operatingCarrierCode() << "\n";

      printBaggageAllowances(btData.allowance);
      printBaggageCharges(btData.charges);
    }
    _dc.printHeader("END "+paxHeaderString.str());
  }
  return _dc;
}

} // namespace tse
