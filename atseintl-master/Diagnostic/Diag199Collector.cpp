//----------------------------------------------------------------------------
//  File:        Diag199Collector.C
//  Authors:     Mike Carroll
//  Created:     July 5
//
//  Description: Diagnostic 199 formatter
//
//  Updates:
//          date - initials - description.
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

#include "Diagnostic/Diag199Collector.h"

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/MarketCriteria.h"
#include "BrandedFares/MarketResponse.h"
#include "Common/ClassOfService.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Customer.h"

#include <iomanip>
#include <iostream>

using namespace std;

namespace tse
{
Diag199Collector&
Diag199Collector::operator << (const TravelSeg& tvlSeg)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << "*********** START TRAVEL SEGMENT **************** " << std::endl;
    addTravelSegment(&tvlSeg);
    dc << "*********** END TRAVEL SEGMENT **************** " << std::endl;
    dc << '\n';
  }

  return *this;
}

Diag199Collector&
Diag199Collector::operator << (const Itin& itn)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << '\n';
  }

  return *this;
}

Diag199Collector&
Diag199Collector::operator << (const PricingTrx& trx)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc.setf(std::ios::left, std::ios::adjustfield);
    std::string givenFM = oneFM(trx);
    std::string compFM("");
    dc << "***************** START DIAG 199 ************************** " << std::endl;

    // Do Validating Carrier

    if (!trx.getRequest()->validatingCarrier().empty())
      dc << "VALIDATING CARRIER: " << trx.getRequest()->validatingCarrier() << std::endl;

    dc << "NUMBER OF FARE MARKETS IN TRX: " << trx.fareMarket().size() << std::endl;
    dc << " \n";

    if (!(givenFM.empty()))
    {
      dc << "*** RESULTS FOR ONLY FAREMARKET " << givenFM << " ARE DISPLAYED BELOW ***"
         << std::endl;
      dc << "*** TO DISPLAY  ALL  THE FAREMARKETS TRY ENTRY WITHOUT /FM ***" << std::endl;
    }

    std::map<FareMarket*, int> fmPointerToIndex;
    uint16_t i = 0;
    // Fare Markets
    for (FareMarket* const fareMarket : trx.fareMarket())
    {
      fmPointerToIndex.emplace(fareMarket, ++i);
      if (!(givenFM.empty()))
        compFM = fareMarket->boardMultiCity() + fareMarket->offMultiCity();

      if ((!(givenFM.empty())) && (!(compFM.empty())))
      {
        if (compFM != givenFM)
          continue;
      }
      dc << " ---------------------------------------------------------- " << std::endl;
      if (TrxUtil::newDiscountLogic(trx))
      {
        dc << " *** " << (TrxUtil::validateDiscountNew(trx, *fareMarket) ? "" : "FAILED ") << "FARE MARKET " << i << " ***" << std::endl;
      }
      else
      {
                dc << " *** " << "FARE MARKET " << i << " ***" << std::endl;
      }
      dc << " ---------------------------------------------------------- " << std::endl;
      addFareMarket(trx, *fareMarket);
    }

    dc << " ---------------------------------------------------------- " << std::endl;
    dc << "\n\n";

    bool isRexPricingTrx = trx.excTrxType() == PricingTrx::AR_EXC_TRX;
    bool isRefundPricingTrx = trx.excTrxType() == PricingTrx::AF_EXC_TRX;

    // Travel Segments
    if (isRefundPricingTrx)
    {
      dc << " *** FLOWN ITINERARY ***" << std::endl;
      const std::vector<Itin*> newItin = static_cast<const BaseExchangeTrx&>(trx).newItin();
      if (!newItin.empty())
      {
        for (const TravelSeg* travelSeg : newItin.front()->travelSeg())
          addTravelSegment(travelSeg);
      }
    }
    else
    {
      dc << " *** ALL TRAVEL SEGMENTS ***" << std::endl;
      for (const TravelSeg* travelSeg : trx.travelSeg())
        addTravelSegment(travelSeg);
    }
    dc << " \n";

    const ExchangePricingTrx* exchangePricingTrx = dynamic_cast<const ExchangePricingTrx*>(&trx);
    if (exchangePricingTrx != 0)
    {
      if (!exchangePricingTrx->exchangeItin().empty())
      {
        dc << " *** EXCHANGE ITIN *** " << std::endl;
        for (const TravelSeg* travelSeg : exchangePricingTrx->exchangeItin().front()->travelSeg())
          addTravelSegment(travelSeg);
        dc << " \n";
      }
    }

    if (isRexPricingTrx || isRefundPricingTrx)
    {
      const RexBaseTrx* rexBaseTrx = static_cast<const RexBaseTrx*>(&trx);

      if (!rexBaseTrx->exchangeItin().empty())
      {
        if (isRexPricingTrx)
          dc << " *** EXCHANGE ITIN *** " << std::endl;
        else
          dc << " *** ORIGINALLY TICKETED ITINERARY *** " << std::endl;
        for (const TravelSeg* travelSeg : rexBaseTrx->exchangeItin().front()->travelSeg())
          addTravelSegment(travelSeg);
        dc << " \n";
      }
    }

    if (trx.paxType().size() > 0)
    {
      dc << " *** PAX TYPES: ";
      for (const PaxType* const paxType : trx.paxType())
      {
        if (paxType)
          dc << paxType->paxType() << "  ";
      }
    }

    dc << "\n";
    dc << " ---------------------------------------------------------- " << std::endl;
    dc << "\n";
    addBilling(*trx.billing());
    dc << " \n";

    if (isRefundPricingTrx)
      addRefundDates(static_cast<const RexBaseTrx&>(trx));
    else
      addPricingDates(trx);

    if (trx.diagnostic().diagParamMapItem("DD") == "INTAVL")
    {
      dc << " ---------------------------------------------------------- " << std::endl;
      dc << " *** INTERLINE AVAILABILITY ***" << std::endl;
      dc << " ---------------------------------------------------------- " << std::endl;

      displayInterlineIntralineTables(trx);

      for (Itin* const itin : trx.itin())
      {
        dc << " ITINERARY";
        if (itin->itinNum() != -1)
          dc << " " << itin->itinNum();
        dc << ":\n";
        for (TravelSeg* const travelSeg : itin->travelSeg())
        {
          addTravelSegment(travelSeg);
          if (itin->interlineJourneyInfo().count(travelSeg) != 0)
          {
            Itin::InterlineJourneyMarketMap::const_iterator mktIt =
                itin->interlineJourneyMarket().find(travelSeg);
            if (mktIt != itin->interlineJourneyMarket().end())
              dc << "   INTERLINE MARKET " << mktIt->second.first->origAirport()
                 << mktIt->second.second->destAirport() << "\n";
            dc << "   INTERLINE:  ";
            ClassOfServiceList* cos = itin->interlineJourneyInfo()[travelSeg];
            addCOS(cos);
            dc << "\n";
          }
          else
          {
            dc << "   NO INTERLINE AVAILABILITY\n";
          }
        }
        dc << "\n";
      }
      dc << " ---------------------------------------------------------- " << std::endl;
    }

    if (trx.getRequest()->isBrandedFaresRequest() &&
        trx.diagnostic().diagParamMapItem("DD") == "CTXSHOP")
    {
      displayContextShoppingInfo(trx);
    }

    dc << "***************** END   DIAG 199 ************************** " << std::endl;
  }
  return *this;
}

