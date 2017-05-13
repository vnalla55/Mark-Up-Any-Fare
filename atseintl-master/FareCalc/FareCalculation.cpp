//----------------------------------------------------------------------------
//  File:        FareCalculation.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 854 to display content of FareCalcConfig data
//
//  Updates:
//          06/16/2004  BT - create.
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
#include "FareCalc/FareCalculation.h"

#include "Common/Assert.h"
#include "Common/BaggageStringFormatter.h"
#include "Common/BrandingUtil.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/DiagMonitor.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/FcConfig.h"
#include "Common/FreeBaggageUtil.h"
#include "Common/IAIbfUtils.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/Money.h"
#include "Common/MultiTicketUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/VecMultiMap.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PrivateIndicator.h"
#include "DataModel/SurchargeData.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/Loc.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FareCalcHelper.h"
#include "FareCalc/FareUsageIter.h"
#include "FareCalc/FcDispFareUsage.h"
#include "FareCalc/FcMessage.h"
#include "FareCalc/FcUtil.h"
#include "Routing/MileageInfo.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Rules/TicketingEndorsement.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"

#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>
#include <utility>

#include <boost/foreach.hpp>

using namespace std;

namespace tse
{
FALLBACK_DECL(endorsementExpansion);
FALLBACK_DECL(fallbackEndorsementsRefactoring)

static Logger
logger("atseintl.FareCalc.FareCalculation");

class FcMessageCollection
{
protected:
  enum ClientType
  {
    CT_all,
    CT_fareDisplay,
    CT_calcTotal
  };

  typedef std::vector<std::string> Messages;
  typedef std::tuple<ClientType, FcMessage::MessageType, bool, Messages> CollectionElement;
  typedef std::vector<CollectionElement> Collection;

  static constexpr int TupleClient = 0;
  static constexpr int TupleMsgType = 1;
  static constexpr int TuplePrefix = 2;
  static constexpr int TupleMessages = 3;

public:
  FcMessageCollection() {}

  void
  add(Messages& messages, FcMessage::MessageType type = FcMessage::WARNING, bool prefix = false)
  {
    _collection.push_back(std::make_tuple(CT_all, type, prefix, messages));
  }

  void add(FcMessage::MessageType type, const char* message)
  {
    _collection.push_back(std::make_tuple(CT_calcTotal, type, true, Messages(1, message)));
  }

  void add(const std::string& message)
  {
    // messageType and prefix is not importing in this case
    _collection.push_back(
        std::make_tuple(CT_fareDisplay, FcMessage::WARNING, true, Messages(1, message)));
  }

  void transform(std::vector<FcMessage>& fcMessage) const
  {
    for (const CollectionElement& element : _collection)
    {
      const ClientType ct = std::get<TupleClient>(element);
      if (ct == CT_all || ct == CT_calcTotal)
      {
        BOOST_FOREACH (const std::string& msg, std::get<TupleMessages>(element))
          fcMessage.push_back(
              FcMessage(std::get<TupleMsgType>(element), 0, msg, std::get<TuplePrefix>(element)));
      }
    }
  }