void
Diag199Collector::addDiscountInfo(const PricingTrx &trx, const FareMarket& fareMarket)
{
  DiagCollector& dc = (DiagCollector&)*this;

  if (!TrxUtil::validateDiscountNew(trx, fareMarket))
  {
    dc << " FAILED FARE MARKET DUE TO DISCOUNT OR MARK UP INCONSISTENCY !!!" << std::endl;
    dc << std::endl;
    return;
  }

  const DiscountAmount* discountAmount = trx.getDiscountAmountNew(fareMarket);

  if (discountAmount != nullptr)
  {
    if (discountAmount->amount < 0)
      dc << " MARK UP";
    else
      dc << " DISCOUNT";

    dc << " AMOUNT: " << fabs(discountAmount->amount) << " " << discountAmount->currencyCode;

    if (!trx.isMip())
      dc << " FOR SEGMENTS: " << discountAmount->startSegmentOrder << "-"
         << discountAmount->endSegmentOrder << std::endl;
  }

  const Percent* discountPercentage = trx.getDiscountPercentageNew(fareMarket);
  if (discountPercentage != nullptr)
  {
    if (*discountPercentage < 0)
      dc << " MARK UP";
    else
      dc << " DISCOUNT";
    dc << " PERCENTAGE: " << fabs(*discountPercentage) << " PERCENT." << std::endl;
  }

  dc << std::endl;
}

void
Diag199Collector::addFareMarket(const PricingTrx& trx, const FareMarket& fareMarket)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc.setf(std::ios::left, std::ios::adjustfield);

    if (trx.getRequest()->multiTicketActive() && !trx.multiTicketMap().empty())
    {
      dc.displayFareMarketItinNumsMultiTkt(" APPLIES TO ITINS   :", trx, fareMarket);
    }
    else
    {
      dc.displayFareMarketItinNums(" APPLIES TO ITINS   :", trx, fareMarket);
    }

    dc << " ORIGIN-DESTINATION : " << fareMarket.origin()->loc() << "-"
       << fareMarket.destination()->loc() << std::endl;

    dc << " BOARD-OFF CITIES   : " << fareMarket.boardMultiCity() << "-"
       << fareMarket.offMultiCity() << std::endl;

    std::string boardAirport("***");
    std::string offAirport("***");

    // Do board-off cities and airports
    if (fareMarket.travelSeg().front() != 0)
      boardAirport =  fareMarket.travelSeg().front()->origAirport();
    if (fareMarket.travelSeg().back() != 0)
      offAirport = fareMarket.travelSeg().back()->destAirport();

    dc << " BOARD-OFF AIRPORTS : " << boardAirport << "-" << offAirport << std::endl;

    // Do Governing Carrier
    dc << " GOVERNING CARRIER  : " << fareMarket.governingCarrier() << std::endl;

    dc << " VALIDATING CARRIERS: " << getFMValidatingCarriers(trx, fareMarket) << "\n";

    // Do Global Direction
    std::string gbd("UNK");
    gbd = addGlobalDirection(fareMarket);
    // globalDirectionToStr(gbd, fareMarket.globalDirection());
    dc << " GLOBAL DIRECTION   : " << gbd << std::endl;

    // Do Geo Travel type
    dc << " GEO  TRAVEL  TYPE  : " << DiagnosticUtil::geoTravelTypeToString(fareMarket.geoTravelType()) << std::endl;

    // Do Direction e.g. INBOUND/OUTBOUND
    std::string directn("UNKNOWN");
    switch (fareMarket.direction())
    {
    case FMDirection::UNKNOWN:
      directn = "UNKNOWN";
      break;
    case FMDirection::INBOUND:
      directn = "INBOUND";
      break;
    case FMDirection::OUTBOUND:
      directn = "OUTBOUND";
      break;
    default:
      break;
    }
    dc << " DIRECTION          : " << directn << std::endl;

    // Do Flight Track Indicator
    if (fareMarket.fltTrkIndicator())
      dc << " FLT TRACK INDICATOR: TRUE" << std::endl;
    else
      dc << " FLT TRACK INDICATOR: FALSE" << std::endl;

    if(trx.isPbbRequest() == PBB_RQ_PROCESS_BRANDS &&
       fareMarket.hasBrandCode())
    {
      std::string tmp = fareMarket.getBrandCode();
      boost::to_upper(tmp);
      dc << " BRAND CODE         : " << setw(11) << tmp << std::endl;
    }

    if ( TRAVEL_SEG_DEFAULT_ID != fareMarket.legIndex() )
      dc << " LEG INDEX          : " << fareMarket.legIndex() << std::endl;
    dc << " TRAVEL DATE        : " << fareMarket.travelDate().dateToSqlString() << std::endl;

    if (!fareMarket.fareBasisCode().empty())
      dc << " FARE BASIS CODE    : " << fareMarket.fareBasisCode()
         << "   FBC USAGE : " << fareMarket.fbcUsage() << std::endl;

    if (trx.getTrxType() == PricingTrx::MIP_TRX)
      printItinList(trx, fareMarket);

    dc << " \n";
    if ( trx.isIataFareSelectionApplicable() )
    {
      char tpmStatus = 'N';
      if ( fareMarket.isFirstCrossingAndHighTpm() )
        tpmStatus = 'B';
      else if ( fareMarket.isHighTPMGoverningCxr() )
        tpmStatus = 'Y';
      dc << " HIGHEST TPM GOVERNING CARRIER SELECTED :  " << tpmStatus << std::endl;
      dc << " \n";
    }

    if (TrxUtil::newDiscountLogic(trx))
    {
      addDiscountInfo(trx, fareMarket);
    }

    // Do Travel Segments
    dc << " *** TRAVEL SEGMENTS *** " << std::endl;
    for (const TravelSeg* travelSeg : fareMarket.travelSeg())
      addTravelSegment(travelSeg);

    // Do Primary Sector
    dc << " *** PRIMARY SECTOR ***" << std::endl;
    if (fareMarket.primarySector() == 0)
      dc << " PRIMARY SECTOR IS NULL " << std::endl;
    else
      addTravelSegment(fareMarket.primarySector());
    dc << std::endl;

    // Do Side Trip Travel Segments
    dc << " *** SIDE TRIPS ***" << std::endl;
    dc << " NUMBER OF SIDE TRIPS: " << fareMarket.sideTripTravelSeg().size() << std::endl;
    if (!(fareMarket.sideTripTravelSeg().empty()))
    {
      addSideTripTvlSegs(fareMarket);
    }
    dc << " \n";

    string s1;
    dc << " *** CUSTOMER/CARRIER PREFERENCE DATA ***" << std::endl;
    if (trx.getRequest()->ticketingAgent() == 0)
      dc << " AGENT POINTER: NULL " << std::endl;
    if (trx.getRequest()->ticketingAgent()->agentTJR() == 0)
      dc << " CUSTOMER/AGENT TJR POINTER: NULL " << std::endl;
    if (!(trx.getRequest()->ticketingAgent() == 0 ||
          trx.getRequest()->ticketingAgent()->agentTJR() == 0))
    {
      if (trx.getRequest()->ticketingAgent()->agentTJR()->availIgRul2StWpnc() == YES)
        s1 = "NO ";
      else
        s1 = "YES";

      dc << " SOLO CUSTOMER: " << s1;
    }

    if (trx.getOptions()->soloActiveForPricing())
      s1 = "YES";
    else
      s1 = "NO ";

    dc << " SOLO ACTIVE: " << s1 << " \n";

    if (trx.getOptions()->journeyActivatedForPricing())
      s1 = "YES";
    else
      s1 = "NO ";
    dc << "  JOURNEY ACTIVE: " << s1;

    addCustomerCxrData(trx, fareMarket);

    printBrandingInfo(trx, fareMarket);

    // Do ATAE data for class of services
    dc << " *** ATAE CLASS OF SERVICES ***" << std::endl;
    const std::vector<std::vector<ClassOfService*>*>& cosVecVec = fareMarket.classOfServiceVec();
    uint16_t lenCosVecVec = fareMarket.classOfServiceVec().size();
    uint16_t iCos = 0;
    for (const TravelSeg* travelSeg : fareMarket.travelSeg())
    {
      addTravelSegment(travelSeg);

      if (!travelSeg->isAir())
      {
        dc << " \n";
        continue;
      }

      dc << "   THRU AVAIL: ";
      if (iCos < lenCosVecVec)
      {
        addCOS(cosVecVec[iCos++]);
      }
      dc << " \n";
      dc << "   SOLO AVAIL: ";
      addCOS(&(travelSeg->classOfService()));
      dc << " \n";
      dc << " \n";

      if(travelSeg->getSecondaryRBDReasonCodeStatus() != TravelSeg::DONOTZEROOUT)
      {
        dc << "   ZERO OUT SEATS FOR SECONDARY RBD FOR BOOKING CODES: ";
        for (BookingCode bc:trx.getRequest()->brandedFareSecondaryBookingCode())
          dc << bc << ", ";
        dc << " \n";

        switch(travelSeg->getSecondaryRBDReasonCodeStatus())
        {
          case TravelSeg::INTERNATIONALFLIGHTS_PBKC:
            dc << "   REASON: ITIN HAS ATLEAST 1 INTERNATIONAL FLIGHT WITH PRIMARY BOOKING CODE";
            break;
          case TravelSeg::DOMESTICFLIGHTS_PBKC:
            dc << "   REASON: ITIN HAS ALL DOMESTIC FLIGHTS WITH ATLEAST 1 WITH PRIMARY BOOKING CODE";
            break;
          case TravelSeg::DOMESTICFLIGHTS_NO_PBKC:
            dc << "   REASON: ITIN HAS ALL DOMESTIC FLIGHTS BUT NONE WITH PRIMARY BOOKING CODE";
            break;
          default: break;
        }
        dc << " \n";
        dc << " \n";
      }
    }
    dc << " \n";
  }
  return;
}