  void transform(FareCalc::FcStream& stream) const
  {
    for (const CollectionElement& element : _collection)
    {
      ClientType ct = std::get<TupleClient>(element);
      if (ct == CT_all || ct == CT_fareDisplay)
      {
        BOOST_FOREACH (const std::string& msg, std::get<TupleMessages>(element))
          stream << msg << "\n";
      }
    }
  }

protected:
  Collection _collection;
};

//----------------------------------------------------------------------------
// @function void  FareCalculation::initialize()
// Description:    reset counter before process next passenger type.
// @param   trx    none
// @return  void
//----------------------------------------------------------------------------
void
FareCalculation::initialize(PricingTrx* trx,
                            const FareCalcConfig* fcConfig,
                            FareCalcCollector* fcCollector)
{
  TSE_ASSERT(trx != nullptr);
  TSE_ASSERT(fcConfig != nullptr);
  TSE_ASSERT(fcCollector != nullptr);

  _trx = trx;
  _fcConfig = FcConfig::create(trx, fcConfig);
  _fcCollector = fcCollector;

  _needXTLine = false;
  _dispSegmentFeeMsg = false;
  _totalBaseAmount = 0;
  _xtAmount = 0;
  _totalEquivAmount = 0;
  _totalFareAmount = 0;
  _tempWorkAmount = 0;
  _nbrDec = 0;
  _equivNoDec = 0;
  _fareNoDec = 0;
  _psgrCount = 0;
  _fareAmountLen = 0;

  _warning = (fcConfig->warningMessages() == FareCalcConsts::FC_YES ? "ATTN*" : "");

  _needNetRemitCalc = false;
  _warningEtktCat15 = false;
}

//----------------------------------------------------------------------------
// @function void  FareCalculation::resetCounters()
// Description:    reset counter before process next passenger type.
// @param   trx    none
// @return  void
//----------------------------------------------------------------------------
void
FareCalculation::resetCounters()
{
  _totalBaseAmount = 0;
  _xtAmount = 0;
  _totalEquivAmount = 0;
  _totalFareAmount = 0;
}

//----------------------------------------------------------------------------
// @function void  FareCalculation::process
// Description:    Build Fare Calculation display using data from
//                 input itinerary and FareCalcConfig table.
//                 This is the entry and exit point of this program.
//
//                 The following variables will be treated as TEMPORARY
//                 work / save area.  DO NOT USE these variables to store
//                 long term data.
//                   3.  _nbrDec
//
// @param   trx    PricingTrx
// @return  void
//----------------------------------------------------------------------------
void
FareCalculation::process()
{
  LOG4CXX_INFO(logger, "Entering FareCalculation Process() ...... ");

  PricingTrx& trx = *_trx;
  FareCalcCollector& fcCollector = *_fcCollector;

  _fareAmountLen = _fcConfig->baseTaxEquivTotalLength();

  if ((trx.getRequest()->diagnosticNumber() == Diagnostic854) &&
      (trx.getTrxType() != PricingTrx::MIP_TRX))
  {
    displayRuler();
  }

  _fareCalcDisp << std::fixed << std::right;

  if (trx.getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
  {
    if (trx.getRequest()->isWpNettRequested()) // special processing
    {
      wpNettPrefix(fcCollector);
    }
    else if (trx.altTrxType() == PricingTrx::WP)
    {
      _fareCalcDisp << "VT " << endl;
    }
    else
      _fareCalcDisp << "VD " << endl;
  }

  if (trx.getOptions()->isRecordQuote()) // WPRQ entry
  {
    _fareCalcDisp << "PRICE QUOTE RECORD RETAINED" << endl;
    _fareCalcDisp << "  " << endl;
  }

  const FareCalcCollector::CalcTotalsMap& calcTotalsMap = fcCollector.calcTotalsMap();

  bool firstPaxType = true;
  bool ttlProcessed = false;
  uint16_t multiTktItnOrderNum = 0;

  std::set<PaxType*, PaxType::InputOrder> inOrderPaxType(_trx->paxType().begin(),
                                                         _trx->paxType().end());

  std::vector<CalcTotals*> calcTotalsList;

  for (std::set<PaxType*, PaxType::InputOrder>::const_iterator pti = inOrderPaxType.begin(),
                                                               ptend = inOrderPaxType.end();
       pti != ptend;
       ++pti)
  {
    if (_fcConfig->wpPsgLineBreak() == FareCalcConsts::FC_YES && pti != inOrderPaxType.begin())
    {
      _fareCalcDisp << "  " << endl;
    }

    bool match_found = false;
    for (const auto& elem : calcTotalsMap)
    {
      CalcTotals& calcTotalOriginal = *elem.second;
      CalcTotals* cat35_calcTotals = nullptr;
      CalcTotals* calcTotalsTemp = nullptr;

      calcTotalsTemp = selectCalcTotals(trx, elem.second, _needNetRemitCalc);

      if (_needNetRemitCalc)
      {
        cat35_calcTotals = elem.second;
      }

      CalcTotals& calcTotals = *calcTotalsTemp;

      if (calcTotals.farePath->paxType() != *pti)
        continue;

      if (calcTotals.dispSegmentFeeMsg())
        _dispSegmentFeeMsg = true;

      calcTotals.fcConfig = _fcConfig->fcc();

      calcTotalsList.push_back(&calcTotals);

      getTotalFare(calcTotals);

      if (_needNetRemitCalc)
      {
        calcTotals.getFormattedFareCalcLine();
      }
      else
      {
        calcTotalOriginal.getFormattedFareCalcLine();
      }

      if (checkExistFarePath(trx, calcTotals))
      {
        match_found = true;
        _psgrCount++; // update passenger Calc Totals count
      }
      else
      {
        displayNoMatchMessage(calcTotals);
        continue;
      }

      if (trx.getRequest()->multiTicketActive())
      {
        multiTktItnOrderNum = calcTotals.farePath->itin()->getMultiTktItinOrderNum();
      }

      if (fcCollector.hasNoMatch())
      {
        if (firstPaxType)
        {
          displayPsgrInfo(calcTotals, false);
          displayFareLineHdr();
          firstPaxType = false;
        }
        calcTotals.wpaInfo.psgDetailRefNo = _psgrCount;
        displayFareLineInfo(calcTotals);
      }
      else
      {
        if ((trx.getTrxType() == PricingTrx::MIP_TRX) && firstPaxType)
          displayItinInfo(calcTotals);

        displayDtlFareCalc(calcTotals, *cat35_calcTotals, firstPaxType, ttlProcessed);

        if (_trx->isBRAll() || _trx->getRequest()->isBrandedFaresRequest())
        {
          fillLastTicketDay(calcTotals);
        }
      }

      if (trx.getRequest()->diagnosticNumber() == Diagnostic854)
      {
        diagTruePaxType(calcTotals);
        diagJourney(calcTotals);
      }

      displayNonIATARoundingTextMessage(calcTotals);
    }

    if ((_trx->getRequest()->ticketingAgent()->abacusUser() ||
         trx.getRequest()->ticketingAgent()->infiniUser()) &&
        !match_found)
    {
      displayWpaTrailerMessage();
    }
  }

  if (_fcConfig->itinDisplayInd() != FareCalcConsts::FC_YES)
  {
    if (!ttlProcessed)
    {
      ttlProcessed = processGrandTotalLine(trx, fcCollector);
    }

    if (_fcConfig->fareTaxTotalInd() == FareCalcConsts::HORIZONTAL_FARECALC1)
    {
      for (const auto elem : calcTotalsList)
      {
        CalcTotals& calcTotals = *elem;
        displayPsgFareCalc(trx, fcCollector, calcTotals);
        displayNoBrandsOffered(trx,calcTotals);
        diagBrands(calcTotals);
      }
    }
  }

  displayCorpIdTrailerMessage();

  displayCommandPricingAndVariousMessages();

  if (!ttlProcessed)
  {
    processGrandTotalLine(trx, fcCollector);
  }

  if ((trx.getRequest()->diagnosticNumber() == Diagnostic854) &&
      (trx.getTrxType() != PricingTrx::MIP_TRX))
  {
    displayRuler();
  }

  displayBrandWarning();

  if (trx.isPbbRequest() && TrxUtil::isRequestFromAS(trx))
    displayBrandingLegsInfo();

  DiagMonitor diag(*_trx, DiagnosticNone, Diagnostic854, Diagnostic855);
  diag << _fareCalcDisp.str() << '\n';

  if (trx.getRequest()->multiTicketActive())
  {
    MultiTicketUtil::setRespondMsg(multiTktItnOrderNum, _fareCalcDisp.str());
  }

  LOG4CXX_DEBUG(logger, _fareCalcDisp.str());

  LOG4CXX_INFO(logger, "Leaving FareCalculation ...... ");
}

void
FareCalculation::displayBrandingLegsInfo()
{
  const Itin* itin = _trx->itin().front();
  TSE_ASSERT(itin != NULL);
  if (itin->itinLegs().empty())
    return;

  _fareCalcDisp << "BRANDING FORMAT: ";
  if (BrandingUtil::getBrandRetrievalMode(*_trx) == BrandRetrievalMode::PER_FARE_COMPONENT)
  {
    _fareCalcDisp << "PER FARE COMPONENT";
  }
  else
  {
    _fareCalcDisp << IAIbfUtils::legInfoToString(itin->itinLegs());
  }
  _fareCalcDisp << std::endl;
}

// ----------------------------------------------------------------------------
void
FareCalculation::fillLastTicketDay(CalcTotals& calcTotals)
{
  DateTime latestTktDT;
  DateTime earliestTktDT;
  calcTotals.farePath->determineMostRestrTktDT(
      *_trx, latestTktDT, earliestTktDT, calcTotals.simultaneousResTkt);
  if (latestTktDT.isValid())
  {
    calcTotals.lastTicketDay = latestTktDT.dateToSqlString();
    calcTotals.lastTicketTime = latestTktDT.timeToString(HHMM, ":");
  }
}

// ----------------------------------------------------------------------------
// <PRE>
// @function bool FareCalculation::displayItineraryLine()
// Description:  Build Output Header fare calc display
// @param        trx, calcTotals, fcCollector
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::displayItineraryLine(PricingTrx& trx,
                                      FareCalcCollector& fcCollector,
                                      CalcTotals& calcTotals,
                                      bool firstPaxType)
{
  // Build for diag 854 or WP with itinerary display ONLY
  if (_fcConfig->itinDisplayInd() == FareCalcConsts::FC_YES)
  {
    if (_fcConfig->wpPsgTypDisplay() == FareCalcConsts::FC_YES)
    {
      displayPsgrInfo(calcTotals);
      // if (verifyBookingClass(calcTotals))
      //{
      //    displayVerifyBookingClassMsg();
      //}
    }

    if (_fcConfig->lastDayTicketDisplay() == FareCalcConsts::FC_TOP)
    {
      displayLastTicketDate(trx, calcTotals);
    }

    if (_fcConfig->itinHeaderTextInd() != FareCalcConsts::FC_ONE)
    {
      displayCxrFareBagHeaderLine();
    }
    displayLineOfFlight(trx, calcTotals, _fcConfig->wpConnectionInd());
  }
  else
  {
    if (firstPaxType)
    {
      if (_fcConfig->lastDayTicketDisplay() == FareCalcConsts::FC_TOP)
      {
        displayLastTicketDate(trx, calcTotals);
      }
    }
  }

  if (_fcConfig->fareTaxTotalInd() == FareCalcConsts::HORIZONTAL_FARECALC1)
  {
    displayBaseTaxHeader(trx, calcTotals);
  }
}

// ----------------------------------------------------------------------------
// <PRE>
// @function bool FareCalculation::vertical()
// Description:  Build Vertical Fare and Tax fare calc display
// @param        trx
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::vertical(PricingTrx& trx, CalcTotals& calcTotals)
{
  // Process base fare
  verticalProcessBaseAmount(calcTotals, trx.ticketingDate());
  _equivNoDec = _fareNoDec; // currency of Equiv/Tax amount in case no conversion

  if (trx.getOptions()->isMslRequest() &&
      calcTotals.farePath->isAdjustedSellingFarePath())
    _totalEquivAmount = calcTotals.equivFareAmount;

  // Process equivalent amount
  verticalProcessEquivAmount(trx);

  // Process Tax
  verticalProcessTaxAmount(trx, calcTotals);

  // Process Total Fare
  verticalProcessTotalAmount(trx);
}

// ----------------------------------------------------------------------------
// <PRE>
// @function bool FareCalculation::mix()
// Description:  Build Vertical Fare / Horizontal Tax fare calc display
// @param        trx
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::mix(PricingTrx& trx, CalcTotals& calcTotals)
{
  vertical(trx, calcTotals);
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::displayPsgrInfo()
// Description:   Build passenger info display
// @param         trx
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::displayPsgrInfo(const CalcTotals& calcTotals, bool detailFormat)
{
  if (_fcConfig->wpFareLinePsgType() == '2')
  {
    _fareCalcDisp << "INPUT PSGR TYPE  " << calcTotals.requestedPaxType;
  }
  else if (_fcConfig->wpFareLinePsgType() == '3')
  {
    int paxTypeNbr = FareCalcUtil::getPtcRefNo(*_trx, calcTotals.farePath->paxType());

    _fareCalcDisp << paxTypeNbr;

    switch (paxTypeNbr % 10)
    {
    case 1:
      _fareCalcDisp << "ST";
      break;
    case 2:
      _fareCalcDisp << "ND";
      break;
    case 3:
      _fareCalcDisp << "RD";
      break;
    default:
      _fareCalcDisp << "TH";
      break;
    }
    _fareCalcDisp << " PSGR TYPE";
  }
  else
  {
    if (_fcConfig->wpFareLinePsgType() == '2')
    {
      _fareCalcDisp << "INPUT PSGR TYPE  ";
    }
    else // _fcConfig->wpaFareLinePsgType() == '1'
    {
      _fareCalcDisp << "PSGR TYPE  ";
    }

    displayPsgrType(calcTotals);
  }

  if (_fcConfig->wpPrimePsgRefNo() == 'Y')
  {
    _fareCalcDisp << " - " << right << setw(2) << setfill('0')
                  << FareCalcUtil::getPtcRefNo(*_trx, calcTotals.farePath->paxType());
  }

  _fareCalcDisp << endl;
}

void
FareCalculation::displayPsgrType(const CalcTotals& calcTotals)
{
  if (_fcConfig->truePsgrTypeInd() == FareCalcConsts::FC_YES &&
      checkExistFarePath(*_trx, calcTotals))
  {
    _fareCalcDisp << calcTotals.truePaxType;
  }
  else
  {
    _fareCalcDisp << calcTotals.requestedPaxType;
  }
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::displayCxrFareBagHeaderLine()
// Description:   Build carrier, fare basis NVA, NVB line
// @param         trx
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::displayCxrFareBagHeaderLine()
{
  _fareCalcDisp << "     CXR RES DATE  FARE BASIS      ";

  if (_fcConfig->itinHeaderTextInd() == FareCalcConsts::FC_TWO)
  {
    _fareCalcDisp << "NVB   NVA    BG";
  }
  else
  {
    _fareCalcDisp << "NVB   NVA";
  }

  _fareCalcDisp << endl;
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::displayCxrBkgCodeFareBasis()
// Description:   Build Carrier, Booking Code and Fare Basis Code
// @param         diagnostic, TravelSeg and AirSeg
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::displayCxrBkgCodeFareBasis(PricingTrx& trx,
                                            CalcTotals& calcTotals,
                                            AirSeg* airSeg,
                                            std::string& fareBasisCode)
{
  std::string fbPart;
  std::string tktPart;

  if (_fcConfig->fareBasisTktDesLng() == FareCalcConsts::FC_THREE)
  {
    fbPart.clear();
    tktPart.clear();
    int16_t i = 0;
    int16_t j = fareBasisCode.size();
    for (; i < j; ++i)
    {
      if (fareBasisCode.compare(i, 1, "/") == 0) // if string = a slash "/"
      {
        fbPart += tktPart; // move to fbPart
        tktPart.clear();
      }
      else
      {
        tktPart += fareBasisCode[i];
      }
    }
    if (fbPart.empty()) // if no Tkt designator
    {
      fbPart += tktPart;
      tktPart.clear();
    }
  }

  _fareCalcDisp << left << setfill(' ');
  _fareCalcDisp << ' ';
  _fareCalcDisp << setw(4) << MCPCarrierUtil::swapToPseudo(_trx, airSeg->carrier());

  uint16_t i = airSeg->pnrSegment() - 1;
  {
    std::vector<TravelSeg*>::const_iterator tsI =
        std::find(calcTotals.farePath->itin()->travelSeg().begin(),
                  calcTotals.farePath->itin()->travelSeg().end(),
                  airSeg);
    if (LIKELY(tsI != calcTotals.farePath->itin()->travelSeg().end()))
    {
      i = std::distance(calcTotals.farePath->itin()->travelSeg().begin(), tsI);
    }
  }

  if (i < calcTotals.bookingCodeRebook.size() && !calcTotals.bookingCodeRebook[i].empty())
  {
    _fareCalcDisp << setw(4) << calcTotals.bookingCodeRebook[i][0];
  }
  else
    _fareCalcDisp << setw(4) << airSeg->getBookingCode();

  _fareCalcDisp << setw(6);

  if (airSeg->segmentType() == Open)
    _fareCalcDisp << "OPEN";
  else
    _fareCalcDisp << airSeg->departureDT().dateToString(DDMMM, "");

  if (LIKELY(tktPart.empty()))
  {
    _fareCalcDisp << setw(15) << fareBasisCode << ' ';
  }
  else
  {
    _fareCalcDisp << setw(10) << fbPart << ' ';
    if (tktPart.size() <= 4)
      _fareCalcDisp << setw(4) << tktPart << ' ';
    else
      _fareCalcDisp << setw(4) << tktPart.substr(0, 4) << ' ';
  }
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::displayNVAandNVBDate()
// Description:   display applicable NVA and NVB date
// @param         diagnostic, TravelSeg and AirSeg
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::displayNVAandNVBDate(PricingTrx& trx,
                                      CalcTotals& calcTotals,
                                      AirSeg* airSeg,
                                      TravelSeg* tvlS)
{
  const FarePath* farePath = calcTotals.farePath;

  if (farePath == nullptr)
    return;

  int16_t segNumber = farePath->itin()->segmentOrder(tvlS);

  // Search for a not valid before date for this travel segment
  _fareCalcDisp << setw(6) << setfill(' ');
  const DateTime& nvbDate = calcTotals.tvlSegNVB[segNumber];
  if (nvbDate.isValid())
    _fareCalcDisp << nvbDate.dateToString(DDMMM, "");
  else
    _fareCalcDisp << ' ';

  // Search for a not valid after date for this travel segment
  _fareCalcDisp << setw(6) << setfill(' ');
  const DateTime& nvaDate = calcTotals.tvlSegNVA[segNumber];
  if (LIKELY(nvaDate.isValid()))
    _fareCalcDisp << nvaDate.dateToString(DDMMM, "");
  else
    _fareCalcDisp << ' ';

} // end of displayNVAandNVBDate ()

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::displayBaggageAllowance()
// Description:   display number of bags allow per passenger
// @param         travelseg, inPsgrType
// @return        none
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::displayBaggageAllowance(const CalcTotals& calcTotals, const AirSeg* airSeg)
{
  if (_fcConfig->itinHeaderTextInd() == FareCalcConsts::FC_THREE)
    return;

  _fareCalcDisp << setw(3) << setfill(' ');

  const auto i = calcTotals.farePath->baggageAllowance().find(airSeg);

  if (i != calcTotals.farePath->baggageAllowance().end())
    _fareCalcDisp << i->second.c_str();
  else
    _fareCalcDisp << ' ';
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::horizontal()
// Description:   Build horizontal Fare Calculation display
// @param         trx
//                _totalBaseAmount
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::horizontal(PricingTrx& trx, FareCalcCollector& fcCollector, CalcTotals& calcTotals)
{
  int16_t nbrTaxBoxes = 0;
  _fareCalcDisp.setf(std::ios::right, std::ios::adjustfield);
  _fareCalcDisp.setf(std::ios::fixed, std::ios::floatfield);
  _fareCalcDisp.precision(_fareNoDec);

  if (calcTotals.farePath->paxType()->number() < 10)
  {
    _fareCalcDisp << ' ';
  }
  _fareCalcDisp << calcTotals.farePath->paxType()->number() << '-';

  horizontalProcessBaseAmount(calcTotals, trx.ticketingDate());
  _equivNoDec = _fareNoDec; // currency of Equiv/Tax amount in case no conversion

  if (trx.getOptions()->currencyOverride().empty() ||
      trx.getOptions()->currencyOverride() == _fareCurrencyCode)
  {
    _fareCalcDisp << setw(15) << setfill(' ') << ' ';
  }
  else
  {
    horizontalProcessEquivAmount(calcTotals);
  }

  horizontalProcessTaxAmount(trx, calcTotals, nbrTaxBoxes);
  _fareCalcDisp.precision(_equivNoDec);

  // get total fare amount for one passenger
  _totalEquivAmount != 0 ? _totalFareAmount = _totalEquivAmount + _xtAmount
                         : _totalFareAmount = _totalBaseAmount + _xtAmount;

  horizontalProcessTotalAmount(calcTotals);
  horizontalProcessTaxBreakDown(trx, calcTotals);

} // enf of horizontalFareTaxTotal()

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::displayRuler()
// Description:   Build ruler display
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::displayRuler()
{
  _fareCalcDisp << "         1         2         3         4         5         6" << endl;
  _fareCalcDisp << "123456789012345678901234567890123456789012345678901234567890123" << endl;
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::getEquivalentAmount()
// Description:   Convert base fare to equivalent amount
// @param         trx, inAmount, outAmount, toCurrency
// @return        convertAmount
// </PRE>
// ----------------------------------------------------------------------------

MoneyAmount&
FareCalculation::getEquivalentAmount(PricingTrx& trx,
                                     const MoneyAmount& inAmount,
                                     MoneyAmount& outAmount,
                                     const std::string& fromCurrency,
                                     const std::string& toCurrency,
                                     CurrencyConversionRequest::ApplicationType applType)
{
  CurrencyConversionFacade ccFacade;
  outAmount = 0;

  const Money sourceMoney(inAmount, fromCurrency);
  Money targetMoney(toCurrency);

  if (!ccFacade.convert(targetMoney, sourceMoney, trx, false, applType))
  {
    outAmount = inAmount;
    return (outAmount);
  }
  outAmount = targetMoney.value();

  LOG4CXX_DEBUG(logger,
                " GET BASE AMT: " << inAmount << " OUT TOTAL EQUIV AMT: " << outAmount
                                  << " FROM CURR CODE: " << fromCurrency
                                  << " TO CURR CODE: " << toCurrency);
  return (outAmount);
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::processTax()
// Description:   Process Tax data
// @param         trx
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::horizontalProcessTaxBreakDown(PricingTrx& trx, CalcTotals& calcTotals)
{
  bool first_Tax = true;
  _fareCalcDisp.precision(calcTotals.equivNoDec);

  TSE_ASSERT(_fcConfig != nullptr);
  _fareAmountLen = _fcConfig->baseTaxEquivTotalLength();

  int taxBreakDownItemCnt = trx.getRequest()->taxOverride().size() + calcTotals.taxRecords().size();

  // There is only one tax, do not display tax break down
  if (taxBreakDownItemCnt == 1)
    return;

  // const Itin* itin = calcTotals.farePath->itin();
  //
  // if (!itin ||
  //    (itin->taxResponse().empty() &&
  //     (trx.getRequest()->taxOverride().empty() ||
  //      trx.getRequest()->taxOverride().size() == 1)))
  //    return;

  FareCalc::Margin xtMargin(_fareCalcDisp, 0);

  if (taxBreakDownItemCnt > 1)
  {
    _fareCalcDisp << "    XT";
    xtMargin.setMargin("      ");
  }

  // Display Tax Override
  std::vector<TaxOverride*>::const_iterator taxOverrideI = trx.getRequest()->taxOverride().begin();
  for (; taxOverrideI != trx.getRequest()->taxOverride().end(); ++taxOverrideI)
  {
    if (first_Tax)
    {
      first_Tax = false;
    }

    _fareCalcDisp << setw(_fareAmountLen) << setfill(' ') << (*taxOverrideI)->taxAmt()
                  << (*taxOverrideI)->taxCode().substr(0, 2) << ' ';
  }

  // Display Tax
  for (std::vector<TaxRecord*>::const_iterator taxRecordI = calcTotals.taxRecords().begin(),
                                               iend = calcTotals.taxRecords().end();
       taxRecordI != iend;
       ++taxRecordI)
  {
    _fareCalcDisp.precision((*taxRecordI)->taxNoDec());

    if (UNLIKELY(((*taxRecordI)->getTaxAmount() < EPSILON) && (*taxRecordI)->isTaxFeeExempt() == false))
      continue;

    if (first_Tax)
    {
      if (!trx.getRequest()->taxOverride().empty())
      {
        std::vector<TaxOverride*>::const_iterator taxOverrideI =
            trx.getRequest()->taxOverride().begin();

        if ((*taxRecordI)->taxCode() == (*taxOverrideI)->taxCode())
        {
          if (trx.getRequest()->taxOverride().size() == 1 && calcTotals.getTaxExemptCodes().empty())
            return;
        }
      }
      else
      {
        if (calcTotals.taxRecords().size() == 1 && calcTotals.getTaxExemptCodes().empty())
          return;
      }

      first_Tax = false;

      if (trx.getRequest()->isExemptSpecificTaxes())
      {
        if (_fcConfig->taxExemptionInd() == FareCalcConsts::FC_ONE)
          _fareCalcDisp << "          TE ";
      }
    }

    if (UNLIKELY((*taxRecordI)->isTaxFeeExempt()))
    {
      _fareCalcDisp << setw(_fareAmountLen) << setfill(' ') << right;
      if (_fcConfig->taxExemptionInd() == FareCalcConsts::FC_TWO)
      {
        _fareCalcDisp << "EXEMPT" << (*taxRecordI)->taxCode().substr(0, 2) << ' ';
      }
      else
      {
        _fareCalcDisp << "EX" << (*taxRecordI)->taxCode().substr(0, 2) << ' ';
      }
    }
    else
    {
      if (LIKELY((*taxRecordI)->taxCurrencyCode() == trx.getOptions()->currencyOverride() ||
          trx.getOptions()->currencyOverride().empty()))
      {
        _fareCalcDisp << setw(_fareAmountLen) << setfill(' ') << (*taxRecordI)->getTaxAmount()
                      << (*taxRecordI)->taxCode().substr(0, 2) << ' ';
      }
      else
      {
        _fareCalcDisp << setw(_fareAmountLen) << setfill(' ')
                      << getEquivalentAmount(trx,
                                             (*taxRecordI)->getTaxAmount(),
                                             _tempWorkAmount,
                                             (*taxRecordI)->taxCurrencyCode(),
                                             trx.getOptions()->currencyOverride(),
                                             CurrencyConversionRequest::TAXES)
                      << (*taxRecordI)->taxCode().substr(0, 2) << ' ';
      }
    }
  }

  // Check Tax Exempts should be in TaxResponse::taxRecord() already
  if ((_fcConfig->taxExemptionInd() != FareCalcConsts::FC_ONE) &&
      ((_fcConfig->taxPlacementInd() != FareCalcConsts::FC_THREE) ||
       ((!trx.getRequest()->isExemptSpecificTaxes() && !trx.getRequest()->isExemptAllTaxes()) ||
        !trx.getRequest()->taxOverride().empty())))
  {
    std::set<TaxCode>::const_iterator taxCodeI = calcTotals.getTaxExemptCodes().begin();

    for (; taxCodeI != calcTotals.getTaxExemptCodes().end(); taxCodeI++)
    {
      if (first_Tax)
      {
        first_Tax = false;
      }

      _fareCalcDisp << setw(_fareAmountLen) << setfill(' ') << right;
      if (_fcConfig->taxExemptionInd() == FareCalcConsts::FC_TWO)
        _fareCalcDisp << "EXEMPT" << (*taxCodeI).substr(0, 2) << ' ';
      else if (_fcConfig->taxExemptionInd() == FareCalcConsts::FC_THREE)
        _fareCalcDisp << "EX" << (*taxCodeI).substr(0, 2) << ' ';
    }
  }

  if (taxBreakDownItemCnt > 1)
    _fareCalcDisp << endl;
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::getTotalXT()
// Description:   Total the XT taxes per passenger
// @param         trx
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::getTotalXT(PricingTrx& trx,
                            bool& xtInd,
                            int16_t& nbrTaxBoxes,
                            CalcTotals& calcTotals)
{
  nbrTaxBoxes = 0;
  xtInd = getTaxOverride(trx, nbrTaxBoxes);

  if (calcTotals.farePath->itin()->getTaxResponses().empty())
    return;

  if (const TaxResponse* taxResponse = TaxResponse::findFor(calcTotals.farePath))
  {
    std::vector<TaxRecord*>::const_iterator taxRecordI = taxResponse->taxRecordVector().begin();
    if (taxResponse->taxRecordVector().empty())
    {
      return;
    }

    if (taxResponse->taxRecordVector().size() > 1 || !calcTotals.getTaxExemptCodes().empty())
      xtInd = true;

    if (!xtInd)
    {
      if (!trx.getRequest()->taxOverride().empty())
      {
        std::vector<TaxOverride*>::const_iterator taxOverrideI =
            trx.getRequest()->taxOverride().begin();

        if ((*taxRecordI)->taxCode() != (*taxOverrideI)->taxCode())
          xtInd = true;
      }
    }

    for (; taxRecordI != taxResponse->taxRecordVector().end(); taxRecordI++)
    {
      if (((*taxRecordI)->getTaxAmount() < EPSILON) &&
          !(*taxRecordI)->isTaxFeeExempt()) // Skip zero amount Tax.
      {
        continue;
      }

      if (LIKELY((*taxRecordI)->taxCurrencyCode() == trx.getOptions()->currencyOverride() ||
          trx.getOptions()->currencyOverride().empty()))
      {
        _xtAmount += (*taxRecordI)->getTaxAmount();
      }
      else
      {
        _xtAmount += getEquivalentAmount(trx,
                                         (*taxRecordI)->getTaxAmount(),
                                         _tempWorkAmount,
                                         (*taxRecordI)->taxCurrencyCode(),
                                         trx.getOptions()->currencyOverride());
      }

      nbrTaxBoxes++;

      LOG4CXX_DEBUG(logger,
                    " TAXCURR CODE: " << (*taxRecordI)->taxCurrencyCode()
                                      << " TAX CODE: " << (*taxRecordI)->taxCode()[0] << " "
                                      << (*taxRecordI)->taxCode()[1]
                                      << " EQV CODE: " << trx.getOptions()->currencyOverride()
                                      << " TAX AMT: " << (*taxRecordI)->getTaxAmount()
                                      << " TAX NODEC: " << (*taxRecordI)->taxNoDec()
                                      << " EQUIV AMT: " << _tempWorkAmount);
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::processTotalLine()
// Description:   Display total fare and tax amount by passenger
// @param         totalFare, totalTax
// @return
// </PRE>
// ----------------------------------------------------------------------------

bool
FareCalculation::processGrandTotalLine(PricingTrx& trx, FareCalcCollector& fcCollector)
{
  // We don't display the TTL line if there is no-match
  if (fcCollector.hasNoMatch())
    return true;

  // For vertical fare/tax, only display the TTL line if there is more than
  // one pax, or single-pax and more than one passenger.
  if (_fcConfig->fareTaxTotalInd() != FareCalcConsts::HORIZONTAL_FARECALC1 &&
      fcCollector.calcTotalsMap().size() == 1 &&
      fcCollector.calcTotalsMap().begin()->first->paxType()->number() == 1)
  {
    return true;
  }

  if (trx.getOptions()->currencyOverride() == _fareCurrencyCode ||
      trx.getOptions()->currencyOverride().empty())
  {
    getCurrencyNoDec(trx, trx.getOptions()->currencyOverride());
    _fareNoDec = _nbrDec;
    _equivNoDec = _nbrDec;
  }
  else
  {
    getCurrencyNoDec(trx, _fareCurrencyCode);
    _fareNoDec = _nbrDec;
    getCurrencyNoDec(trx, trx.getOptions()->currencyOverride());
    _equivNoDec = _nbrDec;
  }

  CurrencyCode curCode;
  CurrencyCode baseCurCode;
  CurrencyNoDec noDec;

  _fareCalcDisp << setfill(' ') << setw(16);

  MoneyAmount totalBaseFare = 0;
  if (!fcCollector.isMixedBaseFareCurrency())
  {
    totalBaseFare = fcCollector.getBaseFareTotal(trx, baseCurCode, noDec, _needNetRemitCalc);
    _fareCalcDisp << setprecision(_fareNoDec);
    _fareCalcDisp << totalBaseFare;
  }
  else
  {
    _fareCalcDisp << ' ';
  }

  MoneyAmount totalEquivAmount = 0;
  if (!fcCollector.isMixedEquivFareAmountCurrency())
  {
    totalEquivAmount = fcCollector.getEquivFareAmountTotal(trx, curCode, noDec, _needNetRemitCalc);
  }
  _fareCalcDisp << setprecision(_equivNoDec);

  _fareCalcDisp << setw(15);
  if (totalEquivAmount == 0 || curCode == baseCurCode)
  {
    _fareCalcDisp << ' ';
    totalEquivAmount = 0;
  }
  else
  {
    _fareCalcDisp << totalEquivAmount;
  }

  MoneyAmount totalTax = fcCollector.getTaxTotal(trx, curCode, noDec, _needNetRemitCalc);
  _fareCalcDisp << setprecision(_equivNoDec);
  _fareCalcDisp << setw(11);
  if (totalTax == 0)
    _fareCalcDisp << ' ';
  else
    _fareCalcDisp << totalTax;

  MoneyAmount grandTotal = totalTax;
  if (totalEquivAmount != 0)
    grandTotal += totalEquivAmount;
  else if ((totalBaseFare != 0) && (curCode == baseCurCode))
    grandTotal += totalBaseFare;
  else
    grandTotal += fcCollector.getEquivFareAmountTotal(trx, curCode, noDec, _needNetRemitCalc);

  if (grandTotal != 0)
  {
    _fareCalcDisp << setw(18) << grandTotal << "TTL";
  }

  _fareCalcDisp << endl;

  // Check grand total amount length
  Money amount(grandTotal, curCode);
  int16_t i = amount.toString(_trx->ticketingDate()).size() - curCode.size();
  checkFareAmountLength(i, _fareAmountLen);

  return true;
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::getTotalFare()
// Description:   Total fares from Pricing Units
// @param         trx, _totalBaseAmount,
//                none
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::getTotalFare(CalcTotals& calcTotals)

{
  _totalBaseAmount = calcTotals.convertedBaseFare;
  _fareNoDec = calcTotals.convertedBaseFareNoDec;
  _fareCurrencyCode = calcTotals.convertedBaseFareCurrencyCode;
}

std::string
FareCalculation::getNetRemitFbc(PricingTrx& trx,
                                const FareUsage* fareUsage,
                                const TravelSeg* travelSeg)
{
  if (fareUsage && TrxUtil::optimusNetRemitEnabled(trx) && !fareUsage->netRemitPscResults().empty())
  {
    const TravelSeg* tvlSeg = TravelSegUtil::lastAirSeg(fareUsage->travelSeg());
    if (tvlSeg != nullptr && tvlSeg->specifiedFbc().empty())
    {
      FareUsage::TktNetRemitPscResultVec::const_iterator nrResults =
          findNetRemitPscResults(*fareUsage, travelSeg);
      if (nrResults != fareUsage->netRemitPscResults().end() && nrResults->_tfdpscSeqNumber)
      {
        if (!nrResults->_tfdpscSeqNumber->uniqueFareBasis().empty())
        {
          const std::string uniqueFareBasis =
              nrResults->_tfdpscSeqNumber->uniqueFareBasis().c_str();
          return fareUsage->paxTypeFare()->createFareBasis(trx, uniqueFareBasis);
        }
      }
    }
  }

  return std::string("");
}

FareUsage::TktNetRemitPscResultVec::const_iterator
FareCalculation::findNetRemitPscResults(const FareUsage& fareUsage, const TravelSeg* tvlSeg)
{
  FareUsage::TktNetRemitPscResultVec::const_iterator nrResults =
      fareUsage.netRemitPscResults().begin();

  for (; nrResults != fareUsage.netRemitPscResults().end() &&
             nrResults->_startTravelSeg->segmentOrder() <= tvlSeg->segmentOrder();
       nrResults++)
  {
    if (nrResults->_endTravelSeg->segmentOrder() >= tvlSeg->segmentOrder())
      return nrResults;
  }

  return fareUsage.netRemitPscResults().end();
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::displayPsgrFareBasisLine()
// Description:   Display passenger and fare basis information
// @param         psgrType, _fareCalcDisp
// @return
// </PRE>
// ----------------------------------------------------------------------------
void
FareCalculation::displayPsgrFareBasisLine(PricingTrx& trx, CalcTotals& calcTotals)

{
  if (LIKELY(dynamic_cast<NoPNRPricingTrx*>(&trx) == nullptr || _fcConfig->fcPsgTypDisplay() == 'Y'))
  {
    displayPsgrType(calcTotals);
    _fareCalcDisp << '-' << setw(2) << setfill('0') << calcTotals.farePath->paxType()->number()
                  << ' ';
  }

  std::string prevFareBasis;

  std::map<uint16_t, TravelSeg*, std::less<uint16_t> >::const_iterator i =
      calcTotals.travelSegs.begin();

  FareCalc::Margin fcMargin(_fareCalcDisp, 7);

  for (; i != calcTotals.travelSegs.end(); ++i)
  {
    AirSeg* travelSeg = dynamic_cast<AirSeg*>((*i).second);
    if (travelSeg == nullptr)
      continue;

    // Get differenial FBC
    std::string fareBasis = calcTotals.getDifferentialFbc(travelSeg);
    if (fareBasis.empty())
      fareBasis = getFareBasisCode(calcTotals, travelSeg);

    if (!fareBasis.empty() && fareBasis != prevFareBasis)
    {
      prevFareBasis = fareBasis;

      SpanishFamilyDiscountDesignator appender =
          spanishFamilyDiscountDesignatorBuilder(*_trx,
                                                 calcTotals,
                                                 FareCalcConsts::MAX_FARE_BASIS_SIZE);
      appender(fareBasis);

      _fareCalcDisp << ' ' << fareBasis;
    }
  }

  if (trx.getRequest()->getBrandedFareSize() > 1)
  {
    _fareCalcDisp << " BRAND ID: " << trx.getRequest()->brandId(calcTotals.farePath->brandIndex());
  }

  _fareCalcDisp << endl;
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::getCurrencyNoDec()
// Description:   get number of decimal for base fare currency
// @param         trx, _nbrDec
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::getCurrencyNoDec(PricingTrx& trx, const std::string inCurrencyCode)
{
  const tse::Currency* currency = nullptr;
  DataHandle dataHandle;
  currency = dataHandle.getCurrency( inCurrencyCode );

  if (LIKELY(currency != nullptr))
  {
    _nbrDec = currency->noDec();
  }
}

void
FareCalculation::displayLastTicketDate(PricingTrx& trx, CalcTotals& calcTotals)
{
  DateTime latestTktDT;
  DateTime earliestTktDT;

  if (!calcTotals.farePath || !calcTotals.farePath->itin())
    return;

  const Itin& itin = *calcTotals.farePath->itin();

  calcTotals.farePath->determineMostRestrTktDT(
      trx, latestTktDT, earliestTktDT, _fcCollector->simultaneousResTkt());

  if (latestTktDT.isValid())
  {
    if (itin.errResponseCode() == ErrorResponseException::NO_ERROR && !itin.farePath().empty() &&
        _fcCollector->lastTicketDay().empty())
    {
      bool needDisplay = true;

      if (trx.excTrxType() == PricingTrx::AR_EXC_TRX ||
          trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
      {
        BaseExchangeTrx* beTrx = static_cast<BaseExchangeTrx*>(&trx);

        if (latestTktDT <= beTrx->currentTicketingDT())
          needDisplay = false;
      }

      if (needDisplay)
      {
        if (trx.altTrxType() == PricingTrx::WP)
        {
          _fcCollector->lastTicketDay() = latestTktDT.dateToSqlString();
          _fcCollector->lastTicketTime() = latestTktDT.timeToString(HHMM, ":");
        }
        else
        {
          calcTotals.lastTicketDay = latestTktDT.dateToSqlString();
          calcTotals.lastTicketTime = latestTktDT.timeToString(HHMM, ":");
        }
      }
    }
  }
  else
  {
    LOG4CXX_DEBUG(logger, " LastTicketDate was invalid ... return to caller !! ");
    return;
  }

  if (_fcConfig->lastDayTicketOutput() == FareCalcConsts::FC_ONE)
  {
    _fareCalcDisp << itin.travelSeg()[0]->departureDT().dateToString(DDMMM, "") << ' ';
    _fareCalcDisp << "DEPARTURE DATE-----LAST DAY TO PURCHASE ";
    _fareCalcDisp << latestTktDT.dateToString(DDMMM, "");
  }
  else
  {
    _fareCalcDisp << "TKT/TL";
    _fareCalcDisp << latestTktDT.dateToString(DDMMMYY, "");
  }

  displayTimeStamp(trx, latestTktDT); //  add Time Stamp

  _fareCalcDisp << endl;
}

void
FareCalculation::displayTimeStamp(PricingTrx& trx, const DateTime& dt)
{
  bool isAxessUser = (trx.getRequest() && trx.getRequest()->ticketingAgent() &&
                      trx.getRequest()->ticketingAgent()->axessUser());

  if (!isAxessUser)
    _fareCalcDisp << "/" << dt.timeToString(HHMM, "");
}

void
FareCalculation::displayBaseTaxHeaderMultiVersion(PricingTrx& trx,
                                                  const CalcTotals& calcTotals,
                                                  bool activeNewVersion)
{
  if (activeNewVersion)
  {
    _fareCalcDisp << "       "
                  << "BASE FARE      ";
    if (calcTotals.convertedBaseFareCurrencyCode != calcTotals.equivCurrencyCode)
    {
      _fareCalcDisp << "EQUIV AMT  TAXES/FEES/CHARGES    TOTAL" << endl;
    }
    else
    {
      _fareCalcDisp << "           TAXES/FEES/CHARGES    TOTAL" << endl;
    }
  }
  else
  {
    _fareCalcDisp << "       "
                  << "BASE FARE      ";
    if (calcTotals.convertedBaseFareCurrencyCode != calcTotals.equivCurrencyCode)
    {
      _fareCalcDisp << "EQUIV AMT      TAXES             TOTAL" << endl;
    }
    else
    {
      _fareCalcDisp << "               TAXES             TOTAL" << endl;
    }
  }
}
// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::displayBaseTaxHeader()
// Description:   build base fare, tax and total header line
// @param         trx
// @return
// </PRE>
// ----------------------------------------------------------------------------
void
FareCalculation::displayBaseTaxHeader(PricingTrx& trx, const CalcTotals& calcTotals)
{
  if (_fcConfig->itinDisplayInd() == FareCalcConsts::FC_YES || _psgrCount == 1 ||
      (trx.altTrxType() == PricingTrx::WPA && FareCalcUtil::isOneSolutionPerPaxType(&trx)))
  {
    const Agent* agent = trx.getRequest()->ticketingAgent();
    if (agent->tvlAgencyPCC().empty() || agent->axessUser())
    { // AS
      displayBaseTaxHeaderMultiVersion(trx, calcTotals, TrxUtil::isTaxesNewHeaderASActive(trx));
    }
    else
    { // TN
      displayBaseTaxHeaderMultiVersion(trx, calcTotals, true);
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::buildTaxExempt()
// Description:   build Tax Exempt output
// @param         trx
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::buildTaxExempt(PricingTrx& trx)
{
  _fareCalcDisp << setw(13) << setfill(' ');

  switch (_fcConfig->taxExemptionInd())
  {
  case FareCalcConsts::FC_ONE:
    _fareCalcDisp << "TE";
    break;

  case FareCalcConsts::FC_TWO:
    _fareCalcDisp << "EXEMPT";
    break;

  case FareCalcConsts::FC_THREE:
    _fareCalcDisp << "EX";
    break;

  default:
    _fareCalcDisp << ' ';
    break;
  }
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::horizontalProcessBaseAmount()
// Description:   build Tax Exempt output
// @param         calcTotals, ticketingDate
// @return
// </PRE>
// ----------------------------------------------------------------------------
void
FareCalculation::horizontalProcessBaseAmount(CalcTotals& calcTotals, const DateTime& ticketingDate)
{
  Money amount(calcTotals.convertedBaseFare, calcTotals.convertedBaseFareCurrencyCode);

  if (!calcTotals.roundBaseFare)
  {
    const std::string totalBaseAmtStr =
        CurrencyUtil::toString(amount.value(), amount.noDec(ticketingDate));

    _fareCalcDisp << fixed << setw(13) << setfill(' ')
                  << setprecision(calcTotals.convertedBaseFareNoDec) << totalBaseAmtStr;
  }
  else
  {
    _fareCalcDisp << fixed << setw(13) << setfill(' ')
                  << setprecision(calcTotals.convertedBaseFareNoDec)
                  << amount.toString(ticketingDate);
  }
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::horizontalProcessEquivAmount()
// Description:   build Tax Exempt output
// @param         trx
// @return
// </PRE>
// ----------------------------------------------------------------------------
void
FareCalculation::horizontalProcessEquivAmount(CalcTotals& calcTotals)
{
  _fareCalcDisp << setw(15) << std::right << setfill(' ');
  if (calcTotals.convertedBaseFareCurrencyCode == calcTotals.equivCurrencyCode)
  {
    _fareCalcDisp << ' ';
  }
  else
  {
    _totalEquivAmount = calcTotals.equivFareAmount;
    Money amount(calcTotals.equivFareAmount, calcTotals.equivCurrencyCode);
    std::string equivAmountStr = amount.toString(_trx->ticketingDate());
    _fareCalcDisp << equivAmountStr;
    int16_t i = equivAmountStr.size() - calcTotals.equivCurrencyCode.size();
    checkFareAmountLength(i, _fareAmountLen);
  }
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::horizontalProcessTaxAmount()
// Description:   build Tax Exempt output
// @param         trx
// @return
// </PRE>
// ----------------------------------------------------------------------------
void
FareCalculation::horizontalProcessTaxAmount(PricingTrx& trx,
                                            CalcTotals& calcTotals,
                                            int16_t& nbrTaxBoxes)
{
  if (trx.getRequest()->isExemptAllTaxes())
  {
    buildTaxExempt(trx);
  }
  else
  {
    MoneyAmount taxAmount = calcTotals.taxAmount();
    if (taxAmount < EPSILON)
    {
      if (trx.getRequest()->isExemptSpecificTaxes())
        _fareCalcDisp << setw(13) << setfill(' ') << "TE";
      else
        _fareCalcDisp << setw(13) << setfill(' ') << ' ';
    }
    else
    {
      nbrTaxBoxes = trx.getRequest()->taxOverride().size() + calcTotals.taxRecords().size();

      _fareCalcDisp << setw(11) << setfill(' ') << std::right
                    << std::setprecision(calcTotals.taxNoDec()) << calcTotals.taxAmount();

      if (nbrTaxBoxes == 1)
      {
        if (!trx.getRequest()->taxOverride().empty())
          _fareCalcDisp << trx.getRequest()->taxOverride().front()->taxCode().substr(0, 2);
        else if (!calcTotals.taxRecords().empty())
          _fareCalcDisp << calcTotals.taxRecords().front()->taxCode().substr(0, 2);
        else // shouldn't get here
          _fareCalcDisp << "XT";
      }
      else
      {
        _fareCalcDisp << "XT";
      }
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::horizontalProcessTotalAmount()
// Description:   build Tax Exempt output
// @param         trx
// @return
// </PRE>
// ----------------------------------------------------------------------------
void
FareCalculation::horizontalProcessTotalAmount(CalcTotals& calcTotals)
{
  MoneyAmount totalFareAmount;
  if (calcTotals.equivFareAmount == 0 &&
      calcTotals.convertedBaseFareCurrencyCode != calcTotals.equivCurrencyCode)
    totalFareAmount = calcTotals.taxAmount();
  else if (calcTotals.convertedBaseFareCurrencyCode == calcTotals.equivCurrencyCode)
    totalFareAmount = calcTotals.convertedBaseFare + calcTotals.taxAmount();
  else
    totalFareAmount = calcTotals.equivFareAmount + calcTotals.taxAmount();

  Money amount(totalFareAmount, calcTotals.equivCurrencyCode);
  std::string totalAmtStr = amount.toString(_trx->ticketingDate());

  _fareCalcDisp << setw(16) << setfill(' ') << std::right << totalAmtStr;
  int16_t i = totalAmtStr.size() - calcTotals.equivCurrencyCode.size();
  checkFareAmountLength(i, _fareAmountLen);

  displayPsgrType(calcTotals);

  _fareCalcDisp << endl;

} // end horizontalProcessTotalAmount()

void
FareCalculation::displayPsgrAndFareCalcLines(PricingTrx& trx,
                                             FareCalcCollector& fcCollector,
                                             CalcTotals& calcTotals)
{
  displayPsgrFareBasisLine(trx, calcTotals);

  const std::string& fareCalcLine = calcTotals.getFormattedFareCalcLine();
  if (LIKELY(!fareCalcLine.empty()))
    _fareCalcDisp << fareCalcLine << endl;
}

// ----------------------------------------------------------------------------
// <PRE>
// @function bool FareCalculation::verticalProcessBaseAmount()
// Description:  Build Vertical Fare and Tax fare calc display
// @param        calcTotals, ticketingDate
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::verticalProcessBaseAmount(CalcTotals& calcTotals, const DateTime& ticketingDate)
{
  Money amount(_totalBaseAmount, _fareCurrencyCode);
  int16_t i = amount.toString(ticketingDate).size() - _fareCurrencyCode.size();
  int16_t length = checkFareAmountLength(i, _fareAmountLen);

  _fareCalcDisp << "FARE  " << _fareCurrencyCode;

  if (!calcTotals.roundBaseFare)
  {
    const std::string totalBaseAmtStr =
        CurrencyUtil::toString(_totalBaseAmount, amount.noDec(ticketingDate));

    _fareCalcDisp << fixed << right << setprecision(_fareNoDec) << setw(length)
                  << setfill(' ') << totalBaseAmtStr;
  }
  else
  {
    _fareCalcDisp << fixed << right << setprecision(_fareNoDec) << setw(length)
                  << setfill(' ') << _totalBaseAmount;
  }

  if (_fcConfig->fareTaxTotalInd() == FareCalcConsts::VERTICAL_FARECALC2)
    _fareCalcDisp << endl;
  else
    _fareCalcDisp << ' ';
}

// ----------------------------------------------------------------------------
// <PRE>
// @function bool FareCalculation::verticalProcessEquivAmount()
// Description:  Build Vertical Fare and Tax fare calc display
// @param        trx
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::verticalProcessEquivAmount(PricingTrx& trx)
{
  if (trx.getOptions()->currencyOverride() == _fareCurrencyCode ||
      trx.getOptions()->currencyOverride().empty())
  {
    _fareCalcDisp << ' ' << endl; // leave a blank line as specified in requirement
  }
  else
  {
    bool alreadyCalculated = trx.getOptions()->isMslRequest() &&
                             _totalEquivAmount != 0.0;
    if (!alreadyCalculated)
      getEquivalentAmount(trx,
                          _totalBaseAmount,
                          _totalEquivAmount,
                          _fareCurrencyCode,
                          trx.getOptions()->currencyOverride());

    getCurrencyNoDec(trx, trx.getOptions()->currencyOverride());
    _equivNoDec = _nbrDec;
    Money amount(_totalEquivAmount, trx.getOptions()->currencyOverride());
    int16_t i =
        amount.toString(trx.ticketingDate()).size() - trx.getOptions()->currencyOverride().size();
    int16_t length = checkFareAmountLength(i, _fareAmountLen);

    _fareCalcDisp.precision(_equivNoDec);
    _fareCalcDisp << "EQUIV " << trx.getOptions()->currencyOverride();
    _fareCalcDisp << right << setw(length) << setfill(' ') << _totalEquivAmount << endl;
  }
}

// ----------------------------------------------------------------------------
// <PRE>
// @function bool FareCalculation::verticalProcessTaxAmount()
// Description:  Build Vertical Fare and Tax fare calc display
// @param        trx
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::verticalProcessTaxAmount(PricingTrx& trx, CalcTotals& calcTotals)
{
  _needXTLine = false;
  bool xtInd = false;
  int16_t nbrTaxBoxes = 0;
  int16_t fcNbrTaxBoxes = 0;
  int16_t loopCount = 0;
  MoneyAmount tempXTAmount = 0;
  getTotalXT(trx, xtInd, nbrTaxBoxes, calcTotals);
  fcNbrTaxBoxes = _fcConfig->noofTaxBoxes() - '0'; // get nbr tax box from database

  if (fcNbrTaxBoxes < nbrTaxBoxes)
  {
    fcNbrTaxBoxes--; // leave the last slot for XT tax
  }

  const Itin* itin = calcTotals.farePath->itin();

  // Display Tax Override
  std::vector<TaxOverride*>::const_iterator taxOverrideI = trx.getRequest()->taxOverride().begin();

  FareCalc::Margin taxMargin(_fareCalcDisp, "TAX  ");

  for (; taxOverrideI != trx.getRequest()->taxOverride().end(); taxOverrideI++)
  {
    if (fcNbrTaxBoxes >= nbrTaxBoxes || loopCount < fcNbrTaxBoxes)
    {
      loopCount++;

      _fareCalcDisp << ' ' << trx.getOptions()->currencyOverride();

      _fareCalcDisp << setw(_fareAmountLen) << setfill(' ') << (*taxOverrideI)->taxAmt()
                    << (*taxOverrideI)->taxCode().substr(0, 2);

      if (_fcConfig->fareTaxTotalInd() == FareCalcConsts::VERTICAL_FARECALC2)
      {
        _fareCalcDisp << endl;
      }
    }
    else
    {
      tempXTAmount += (*taxOverrideI)->taxAmt();
    }
  }

  // Loop through Tax Record and build tax display
  std::vector<TaxResponse*>::const_iterator taxResponseI = itin->getTaxResponses().begin();
  for (; taxResponseI != itin->getTaxResponses().end(); taxResponseI++)
  {
    if ((*taxResponseI)->farePath() != calcTotals.farePath)
    {
      continue;
    }
    std::vector<TaxRecord*>::iterator taxRecordI = (*taxResponseI)->taxRecordVector().begin();
    if ((*taxResponseI)->taxRecordVector().empty())
    {
      return;
    }

    for (; taxRecordI != (*taxResponseI)->taxRecordVector().end(); ++taxRecordI)
    {
      if ((*taxRecordI)->getTaxAmount() == 0 && !(*taxRecordI)->isTaxFeeExempt())
        continue;

      if (fcNbrTaxBoxes >= nbrTaxBoxes || loopCount < fcNbrTaxBoxes)
      {
        loopCount++; // update loop count

        _fareCalcDisp << ' ' << trx.getOptions()->currencyOverride();

        if ((*taxRecordI)->isTaxFeeExempt())
        {
          if (_fcConfig->taxExemptionInd() == FareCalcConsts::FC_TWO)
          {
            _fareCalcDisp << " EXEMPT " << (*taxRecordI)->taxCode().substr(0, 2);
          }
          else
          {
            _fareCalcDisp << " EX" << (*taxRecordI)->taxCode().substr(0, 2);
          }
        }
        else
        {
          if ((*taxRecordI)->taxCurrencyCode() == trx.getOptions()->currencyOverride() ||
              trx.getOptions()->currencyOverride().empty())
          {
            _fareCalcDisp << setw(_fareAmountLen) << setfill(' ') << (*taxRecordI)->getTaxAmount()
                          << (*taxRecordI)->taxCode().substr(0, 2);
          }
          else
          {
            _fareCalcDisp << ' ' << setw(_fareAmountLen) << setfill(' ')
                          << getEquivalentAmount(trx,
                                                 (*taxRecordI)->getTaxAmount(),
                                                 _tempWorkAmount,
                                                 (*taxRecordI)->taxCurrencyCode(),
                                                 trx.getOptions()->currencyOverride())
                          << (*taxRecordI)->taxCode().substr(0, 2);
          }
        }

        if (_fcConfig->fareTaxTotalInd() == FareCalcConsts::VERTICAL_FARECALC2)
        {
          _fareCalcDisp << endl;
        }
      }
      else
      {
        tempXTAmount += (*taxRecordI)->getTaxAmount();
      }

    } // end of for ( ; taxRecordI != ...)

    if (_fcConfig->taxExemptionInd() == FareCalcConsts::FC_ONE)
      break;

  } // end for loop( ; taxResponseI != trx.taxResponse().end()

  // display XT tax box which contained several taxes
  if (nbrTaxBoxes > fcNbrTaxBoxes)
  {
    _needXTLine = true;

    _fareCalcDisp << ' ' << trx.getOptions()->currencyOverride();

    _fareCalcDisp << setw(_fareAmountLen) << setfill(' ') << tempXTAmount << "XT" << endl;
  }
  else
  {
    if (_fcConfig->fareTaxTotalInd() != FareCalcConsts::VERTICAL_FARECALC2)
    {
      if (fcNbrTaxBoxes >= nbrTaxBoxes || loopCount < fcNbrTaxBoxes)
      {
        _fareCalcDisp << endl;
      }
    }
  }

} // end  verticalProcessTaxAmount()

// ----------------------------------------------------------------------------
// <PRE>
// @function bool FareCalculation::verticalProcessTotalAmount()
// Description:  Display total amount
// @param        trx
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::verticalProcessTotalAmount(PricingTrx& trx)
{

  if (_totalEquivAmount != 0)
    _totalFareAmount = _totalEquivAmount + _xtAmount;
  else
  {
    if (trx.getOptions()->currencyOverride() == _fareCurrencyCode ||
        trx.getOptions()->currencyOverride().empty())
      _totalFareAmount = _totalBaseAmount + _xtAmount;
    else
      _totalFareAmount = _xtAmount;
  }
   Money amount(_totalFareAmount, trx.getOptions()->currencyOverride());
   int16_t i = amount.toString().size() - trx.getOptions()->currencyOverride().size();
   int16_t length = checkFareAmountLength (i, _fareAmountLen);
  _fareCalcDisp << "TOTAL " << trx.getOptions()->currencyOverride() << setw(length)
                << setfill(' ') << _totalFareAmount << endl;
}

// ----------------------------------------------------------------------------
// <PRE>
// @function bool FareCalculation:: verticalProcessXTLineverticalProcessXTLine()
// Description:  Display tax break down for XT line
// @param        trx
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::verticalProcessXTLine(PricingTrx& trx, CalcTotals& calcTotals)
{
  int16_t fcNbrTaxBoxes = _fcConfig->noofTaxBoxes() - '0';
  int16_t loopCount = 0;
  std::string amountString;
  MoneyAmount tempAmount;

  if (!_needXTLine)
    return;

  FareCalc::Margin txMargin(_fareCalcDisp, FareCalcConsts::TAX_CODE_XT.data());

  fcNbrTaxBoxes--;

  if (_needXTLine && !trx.getRequest()->taxOverride().empty())
  {
    // Display Tax Override
    std::vector<TaxOverride*>::const_iterator taxOverrideI =
        trx.getRequest()->taxOverride().begin();

    // Skip tax override that were not part of XT tax
    for (; loopCount < fcNbrTaxBoxes; ++loopCount)
    {
      if (taxOverrideI == trx.getRequest()->taxOverride().end())
        break;

      taxOverrideI++;
    }

    for (; taxOverrideI != trx.getRequest()->taxOverride().end(); taxOverrideI++)
    {
      // Skip zero amount Tax.
      if ((*taxOverrideI)->taxAmt() > EPSILON)
      {
        _fareCalcDisp << ' ' << trx.getOptions()->currencyOverride() << (*taxOverrideI)->taxAmt()
                      << (*taxOverrideI)->taxCode().substr(0, 2);
      }
    }
  }

  std::vector<TaxResponse*>::const_iterator taxResponseI = trx.taxResponse().begin();

  ostringstream tempXTLine;
  for (; taxResponseI != trx.taxResponse().end(); taxResponseI++)
  {
    // if ((*taxResponseI)->farePath()->paxType() != calcTotals.farePath->paxType())
    if ((*taxResponseI)->farePath() != calcTotals.farePath)
      continue;

    std::vector<TaxRecord*>::iterator taxRecordI = (*taxResponseI)->taxRecordVector().begin();
    // Skip tax items that were not part of XT tax
    for (; loopCount < fcNbrTaxBoxes; ++loopCount)
    {
      if (taxRecordI == (*taxResponseI)->taxRecordVector().end())
        break;

      taxRecordI++;

      //APO-36498 - skip 0 tax amount
      while ((*taxRecordI)->getTaxAmount() < EPSILON &&
             !(*taxRecordI)->isTaxFeeExempt() &&
             taxRecordI != (*taxResponseI)->taxRecordVector().end() )
        taxRecordI++;
    }

    for (; taxRecordI != (*taxResponseI)->taxRecordVector().end(); ++taxRecordI)
    {
      tempXTLine.str("");

      if ((*taxRecordI)->isTaxFeeExempt())
      {
        if (_fcConfig->taxExemptionInd() == FareCalcConsts::FC_TWO)
        {
          tempXTLine << " EXEMPT " << (*taxRecordI)->taxCode().substr(0, 2);
        }
        else
        {
          tempXTLine << " EX" << (*taxRecordI)->taxCode().substr(0, 2);
        }

        _fareCalcDisp << tempXTLine.str();
      }
      else
      {
        // Skip zero amount Tax.
        if ((*taxRecordI)->getTaxAmount() < EPSILON)
          continue;

        tempXTLine << ' ' << trx.getOptions()->currencyOverride();

        if ((*taxRecordI)->taxCurrencyCode() == trx.getOptions()->currencyOverride() ||
            trx.getOptions()->currencyOverride().empty())
        {
          convertAmount2String((*taxRecordI)->getTaxAmount(), _equivNoDec, amountString);
        }
        else
        {
          tempAmount = getEquivalentAmount(trx,
                                           (*taxRecordI)->getTaxAmount(),
                                           _tempWorkAmount,
                                           (*taxRecordI)->taxCurrencyCode(),
                                           trx.getOptions()->currencyOverride());
          convertAmount2String(tempAmount, _equivNoDec, amountString);
        }
        tempXTLine << amountString << (*taxRecordI)->taxCode().substr(0, 2);

        if ((*taxRecordI)->taxCode() == FareCalcConsts::TAX_CODE_ZP)
        {
          if (_fcConfig->zpAmountDisplayInd() == FareCalcConsts::FC_YES)
          {
            const std::vector<std::string>& zpTaxInfo = calcTotals.zpTaxInfo();
            std::copy(
                zpTaxInfo.begin(), zpTaxInfo.end(), std::ostream_iterator<std::string>(tempXTLine));
          }
        }
        else if ((*taxRecordI)->taxCode() == FareCalcConsts::TAX_CODE_XF)
        {
          const std::vector<std::string>& xfTaxInfo = calcTotals.xfTaxInfo();
          if (!xfTaxInfo.empty())
          {
            std::copy(xfTaxInfo.begin() + 1,
                      xfTaxInfo.end(),
                      std::ostream_iterator<std::string>(tempXTLine));
          }
        }

        _fareCalcDisp << tempXTLine.str();
      }

    } // end of for ( ; taxRecordI != ...)

  } // end for loop( ; taxResponseI != trx.taxResponse().end()

  _fareCalcDisp << endl;

  if ((_fcConfig->taxPlacementInd() == FareCalcConsts::FC_THREE) &&
      (trx.getRequest()->isExemptSpecificTaxes() || trx.getRequest()->isExemptAllTaxes()) &&
      trx.getRequest()->taxOverride().empty())
  {
    // Tax Exempts should be in TaxResponse::taxRecord() already
    return;
  }

  if (calcTotals.getTaxExemptCodes().size() > 0)
  {
    for (const auto& taxCode : calcTotals.getTaxExemptCodes())
    {
      tempXTLine.str("");

      if (_fcConfig->taxExemptionInd() == FareCalcConsts::FC_TWO)
        tempXTLine << " EXEMPT ";
      else
        tempXTLine << " EX";

      tempXTLine << taxCode.substr(0, 2);

      _fareCalcDisp << tempXTLine.str();
    }

    _fareCalcDisp << endl;
  }

} // end of verticalProcessXTLine ()

// ----------------------------------------------------------------------------
// <PRE>
// @function bool FareCalculation::verticalProcessRateLine()
// Description:  Display conversion rate
// @param        trx, _fcConfig
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::verticalProcessRateLine(PricingTrx& trx, const CalcTotals& calcTotals)
{
  if (_fcConfig->displayBSR() == FareCalcConsts::FC_NO &&
      (_fcConfig->fareTaxTotalInd() == FareCalcConsts::HORIZONTAL_FARECALC1 ||
       _totalEquivAmount == 0))
    return;

  if (calcTotals.equivCurrencyCode == calcTotals.convertedBaseFareCurrencyCode)
    return;

  if (TrxUtil::isIcerActivated(trx))
  {
    std::string formattedDouble =
        FareCalcUtil::formatExchangeRate(calcTotals.bsrRate1, calcTotals.bsrRate1NoDec);

    _fareCalcDisp << "RATE USED " << '1' << _fareCurrencyCode << '-'
                  << formattedDouble.substr(0, formattedDouble.size()) << trx.getOptions()->currencyOverride()
                  << endl;
    return;
  }

  // Per Darrin Wei No Truncation of ROE. PL#11553
  std::string formattedDouble;
  FareCalcUtil::doubleToStringTruncate(calcTotals.bsrRate1, formattedDouble, 15);

  uint16_t strSize = 14;

  for (; strSize > 0; strSize--)
  {
    if (formattedDouble[strSize] != '0')
      break;
  }
  strSize++;

  if (calcTotals.bsrRate2 == 0.0) // single rate conversion
  {
    _fareCalcDisp << "RATE USED " << '1' << _fareCurrencyCode << '-'
                  << formattedDouble.substr(0, strSize) << trx.getOptions()->currencyOverride()
                  << endl;
  }
  else // double rates conversion
  {
    _fareCalcDisp << "RATE USED " << '1' << _fareCurrencyCode << '-'
                  << formattedDouble.substr(0, strSize) << calcTotals.interCurrencyCode;

    FareCalcUtil::doubleToStringTruncate(calcTotals.bsrRate2, formattedDouble, 15);

    strSize = 14;

    for (; strSize > 0; strSize--)
    {
      if (formattedDouble[strSize] != '0')
        break;
    }
    strSize++;

    _fareCalcDisp << ' ' << formattedDouble.substr(0, strSize)
                  << trx.getOptions()->currencyOverride() << endl;
  }
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::displayLOFBoardOffPoint()
// Description:   Build line of flight output display
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::displayLineOfFlight(PricingTrx& trx, CalcTotals& calcTotals, char fcConnectionInd)
{
  bool firstTime = true;
  LocCode orgLoc;
  LocCode destLoc;
  LocCode previousDestCity;

  bool isWqTrx = (dynamic_cast<NoPNRPricingTrx*>(&trx) != nullptr);

  DataHandle dataHandle(trx.ticketingDate());

  // Loop through travelSeg
  std::map<uint16_t, TravelSeg*, std::less<uint16_t> >::const_iterator i =
      calcTotals.travelSegs.begin();
  for (; i != calcTotals.travelSegs.end(); ++i)
  {
    TravelSeg* travelSeg = (*i).second;

    const FareUsage* fareUsage = nullptr;

    std::map<const TravelSeg*, const FareUsage*>::iterator iter =
        calcTotals.fareUsages.find(travelSeg);

    if (LIKELY(calcTotals.fareUsages.find(travelSeg) != calcTotals.fareUsages.end()))
      fareUsage = iter->second;

    CarrierCode carrier;
    if (LIKELY(calcTotals.fareUsages.find(travelSeg) != calcTotals.fareUsages.end()))
      carrier = calcTotals.fareUsages[travelSeg]->paxTypeFare()->carrier();

    orgLoc = FcDispFareUsage::getDisplayLoc(*_fcConfig->fcc(),
                                            calcTotals.farePath->itin()->geoTravelType(),
                                            travelSeg->boardMultiCity(),
                                            travelSeg->origAirport(),
                                            carrier,
                                            travelSeg->departureDT(),
                                            isWqTrx);

    destLoc = FcDispFareUsage::getDisplayLoc(*_fcConfig->fcc(),
                                             calcTotals.farePath->itin()->geoTravelType(),
                                             travelSeg->offMultiCity(),
                                             travelSeg->destAirport(),
                                             carrier,
                                             travelSeg->departureDT(),
                                             isWqTrx);

    if ((orgLoc == destLoc) && (dynamic_cast<const AirSeg*>(travelSeg) == nullptr))
      continue;

    if (firstTime)
    {
      firstTime = false;
      _fareCalcDisp << ' ' << orgLoc << endl;
    }
    else
    {
      if (previousDestCity != orgLoc)
      {
        _fareCalcDisp << ' ' << orgLoc << "     S U R F A C E" << endl;
      }
    }
    previousDestCity = destLoc;

    if (calcTotals.getDispConnectionInd(trx, travelSeg, fcConnectionInd))
    {
      _fareCalcDisp << 'X';
    }
    else
    {
      _fareCalcDisp << ' ';
    }

    _fareCalcDisp << destLoc;
    AirSeg* airSeg = dynamic_cast<AirSeg*>(travelSeg);

    if (airSeg == nullptr)
    {
      _fareCalcDisp << "     S U R F A C E" << endl;
    }
    else
    {
      std::string fareBasisCode = calcTotals.getDifferentialFbc(travelSeg);

      if (fareBasisCode.empty())
      {
        fareBasisCode = getNetRemitFbc(trx, fareUsage, travelSeg);
        if (LIKELY(fareBasisCode.empty()))
        {
          fareBasisCode = getFareBasisCode(calcTotals, travelSeg);
        }
      }

      displayCxrBkgCodeFareBasis(trx, calcTotals, airSeg, fareBasisCode);
      if (LIKELY(travelSeg->segmentType() == Air || travelSeg->segmentType() == Open))
      {
        displayNVAandNVBDate(trx, calcTotals, airSeg, travelSeg);
        displayBaggageAllowance(calcTotals, airSeg);
      }
      _fareCalcDisp << endl;
    }
  } // end for  ( ; i != calcTotals.travelSegs.end(); ++i)

} // end of displayLineOfFlight (PricingTrx& trx, ....)

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::convertAmount2String()
// Description:   convert amount to string for display
// @param         inAmount, inNbrDec, outString
// @return
// </PRE>
// ----------------------------------------------------------------------------
void
FareCalculation::convertAmount2String(const MoneyAmount& inAmount,
                                      const int16_t& inNbrDec,
                                      std::string& outString,
                                      const int16_t& outNbrDigit,
                                      const bool checkAmount)
{
  outString.clear();
  ostringstream numb;
  numb.setf(ios::fixed, ios::floatfield);
  numb.precision(inNbrDec);
  numb << inAmount;
  outString += numb.str();
  int16_t i = outString.size() - 1;
  if (inNbrDec > outNbrDigit)
  {
    for (; i >= outNbrDigit; i--)
    {
      LOG4CXX_DEBUG(logger,
                    " **OUTSTRING:  " << outString << "  OutStr_Sub: " << outString.substr(i, 1)
                                      << " Value I: " << i);
      if (outString.compare(i, 1, "0") == 0 ||
          outString.compare(i, 1, ".") == 0) // if string = 0  or .
        outString.erase(i, 1);
      else
        break;
    }
  }

} // end of  convertAmount2String ()

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::getTaxOverride()
// Description:   Display taxOverride data in pricing response.
// @param         trx
// @param         bool display
// @return        output response
// </PRE>
// ----------------------------------------------------------------------------

bool
FareCalculation::getTaxOverride(PricingTrx& trx, int16_t& nbrTaxBoxes)

{
  std::vector<TaxOverride*>::const_iterator taxI = trx.getRequest()->taxOverride().begin();

  for (; taxI != trx.getRequest()->taxOverride().end(); taxI++)
  {
    _xtAmount += (*taxI)->taxAmt();
    nbrTaxBoxes++;
  }

  if (trx.getRequest()->taxOverride().size() > 1)
    return true;

  return false;
}

void
FareCalculation::displayEndorsements(PricingTrx& pricingTrx, const FarePath* farePath)
{
  if (farePath == nullptr)
    return;

  bool forTicketing = pricingTrx.getRequest()->isTicketEntry();

  TicketingEndorsement::TicketEndoLines messages;
  TicketingEndorsement tktEndo;

  if (!fallback::fallbackEndorsementsRefactoring(&pricingTrx))
  {
    // this refactoring strongly depends on endorsementExpansion
    // both need to be OFF (value 'Y') to return to Endorse Cutter Limited
    tktEndo.collectEndorsements(pricingTrx, *farePath, messages, EndorseCutter());
  }
  else
  {
    std::shared_ptr<EndorseCutter> endoCut;
    if (!fallback::endorsementExpansion(_trx))
      endoCut = std::make_shared<EndorseCutterUnlimited>();
    else
    if (forTicketing || _fcConfig->endorsements() != FareCalcConsts::FC_YES)
      endoCut = std::make_shared<EndorseCutterLimited>(
          TicketingEndorsement::maxEndorsementMsgLen(pricingTrx));
    else
      endoCut = std::make_shared<EndorseCutterUnlimited>();

    tktEndo.collectEndorsements(pricingTrx, *farePath, messages, *endoCut);
  }

  if (forTicketing)
    tktEndo.sortLinesByPrio(pricingTrx, *farePath, messages);
  else
    tktEndo.sortLinesByPnr(pricingTrx, messages);

  display_endorsements dispEndorse(pricingTrx, *this, forTicketing);
  for (TicketEndorseLine* line : messages)
    dispEndorse(*line);
}

void
display_endorsements::
operator()(const TicketEndorseLine& endorseLine)
{
  if (!_forTicketing && _fareCalculation._fcConfig->endorsements() == FareCalcConsts::FC_YES)
  {
    FareCalc::Margin margin(_fareCalculation._fareCalcDisp, "ENDOS*");

    if (_fareCalculation._fareCalcDisp.lineLength() > 0)
      _fareCalculation._fareCalcDisp << '\n';

    if (endorseLine.segmentOrders.size() > 0)
      _fareCalculation._fareCalcDisp << "SEG";

    for (std::set<int16_t>::const_iterator iter = endorseLine.segmentOrders.begin();
         iter != endorseLine.segmentOrders.end();
         ++iter)
    {
      if (*iter == ARUNK_PNR_SEGMENT_ORDER)
        continue;

      if (iter != endorseLine.segmentOrders.begin())
        _fareCalculation._fareCalcDisp << '/';

      _fareCalculation._fareCalcDisp << *iter;
    }
    _fareCalculation._fareCalcDisp << '*';

    _fareCalculation._fareCalcDisp.displayMessage(endorseLine.endorseMessage);
    _fareCalculation._fareCalcDisp << std::endl;
  }
  else
  {
    _fareCalculation._fareCalcDisp.displayMessage(endorseLine.endorseMessage);
    _fareCalculation._fareCalcDisp << std::endl;
  }
}

void
FareCalculation::displayWarnings(const CalcTotals& calcTotals)
{
  if (calcTotals.farePath == nullptr)
    return;

  DisplayWarningsForFC displayWarnings(*_trx,
                                       _fareCalcDisp,
                                       _fcConfig,
                                       _warningFopMsgs,
                                       _warningEtktCat15,
                                       calcTotals,
                                       _fcCollector);
  displayWarnings.display();
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation::checkFareAmountLength()
// Description:   Check length of the amount and issue error message if exceed max length.
// @param         inAmountLen and maxAmountLen
// @return        void
// </PRE>
// ----------------------------------------------------------------------------

int16_t
FareCalculation::checkFareAmountLength(const int16_t& inAmountLen, const int16_t& maxAmountLen)
{
  if (UNLIKELY(_trx->getRequest()->diagArgType().size() > 0)) // Example Q/*854/1 type entry
    return maxAmountLen;

  if (UNLIKELY(maxAmountLen > 0 && maxAmountLen < inAmountLen))
  {
    if (TrxUtil::isRequestFromAS(*_trx) && _trx->billing()->partitionID() != "AA") // Hosted CXR, except 'AA'
    {
      const uint16_t aSBaseTaxEquivTotalLength = TrxUtil::getASBaseTaxEquivTotalLength(*_trx);
      if(aSBaseTaxEquivTotalLength >= inAmountLen)
        return aSBaseTaxEquivTotalLength;
    }

    LOG4CXX_FATAL(logger,
                  "Amount " << inAmountLen << " Exceeds Max Allowable Length in FareCalcConfig "
                            << maxAmountLen);
    throw ErrorResponseException(ErrorResponseException::EXCEED_LENGTH_UNABLE_TO_CALCULATE_FARE);
  }
  return maxAmountLen;
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void FareCalculation:: displayCurrencyTextMessage()
// Description:   Display currency text message
// @param         itin
// @return        void
// </PRE>
// ----------------------------------------------------------------------------

void
FareCalculation::displayCurrencyTextMessage(const Itin& itin)
{
  if (LIKELY(!_fcCollector->calcTotalsMap().empty()))
  {
    CalcTotals& calcTotals = *_fcCollector->calcTotalsMap().rbegin()->second;
    for (auto& elem : itin.csTextMessages())
    {
      calcTotals.fcMessage.push_back(FcMessage(FcMessage::WARNING, 0, elem));
    }
  }

  if (itin.csTextMessages().size() > 0)
  {
    std::string warning = "\n" + _warning;

    _fareCalcDisp << warning;

    std::copy(itin.csTextMessages().begin(),
              itin.csTextMessages().end(),
              std::ostream_iterator<std::string>(_fareCalcDisp, warning.data()));
  }
}

// ----------------------------------------------------------------------------
// <PRE>
// @function bool FareCalculation:: checkExistFarePath()
// Description:   if XO qulifier and no FarePath for paxType in calcTotals
//                do not build horizontal line in Pricing Response
// @param         trx, calcTotals
// @return        bool
// </PRE>
// ----------------------------------------------------------------------------

bool
FareCalculation::checkExistFarePath(PricingTrx& trx, const CalcTotals& calcTotals)
{
  // Don't count the dummy fare path
  if (calcTotals.farePath == nullptr || calcTotals.farePath->processed() == false)
  {
    return false;
  }

  const std::vector<FarePath*>& fps = calcTotals.farePath->itin()->farePath();
  std::vector<FarePath*>::const_iterator i = fps.begin();
  for (; i != fps.end(); ++i)
  {
    FarePath& farePath = **i;
    if (calcTotals.farePath->paxType() == farePath.paxType())
    {
      return true;
    }
  }

  return false;
}

void
FareCalculation::addToCalcTotals(CalcTotals* calcTotals, FcMessageCollection& collection)
{
  if (calcTotals)
    collection.transform(calcTotals->fcMessage);
  else
  {
    for (FareCalcCollector::CalcTotalsMap::const_reference pair : _fcCollector->calcTotalsMap())
    {
      if (LIKELY(pair.second))
        collection.transform(pair.second->fcMessage);
    }
  }
}

void
FareCalculation::displayCommandPricingAndVariousMessages(CalcTotals* calcTotals)
{
  if (!calcTotals && _fcCollector->calcTotalsMap().empty())
    return;

  FcMessageCollection collection;
  displayIndustryAndGoverningWarning(collection);
  displayBookingCodeWarning(collection);

  // Display Warning Message for WPQ//DA148.00 or WPQ//DP50 type entries fix PL 12192
  PricingRequest* request = _trx->getRequest();

  bool isDAEntry;
  bool isDPEntry;
  if (TrxUtil::newDiscountLogic(*_trx))
  {
    isDAEntry = request && request->isDAEntryNew();
    isDPEntry = request && request->isDPEntryNew();
  }
  else
  {
    isDAEntry = request && request->isDAEntry();
    isDPEntry = request && request->isDPEntry();
  }

  if (request && (isDAEntry || isDPEntry))
  {
    collection.add(FcMessage::WARNING, "MANUAL DISCOUNT APPLIED/VERIFY ALL RULES");
    collection.add(_warning + "MANUAL DISCOUNT APPLIED/VERIFY ALL RULES");
  }

  if (_dispSegmentFeeMsg)
  {
    collection.add(FcMessage::SEGMENT_FEE, "TOTAL INCLUDES CARRIER IMPOSED SURCHARGES");
    collection.add(_warning + "TOTAL INCLUDES CARRIER IMPOSED SURCHARGES");
  }

  addToCalcTotals(calcTotals, collection);
  collection.transform(_fareCalcDisp);

  const Itin* itin = nullptr;
  const FareCalcCollector::CalcTotalsMap& calcTotalsMap = _fcCollector->calcTotalsMap();
  if (calcTotals != nullptr)
  {
    itin = calcTotals->farePath->itin();
  }
  else if (LIKELY(!calcTotalsMap.empty() && calcTotalsMap.begin()->second &&
           calcTotalsMap.begin()->second->farePath))
  {
    itin = calcTotalsMap.begin()->second->farePath->itin();
  }

  displayCurrencyTextMessage(itin != nullptr ? *itin : *_trx->itin().front());

  if (calcTotals)
    displayNonIATARoundingTextMessage(*calcTotals);

  displaySpanishLargeFamilyDiscountMessage(*_trx, itin != nullptr ? *itin : *_trx->itin().front());

  const FarePath* fp_ptr = nullptr;
  if (LIKELY(!calcTotalsMap.empty()))
    fp_ptr = calcTotalsMap.begin()->first;
  if (LIKELY(fp_ptr))
    displayCat05BookingDTOverrideMessage(fp_ptr);
}

void
FareCalculation::displayIndustryAndGoverningWarning(FcMessageCollection& collection)
{
  // Display warning message for WPC-YY and WPC-xx
  PricingRequest* request = _trx->getRequest();
  if (request &&
      (request->isIndustryFareOverrideEntry() || request->isGoverningCarrierOverrideEntry()))
  {
    FareCalc::FcStream stream;

    stream << std::endl << "**" << std::endl;
    stream << "PRICED USING ";

    if (request->isIndustryFareOverrideEntry())
      stream << "YY / INDUSTRY FARES / ";

    if (request->isGoverningCarrierOverrideEntry())
    {
      std::map<int16_t, CarrierCode>::const_iterator govCxrI =
          request->governingCarrierOverrides().begin();

      CarrierCode curCarrierCode = MCPCarrierUtil::swapToPseudo(_trx, govCxrI->second);
      stream << curCarrierCode;

      govCxrI++;
      for (; govCxrI != request->governingCarrierOverrides().end(); ++govCxrI)
      {
        CarrierCode govCarried = MCPCarrierUtil::swapToPseudo(_trx, govCxrI->second);
        if (govCarried != curCarrierCode)
        {
          curCarrierCode = govCarried;
          stream << " / " << curCarrierCode;
        }
      }
      stream << " FARE OVERRIDE" << std::endl;
    }
    else
    {
      stream << "OVERRIDE" << std::endl;
    }

    stream << "FARE NOT GUARANTEED IF TICKETED" << std::endl;
    stream << "**" << std::endl;

    std::vector<std::string> warnings;
    stream.split(warnings);
    collection.add(warnings);
  }
}

void
FareCalculation::displayBookingCodeWarning(FcMessageCollection& collection)
{
  // Display warning message for WPQB-X - Booking Code Override
  if (_trx->getOptions()->bookingCodeOverride())
  {
    std::vector<std::string> warnings;
    warnings.push_back("  ");
    warnings.push_back("**");
    warnings.push_back("PRICED USING BOOKING CODE OVERRIDE");
    warnings.push_back("FARE NOT GUARANTEED IF TICKETED");
    warnings.push_back("**");
    collection.add(warnings);
  }
}

void
FareCalculation::displayFareTax(PricingTrx& trx,
                                FareCalcCollector& fcCollector,
                                CalcTotals& calcTotals)
{
  getTotalFare(calcTotals);
  _fareAmountLen = _fcConfig->baseTaxEquivTotalLength();

  switch (_fcConfig->fareTaxTotalInd())
  {
  case FareCalcConsts::VERTICAL_FARECALC2:
    vertical(trx, calcTotals);
    break;

  case FareCalcConsts::MIX_FARECALC3:
    mix(trx, calcTotals);
    break;

  case FareCalcConsts::HORIZONTAL_FARECALC1:
  default:
    horizontal(trx, fcCollector, calcTotals);
    break;
  }
}

void
FareCalculation::displayPsgFareCalc(PricingTrx& trx,
                                    FareCalcCollector& fcCollector,
                                    CalcTotals& calcTotals)
{
  if (!checkExistFarePath(trx, calcTotals)) // if XO or XC qulifier and no FarePath foR PAXTYPE
  {
    displayNoMatchMessage(calcTotals);
    return;
  }

  displayPsgrAndFareCalcLines(trx, fcCollector, calcTotals);

  if (_fcConfig->fareTaxTotalInd() != FareCalcConsts::HORIZONTAL_FARECALC1)
  {
    verticalProcessXTLine(trx, calcTotals);
  }

  displayPtcMessages(trx, calcTotals);

  calcTotals.warningFopMsgs = _warningFopMsgs;

  if (trx.getRequest()->ticketingAgent()->axessUser() &&
      (trx.getRequest()->isWpNettRequested() || trx.getRequest()->isWpSelRequested()))
  {
    displayAxessCat35Lines(trx, calcTotals);
  }
}

void
FareCalculation::diagTruePaxType(const CalcTotals& calcTotals)
{
  LOG4CXX_DEBUG(logger, "processing Diagnostic 854/FCTPT");
  typedef std::map<std::string, std::string>::iterator DiagParamMapVecI;
  DiagnosticTypes diagType = _trx->diagnostic().diagnosticType();
  if (!_trx->diagnostic().isActive() || (diagType != Diagnostic854))
    return;

  DiagParamMapVecI endI = _trx->diagnostic().diagParamMap().end();
  DiagParamMapVecI beginI = _trx->diagnostic().diagParamMap().find(Diagnostic::DISPLAY_DETAIL);
  if (beginI == endI)
    return;
  size_t len = ((*beginI).second).size();
  if (len == 0 || ((*beginI).second).substr(0, len) != "TPT")
    return;

  const PaxType* requestedPaxType = calcTotals.farePath->paxType();
  const FarePath* fp = calcTotals.farePath;

  _fareCalcDisp << "----------------------------------------------------------" << std::endl;
  _fareCalcDisp << "REQUESTED PAXTYPE : ";
  if (requestedPaxType == nullptr)
  {
    _fareCalcDisp << "0" << std::endl;
    return;
  }
  _fareCalcDisp << requestedPaxType->paxType() << "/" << requestedPaxType->vendorCode();
  _fareCalcDisp << "      FARE PATH PAXTYPE: ";

  if (fp == nullptr)
  {
    _fareCalcDisp << "0" << std::endl;
    return;
  }
  _fareCalcDisp << fp->paxType()->paxType() << std::endl;
  uint16_t iPU = 0;
  uint16_t iFU = 0;
  PaxTypeCode farePaxType;
  std::vector<PricingUnit*>::const_iterator puI = fp->pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puIEnd = fp->pricingUnit().end();
  for (; puI != puIEnd; puI++, iPU++)
  {
    _fareCalcDisp << "   PU NUMBER " << iPU + 1 << " : " << std::endl;
    std::vector<FareUsage*>::const_iterator fuI = (*puI)->fareUsage().begin();
    std::vector<FareUsage*>::const_iterator fuIEnd = (*puI)->fareUsage().end();
    for (iFU = 0; fuI != fuIEnd; fuI++, iFU++)
    {
      _fareCalcDisp << "     FU " << iFU + 1 << " "
                    << (*fuI)->paxTypeFare()->fareMarket()->boardMultiCity() << "-"
                    << (*fuI)->paxTypeFare()->fareMarket()->offMultiCity() << " PAXTYPE : ";
      farePaxType = (*fuI)->paxTypeFare()->fcasPaxType();
      if (farePaxType.empty())
        _fareCalcDisp << "ADT/EMPTY";
      else
        _fareCalcDisp << farePaxType;

      if ((*fuI)->paxTypeFare()->isWebFare())
        _fareCalcDisp << "/WEB ";
      _fareCalcDisp << "    F.C.: " << (*fuI)->paxTypeFare()->fareClass() << std::endl;
    }
  }
  _fareCalcDisp << "   TRUE PAXTYPE   : " << calcTotals.truePaxType << std::endl;
  _fareCalcDisp << "TRAILER MESSAGE : " << std::endl;
  if (calcTotals.fcMessage.empty())
    _fareCalcDisp << "NO TRAILER MESSAGE AVAILABLE " << std::endl;
  else
  {
    for (const auto& elem : calcTotals.fcMessage)
    {
      _fareCalcDisp << elem.messageText() << endl;
    }
  }
  _fareCalcDisp << "----------------------------------------------------------" << std::endl;

  return;
}

void
FareCalculation::displayItinInfo(const CalcTotals& calcTotals)
{
  if (_trx->getTrxType() != PricingTrx::MIP_TRX)
    return;

  if (LIKELY(!_trx->diagnostic().isActive() || (_trx->diagnostic().diagnosticType() != Diagnostic854)))
    return;

  const FarePath* fp = calcTotals.farePath;
  if (fp == nullptr)
    return;

  _fareCalcDisp << "ITIN " << fp->itin()->itinNum() << ":\n";

  if (!fp->getBrandCode().empty())
  {
    _fareCalcDisp << "BRAND: ";
    _fareCalcDisp << fp->getBrandCode() << "\n";
  }
}

void
FareCalculation::diagBrands(const CalcTotals& calcTotals)
{
  if (_trx->getTrxType() != PricingTrx::MIP_TRX)
    return;

  if (!_trx->diagnostic().isActive() || (_trx->diagnostic().diagnosticType() != Diagnostic854))
    return;

  if (!_trx->getRequest()->isBrandedFaresRequest())
    return;

  std::map<std::string, std::string>::iterator i =
      _trx->diagnostic().diagParamMap().find(Diagnostic::DISPLAY_DETAIL);
  if (i == _trx->diagnostic().diagParamMap().end() || (i->second != "BRAND"))
    return;

  const FarePath* fp = calcTotals.farePath;
  if (fp == nullptr)
    return;

  _fareCalcDisp << "\nBRANDS IN FAREPATH:\n";

  uint16_t iPU = 0;
  PaxTypeCode farePaxType;
  std::vector<PricingUnit*>::const_iterator puI = fp->pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puIEnd = fp->pricingUnit().end();
  for (; puI != puIEnd; puI++, iPU++)
  {
    _fareCalcDisp << " PRICING UNIT " << iPU + 1 << ": " << std::endl;
    std::vector<FareUsage*>::const_iterator fuI = (*puI)->fareUsage().begin();
    std::vector<FareUsage*>::const_iterator fuIEnd = (*puI)->fareUsage().end();
    for (; fuI != fuIEnd; fuI++)
    {
      PaxTypeFare* ptf = (*fuI)->paxTypeFare();
      _fareCalcDisp << "  FARE " << (ptf->isWebFare() ? "/WEB" : ptf->fareClass()) << " BRANDS: ";

      std::vector<int> brandIndices;
      ptf->getValidBrands(*_trx, brandIndices);

      vector<std::string> brands;
      for (int index : brandIndices)
      {
        std::string brandInfo = ShoppingUtil::getBrandCode(*_trx, index);
        brandInfo += "(" + ShoppingUtil::getProgramCode(*_trx, index) + ")";
        brands.push_back(brandInfo);
      }

      _fareCalcDisp << DiagnosticUtil::containerToString(brands) << "\n";
    }
  }

  _fareCalcDisp << "----------------------------------------------------------\n";
}

void
FareCalculation::displayNoBrandsOffered(PricingTrx& trx, CalcTotals& calcTotals)
{
  if ( trx.activationFlags().isSearchForBrandsPricing() )
  {
    if ( BrandingUtil::isNoBrandsOffered(trx, calcTotals) )
    {
      _fareCalcDisp << "BRANDS NOT AVAILABLE FOR THIS REQUEST" << std::endl;
    }
  }
}

void
FareCalculation::diagJourney(const CalcTotals& calcTotals)
{
  PricingTrx& pricingTrx = *_trx;
  const Itin* itin = calcTotals.farePath->itin();

  if ((pricingTrx.getTrxType() != PricingTrx::PRICING_TRX) &&
      (pricingTrx.getTrxType() != PricingTrx::MIP_TRX))
    return;

  typedef std::map<std::string, std::string>::iterator DiagParamMapVecI;
  DiagnosticTypes diagType = pricingTrx.diagnostic().diagnosticType();
  if (!pricingTrx.diagnostic().isActive() || (diagType != Diagnostic854))
    return;

  DiagParamMapVecI endI = pricingTrx.diagnostic().diagParamMap().end();
  DiagParamMapVecI beginI = pricingTrx.diagnostic().diagParamMap().find(Diagnostic::DISPLAY_DETAIL);

  if (beginI == endI)
    return;

  size_t len = ((*beginI).second).size();
  if (len == 0 || ((*beginI).second).substr(0, len) != "JRN")
    return;

  std::string s1;
  _fareCalcDisp << "--------------------- JOURNEY DETAILS --------------------" << std::endl;

  if (pricingTrx.getRequest()->ticketingAgent() == nullptr)
    _fareCalcDisp << "AGENT POINTER: NULL" << std::endl;

  if (pricingTrx.getRequest()->ticketingAgent()->agentTJR() == nullptr)
    _fareCalcDisp << "CUSTOMER/AGENT TJR POINTER: NULL" << std::endl;

  if (pricingTrx.getOptions()->soloActiveForPricing())
    s1 = "YES";
  else
    s1 = "NO";
  _fareCalcDisp << "SOLO ACTIVE: " << s1;
  if (pricingTrx.getOptions()->journeyActivatedForPricing())
    s1 = "YES";
  else
    s1 = "NO";

  _fareCalcDisp << "  JOURNEY ACTIVE: " << s1;

  if (pricingTrx.getOptions()->applyJourneyLogic())
    s1 = "YES";
  else
    s1 = "NO";

  _fareCalcDisp << "  JOURNEY APPLIED: " << s1 << std::endl;

  _fareCalcDisp << "JOURNEYS:";
  std::vector<FareMarket*>::const_iterator fmI = itin->flowFareMarket().begin();
  std::vector<FareMarket*>::const_iterator fmE = itin->flowFareMarket().end();

  bool atleastOneFlow = false;
  for (; fmI != fmE; ++fmI)
  {
    const FareMarket& fm = *(*fmI);
    _fareCalcDisp << " " << fm.boardMultiCity() << "-"
                  << MCPCarrierUtil::swapToPseudo(_trx, fm.governingCarrier()) << "-"
                  << fm.offMultiCity() << " ";
    atleastOneFlow = true;
  }
  if (!atleastOneFlow)
    _fareCalcDisp << " --NONE--" << std::endl;
  else
    _fareCalcDisp << std::endl;

  const FareUsage* fu = nullptr;
  uint16_t tvlSegIndex = 0;

  std::vector<TravelSeg*>::const_iterator tvlI = itin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlE = itin->travelSeg().end();
  const AirSeg* airSeg = nullptr;
  for (; tvlI != tvlE; tvlI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*tvlI);
    if (airSeg == nullptr)
    {
      _fareCalcDisp << "   ARUNK" << std::endl;
      continue;
    }

    _fareCalcDisp << std::setw(2) << (*tvlI)->pnrSegment() << " "
                  << MCPCarrierUtil::swapToPseudo(_trx, airSeg->carrier()) << std::setw(4)
                  << airSeg->flightNumber() << airSeg->getBookingCode() << " "
                  << airSeg->origAirport() << airSeg->destAirport();

    _fareCalcDisp << "  JOURNEY:";

    if (airSeg->flowJourneyCarrier())
      s1 = "F";
    else if (airSeg->localJourneyCarrier())
      s1 = "L";
    else
      s1 = "N";

    _fareCalcDisp << s1;

    _fareCalcDisp << "  SOLO:";
    if ((*tvlI)->carrierPref() == nullptr)
      s1 = "000";
    else if ((*tvlI)->carrierPref()->availabilityApplyrul2st() == YES)
      s1 = "N";
    else
      s1 = "Y";

    _fareCalcDisp << s1;

    _fareCalcDisp << "  AVL BRK:";

    fu = calcTotals.getFareUsage(*tvlI, tvlSegIndex);

    if (fu == nullptr)
    {
      _fareCalcDisp << "-NO FU- " << std::endl;
      continue;
    }

    const PaxTypeFare::SegmentStatus& fuSegStat = fu->segmentStatus()[tvlSegIndex];
    if (fuSegStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
      _fareCalcDisp << "T";
    else
      _fareCalcDisp << "F";

    _fareCalcDisp << "  BKG:";
    if (fuSegStat._bkgCodeReBook.empty())
      _fareCalcDisp << airSeg->getBookingCode() << std::endl;
    else
      _fareCalcDisp << fuSegStat._bkgCodeReBook << std::endl;

    _fareCalcDisp << "   F.C.:" << fu->paxTypeFare()->fareClass();
    _fareCalcDisp << "   F.B.:" << getFareBasisCode(calcTotals, airSeg) << " "
                  << fu->paxTypeFare()->fareMarket()->boardMultiCity() << "-"
                  << fu->paxTypeFare()->fareMarket()->offMultiCity();

    std::string diffFare = calcTotals.getDifferentialFbc(airSeg);
    if (!diffFare.empty())
      _fareCalcDisp << " DIFF:" << diffFare;

    _fareCalcDisp << std::endl;
  }
  _fareCalcDisp << "----------------------------------------------------------" << std::endl;
  return;
}

std::string
FareCalculation::getFareBasisCode(const CalcTotals& ct, const FareUsage* fu) const
{
  const FareBreakPointInfo* fbp = ct.getFareBreakPointInfo(fu);
  if (LIKELY(fbp))
  {
    return fbp->fareBasisCode;
  }
  return "";
}

std::string
FareCalculation::getFareBasisCode(const CalcTotals& ct, const TravelSeg* ts) const
{
  const FareUsage* fu = ct.getFareUsage(ts);
  return getFareBasisCode(ct, fu);
}

std::string
FareCalculation::getDifferential(const FareUsage* fu)
{
  stringstream buf;
  for (const auto diff : fu->differentialPlusUp())
  {
    if (diff == nullptr)
      continue;

    DifferentialData::STATUS_TYPE status = diff->status();
    if (status == DifferentialData::SC_PASSED || status == DifferentialData::SC_CONSOLIDATED_PASS)
    {
      if (!diff->fareClassHigh().empty() && diff->amount())
      {
        if (!buf.str().empty())
          buf << ' ';
        buf << diff->fareClassHigh();
      }
    }
  }
  return buf.str();
}

bool
FareCalculation::getMsgAppl(FareCalcConfigText::TextAppl appl,
                            std::string& msg,
                            PricingTrx& pricingTrx)
{
  const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(pricingTrx);
  if (fcConfig)
    return getMsgAppl(appl, msg, pricingTrx, *fcConfig);
  return false;
}

bool
FareCalculation::getMsgAppl(FareCalcConfigText::TextAppl appl,
                            std::string& msg,
                            PricingTrx& pricingTrx,
                            const FareCalcConfig& fcConfig)
{
  const FcConfig* fc = FcConfig::create(&pricingTrx, &fcConfig);
  if (fc)
    return fc->getMsgAppl(appl, msg);
  return false;
}

////////////////////////////////////////////////////////////////////////////
//
// No Match Verify Booking Class
//
bool
FareCalculation::verifyBookingClass(std::vector<CalcTotals*>& calcTotalsList)
{

  if (!_trx->getRequest()->isLowFareRequested())
    return false;

  bool verifyBooking = false;

  if (_trx->isLowestFareOverride() && _trx->altTrxType() == PricingTrx::WPA)
  {
    for (const auto elem : calcTotalsList)
    {
      if (elem->farePath->noMatchOption() == true)
      {
        verifyBooking = true;
        break;
      }
    }
    if (!verifyBooking)
      return false;
  }

  for (const auto elem : calcTotalsList)
  {
    verifyBooking |= verifyBookingClass(*elem);
  }

  if (verifyBooking)
  {
    displayVerifyBookingClassMsg();
  }

  return verifyBooking;
}

bool
FareCalculation::verifyBookingClass(CalcTotals& ct)
{
  bool verifyBooking = false;
  CheckBookingClass checkBookingClass(verifyBooking);
  FareCalc::forEachFareUsage(*ct.farePath, checkBookingClass);
  return verifyBooking;
}

void
FareCalculation::displayVerifyBookingClassMsg()
{
  if ((_trx->altTrxType() == PricingTrx::WPA && _trx->getRequest()->lowFareRequested() == 'T') ||
      (_trx->altTrxType() == PricingTrx::WP_NOMATCH))
  {
    std::string verifyBookingClass;
    if (_fcConfig->getMsgAppl(FareCalcConfigText::WPA_NO_MATCH_VERIFY_BOOKING_CLASS,
                              verifyBookingClass))
    {
      _fareCalcDisp << _warning << verifyBookingClass << endl;
    }
  }
}

bool
FareCalculation::CheckBookingClass::
operator()(const FareUsage* fu)
{
  if (fu == nullptr)
    return false;

  if (fu->differentialPlusUp().size() == 0)
  {
    return (_status = true);
  }

  return false;
}

void
FareCalculation::displayWpaTrailerMessage()
{
  if (_trx->altTrxType() == PricingTrx::WPA && !_allRequireRebook && !_trx->getRequest()->wpaXm() &&
      !_trx->getOptions()->isMatchAndNoMatchRequested())
  {
    if (_trx->getRequest()->isWpas())
    {
      _fareCalcDisp << _warning << "SEE OTHER FARES - USE WPAS$XM" << std::endl;
      _fareCalcDisp << _warning << "SEE ALL FARES - USE WPAS$AL" << std::endl;
    }
    else
    {
      _fareCalcDisp << _warning << "SEE OTHER FARES - USE WPA$XM" << std::endl;
      _fareCalcDisp << _warning << "SEE ALL FARES - USE WPA$AL" << std::endl;
    }
  }
}

void
FareCalculation::displayPtcMessages(PricingTrx& trx, CalcTotals& calcTotals)
{
  std::string msg = AdjustedSellingUtil::getADJSellingLevelMessage(trx, calcTotals);
  if (!msg.empty())
    _fareCalcDisp << msg << endl;

  msg = AdjustedSellingUtil::getADJSellingLevelOrgMessage(trx, calcTotals);
  if (!msg.empty())
    _fareCalcDisp << msg << endl;

  displayEndorsements(trx, calcTotals.farePath);

  if (_fcConfig->lastDayTicketDisplay() == FareCalcConsts::FC_BOTTOM)
  {
    displayLastTicketDate(trx, calcTotals);
  }

  if ((_fcConfig->fareTaxTotalInd() != FareCalcConsts::HORIZONTAL_FARECALC1) ||
      (_fcConfig->displayBSR() == FareCalcConsts::FC_YES))
  {
    verticalProcessRateLine(*_trx, calcTotals);
  }

  displayWarnings(calcTotals);
}

void
FareCalculation::displayAxessCat27Lines(const FarePath* farePath)
{
  if (!farePath)
    return;

  const string& tourCode = farePath->cat27TourCode();
  if (!tourCode.empty())
  {
    _fareCalcDisp << "CODE*T/";
    _fareCalcDisp << tourCode;
    if (farePath->multipleTourCodeWarning())
      _fareCalcDisp << "*";
    _fareCalcDisp << endl;
  }
}

void
FareCalculation::displayAxessCat35Lines(PricingTrx& trx, const CalcTotals& calcTotals)
{
  const FarePath* farePath = calcTotals.farePath;

  if (farePath == nullptr)
    return;

  const CollectedNegFareData* cNegFareData = farePath->collectedNegFareData();
  if (cNegFareData == nullptr)
    return;

  // BASE DATA
  if (cNegFareData->netTotalAmt() != 0)
  {
    MoneyAmount outAmount = cNegFareData->netTotalAmt();
    MoneyAmount inAmount = cNegFareData->netTotalAmt();

    if (trx.getRequest()->isWpNettRequested())
    {
      _fareCalcDisp << "BASE FARE*";
      inAmount = farePath->getTotalNUCAmount();
    }
    else
    {
      _fareCalcDisp << "BASE DATA*";
    }
    _fareCalcDisp << _fareCurrencyCode;

    CurrencyConversionFacade ccFacade;
    const Money sourceMoney(inAmount, farePath->itin()->calculationCurrency());
    Money targetMoney(_fareCurrencyCode);

    if (ccFacade.convert(
            targetMoney, sourceMoney, trx, farePath->itin()->useInternationalRounding()))
    {
      outAmount = targetMoney.value();
    }
    _fareCalcDisp.precision(_fareNoDec);
    _fareCalcDisp << outAmount;

    if (cNegFareData->differentNetFare())
    {
      _fareCalcDisp << "*";
    }
    _fareCalcDisp << endl;
  }

  // CODE
  if (!cNegFareData->tourCode().empty())
  {
    _fareCalcDisp << "CODE*";
    _fareCalcDisp << cNegFareData->indTypeTour() << "/";
    _fareCalcDisp << cNegFareData->tourCode();
    if (cNegFareData->differentCode())
    {
      _fareCalcDisp << "*";
    }
    _fareCalcDisp << endl;
  }
  else
  {
    displayAxessCat27Lines(farePath);
  }

  // TKT DESG
  if (!cNegFareData->tDesignator().empty())
  {
    _fareCalcDisp << "TKTDESG*";
    _fareCalcDisp << cNegFareData->tDesignator();
    if (cNegFareData->differentTktDesg())
    {
      _fareCalcDisp << "*";
    }
    _fareCalcDisp << endl;
  }

  // COMM
  if (cNegFareData->comPercent() != RuleConst::PERCENT_NO_APPL)
  {
    _fareCalcDisp << "COMM*";
    if (cNegFareData->indNetGross() == 'G')
    {
      _fareCalcDisp << "GROSS/";
    }
    else
    {
      _fareCalcDisp << "NET/";
    }
    std::ostringstream os;
    os << cNegFareData->comPercent();
    _fareCalcDisp << os.str() << "PCT";
    if (cNegFareData->differentComm())
    {
      _fareCalcDisp << "*";
    }
    _fareCalcDisp << endl;
  }

  // FAREBOX
  if (!cNegFareData->fareBox().empty())
  {
    _fareCalcDisp << "FARE BOX*";
    _fareCalcDisp << cNegFareData->fareBox();
    _fareCalcDisp << endl;
  }
}

void
FareCalculation::wpNettPrefix(FareCalcCollector& fcCollector)
{
  bool no_netRemitCalc_found = false;
  const FareCalcCollector::CalcTotalsMap& calcTotalsMap = fcCollector.calcTotalsMap();

  for (const auto& elem : calcTotalsMap)
  {
    if (elem.second->netRemitCalcTotals == nullptr)
    {
      no_netRemitCalc_found = true;
      break;
    }
  }

  if (!no_netRemitCalc_found)
  {
    _fareCalcDisp << "VT " << endl;
    _needNetRemitCalc = true;
    return;
  }

  _fareCalcDisp << "VE " << endl;

  for (const auto& elem : calcTotalsMap)
  {
    if (elem.second->netRemitCalcTotals != nullptr)
    {
      FarePath* farePath = const_cast<FarePath*>(elem.second->farePath);
      farePath->collectedNegFareData()->trailerMsg() =
          "INVALID NET REMIT FARE - UNABLE TO AUTO TICKET";
    }
  }
}

void
FareCalculation::displayPsgFareLineFormat(CalcTotals& calcTotals)
{
  if (!checkExistFarePath(*_trx, calcTotals))
    return;

  displayFareLineInfo(calcTotals);

  if (_trx->diagnostic().diagnosticType() == Diagnostic854)
  {
    diagTruePaxType(calcTotals);
    diagJourney(calcTotals);
  }
}

void
FareCalculation::displayFareLineInfo(CalcTotals& calcTotals)
{
  calcTotals.fcConfig = _fcConfig->fcc();
  resetCounters();
  getTotalFare(calcTotals);

  const uint8_t factor = calcTotals.wpaInfo.psgDetailRefNo / 100;

  _fareCalcDisp << right << std::setw(2) << std::setfill('0')
                << (calcTotals.wpaInfo.psgDetailRefNo - (factor * 100));
  _fareCalcDisp << PrivateIndicator::privateFareIndicator(calcTotals.privateFareIndSeq);

  std::string fareBasis = getFareBasis(calcTotals);
  SpanishFamilyDiscountDesignator appender =
      spanishFamilyDiscountDesignatorBuilder(*_trx,
                                             calcTotals,
                                             FareCalcConsts::MAX_FARE_BASIS_SIZE_WQ_WPA);
  appender(fareBasis);

  _fareCalcDisp << std::setw(FareCalcConsts::MAX_FARE_BASIS_SIZE_WQ_WPA) << std::left
                << std::setfill(' ') << fareBasis << ' ';

  std::string bookingCode = getBookingCode(calcTotals);
  _fareCalcDisp << std::setw(9) << std::left << std::setfill(' ') << bookingCode << ' ';

  if (calcTotals.farePath && calcTotals.farePath->intlSurfaceTvlLimit())
  {
    _fareCalcDisp << "ISSUE SEP TKTS-INTL SURFACE RESTRICTED";
  }
  else
  {
    if (_fcConfig->wpaFareLineHdr() == '2')
    {
      displayTotalFareAmount(calcTotals);
      displayCurrencyCode(calcTotals);
      displayRO(calcTotals);
      _fareCalcDisp << "  INCL TAX";
    }
    else
    {
      displayCurrencyCode(calcTotals);
      displayBaseFareAmount(calcTotals);
      displayTaxAmount(calcTotals);
      displayTotalFareAmount(calcTotals);
      displayRO(calcTotals);
    }
  }
  _fareCalcDisp << endl;
}

std::string
FareCalculation::getFareBasis(CalcTotals& calcTotals)
{
  std::string line;
  std::string fareBasis, prev_fareBasis, fareDiff;

  FareUsageIter fuIter(*calcTotals.farePath);
  for (auto& elem : fuIter)
  {
    fareBasis = getFareBasisCode(calcTotals, elem);
    fareDiff = getDifferential(elem);
    if (!fareBasis.empty())
    {
      if (fareBasis != prev_fareBasis)
      {
        if (!line.empty())
          line.append("-");
        line.append(fareBasis);
      }
      if (!fareDiff.empty())
        line.append("-DIFF ").append(fareDiff);
    }
    prev_fareBasis = fareBasis;
  }

  if (line.size() > 10)
  {
    line.replace(9, line.size() - 9, "*");
  }

  return line;
}

std::string
FareCalculation::getBookingCode(CalcTotals& calcTotals)
{
  std::string bookingCode;

  unsigned int bcrCount = calcTotals.bookingCodeRebook.size();
  unsigned int tsIndex = 0;
  for (std::vector<TravelSeg*>::const_iterator
           i = calcTotals.farePath->itin()->travelSeg().begin(),
           iend = calcTotals.farePath->itin()->travelSeg().end();
       i != iend;
       ++i, ++tsIndex)
  {
    AirSeg* as = dynamic_cast<AirSeg*>(*i);
    if (as == nullptr)
      continue;

    if (bookingCode.size() > 0)
      bookingCode += '/';

    if (bcrCount > 0 && tsIndex < bcrCount)
    {
      if (calcTotals.bookingCodeRebook[tsIndex].empty())
        bookingCode += (*i)->getBookingCode();
      else
        bookingCode += calcTotals.bookingCodeRebook[tsIndex];
    }
    else
    {
      bookingCode += (*i)->getBookingCode();
    }
  }

  // Formatting the Booking codes portion
  if (bookingCode.size() > 9)
  {
    bookingCode.erase(8);
    bookingCode += '*';
  }

  return bookingCode;
}

void
FareCalculation::displayTotalFareAmount(CalcTotals& calcTotals)
{
  _fareCalcDisp << fixed << right << setw(12) << setfill(' ');
  _equivNoDec = _fareNoDec;

  if (!(_trx->getOptions()->currencyOverride().empty() ||
        _trx->getOptions()->currencyOverride() == _fareCurrencyCode) &&
      !(calcTotals.convertedBaseFareCurrencyCode == calcTotals.equivCurrencyCode))
  {
    _equivNoDec = calcTotals.equivNoDec;
  }

  LOG4CXX_DEBUG(
      logger,
      "\nCur. Orride: " << _trx->getOptions()->currencyOverride()
                        << ", _fareCurrencyCode: " << _fareCurrencyCode
                        << ", Alt. Cur: " << _trx->getOptions()->alternateCurrency()
                        << "\nct.convertedBaseFare: " << calcTotals.convertedBaseFare
                        << "\nct.convertedBaseFareCurrencyCode: "
                        << calcTotals.convertedBaseFareCurrencyCode
                        << "\nct.equivFareAmount: " << calcTotals.equivFareAmount
                        << "\nct.equivCurrencyCode: " << calcTotals.equivCurrencyCode
                        << "\ntotalBaseAmount: " << _totalBaseAmount
                        << "\n_totalFareAmount: " << _totalFareAmount
                        << "\nfp.totalNUCAmount: " << calcTotals.farePath->getTotalNUCAmount()
                        << "\nfp.calcCurrency: " << calcTotals.farePath->calculationCurrency()
                        << "\nct.taxAmount: " << calcTotals.taxAmount());

  if (_trx->getOptions()->currencyOverride().empty() ||
      _trx->getOptions()->currencyOverride() == _fareCurrencyCode)
  {
    if (calcTotals.convertedBaseFareCurrencyCode == calcTotals.equivCurrencyCode)
    {
      _totalFareAmount = _totalBaseAmount + calcTotals.taxAmount();
    }
    else
    {
      _totalFareAmount = calcTotals.convertedBaseFare + calcTotals.taxAmount();
    }
  }
  else
  {
    if (calcTotals.convertedBaseFareCurrencyCode == calcTotals.equivCurrencyCode)
    {
      _totalFareAmount = _totalBaseAmount + calcTotals.taxAmount();
    }
    else
    {
      _totalFareAmount = calcTotals.equivFareAmount + calcTotals.taxAmount();
    }
  }

  if (_fcConfig->wpaFareLineHdr() == '2')
    _fareCalcDisp << setw(15);

  _fareCalcDisp << setprecision(_equivNoDec) << _totalFareAmount;
}

void
FareCalculation::displayCurrencyCode(CalcTotals& calcTotals)
{
  if (_fcConfig->wpaFareLineHdr() == '2')
    _fareCalcDisp << ' ';

  if (calcTotals.convertedBaseFareCurrencyCode == calcTotals.equivCurrencyCode)
    _fareCalcDisp << setw(3) << setfill(' ') << right << _fareCurrencyCode;
  else
    _fareCalcDisp << setw(3) << setfill(' ') << right << calcTotals.equivCurrencyCode;
}

void
FareCalculation::displayRO(CalcTotals& calcTotals)
{
  if (_fcConfig->wpaFareLineHdr() != '2')
    _fareCalcDisp << ' '; // 1 spaces
  else
    _fareCalcDisp << "     "; // 5 spaces

  // RO Indicator: 1 - Display RO in Fare Line along with message
  //               2 - display PTC in Fare Line and RO message
  //               blank - display nothing

  Indicator wpaRoInd = _fcConfig->wpaRoInd();

  if (wpaRoInd == '1' || wpaRoInd == '2')
  {
    if (wpaRoInd == '2')
    {
      _fareCalcDisp << left << setw(3) << calcTotals.truePaxType;
    }
    else
    {
      _fareCalcDisp << "   ";
    }
  }
  else
  {
    _fareCalcDisp << "   ";
  }
}

void
FareCalculation::displayTaxAmount(CalcTotals& calcTotals, unsigned int taxFieldWidth)
{
  _fareCalcDisp << fixed << right << setw(taxFieldWidth) << setfill(' ');
  if (_trx->getRequest()->isExemptAllTaxes())
  {
    _fareCalcDisp << ' ';
  }
  else
  {
    _fareCalcDisp << setprecision(calcTotals.taxNoDec()) << calcTotals.taxAmount();
  }
}

void
FareCalculation::displayBaseFareAmount(CalcTotals& calcTotals)
{
  _fareCalcDisp << fixed << right << setw(11) << setfill(' ');
  if (_trx->getOptions()->currencyOverride().empty() ||
      _trx->getOptions()->currencyOverride() == _fareCurrencyCode)
  {
    // Process Base Amount
    if (_totalBaseAmount == calcTotals.convertedBaseFare)
      _fareCalcDisp << setprecision(_fareNoDec) << _totalBaseAmount;
    else
      _fareCalcDisp << setprecision(_fareNoDec) << calcTotals.convertedBaseFare;
  }
  else
  {
    // Process Equivalent Amount
    if (calcTotals.convertedBaseFareCurrencyCode == calcTotals.equivCurrencyCode)
    {
      _fareCalcDisp << setprecision(_fareNoDec) << calcTotals.convertedBaseFare;
    }
    else
    {
      _fareCalcDisp << setprecision(calcTotals.equivNoDec) << calcTotals.equivFareAmount;
    }
  }
}

void
FareCalculation::displayDtlFareCalc(CalcTotals& calcTotals,
                                    CalcTotals& cat35_calcTotals,
                                    bool& firstPaxType,
                                    bool& ttlProcessed)
{
  if (LIKELY(checkExistFarePath(*_trx, calcTotals)))
  {
    resetCounters(); // reset before process next passenger type

    displayItineraryLine(*_trx, *_fcCollector, calcTotals, firstPaxType);
    displayFareTax(*_trx, *_fcCollector, calcTotals);

    firstPaxType = false;
  }

  if (_fcConfig->itinDisplayInd() == FareCalcConsts::FC_YES)
  {
    // If there is only one pax (all match) - display the TTL before the FCL
    if (_fcCollector->calcTotalsMap().size() == 1 &&
        _fcConfig->fareTaxTotalInd() == FareCalcConsts::HORIZONTAL_FARECALC1)
    {
      ttlProcessed = processGrandTotalLine(*_trx, *_fcCollector);
    }

    displayPsgFareCalc(*_trx, *_fcCollector, calcTotals);

    // Display Net fare result w/o Tax and Total for WPNETT
    if (_needNetRemitCalc)
    {
      displayPsgFareCalc(*_trx, *_fcCollector, cat35_calcTotals);
    }
  }
}

void
FareCalculation::displayFareLineHdrMultiVersion(bool activeNewVersion)
{
  if (activeNewVersion)
  {
    if (_fcConfig->wpaFareLineHdr() == '2')
    {
      _fareCalcDisp << "   FARE BASIS BOOK CODE           FARE" << endl;
    }
    else
    {
      _fareCalcDisp << "   FARE BASIS BOOK CODE           FARE TAX/FEES/CHGS  TOTAL" << endl;
    }
  }
  else
  {
    if (_fcConfig->wpaFareLineHdr() == '2')
    {
      _fareCalcDisp << "   FARE BASIS BOOK CODE           FARE" << endl;
    }
    else
    {
      _fareCalcDisp << "   FARE BASIS BOOK CODE           FARE      TAX       TOTAL" << endl;
    }
  }
}

void
FareCalculation::displayFareLineHdr()
{
  const Agent* agent = _trx->getRequest()->ticketingAgent();
  if (agent->tvlAgencyPCC().empty() || agent->axessUser())
  { // AS
    displayFareLineHdrMultiVersion(TrxUtil::isTaxesNewHeaderASActive(*_trx));
  }
  else
  { // TN
    displayFareLineHdrMultiVersion(true);
  }
}

void
FareCalculation::displayNoMatchMessage(CalcTotals& calcTotals)
{
  displayPsgrInfo(calcTotals, false);

  // if (_fcConfig->fareTaxTotalInd() == FareCalcConsts::HORIZONTAL_FARECALC1)
  // {
  //     _fareCalcDisp << calcTotals.requestedPaxType
  //                   << '-' << setw(2) << setfill('0')
  //                   << calcTotals.farePath->paxType()->number() << endl;
  // }
  // else
  // {
  //     displayPsgrInfo(calcTotals);
  // }

  std::string noMatchNoFare;
  if (_fcConfig->getMsgAppl(FareCalcConfigText::WP_NO_MATCH_NO_FARE, noMatchNoFare))
  {
    _fareCalcDisp << noMatchNoFare << endl;
  }
  else
  {
    if (_trx->altTrxType() == PricingTrx::WPA && _trx->getOptions()->isCat35Net())
    {
      _fareCalcDisp << "*NO NET FARE AMOUNT" << endl;
    }
    else if (_trx->getRequest()->ticketingAgent()->abacusUser() ||
             _trx->getRequest()->ticketingAgent()->infiniUser())
    {
      _fareCalcDisp << "*NO FARES/RBD/CARRIER" << endl;
    }
    else if (_trx->getOptions()->forceCorpFares())
    {
      _fareCalcDisp << "ATTN* DISCOUNT NOT APPLICABLE" << endl;
    }
    else
    {
      _fareCalcDisp << " NO RULES VALID FOR PASSENGER TYPE/CLASS OF SERVICE" << endl;
    }
  }
}

// ---------------------------------------------------------
// display multi AccCode/ CorpID trailer message
//   when 1-4 CorpIDs are invalid
// ---------------------------------------------------------

void
FareCalculation::displayCorpIdTrailerMessage()
{
  const std::vector<std::string>& corpIdVec = _trx->getRequest()->incorrectCorpIdVec();

  if (!corpIdVec.empty())
  {
    if (_fcConfig->warningMessages() == FareCalcConsts::FC_YES)
      _fareCalcDisp << "ATTN*";

    _fareCalcDisp << "INVALID CORPORATE ID ";
    std::copy(
        corpIdVec.begin(), corpIdVec.end(), std::ostream_iterator<std::string>(_fareCalcDisp, " "));
    _fareCalcDisp << std::endl;
  }
}

void
FareCalculation::displaySegmentFeeMessage()
{
  FareCalcCollector::CalcTotalsMap::const_iterator i = _fcCollector->calcTotalsMap().begin();
  if (i->second->dispSegmentFeeMsg())
  {
    _fareCalcDisp << _warning << "TOTAL INCLUDES CARRIER IMPOSED SURCHARGES";
  }
}

void
FareCalculation::displayNonIATARoundingTextMessage(CalcTotals& calcTotals)
{
  const FarePath* farePath = calcTotals.farePath;
  if (LIKELY(!farePath || !farePath->applyNonIATARounding(*_trx)))
    return;

  // calcTotals.fcMessage.push_back(FcMessage(FcMessage::WARNING, 0, "NON-IATA ROUNDING APPLIED"));

  _fareCalcDisp << _warning << "NON-IATA ROUNDING APPLIED" << std::endl;
}
void
FareCalculation::displayCat05BookingDTOverrideMessage(const FarePath* fp)
{
  const Itin* itin = fp->itin();

  if (itin && itin->cat05BookingDateValidationSkip())
  {
    std::vector<PricingUnit*>::const_iterator puI = fp->pricingUnit().begin();
    std::vector<PricingUnit*>::const_iterator puIEnd = fp->pricingUnit().end();
    for (; puI != puIEnd; ++puI)
    {
      if ((*puI)->isOverrideCxrCat05TktAftRes())
      {
        _fareCalcDisp << "ATTN*BOOKING DATE/TIME OVERRIDDEN WITH PRICING DATE/TIME" << std::endl;
        break;
      }
    }
  }
  return;
}

void
FareCalculation::displaySpanishLargeFamilyDiscountMessage(PricingTrx& trx, const Itin& itin)
{
  if (trx.getOptions()->getSpanishLargeFamilyDiscountLevel() != SLFUtil::DiscountLevel::NO_DISCOUNT)
  {
    if (LocUtil::isSpain(*(trx.getRequest()->ticketingAgent()->agentLocation())) &&
        LocUtil::isWholeTravelInSpain(itin.travelSeg()))
      _fareCalcDisp << _warning << "DFN APPLIED - PASSENGER DATA REQUIRED AT TICKETING"
                    << std::endl;
    else
      _fareCalcDisp << _warning << "DFN DOES NOT APPLY - VERIFY RESTRICTIONS" << std::endl;
  }
}

void
FareCalculation::displayBrandWarning()
{
  if (_trx->isPbbRequest() == PBB_RQ_DONT_PROCESS_BRANDS)
  {
    _fareCalcDisp << " " << std::endl;
    _fareCalcDisp << "BRAND CODE NOT APPLIED" << std::endl;
  }
}

CalcTotals*
FareCalculation::selectCalcTotals(PricingTrx& trx,
                                  CalcTotals* calcTotals,
                                  bool _needNetRemitCalc)
{
  if (_needNetRemitCalc && calcTotals->netRemitCalcTotals)
    return calcTotals->netRemitCalcTotals;

  if (trx.getOptions() && !trx.getOptions()->isPDOForFRRule() && calcTotals->adjustedCalcTotal)
  {
    return (calcTotals->adjustedCalcTotal);
  }
  return calcTotals;
}

DisplayWarningsForFC::DisplayWarningsForFC(PricingTrx& trx,
                                           FareCalc::FcStream& fareCalcDisp,
                                           const FcConfig* fcConfig,
                                           std::vector<std::string>& warningFopMsgs,
                                           bool& warningEtktCat15,
                                           const CalcTotals& calcTotals,
                                           FareCalcCollector* fcCollector)
  : _trx(trx),
    _fareCalcDisp(fareCalcDisp),
    _fcConfig(fcConfig),
    _warningFopMsgs(warningFopMsgs),
    _warningEtktCat15(warningEtktCat15),
    _calcTotals(calcTotals),
    _fcCollector(fcCollector)

{
}

void
DisplayWarningsForFC::display()
{
  const FarePath& farePath = *_calcTotals.farePath;

  _warningFopMsgs.clear();
  _warningEtktCat15 = false;

  for (PricingUnit* pu : farePath.pricingUnit())
    display(pu);

  // Display unique FOP Warning
  for (std::string& wfm : _warningFopMsgs)
    displayWarning(wfm);

  // Display Currency Adjustment warning MSG for Nigeria
  if (!farePath.trailerCurrAdjMsg().empty())
    displayWarning(farePath.trailerCurrAdjMsg());

  // Display Paper tkt surcharge applies warning MSG
  if (!farePath.paperTktSurcharge().empty())
    displayWarning(farePath.paperTktSurcharge());

  // display E_ticketing warning MSG
  if (_warningEtktCat15)
    displayWarning("ELECTRONIC TICKETING REQUIRED");

  // Display Cat35 trailer message/ Cat35 commission msg
  const CollectedNegFareData* cNegFareData = farePath.collectedNegFareData();
  if (LIKELY(cNegFareData != nullptr))
  {
    if (!cNegFareData->trailerMsg().empty())
    {
      if (_fcConfig->warningMessages() == FareCalcConsts::FC_YES)
        _fareCalcDisp << "ATTN*";

      _fareCalcDisp << cNegFareData->trailerMsg() << std::endl;
    }

    if (!cNegFareData->commissionMsg().empty())
    {
      if (_fcConfig->warningMessages() == FareCalcConsts::FC_YES)
        _fareCalcDisp << "ATTN*";

      _fareCalcDisp << cNegFareData->commissionMsg() << std::endl;
    }
  }

  // Display Cat 27 tour code warning
  if (farePath.multipleTourCodeWarning() &&
      (cNegFareData == nullptr || cNegFareData->trailerMsg().empty()))
    displayWarning("UNABLE TO AUTO TICKET - MULTIPLE TOUR CODES");

  displayFcMessages();
}

void
DisplayWarningsForFC::display(const FareUsage* fareUsage)
{
  const PaxTypeFare& paxTypeFare = *(fareUsage->paxTypeFare()); // lint !e530

  for (const std::string msg : paxTypeFare.csTextMessages())
    displayWarning(msg);

  // Save an unique FOP warnings
  const std::string& fopMsg = fareUsage->getFopTrailerMsg();
  if (!fopMsg.empty() &&
      std::find(_warningFopMsgs.begin(), _warningFopMsgs.end(), fopMsg) == _warningFopMsgs.end())
    _warningFopMsgs.push_back(fopMsg);

  // Display Minimum Fare Warning
  const VecMultiMap<MinimumFareModule, std::string>& minFareWarnings =
      fareUsage->minFareUnchecked();
  VecMultiMap<MinimumFareModule, std::string>::const_iterator minFareIter = minFareWarnings.begin();
  for (; minFareIter != minFareWarnings.end(); minFareIter++)
    displayWarning(minFareIter->second);

  // Cat15 E_ticketing warning MSG
  if (UNLIKELY(paxTypeFare.fare()->isWarningEtktInCat15()))
    _warningEtktCat15 = true;
  else if (paxTypeFare.isFareByRule())
  {
    FBRPaxTypeFareRuleData* fbrPaxTypeFare = nullptr;
    const FareByRuleItemInfo* fbrItemInfo = nullptr;
    bool fbrCalcFare = false;

    fbrPaxTypeFare = paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
    if (fbrPaxTypeFare != nullptr)
    {
      fbrItemInfo = dynamic_cast<const FareByRuleItemInfo*>(fbrPaxTypeFare->ruleItemInfo());
      fbrCalcFare = (fbrItemInfo != nullptr && !fbrPaxTypeFare->isSpecifiedFare());
      if (fbrCalcFare && fbrPaxTypeFare->baseFare()->fare()->isWarningEtktInCat15())
        _warningEtktCat15 = true;
    }
  }
}

void
DisplayWarningsForFC::display(const PricingUnit* pricingUnit)
{
  for (const FareUsage* fareUsage : pricingUnit->fareUsage())
    display(fareUsage);

  const VecMultiMap<MinimumFareModule, std::string>& minFareWarnings =
      pricingUnit->minFareUnchecked();
  VecMultiMap<MinimumFareModule, std::string>::const_iterator minFareIter = minFareWarnings.begin();
  for (; minFareIter != minFareWarnings.end(); minFareIter++)
    displayWarning(minFareIter->second);
}

void
DisplayWarningsForFC::displayWarning(const std::string& message)
{
  if (_fcConfig->warningMessages() == FareCalcConsts::FC_YES)
    _fareCalcDisp << "ATTN*";

  _fareCalcDisp << message << std::endl;
}

void
DisplayWarningsForFC::displayFcMessages()
{
  for (const FcMessage& fc : _calcTotals.fcMessage)
    display(fc);
}

void
DisplayWarningsForFC::display(const FcMessage& message)
{
  // NoPNR rule warning messages are prefixed with 3-digits -
  // - order in which they are to be displayed
  std::string msg;
  if (UNLIKELY(message.messageType() == FcMessage::NOPNR_RULE_WARNING && message.messageText().size() > 3))
    msg = message.messageText().substr(3);
  else
    msg = message.messageText();

  if (message.messagePrefix() && _fcConfig->warningMessages() == FareCalcConsts::FC_YES &&
      string::npos == message.messageText().find(FreeBaggageUtil::BaggageTagHead))
    _fareCalcDisp << "ATTN*";

  _fareCalcDisp << msg << "\n";
}


} // namespace