void
Diag199Collector::printItinList(const PricingTrx& trx, const FareMarket& fm)
{
  *this << " ITINERARIES        : ";

  std::set<std::string> itinList;
  for (Itin* itin : trx.itin())
  {
    if (std::find(itin->fareMarket().begin(), itin->fareMarket().end(), &fm) != itin->fareMarket().end())
    {
      std::stringstream out;
      out << itin->itinNum();
      if (itin->errResponseCode() != ErrorResponseException::NO_ERROR)
        out << " (BAD ITIN WITH ERROR RESPONSE CODE = " << itin->errResponseCode() << ")";
      itinList.insert(out.str());
    }
  }
  *this << DiagnosticUtil::containerToString(itinList) << std::endl;
}

std::string
Diag199Collector::getFMValidatingCarriers(const PricingTrx& trx, const FareMarket& fareMarket)
{
  if (!fareMarket.validatingCarriers().empty())
    return DiagnosticUtil::containerToString(fareMarket.validatingCarriers());

  if (trx.isValidatingCxrGsaApplicable() && (trx.itin().size() == 1) &&
      (trx.itin().front()->validatingCxrGsaData()->size() == 1))
  {
    return trx.itin().front()->validatingCxrGsaData()->validatingCarriersData().begin()->first;
  }

  return "";
}

void
Diag199Collector::addSideTripTvlSegs(const FareMarket& fareMarket)
{
  DiagCollector& dc = (DiagCollector&)*this;
  uint16_t i = 0;
  for (const std::vector<TravelSeg*>& travelSegs : fareMarket.sideTripTravelSeg())
  {
    dc << " SIDE TRIP NUMBER " << ++i << std::endl;
    for (const TravelSeg* const travelSeg : travelSegs)
      addTravelSegment(travelSeg);
  }
}

void
Diag199Collector::addTravelSegment(const TravelSeg* tvlSeg)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(tvlSeg);
  if (airSeg == 0)
  {
    dc << "   ARUNK   ";
    if (tvlSeg != 0)
      dc << tvlSeg->origAirport() << "-" << tvlSeg->destAirport();
    dc << std::endl;
    return;
  }
  dc << " " << tvlSeg->pnrSegment();
  dc << " " << airSeg->carrier();
  dc << setw(4) << airSeg->flightNumber();
  dc << setw(2) << airSeg->getBookingCode();
  dc << " " << setw(5) << airSeg->departureDT().dateToString(DDMMM, "");
  dc << " " << airSeg->origAirport() << airSeg->destAirport();
  dc << " " << setw(6) << airSeg->departureDT().timeToString(HHMM_AMPM, "");
  dc << " " << setw(6) << airSeg->arrivalDT().timeToString(HHMM_AMPM, "");
  std::string conStop("");
  if (airSeg->stopOver())
    conStop = "O-";
  else
    conStop = "X-";

  conStop = conStop + tvlSeg->destAirport();
  dc << " " << conStop;
  dc << " CAB:" << airSeg->bookedCabin();
  dc << " " << airSeg->resStatus() << "/" << airSeg->realResStatus();

  dc << " " << DiagnosticUtil::geoTravelTypeTo3CharString(airSeg->geoTravelType()) << " " << airSeg->marriageStatus();

  if (!airSeg->unflown())
    dc << " F";

  dc << " \n";
  return;
}

void
Diag199Collector::addCOS(const std::vector<ClassOfService*>* cosVec)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  uint16_t adj = 0;
  for (const ClassOfService* classOfService : *cosVec)
  {
    dc << classOfService->bookingCode() << classOfService->numSeats() << " ";

    // adj will keep an account if we should keep displaying the booking codes
    // in the same line or should we start a new line
    if (classOfService->numSeats() > 99)
    {
      adj = adj + 5;
    }
    else
    {
      if (classOfService->numSeats() < 10)
        adj = adj + 3;
      else
        adj = adj + 4;
    }
    // only display 40 characters in one line
    if (adj > 40)
    {
      dc << " \n";
      dc << "               ";
      adj = 0;
    }
  }
}

std::string
Diag199Collector::addGlobalDirection(const FareMarket& fareMarket)
{
  std::string gbd("UNKNOWN");
  switch (fareMarket.getGlobalDirection())
  {
  case GlobalDirection::AF:
    gbd = "AF   -VIA AFRICA";
    break;

  case GlobalDirection::AL:
    gbd = "AL   -FBR ALL FARES INCLUDING EH/TS";
    break;

  case GlobalDirection::AP:
    gbd = "AP   -VIA ATLANTIC AND PACIFIC";
    break;

  case GlobalDirection::AT:
    gbd = "AT   -VIA ATLANTIC";
    break;

  case GlobalDirection::CA:
    gbd = "CA   -CANADA";
    break;

  case GlobalDirection::CT:
    gbd = "CT   -CIRCLE TRIP";
    break;

  case GlobalDirection::DI:
    gbd = "DI   -SPECIAL USSR TC3 APP BRITSH AIRWAYS";
    break;

  case GlobalDirection::DO:
    gbd = "DO   -DOMESTIC";
    break;

  case GlobalDirection::DU:
    gbd = "DU   -SPECIAL USSR TC2 APP BRITSH AIRWAYS";
    break;

  case GlobalDirection::EH:
    gbd = "EH   -WITHIN EASTERN HEMISPHERE";
    break;

  case GlobalDirection::EM:
    gbd = "EM   -VIA EUROPE MIDDLE EAST";
    break;

  case GlobalDirection::EU:
    gbd = "EU   -VIA EUROPE";
    break;

  case GlobalDirection::FE:
    gbd = "FE   -FAR EAST";
    break;

  case GlobalDirection::IN:
    gbd = "IN   -FBR FOR INTL INCLDNG AT/PA/WH/CT/PV";
    break;

  case GlobalDirection::ME:
    gbd = "ME   -VIA MIDDLE EAST OTHER THAN ADEN";
    break;

  case GlobalDirection::NA:
    gbd = "NA   -FBR FOR N AMERICA INCL US/CA/TB/PV";
    break;

  case GlobalDirection::NP:
    gbd = "NP   -VIA NORTH OR CENTRAL PACIFIC";
    break;

  case GlobalDirection::PA:
    gbd = "PA   -VIA SOUTH/CENTRAL OR NORTH PACIFIC";
    break;

  case GlobalDirection::PE:
    gbd = "PE   -TC1 CENTRAL/SOUTHERN AFRICA VIA TC3";
    break;

  case GlobalDirection::PN:
    gbd = "PN   -BTWN TC1/TC3 VIA PCIFIC/VIA N AMRCA";
    break;

  case GlobalDirection::PO:
    gbd = "PO   -VIA POLAR ROUTE";
    break;

  case GlobalDirection::PV:
    gbd = "PV   -PR/VI TO US/CA";
    break;

  case GlobalDirection::RU:
    gbd = "RU   -RUSSIA TO AREA 3";
    break;

  case GlobalDirection::RW:
    gbd = "RW   -ROUND THE WORLD";
    break;

  case GlobalDirection::SA:
    gbd = "SA   -SOUTH ATLANTIC ONLY";
    break;

  case GlobalDirection::SN:
    gbd = "SN   -VIA S ATLC RTG 1 DIR VIA N/MID ATLC";
    break;

  case GlobalDirection::SP:
    gbd = "SP   -VIA SOUTH POLAR";
    break;

  case GlobalDirection::TB:
    gbd = "TB   -TRANSBORDER";
    break;

  case GlobalDirection::TS:
    gbd = "TS   -VIA SIBERIA";
    break;

  case GlobalDirection::TT:
    gbd = "TT   -AREA2";
    break;

  case GlobalDirection::US:
    gbd = "US   -INTRA US";
    break;

  case GlobalDirection::WH:
    gbd = "WH   -WITHIN WESTERN HEMISPHERE";
    break;

  case GlobalDirection::XX:
    gbd = "XX   -UNIVERSAL";
    break;

  case GlobalDirection::ZZ:
    gbd = "ZZ   -ANY GLOBAL";
    break;

  default:
    break;
  }
  return gbd;
}

std::string
Diag199Collector::oneFM(const PricingTrx& trx)
{
  std::string specifiedFm("");
  typedef std::map<std::string, std::string>::const_iterator DiagParamMapVecIC;
  DiagParamMapVecIC end = trx.diagnostic().diagParamMap().end();
  DiagParamMapVecIC begin = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_MARKET);
  if (begin != end)
  {
    size_t len = ((*begin).second).size();
    if (len != 0)
    {
      specifiedFm = ((*begin).second);
      // if ( ((*begin).second).substr(0, len ) !=  paxTfare.fareClass() )
      //  return;
    }
  }
  return specifiedFm;
}

void
Diag199Collector::addCustomerCxrData(const PricingTrx& trx, const FareMarket& fareMarket)
{
  DiagCollector& dc = (DiagCollector&)*this;

  std::string s1;
  if (fareMarket.flowMarket())
    s1 = "YES";
  else
    s1 = "NO";
  dc << "  FLOW FARE MARKET: " << s1;

  std::map<const TravelSeg*, bool> marriedSegments;
  bool outOfSeqMarriage, journeyByMarriage;
  bool married = JourneyUtil::getMarriedSegments(
      trx, fareMarket, marriedSegments, journeyByMarriage, outOfSeqMarriage);
  if (outOfSeqMarriage)
    s1 = "OUT OF SEQ";
  else if (married)
    s1 = "YES";
  else
    s1 = "NO";

  dc << "  MARRIED: " << s1 << std::endl;

  uint16_t iAvlBrk = 0;
  for (TravelSeg* travelSeg : fareMarket.travelSeg())
  {
    const AirSeg* airSegJ = dynamic_cast<const AirSeg*>(travelSeg);
    if (airSegJ == 0)
    {
      dc << "   ARUNK ";
    }
    else
    {
      dc << " " << travelSeg->pnrSegment();
      dc << " " << airSegJ->carrier();
      dc << setw(4) << airSegJ->flightNumber();
    }
    dc << "  JOURNEY TYPE:";
    if (travelSeg->carrierPref() == 0)
    {
      s1 = "000  ";
    }
    else
    {
      if (airSegJ->flowJourneyCarrier())
        s1 = "FLOW ";
      else if (airSegJ->localJourneyCarrier())
        s1 = "LOCAL";
      else
        s1 = "NONE ";
    }
    dc << s1;

    dc << "  SOLO CXR:";
    if (travelSeg->carrierPref() == 0)
      s1 = "000";
    else if (travelSeg->carrierPref()->availabilityApplyrul2st() == NO)
      s1 = "YES";
    else
      s1 = "NO ";
    dc << s1;

    dc << "  AVL BREAK:";
    if (fareMarket.availBreaks().empty() ||
        fareMarket.travelSeg().size() != fareMarket.availBreaks().size())
    {
      s1 = "UNK";
    }
    else
    {
      if (fareMarket.availBreaks()[iAvlBrk++])
        s1 = "TRUE";
      else
        s1 = "FALSE";
    }
    dc << s1;
    dc << " \n";
  }

  dc << " \n";
}

void
Diag199Collector::addBilling(const Billing& billing)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " BILLING INFORMATION" << std::endl;
  dc << "     USER PCC      : " << billing.userPseudoCityCode() << std::endl;
  dc << "     USER STATION  : " << billing.userStation() << std::endl;
  dc << "     USER BRANCH   : " << billing.userBranch() << std::endl;
  dc << "     PARTITION     : " << billing.partitionID() << std::endl;
  dc << "     SET ADDR      : " << billing.userSetAddress() << std::endl;
  dc << "     SERVICE NAME  : " << billing.serviceName() << std::endl;
  dc << "     AAA CITY      : " << billing.aaaCity() << std::endl;
  dc << "     AAA SINE      : " << billing.aaaSine() << std::endl;
  dc << "     ACTION CODE   : " << billing.actionCode() << std::endl;
  dc << "     TRANSACTION ID: " << billing.transactionID() << std::endl;
}

void
Diag199Collector::addPricingDates(const PricingTrx& trx)
{
  *this << " \n";
  *this << " DISPLAY ONLY  : " << trx.displayOnly() << std::endl;
  *this << " TRAVEL DATE   : " << trx.travelDate().dateToSqlString() << std::endl;
  *this << " BOOKING DATE  : " << trx.bookingDate().dateToSqlString() << std::endl;
  *this << " TICKETING DATE: " << trx.ticketingDate().dateToSqlString() << std::endl;
  *this << " \n";
}

void
Diag199Collector::addRefundDates(const RexBaseTrx& trx)
{
  *this << " \n";
  *this << " DISPLAY ONLY  : " << trx.displayOnly() << std::endl;
  *this << " TRAVEL DATE   : " << trx.travelDate().dateToSqlString() << std::endl;
  *this
      << " REFUND DATE   : "
      << (static_cast<const RexBaseRequest*>(trx.getRequest()))->getTicketingDT().dateToSqlString()
      << std::endl;
  *this << " TICKETING DATE: " << trx.originalTktIssueDT().dateToSqlString() << std::endl;
  *this << " \n";
}

void
Diag199Collector::printBrandingInfo(const PricingTrx& trx, const FareMarket& fm)
{
  uint16_t brandSize = fm.marketIDVec().size();
  if (brandSize == 0)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << " *** BRAND INFORMATION ***\n";

  for (const int& marketId : fm.marketIDVec())
    printBrandingInfo(trx, fm, marketId);
}

void
Diag199Collector::printBrandingInfo(const PricingTrx& trx,
                                    const FareMarket& fm,
                                    const int& marketId)
{
  const auto brandedMarket = trx.brandedMarketMap().find(marketId);
  if (brandedMarket == trx.brandedMarketMap().end())
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  const std::vector<MarketResponse*>& mkts = brandedMarket->second;
  for (MarketResponse* mkt : mkts)
  {
    dc << " BRAND MARKET " << mkt->getMarketID() << ": "
       << mkt->marketCriteria()->departureAirportCode() << "-" << mkt->carrier() << "-"
       << mkt->marketCriteria()->arrivalAirportCode() << "\n";

    dc << "   BRAND PROGRAMS:\n";

    for (BrandProgram* brandProgram : mkt->brandPrograms())
    {
      dc << "     ID: " << brandProgram->programID() << "  ";
      dc << "     CODE: " << brandProgram->programCode() << "\n";
      dc << "     NAME: " << brandProgram->programName() << "\n";

      std::string desc = brandProgram->programDescription();
      if (!desc.empty())
        dc << "     DESCRIPTION: " << desc << "\n";

      dc << "     BRANDS:\n";

      for (BrandInfo* brand : brandProgram->brandsData())
        dc << "       " << brand->brandCode() << " - " << brand->brandName() << "\n";
    }
  }

  dc << std::endl;
}

//----------------------------------------------------------------------------
// Diag199Collector::displayInterlineIntralineTables
//----------------------------------------------------------------------------
void
Diag199Collector::displayInterlineIntralineTables(const PricingTrx& trx)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  TrxUtil::InterMap interMap;
  TrxUtil::getInterlineAvailabilityCxrsNPartners(trx, interMap);

  for (const auto& inter : interMap)
  {
    dc << " CXR: " << inter.first <<" PARTNERS: ";

    std::vector<CarrierCode> cxrVec = inter.second;
    for (CarrierCode cxr : cxrVec)
      dc << cxr << ",";
    dc << "\n";
  }

  dc << " ----------------------------------------------------- " << std::endl;
  dc << " ***            INTRALINE AVAILABILITY             *** " << std::endl;
  dc << " ----------------------------------------------------- " << std::endl;

  TrxUtil::IntraMap intraMap;
  TrxUtil::getIntralineAvailabilityCxrsNPartners(trx, intraMap);

  for (const auto& intra : intraMap)
  {
    dc << " NAME: " << setw(11) << intra.first <<" PARTNERS: ";

    std::vector<CarrierCode> cxrVec = intra.second;
    for (CarrierCode cxr : cxrVec)
      dc << cxr << ",";
    dc << "\n";
  }
  dc << "\n";
}

//----------------------------------------------------------------------------
// Diag199Collector::displayContextShoppingInfo
//----------------------------------------------------------------------------
void
Diag199Collector::displayContextShoppingInfo(const PricingTrx& trx)
{
  // We expect fixed legs to be the same in all itins.
  // Let's collect all fixed segments from the first itin.

  if (trx.itin().empty())
    return;

  DiagCollector& dc = *this;

  auto& contexts = trx.getFareComponentShoppingContexts();
  skipper::FareComponentShoppingContext* prevContext = nullptr;

  auto itin = trx.itin().front();
  for (auto seg : itin->travelSeg())
  {
    auto ctxIt = contexts.find(seg->pnrSegment());
    if (ctxIt != contexts.end())
    {
      auto context = ctxIt->second;
      if (context != prevContext)
      {
        if (!prevContext)
        {
          dc << std::endl;
        }

        prevContext = context;

        dc << " ---------------------------------------------------------- " << std::endl;
        dc << " *** CONTEXT SHOPPING FARE COMPONENT ***" << std::endl;

        dc << " FARE BASIS CODE      : " << context->fareBasisCode << std::endl;
        dc << " FARE CALC FARE AMOUNT: " << context->fareCalcFareAmt << std::endl;
        dc << " BRAND CODE           : " << context->brandCode << std::endl;

        dc << std::endl;
        dc << " *** VCTR ***" << std::endl;
        if (context->vctr)
        {
          dc << " VENDOR: " << context->vctr->vendor();
          dc << "  CARRIER: " << context->vctr->carrier();
          dc << "  TARIFF: " << context->vctr->tariff();
          dc << "  RULE: " << context->vctr->rule();
        }
        else
        {
          dc << " N/A";
        }
        dc << std::endl;

        dc << std::endl;
        dc << " *** TRAVEL SEGMENTS *** " << std::endl;
      }

      addTravelSegment(seg);
    }
  }

  dc << " ---------------------------------------------------------- " << std::endl;
  dc << " *** CONTEXT SHOPPING FIXED LEGS ***" << std::endl;
  for (size_t i = 0; i < trx.getFixedLegs().size(); ++i)
  {
    dc << " LEG " << (i+1) << ": " << (trx.getFixedLegs()[i] ? "FIXED" : "SHOPPED") << std::endl;
  }
  dc << " ---------------------------------------------------------- " << std::endl;
}
}
