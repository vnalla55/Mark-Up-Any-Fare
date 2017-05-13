//-------------------------------------------------------------------------------
// Copyright 2015, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "Common/RBDByCabinUtil.h"
#include "Common/ClassOfService.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/Logger.h"
#include "Common/TravelSegAnalysis.h"
#include "Common/FallbackUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/Customer.h"
#include "DBAccess/RBDByCabinInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag187Collector.h"

#include <algorithm>

namespace tse
{
  FALLBACK_DECL(fallbackRBDByCabinOpt);
  FALLBACK_DECL(fallbackRBDByCabinPh2);

const Indicator RBDByCabinUtil::CHAR_BLANK;

static Logger
logger("atseintl.Common.RBDByCabinUtil");

RBDByCabinUtil::RBDByCabinUtil(PricingTrx& trx, RBDByCabinCall call, Diag187Collector* diag)
 : _trx(trx),
   _diag(diag),
   _call(call)
{
}

void
RBDByCabinUtil::setsUpFMInds(const PaxTypeFare& ptf)
{
  _ptf = &ptf;
  for(const TravelSeg* tvl  : ptf.fareMarket()->travelSeg())
    _travelSegs.emplace_back(tvl);
}

void
RBDByCabinUtil::setsUpSegInds(const AirSeg& seg, bool fakeSeg, bool ancilBagServices)
{
  _airSeg = &seg;
  _ancilBagServices = ancilBagServices;
  _segmentVSfareComp = true;
  if(!fakeSeg)
    _travelSegs.emplace_back(_airSeg);
}

// FBCV - Fare Component Cat31
void
RBDByCabinUtil::getCabinsByRbd(const PaxTypeFare& ptf, std::vector<ClassOfService>& classOfService)
{
  createDiag187();
  setsUpFMInds(ptf);

  if(needToPrintDiagNoRBD() && isFareClassOrBasisSelected(ptf))
  {
    printHeader();
    printAirSegSearchData(ptf, classOfService);
    printRecordHeader();
  }

  const Cabin* cabin = nullptr;
  std::vector<ClassOfService>::iterator cI = classOfService.begin();
  std::vector<ClassOfService>::iterator cE = classOfService.end();

  DateTime tvlDate(ptf.fareMarket()->travelDate());
  const std::vector<RBDByCabinInfo*>& cabinInfos =
      getRBDByCabin(ATPCO_VENDOR_CODE, ptf.fareMarket()->governingCarrier(), tvlDate);

  bool cabinNotFound = false;
  for (; cI != cE; ++cI)
  {
    ClassOfService& cs = *cI;

    if(needToPrintDiag(cs.bookingCode()))
      printCurrentRBD(cs.bookingCode());

    const Cabin* cabin = validateRBDByCabin(cabinInfos, cs.bookingCode(), tvlDate, tvlDate); // current logic

    if (cabin != nullptr)
    {
      cs.cabin() = cabin->cabin();
    }
    else
    {
      cs.cabin().setInvalidClass();
      cabinNotFound = true;
    }
  }

  if(!cabinNotFound)
  {
     finishDiag();
     return;
  }

  if(!cabinInfos.empty() && cabinInfos.front()->carrier() == INDUSTRY_CARRIER)
  {
    cI = classOfService.begin();
    cE = classOfService.end();
    for (; cI != cE; ++cI)
    {
      ClassOfService& cs = *cI;
      if(cs.cabin().isInvalidClass())
      {
        LOG4CXX_ERROR(logger,
                      "RBDByCabinUtil::getCabins() for CAT31 - CABIN TABLE ERROR:"
                          << "CXR: " << ptf.fareMarket()->governingCarrier() << " "
                          << cs.bookingCode());
      }
    }
    finishDiag();
    return;
  }

  if(isCabinSelected())
  {
    finishDiag();
    return;
  }

  const std::vector<RBDByCabinInfo*>& cabinInfos1 =
                          getRBDByCabin(ATPCO_VENDOR_CODE, INDUSTRY_CARRIER, tvlDate);

  if(needToPrintDiagNoRBD())
    printRecordHeader();

  cI = classOfService.begin();
  cE = classOfService.end();
  for (; cI != cE; ++cI)
  {
    ClassOfService& cs = *cI;
    if(cs.cabin().isInvalidClass())
    {
      if(needToPrintDiag(cs.bookingCode()))
        printCurrentRBD(cs.bookingCode());

      cabin = validateRBDByCabin(cabinInfos1, cs.bookingCode(), tvlDate, tvlDate); // current logic 
      if (cabin != nullptr)
      {
        cs.cabin() = cabin->cabin();
      }
      else
      {
        LOG4CXX_ERROR(logger,
                      "RBDByCabinUtil::getCabins() for CAT31 - CABIN TABLE ERROR:"
                          << "CXR: " << ptf.fareMarket()->governingCarrier() << " "
                          << cs.bookingCode());
      }
    }
  }
  finishDiag();
}

// ItinAnalyzer - gets cabin for the fake AirSeg
void
RBDByCabinUtil::getCabinsByRbd(const AirSeg& airSeg,
                               std::vector<ClassOfService*>& classOfService)
{
  createDiag187();
  setsUpSegInds(airSeg, true);
  DateTime tvlDate(airSeg.departureDT());

  if(needToPrintDiagNoRBD())
  {
    printHeader();
    printAirSegSearchData(classOfService, tvlDate);
  }

  const Cabin* cabin = nullptr;

  const std::vector<RBDByCabinInfo*>& cabinInfos =
                          getRBDByCabin(ATPCO_VENDOR_CODE, airSeg.carrier(), tvlDate);

  if(needToPrintDiagNoRBD())
    printRecordHeader();

  bool cabinNotFound = false;
  for (ClassOfService* cos : classOfService)
  {
    if (needToPrintDiag(cos->bookingCode()))
      printCurrentRBD(cos->bookingCode());

    cabin = validateRBDByCabin(cabinInfos, cos->bookingCode(), tvlDate, tvlDate); // current logic

    if (cabin != nullptr)
    {
      cos->cabin() = cabin->cabin();
    }
    else
    {
      cos->cabin().setInvalidClass();
      cabinNotFound = true;
    }
  }

  if(!cabinNotFound)
  {
     finishDiag();
     return;
  }

  if(!cabinInfos.empty() && cabinInfos.front()->carrier() == INDUSTRY_CARRIER)
  {
    for (ClassOfService* cos : classOfService)
    {
      if (cos->cabin().isInvalidClass())
      {
        LOG4CXX_ERROR(logger,
                      "RBDByCabinUtil::getCabins() for fake AirSeg CABIN TABLE ERROR:"
                          << "CXR: " << airSeg.carrier() << " " << cos->bookingCode() << " "
                          << airSeg.flightNumber() << " "
                          << airSeg.departureDT().dateToString(DDMMMYYYY, ""));
      }
    }
    finishDiag();
    return;
  }

  if(isCabinSelected())
  {
    finishDiag();
    return;
  }

  const std::vector<RBDByCabinInfo*>& cabinInfos1 =
                          getRBDByCabin(ATPCO_VENDOR_CODE, INDUSTRY_CARRIER, tvlDate);

  if(cabinInfos1.empty() && needToPrintDiagNoRBD())
    printRBDByCabinNotFoundForCarrier(INDUSTRY_CARRIER);

  if(needToPrintDiagNoRBD())
    printRecordHeader();

  for (ClassOfService* cos : classOfService)
  {
    if (cos->cabin().isInvalidClass())
    {
      if (needToPrintDiag(cos->bookingCode()))
        printCurrentRBD(cos->bookingCode());

      cabin =
          validateRBDByCabin(cabinInfos1, cos->bookingCode(), tvlDate, tvlDate); // current logic
      if (cabin != nullptr)
      {
        cos->cabin() = cabin->cabin();
      }
      else
      {
        LOG4CXX_ERROR(logger,
                      "RBDByCabinUtil::getCabins() for fake AirSeg CABIN TABLE ERROR:"
                          << "CXR: " << airSeg.carrier() << " " << cos->bookingCode() << " "
                          << airSeg.departureDT().dateToString(DDMMMYYYY, ""));
      }
    }
  }
  finishDiag();
}

// PricingDssResponseHandler OR AtaeResponseHandler
void
RBDByCabinUtil::getCabinsByRbd(AirSeg& airSeg,
                               const std::vector<BookingCode>& bks,
                               std::vector<ClassOfService*>* cosVec)
{
  createDiag187();
  setsUpSegInds(airSeg);
  DateTime tvlDate(airSeg.departureDT());

  if(needToPrintDiagNoRBD())
  {
    printHeader();
    printAirSegSearchData(bks, tvlDate);
  }

  const Cabin* cabin = nullptr;
  const std::vector<RBDByCabinInfo*>& cabinInfos =
                          getRBDByCabin(ATPCO_VENDOR_CODE, airSeg.carrier(), tvlDate);

  if(cabinInfos.empty())
    printRBDByCabinNotFoundForCarrier(airSeg.carrier());

  if(needToPrintDiagNoRBD())
    printRecordHeader();

  bool cabinNotFound = false;

  for (const BookingCode& bookCode : bks)
  {
    ClassOfService* cos = &_trx.dataHandle().safe_create<ClassOfService>();

    if(needToPrintDiag(bookCode))
      printCurrentRBD(bookCode);

    cabin = validateRBDByCabin(cabinInfos, bookCode, tvlDate, tvlDate); // current logic 

    if (cabin != nullptr)
    {
      cos->cabin() = cabin->cabin();
    }
    else
    {
      cos->cabin().setInvalidClass();
      cabinNotFound = true;
    }
    cos->bookingCode() = bookCode;
    cos->numSeats() = 0;
    if (cosVec)
    {
      cosVec->push_back(cos);
    }
    else
    {
      airSeg.classOfService().push_back(cos);
    }
  }

  if(!cabinNotFound)
  {
     finishDiag();
     return;
  }

  std::vector<ClassOfService*>* analizedCOS = &airSeg.classOfService();
  if (cosVec)
    analizedCOS = cosVec;

  if(!cabinInfos.empty() && cabinInfos.front()->carrier() == INDUSTRY_CARRIER)
  {
    for (ClassOfService* cs : *analizedCOS)
    {
      if(cs->cabin().isInvalidClass())
      {
        LOG4CXX_ERROR(logger,
                      "RBDByCabinUtil::getCabins() AirSeg CABIN TABLE ERROR:"
                          << airSeg.carrier() << airSeg.flightNumber() << " " << cs->bookingCode()
                          << airSeg.departureDT().dateToString(DDMMMYYYY, ""));
      }
    }
    finishDiag();
    return;
  }

  if(isCabinSelected())
  {
    finishDiag();
    return;
  }

  const std::vector<RBDByCabinInfo*>& cabinInfos1 =
                          getRBDByCabin(ATPCO_VENDOR_CODE, INDUSTRY_CARRIER, tvlDate);

  if(needToPrintDiagNoRBD())
    printRecordHeader();

  for (ClassOfService* cs : *analizedCOS)
  {
    if(cs->cabin().isInvalidClass())
    {
      if(needToPrintDiag(cs->bookingCode()))
        printCurrentRBD(cs->bookingCode());

      cabin = validateRBDByCabin(cabinInfos1, cs->bookingCode(), tvlDate, tvlDate); // current logic
      if (cabin != nullptr)
      {
        cs->cabin() = cabin->cabin();
      }
      else
      {
        LOG4CXX_ERROR(logger,
                      "RBDByCabinUtil::getCabins() AirSeg CABIN TABLE ERROR:"
                          << airSeg.carrier() << airSeg.flightNumber() << " " << cs->bookingCode()
                          << airSeg.departureDT().dateToString(DDMMMYYYY, ""));
      }
    }
  }
  finishDiag();
}

// Ancillaries, Baggage, ItinAnalyzerServiceShopping, SoldoutCabinValidator
const Cabin*
RBDByCabinUtil::getCabinByRBD(const CarrierCode& carrier,
                              const BookingCode& bookingCode,
                              const AirSeg& seg,
                              bool ancilBagServices,
                              const DateTime date )
{
  createDiag187();
  setsUpSegInds(seg, ancilBagServices);
  DateTime tvlDate(seg.departureDT());

  if(needToPrintDiag(bookingCode))
  {
    printHeader();
    printAirSegSearchData(carrier, bookingCode, tvlDate);
    printRecordHeader();
  }
  const Cabin* cab = findCabinByRBD(carrier, bookingCode, tvlDate, date);

  finishDiag();
  return cab;
}

// FamilyLogicUtil - carrier = YY, RBD = Y
const Cabin*
RBDByCabinUtil::getCabinByRBD(const CarrierCode& carrier,
                              const BookingCode& bookingCode,
                              const DateTime& date)
{
  createDiag187();

  AirSeg seg;
  seg.carrier() = carrier;
  seg.setBookingCode(bookingCode);
  _travelSegs.emplace_back(&seg);
  DateTime tvlDate(date);

  if(needToPrintDiag(bookingCode, carrier))
  {
    printHeader();
    printAirSegSearchData(seg, date);
    printRecordHeader();
  }
  const Cabin* cab = findCabinByRBD(carrier, bookingCode, tvlDate, date);

  finishDiag();
  return cab;
}

// XformClientShoppingXML
const Cabin*
RBDByCabinUtil::getCabinForAirseg(const AirSeg& seg)
{
  createDiag187();
  setsUpSegInds(seg);
  DateTime tvlDate(seg.departureDT());

  if(needToPrintDiag(seg.getBookingCode()))
  {
    printHeader();
    printAirSegSearchData(tvlDate);
    printRecordHeader();
  }
  const Cabin* cabin = findCabinByRBD(seg.carrier(), seg.getBookingCode(), tvlDate, tvlDate);
  finishDiag();

  return cabin;
}

bool
RBDByCabinUtil::needToPrintDiag(const BookingCode& bookingCode,
                                const CarrierCode carrier) const
{
  if(!isProgramCallSelected())
    return false;
  if(!isCpaSelected())
    return false;
  if(!carrier.empty() && !isCxrSelected(carrier))
    return false;
  if(carrier.empty() && !isCxrSelected(CarrierCode()))
    return false;

  if(!bookingCode.empty() && !isBookingCodeSelected(bookingCode))
    return false;

  if(bookingCode.empty() && 
     _airSeg && !isBookingCodeSelected(_airSeg->getBookingCode()))
    return false;

  if(_ptf && !isFareClassOrBasisSelected(*_ptf))
    return false;

  return true;
}

bool
RBDByCabinUtil::needToPrintDiagNoRBD() const
{
  if(!isProgramCallSelected())
    return false;
  if(!isCpaSelected())
    return false;
  if(!isCxrSelected(CarrierCode()))
    return false;

  return true;
}

// PricingModelMap
void
RBDByCabinUtil::getCabinByRBD(AirSeg& seg)
{
  createDiag187();
  setsUpSegInds(seg);

  seg.bookedCabin().setInvalidClass();
  DateTime tvlDate(_trx.adjustedTravelDate(seg.departureDT()));

  if(needToPrintDiagNoRBD())
  {
    printHeader();
    printAirSegSearchData(tvlDate);
    if(!isBookingCodeSelected(seg.getBookingCode()))
      printRBDinRQNotMatchRBDinSeg(seg.getBookingCode());
    else
      printRecordHeader();
  }

  const Cabin* cabin = findCabinByRBD(seg.carrier(), seg.getBookingCode(), tvlDate, tvlDate);
  if(cabin)
  {
    seg.bookedCabin() = cabin->cabin();
    finishDiag();
    return;
  }
  else
  if (seg.segmentType() == Open)
  {
    tvlDate = _trx.dataHandle().ticketDate();

    if(needToPrintDiagNoRBD())
    {
      printAirSegSearchData(tvlDate, true);
      if(!isBookingCodeSelected(seg.getBookingCode()))
        printRBDinRQNotMatchRBDinSeg(seg.getBookingCode());
      else
        printRecordHeader();
    }

    cabin = findCabinByRBD(seg.carrier(), seg.getBookingCode(), tvlDate, tvlDate);
    if (cabin)
      seg.bookedCabin() = cabin->cabin();
  }
  finishDiag();
  return;
}

// BCEV WPNCS
CabinType
RBDByCabinUtil::getCabinByRbdByType(const AirSeg& seg,
                                    const BookingCode& bookingCode,
                                    const PaxTypeFare& paxTypeFare,
                                    const uint32_t& itemNo,
                                    const BookingCodeExceptionSegment& bceSegment)
{
  createDiag187();
  setsUpSegInds(seg);
  _ptf = &paxTypeFare;
  DateTime tvlDate(_trx.adjustedTravelDate(seg.departureDT()));
  if(needToPrintDiagNoRBD() && isFareClassOrBasisSelected(paxTypeFare))
  {
    printHeader();
    printAirSegSearchData(bookingCode, paxTypeFare, itemNo, bceSegment, tvlDate);

    if( !isBookingCodeSelected(bookingCode))
      printRBDinRQNotMatchRBDinSeg(bookingCode);
    else
      printRecordHeader();
  }

  CabinType cb;
  cb.setInvalidClass();

  const Cabin* cabin = findCabinByRBD(seg.carrier(), bookingCode, tvlDate, tvlDate);
  if(cabin)
  {
    finishDiag();
    return cabin->cabin();
  }
  else
  if (seg.segmentType() == Open)
  {
    // for OPEN segments try to get the cabin value using todays date
    tvlDate = _trx.transactionStartTime();

    if(needToPrintDiagNoRBD() && isFareClassOrBasisSelected(paxTypeFare))
    {
      printAirSegSearchData(bookingCode, paxTypeFare, itemNo, bceSegment, tvlDate, true );

      if( !isBookingCodeSelected(bookingCode))
        printRBDinRQNotMatchRBDinSeg(bookingCode);
      else
        printRecordHeader();
    }

    cabin = findCabinByRBD(seg.carrier(), bookingCode, tvlDate, tvlDate);
    if (cabin)
      cb = cabin->cabin();
  }
  finishDiag();
  return cb;
}

const Cabin*
RBDByCabinUtil::findCabinByRBD(const CarrierCode& carrier,
                               const BookingCode& bookingCode,
                               DateTime& tvlDate,
                               const DateTime& ticketIssueDate)
{
  // Gets carrier's or YY's Rbd/Cabin info
  const std::vector<RBDByCabinInfo*>& cabinInfos =
                          getRBDByCabin(ATPCO_VENDOR_CODE, carrier, tvlDate);

  const Cabin* cabin = validateRBDByCabin(cabinInfos, bookingCode, tvlDate, ticketIssueDate);

  if(cabin)
    return cabin;

  // No-match or not found
  // carrier data match to Sequence but RBD does not found in this sequence
  // Need to gets YY carrier Rbd/Cabin info
  const std::vector<RBDByCabinInfo*>& cabinInfos1 = 
                          getRBDByCabin(ATPCO_VENDOR_CODE, INDUSTRY_CARRIER, tvlDate);

  cabin = validateRBDByCabin(cabinInfos1, bookingCode, tvlDate, ticketIssueDate);
  if(cabin)
    return cabin;

  return nullptr; //there is an option to return prepareCabin(bookingCode);
}

const Cabin*
RBDByCabinUtil::validateRBDByCabin(const std::vector<RBDByCabinInfo*>& cabinInfos,
                                   const BookingCode& bookingCode,
                                   const DateTime& tvlDate,
                                   const DateTime& ticketIssueDate)
{
  for (RBDByCabinInfo* rbdByCabin : cabinInfos)
  {
    if (!isSequenceSelected(rbdByCabin))
       continue;

    if(needToPrintDiag(bookingCode))
      storeRbdByCabinForDiag(rbdByCabin);
// temporary code to SKIP sequence with the FLT 1&2 and EQUIPMENT coded.
// It'll be in scope till Shopping HPS is ready (in 2016)
    if(fallback::fallbackRBDByCabinPh2(&_trx) &&
       isFlightNumberOrEquipmentCoded(rbdByCabin))
    {
       printFailStatus(SKIP_SEQ, bookingCode);
       continue;
    }

    if(!checkTravelDate(*rbdByCabin, tvlDate))
    {
       printFailStatus(FAIL_TRAVEL_DATE, bookingCode);
       continue;
    }
    DateTime ticketIssuanceDate(ticketIssueDate);
    if (ticketIssueDate.isEmptyDate())
    {
      ticketIssuanceDate = _trx.ticketingDate();
    }

    const DateTime& ticketDate =
        _ancilBagServices ? ticketIssuanceDate
                          : (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
                                ? static_cast<const RexPricingTrx&>(_trx).originalTktIssueDT()
                                : _trx.ticketingDate();

    if (!checkTicketDate(*rbdByCabin, ticketDate))
    {
       printFailStatus(FAIL_TICKET_DATE, bookingCode);
       continue;
    }
    if(!checkGlobalInd(*rbdByCabin))  // global direction
    {
       printFailStatus(FAIL_GLOBAL_IND, bookingCode);
       continue;
    }
    if(!checkLocations(*rbdByCabin))
    {
       printFailStatus(FAIL_GEO_LOC, bookingCode);
       continue;
    }
    // Check for flights and equipment type
    if(!fallback::fallbackRBDByCabinPh2(&_trx) &&
       !_travelSegs.empty())
    {
       if (!checkFlights(*rbdByCabin))
       {
         printFailStatus(FAIL_FLIGHT, bookingCode);
         continue;
       }
       if (!checkEquipment(*rbdByCabin))
       {
         printFailStatus(FAIL_EQUP, bookingCode);
         continue;
       }
    }

    bool defaultCabin = false;
    Indicator RBDcabin = fallback::fallbackRBDByCabinOpt(&_trx) ?
        getRBDCabin_old(*rbdByCabin, bookingCode) :
        getRBDCabin(*rbdByCabin, bookingCode);
    if(RBDcabin == CHAR_BLANK)
    {
      printCabinNotFoundForRBD(bookingCode);
      if(rbdByCabin->carrier() != INDUSTRY_CARRIER)
      {
         if(_ancilBagServices)
         {
           printAncBagRBDNotFiledInCxrRbdByCabin(*rbdByCabin, bookingCode);
           return nullptr; // ATPCO DATA RBDBYCABIN match Fixed portion, RBD not filed
         }
         break; // will continue with YY carrier
      }
      else  // INDUSTRY_CARRIER
      {
        printDefaultEconomyCabin(bookingCode);
        defaultCabin = true;
        RBDcabin = 'Y';  // Economy cabin
      }
    }
    printCabinFoundForRBD(bookingCode, RBDcabin, rbdByCabin->carrier(), defaultCabin);
    return prepareCabin(bookingCode, RBDcabin);
  }
  if(!cabinInfos.empty() && cabinInfos.front()->carrier() == INDUSTRY_CARRIER)
  {
    const Cabin* cabin = prepareCabin(bookingCode);
    printCabinFoundForRBD(bookingCode, 'Y', INDUSTRY_CARRIER);

    return cabin;;
  }
  if(!cabinInfos.empty())
    printFailFoundCabinForRBD(bookingCode, cabinInfos.front()->carrier());

  return nullptr;
}

void
RBDByCabinUtil::storeRbdByCabinForDiag(const RBDByCabinInfo* rbdByCabin)
{
  _cabinInfoForDiag = rbdByCabin;
}

const Cabin*
RBDByCabinUtil::prepareCabin(const BookingCode& bookingCode, const Indicator RBDcabin) const
{
  Cabin* cabin = &_trx.dataHandle().safe_create<Cabin>();

  if(RBDcabin == CHAR_BLANK)
    cabin->cabin().setClassFromAlphaNumAnswer('Y');
  else
    cabin->cabin().setClassFromAlphaNumAnswer(RBDcabin);
  cabin->classOfService() = bookingCode;

  return cabin;
}

Indicator
RBDByCabinUtil::getRBDCabin_old(const RBDByCabinInfo& rbdInfo,
                                const BookingCode& bookingCode) const
{
  const std::string& cabinReq = _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CABIN);
  if(!rbdInfo.bookingCodeCabinMap().empty())
  {
    for (const auto& elem : rbdInfo.bookingCodeCabinMap())
     {
       if (elem.first == bookingCode)
       {
         if (!cabinReq.empty() && strcmp(cabinReq.c_str(), &(elem.second)) != 0)
         {
           printReqCabinNotMatch(*(cabinReq.c_str()), bookingCode);
           continue;
         }
         return elem.second;
       }
     }
  }
  return CHAR_BLANK;
}

Indicator
RBDByCabinUtil::getRBDCabin(const RBDByCabinInfo& rbdInfo,
                            const BookingCode& bookingCode) const
{
  const std::string& cabinReq = _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CABIN);
  const auto elem = rbdInfo.bookingCodeCabinMap().find(bookingCode);
  if (elem != rbdInfo.bookingCodeCabinMap().end())
  {
    if (!cabinReq.empty() && cabinReq[0] != elem->second)
      printReqCabinNotMatch(cabinReq[0], bookingCode);
    else
      return elem->second;
  }
  return CHAR_BLANK;
}
void
RBDByCabinUtil::invoke187Diagnostic()
{
  if (_diag == nullptr || _trx.diagnostic().diagnosticType() != Diagnostic187)
    return;

  const std::string& carrierReq = _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);

  if(!_trx.diagnostic().diagParamMapItem(Diagnostic::CITY_PAIR).empty() &&
      _trx.diagnostic().diagParamMapItem(Diagnostic::CITY_PAIR).size() < 6)
    _trx.diagnostic().diagParamMap()[Diagnostic::CITY_PAIR] = "";

  const std::string& cityPair = _trx.diagnostic().diagParamMapItem(Diagnostic::CITY_PAIR);

  _diag->printHeader(carrierReq, cityPair, _trx.dataHandle().ticketDate());

  // call RBD By Cabin for selected carrier
  if(!carrierReq.empty())
  {
     printRBDByCabin(carrierReq);
  }
  else  // call RBD By Cabin for all carriers in the itinerary
  if(_trx.travelSeg().size() > 0)
  {
    std::set<CarrierCode> cxrs;
    for (TravelSeg* tvl : _trx.travelSeg())
    {
      const AirSeg* airSeg(nullptr);
      if (tvl->isAir())
      {
          airSeg = static_cast<const AirSeg*>(tvl);
      }

      if(!airSeg)
        continue;

      if(!cityPair.empty())
      {
        const std::string boardCity(cityPair.substr(0, 3));
        const std::string   offCity(cityPair.substr(3, 3));
        if((boardCity != airSeg->boardMultiCity() && boardCity != airSeg->origAirport()) ||
           (offCity != airSeg->offMultiCity() && offCity != airSeg->destAirport())   )
          continue;
      }
      cxrs.insert(airSeg->carrier());
      cxrs.insert(airSeg->operatingCarrierCode());
    }
    cxrs.insert(INDUSTRY_CARRIER);

    for (CarrierCode cxr : cxrs)
    {
      if(cxr.empty())
        continue;
      printRBDByCabin(cxr);
    }
  }
  else
  {
    // reserved for the request w/o travel segments
  }
  _diag->lineSkip( 1 );
  _diag->printFooter();
}

void
RBDByCabinUtil::printRBDByCabin(CarrierCode carrier)
{
  const std::string& vendorReq = _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_VENDOR);
  const std::string& sequenceNo = _trx.diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER);
  const std::string& cityPair = _trx.diagnostic().diagParamMapItem(Diagnostic::CITY_PAIR);
  // travel date
  //  const std::string& travelDate = trx.diagnostic().diagParamMapItem(Diagnostic::TRAVEL_DATE);
  // ticket date
  //  const std::string& ticketDate = trx.diagnostic().diagParamMapItem(Diagnostic::TICKET_DATE);

  DateTime tvlDate(_trx.dataHandle().ticketDate());
  // call RBD By Cabin for data
  const std::vector<RBDByCabinInfo*>& cabinInfos =
               getRBDByCabin((vendorReq.empty()? ATPCO_VENDOR_CODE : vendorReq), carrier, tvlDate);

  std::vector<RBDByCabinInfo*>::const_iterator cI = cabinInfos.begin();
  std::vector<RBDByCabinInfo*>::const_iterator cIE = cabinInfos.end();
  for(; cI != cIE; ++ cI)
  {
    RBDByCabinInfo* rbdByCabin = *cI;
    if(!cityPair.empty())
    {
      bool cpFound = false;
      std::vector<TravelSeg*>::const_iterator tvI = _trx.travelSeg().begin();
      for(; tvI != _trx.travelSeg().end(); ++tvI)
      {
        TravelSeg* tvl = *tvI;
        const AirSeg* airSeg(nullptr);
        if (tvl->isAir())
        {
          airSeg = static_cast<const AirSeg*>(tvl);
        }
        if(!airSeg)
          continue;

        const std::string boardCity(cityPair.substr(0, 3));
        const std::string   offCity(cityPair.substr(3, 3));
        if((boardCity != airSeg->boardMultiCity() && boardCity != airSeg->origAirport()) ||
           (offCity != airSeg->offMultiCity() && offCity != airSeg->destAirport())   )
          continue;

        setsUpSegInds(*airSeg);
        if(!checkTravelDate(*rbdByCabin, airSeg->departureDT()))
          continue;
        cpFound = true;
      }
      if(!cpFound)
       continue;
    }
    if(!checkTicketDate(*rbdByCabin, _trx.ticketingDate()))
      continue;
    if( sequenceNo.empty() ||
       (!sequenceNo.empty() &&
       (rbdByCabin->sequenceNo() ==
        (uint32_t)atoi(_trx.diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER).c_str()))))
      _diag->printRbdByCabinInfo(_trx, rbdByCabin);
  }
}

bool
RBDByCabinUtil::checkTravelDate(const RBDByCabinInfo& rbdInfo, const DateTime& tvlDate) const
{
  return (tvlDate <= rbdInfo.discDate() &&
          tvlDate >= rbdInfo.effDate());
}

bool
RBDByCabinUtil::checkTicketDate(const RBDByCabinInfo& rbdInfo,
                                const DateTime& ticketDate) const
{
  return (ticketDate <= rbdInfo.lastTicketDate() &&
          ticketDate >= rbdInfo.firstTicketDate());
}

bool
RBDByCabinUtil::checkGlobalInd(const RBDByCabinInfo& rbdInfo)
{
  if(rbdInfo.globalDir().empty())
    return true;

  if(_ptf && !_segmentVSfareComp)
    return rbdInfo.globalDir() == *(globalDirectionToStr(_ptf->fareMarket()->getGlobalDirection()));

  if(_airSeg)
  {
    TravelSeg* tvl = const_cast<AirSeg*>(_airSeg);
    if (_airSeg->globalDirection() == GlobalDirection::NO_DIR)
    {
      GlobalDirection gd;

      std::vector<TravelSeg*> tvlSegs = {tvl};
      // find the global direction
      GlobalDirectionFinderV2Adapter::getGlobalDirection(
          &_trx, _airSeg->departureDT(), tvlSegs, gd);
      tvl->globalDirection() = gd;
    }
    return rbdInfo.globalDir() == *(globalDirectionToStr(tvl->globalDirection()));
  }
  return false;
}

bool
RBDByCabinUtil::checkLocations(const RBDByCabinInfo& rbdInfo)
{
  if(rbdInfo.locKey1().isNull() && rbdInfo.locKey2().isNull())
    return true;
  if(rbdInfo.locKey1().isNull() && !rbdInfo.locKey2().isNull()) // error condition
    return false;

  const Loc* orig = nullptr;
  const Loc* dest = nullptr;
  if(_airSeg && _segmentVSfareComp)
  {
    orig = _airSeg->origin();
    dest = _airSeg->destination();
  }
  else
  if(_ptf)
  {
    orig = _ptf->fareMarket()->travelSeg().front()->origin();
    dest = _ptf->fareMarket()->travelSeg().back()->destination();
  }
  if(!rbdInfo.locKey1().isNull() && rbdInfo.locKey2().isNull()) // LOC1 coded only
    return validateLoc1(rbdInfo, *orig, *dest);

  return validateBothLocs(rbdInfo, *orig, *dest); // LOC1 && LOC2 coded
}

bool
RBDByCabinUtil::checkFlights(const RBDByCabinInfo& rbdInfo)
{
  if(rbdInfo.flightNo1() == 0 && rbdInfo.flightNo2() == 0)
    return true;

  if(rbdInfo.flightNo1() == 0 && rbdInfo.flightNo2() != 0)
    return false;

  if(rbdInfo.flightNo1() != 0 && rbdInfo.flightNo2() != 0 &&
     rbdInfo.flightNo1() > rbdInfo.flightNo2() )
    return false;

  for(const TravelSeg* tvl : _travelSegs)
  {
    const AirSeg* airSeg(nullptr);
    if (tvl->isAir())
    {
      airSeg = static_cast<const AirSeg*>(tvl);
    }
    if(!airSeg)
      continue;

    if ((rbdInfo.flightNo2() == 0 && airSeg->flightNumber() != rbdInfo.flightNo1()) ||
        (rbdInfo.flightNo2() != 0 && (airSeg->flightNumber() < rbdInfo.flightNo1() ||
                                      airSeg->flightNumber() > rbdInfo.flightNo2())  ))
      return false;
  }
  return true;
}

bool
RBDByCabinUtil::checkEquipment(const RBDByCabinInfo& rbdInfo)
{
  if (rbdInfo.equipmentType().empty() || rbdInfo.equipmentType() == BLANK_CODE)
    return true;

  for(const TravelSeg* tvl : _travelSegs)
  {
    if (tvl->equipmentType() != rbdInfo.equipmentType())
      return false;
  }
  return true;
}

GeoTravelType
RBDByCabinUtil::setsUpGeoTravelType()
{
  if(_airSeg && _segmentVSfareComp)
  {
    TravelSeg* tvl = const_cast<AirSeg*>(_airSeg);
    if(_airSeg->geoTravelType() == GeoTravelType::UnknownGeoTravelType)
    {
       TravelSegAnalysis tlvSegAnalysis;
       tlvSegAnalysis.setGeoTravelType(tvl);
    }

    if(tvl->geoTravelType() == GeoTravelType::International || tvl->geoTravelType() == GeoTravelType::ForeignDomestic)
      return GeoTravelType::International;
    else
      return GeoTravelType::Domestic;
  }
  else
  {
    if(_ptf)
     return _ptf->fareMarket()->geoTravelType();
  }
  return GeoTravelType::UnknownGeoTravelType;
}

bool
RBDByCabinUtil::validateLoc1(const RBDByCabinInfo& rbdInfo, const Loc& orig, const Loc& dest)
{
  return (validateLocation(rbdInfo.vendor(),
                           rbdInfo.locKey1(),
                           orig,
                           rbdInfo.carrier())
          ||
          validateLocation(rbdInfo.vendor(),
                           rbdInfo.locKey1(),
                           dest,
                           rbdInfo.carrier()));
}

bool
RBDByCabinUtil::validateBothLocs(const RBDByCabinInfo& rbdInfo, const Loc& orig, const Loc& dest)
{
  return ( (validateLocation(rbdInfo.vendor(),
                             rbdInfo.locKey1(),
                             orig,
                             rbdInfo.carrier())
            &&
            validateLocation(rbdInfo.vendor(),
                             rbdInfo.locKey2(),
                             dest,
                             rbdInfo.carrier()))
         ||
           (validateLocation(rbdInfo.vendor(),
                             rbdInfo.locKey2(),
                             orig,
                             rbdInfo.carrier())
            &&
            validateLocation(rbdInfo.vendor(),
                             rbdInfo.locKey1(),
                             dest,
                             rbdInfo.carrier())));
}

bool
RBDByCabinUtil::validateLocation(const VendorCode& vendor,
                                 const LocKey& locKey,
                                 const Loc& loc,
                                 CarrierCode carrier)
{
  if (locKey.isNull())
    return false;

  if (locKey.locType() == LOCTYPE_USER)
    return isInZone(vendor, locKey.loc(), loc, carrier);

  return isInLoc(vendor, locKey, loc, carrier);
}

bool
RBDByCabinUtil::isInZone(const VendorCode& vendor,
                         const LocCode& zone,
                         const Loc& loc,
                         CarrierCode carrier)
{
  return LocUtil::isInZone(loc,
                           vendor,
                           zone,
                           TAX_ZONE,
                           LocUtil::OTHER,
                           setsUpGeoTravelType(),
                           carrier,
                           _trx.ticketingDate());
}

bool
RBDByCabinUtil::isInLoc(const VendorCode& vendor,
                        const LocKey& locKey,
                        const Loc& loc,
                        CarrierCode carrier)
{
  return LocUtil::isInLoc(loc,
                          locKey.locType(),
                          locKey.loc(),
                          vendor,
                          RESERVED,
                          LocUtil::OTHER,
                          setsUpGeoTravelType(),
                          carrier,
                          _trx.ticketingDate());
}

const std::vector<RBDByCabinInfo*>&
RBDByCabinUtil::getRBDByCabin(const VendorCode& vendor,
                              const CarrierCode& cxr,
                              DateTime& tvlDate)
{
  CarrierCode carrier(cxr);
  if(carrier == ANY_CARRIER)
    carrier = INDUSTRY_CARRIER;

  DateTime tvlDateModified(tvlDate);
  if (tvlDate.isEmptyDate())
  {
    tvlDateModified = _trx.dataHandle().ticketDate();
  }
  const std::vector<RBDByCabinInfo*>& vc = _trx.dataHandle().getRBDByCabin(vendor, carrier, tvlDateModified);
  if(vc.empty())
  {
    printRBDByCabinNotFoundForCarrier(carrier);
    return _trx.dataHandle().getRBDByCabin(vendor, INDUSTRY_CARRIER, tvlDateModified);
  }
  return vc;
}

void
RBDByCabinUtil::createDiag187()
{
  if (_trx.diagnostic().diagnosticType() != Diagnostic187)
  {
    if(_trx.getRequest()->diagnosticNumber() == Diagnostic187 &&
       ( _call == PRICING_RQ || _call == ANC_BAG_RQ || _call == PREFERRED_CABIN ||
         _call == SHP_HANDLER || _call == SHP_WPNI ))
    {
      std::string noValue;
      std::map<std::string, std::string>& argMap = _trx.diagnostic().diagParamMap();
      std::vector<std::string>::const_iterator diagArgI = _trx.getRequest()->diagArgData().begin();
      const std::vector<std::string>::const_iterator diagArgIEnd = _trx.getRequest()->diagArgData().end();

      for (; diagArgI != diagArgIEnd; diagArgI++)
      {
        if (*diagArgI == "ALLRBD")
        {
          return;
        }

        size_t len = (*diagArgI).length();
        if (len > 2)
        {
           argMap[(*diagArgI).substr(0, 2)] = (*diagArgI).substr(2);
        }
        else if (len == 2)
        {
           argMap[(*diagArgI).substr(0, 2)] = noValue;
        }
      }
      const int16_t diagnosticNumber = _trx.getRequest()->diagnosticNumber();

      _trx.diagnostic().diagnosticType() = static_cast<DiagnosticTypes>(diagnosticNumber);
      _trx.diagnostic().activate();
    }
    else
      return;
  }
  if (_trx.diagnostic().diagnosticType() == Diagnostic187 &&
      _trx.diagnostic().diagParamMapItem(Diagnostic::RBD_ALL) != "LRBD")
  {
    if(!_diag)
    {
      DCFactory* factory = DCFactory::instance();
      _diag = dynamic_cast<Diag187Collector*>(factory->create(_trx));
      if (!_diag)
       return;

      _diag->enable(Diagnostic187);
    }
  }
}

void
RBDByCabinUtil::printHeader() const
{
  if (!_diag)
    return;

    _diag->printHeader(_call);
}

void
RBDByCabinUtil::printRecordHeader() const
{
  if (!_diag)
    return;

  if(_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO")
    _diag->printRecordHeader();
  else
    _diag->printRecordHeaderShort();
}

void
RBDByCabinUtil::printAirSegSearchData(const AirSeg& seg, const DateTime& tvlDate, bool open)
{
  if (!_diag)
    return;

  _diag->printAirSegData(seg, tvlDate, _trx.dataHandle().ticketDate(), open);
}

void
RBDByCabinUtil::printAirSegSearchData(const DateTime& tvlDate, bool open)
{
  if (!_diag)
    return;

  _diag->printAirSegData(*_airSeg, tvlDate, _trx.dataHandle().ticketDate(), open);
}

void
RBDByCabinUtil::printAirSegSearchData(const CarrierCode& carrier,
                                      const BookingCode& bookingCode,
                                      const DateTime& tvlDate,
                                      bool open)
{
  if (!_diag)
    return;

  std::vector<BookingCode> bks;
  _diag->printAirSegData(*_airSeg, carrier, bookingCode, tvlDate, _trx.dataHandle().ticketDate(), open, bks);
}

void
RBDByCabinUtil::printAirSegSearchData(const std::vector<BookingCode>& bks,
                                      const DateTime& tvlDate)
{
  if (!_diag)
    return;

  _diag->printAirSegData(*_airSeg, tvlDate, bks, _trx.dataHandle().ticketDate());
}

void
RBDByCabinUtil::printAirSegSearchData(std::vector<ClassOfService*>& classOfService,
                                      const DateTime& tvlDate)
{
  if (!_diag)
    return;

  std::vector<BookingCode> bks;
  std::vector<ClassOfService*>::iterator cI = classOfService.begin();
  std::vector<ClassOfService*>::iterator cE = classOfService.end();
  for (; cI != cE; ++cI)
  {
    ClassOfService& cs = **cI;
    bks.push_back(cs.bookingCode());
  }

  _diag->printAirSegData(*_airSeg, tvlDate, bks, _trx.dataHandle().ticketDate());
}

void
RBDByCabinUtil::printAirSegSearchData(const PaxTypeFare& ptf,
                                      std::vector<ClassOfService>& classOfService)
{
  if (!_diag)
    return;

  std::vector<BookingCode> bks;
  std::vector<ClassOfService>::iterator cI = classOfService.begin();
  std::vector<ClassOfService>::iterator cE = classOfService.end();
  for (; cI != cE; ++cI)
  {
    ClassOfService& cs = *cI;
    bks.push_back(cs.bookingCode());
  }

  _diag->printAirSegData(ptf, ptf.fareMarket()->travelDate(), bks, _trx.dataHandle().ticketDate());
}

void
RBDByCabinUtil::printAirSegSearchData(const BookingCode& bookingCode,
                                      const PaxTypeFare& paxTypeFare,
                                      const uint32_t& itemNo,
                                      const BookingCodeExceptionSegment& bceSegment,
                                      const DateTime& tvlDate,
                                      bool open)
{
  if (!_diag)
    return;

  if(!_airSeg)
    return;

  _diag->printAirSegData(*_airSeg, bookingCode, tvlDate, paxTypeFare, itemNo,
                          bceSegment, _trx.dataHandle().ticketDate(), open);
}

void
RBDByCabinUtil::printCurrentRBD(const BookingCode& bookingCode)
{
  if (!_diag)
    return;

  _diag->printCurrentRBD(bookingCode);
}

void
RBDByCabinUtil::printRbdByCabinInfo(const RBDByCabinInfo* rbdByCabin) const
{
  if (!_diag)
    return;

  if(_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO")
    _diag->printRbdByCabinInfo(_trx, rbdByCabin, true);
  else
    _diag->printRbdByCabinInfoShort(rbdByCabin);
}

void
RBDByCabinUtil::printReqCabinNotMatch(const Indicator& cabin,
                                      const BookingCode& bookingCode) const
{
  if (!_diag)
    return;

  if(!needToPrintDiag(bookingCode))
    return;

  if(_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "PASSED")
    return;

  _diag->printReqCabinNotMatch(cabin, bookingCode);
}

void
RBDByCabinUtil::printDefaultEconomyCabin(const BookingCode& bookingCode) const
{
  if (!_diag)
    return;

  if(!needToPrintDiag(bookingCode))
    return;

  if(_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "PASSED")
    return;

//  if(_cabinInfoForDiag)
//    printRbdByCabinInfo(_cabinInfoForDiag);

  _diag->printDefaultEconomyCabin();
}

void
RBDByCabinUtil::printCabinNotFoundForRBD(const BookingCode& bookingCode) const
{
  if (!_diag)
    return;

  if(!needToPrintDiag(bookingCode))
    return;

  if(_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "PASSED")
    return;

  if(_cabinInfoForDiag)
    printRbdByCabinInfo(_cabinInfoForDiag);

  _diag->printCabinNotFoundForRBD(bookingCode);
}

void
RBDByCabinUtil::printCabinFoundForRBD(const BookingCode& bookingCode,
                                      const Indicator& cabin,
                                      const CarrierCode& carrier,
                                      bool defaultCabin) const
{
  if (!_diag)
    return;

  if(!needToPrintDiag(bookingCode))
    return;

  if(_cabinInfoForDiag && !defaultCabin)
    printRbdByCabinInfo(_cabinInfoForDiag);

  if(_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "INFO" && !defaultCabin)
  {
    _diag->printStatus(PASS_SEQ);
  }
  _diag->printCabinFoundForRBD(bookingCode, cabin, carrier, defaultCabin);
}

void
RBDByCabinUtil::printFailFoundCabinForRBD(const BookingCode& bookingCode,
                                          const CarrierCode& carrier) const
{
  if (!_diag)
    return;

  if(!needToPrintDiag(bookingCode))
    return;

  if(_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "PASSED")
    return;
//  if(_cabinInfoForDiag)
//    printRbdByCabinInfo(_cabinInfoForDiag);

  _diag->printFailFoundCabinForRBD(bookingCode, carrier);
}

void
RBDByCabinUtil::printRBDByCabinNotFoundForCarrier(const CarrierCode& carrier) const
{
  if (!_diag)
    return;

  _diag->printRBDByCabinNotFoundForCarrier(carrier);
}

void
RBDByCabinUtil::printRBDinRQNotMatchRBDinSeg(const BookingCode& bookingCode) const
{
  if (!_diag)
    return;

  const std::string& diagRbd = _trx.diagnostic().diagParamMapItem(Diagnostic::BOOKING_CODE);
  if(diagRbd.empty())
    return;

  _diag->printRBDinRQNotMatchRBDinSeg(diagRbd, bookingCode);
}

void
RBDByCabinUtil::printAncBagRBDNotFiledInCxrRbdByCabin(const RBDByCabinInfo& rbdInfo,
                                                      const BookingCode& bookingCode) const
{
  if (!_diag)
    return;

  if(_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "PASSED")
    return;

  if(_cabinInfoForDiag)
    printRbdByCabinInfo(_cabinInfoForDiag);

  _diag->printAncBagRBDNotFiledInCxrRbdByCabin(rbdInfo, bookingCode);
}

void
RBDByCabinUtil::printFailStatus(const RBDByCabinStatus& status, const BookingCode& bookingCode) const
{
  if (!_diag)
    return;

  if(!needToPrintDiag(bookingCode))
    return;

  if(_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "PASSED")
    return;

  if(_cabinInfoForDiag)
    printRbdByCabinInfo(_cabinInfoForDiag);

  if(_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO")
    _diag->printFailStatus(status);
  else
    _diag->printFailStatusShort(status);
}

bool
RBDByCabinUtil::isProgramCallSelected() const
{
  if (!_diag)
    return true;

  const std::string& progCall = _trx.diagnostic().diagParamMapItem(Diagnostic::PROGRAM_NAME);

  if(progCall.empty())
   return true;
// Current possible call values
  if((progCall == "PRCREQ" && _call == PRICING_RQ) ||
     (progCall == "SHPOBRT" && _call == ITIN_SHP_PYA) ||
     (progCall == "PRCOPEN" && _call == CONTENT_SVC ) ||
     (progCall == "AVFAKE" && _call == CONTENT_SVC_FAKE) ||
     (progCall == "DSS" && _call == DSS_RSP) ||
     (progCall == "ASV2" && _call == AVAIL_RSP) ||
     (progCall == "WPNCS" && _call == T999_VAL) ||
     (progCall == "PRBYCAB" && _call == RBD_VAL) ||
     (progCall == "CAT31" && _call == RBD_CAT31) ||
     (progCall == "OPTIONAL" && _call == OPTIONAL_SVC) ||
     (progCall == "ANCREQ" && _call == ANC_BAG_RQ) ||
     (progCall == "BAGREQ" && _call == ANC_BAG_RQ) ||
     (progCall == "SHPREQ" && _call == SHP_HANDLER) ||
     (progCall == "SHPPREFCAB" && _call == PREFERRED_CABIN) ||
     (progCall == "WPNI" && _call == SHP_WPNI) ||
     (progCall == "SHPITIN" && _call == ITIN_SHP_SVC) ||
     (progCall == "SHPCAL" && _call == SOLD_OUT) ||
     (progCall == "SHPFAMILY" && _call == FAMILY_LOGIC) )
    return true;

  return false;
}

bool
RBDByCabinUtil::isCxrSelected(const CarrierCode& cxr) const
{
  if (!_diag)
    return true;

  const std::string& carrierReq = _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);

  if(carrierReq.empty())
    return true;

  if(cxr != CarrierCode())
    return (cxr == carrierReq);

  if(_ptf && !_segmentVSfareComp)
    return (_ptf->fareMarket()->governingCarrier() == carrierReq);

  if(_airSeg && _segmentVSfareComp)
    return (_airSeg->carrier() == carrierReq);

  return false;
}

bool
RBDByCabinUtil::isCpaSelected() const
{
  if (!_diag)
    return true;

  if(!_trx.diagnostic().diagParamMapItem(Diagnostic::CITY_PAIR).empty() &&
      _trx.diagnostic().diagParamMapItem(Diagnostic::CITY_PAIR).size() < 6)
    _trx.diagnostic().diagParamMap()[Diagnostic::CITY_PAIR] = "";

  const std::string& cityPair = _trx.diagnostic().diagParamMapItem(Diagnostic::CITY_PAIR);
  if(cityPair.empty())
    return true;

  const std::string boardCity(cityPair.substr(0, 3));
  const std::string   offCity(cityPair.substr(3, 3));

  if(_ptf && !_segmentVSfareComp)
  {
    const AirSeg* airSegOrig(nullptr);
    const AirSeg* airSegDest(nullptr);

    std::vector<TravelSeg*>::const_iterator tvI = _ptf->fareMarket()->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator tvE = _ptf->fareMarket()->travelSeg().end();

    for(; tvI != tvE; ++tvI)
    {
      TravelSeg* tvl = *tvI;

      if(tvl->isAir())
      {
        airSegOrig = static_cast<const AirSeg*>(tvl);
      }
      if(!airSegOrig)
        continue;
      break;
    }
    if(!airSegOrig)
      return false;

    std::vector<TravelSeg*>::const_reverse_iterator tvIr = _ptf->fareMarket()->travelSeg().rbegin();
    std::vector<TravelSeg*>::const_reverse_iterator tvEr = _ptf->fareMarket()->travelSeg().rend();
    for(; tvIr != tvEr; ++tvIr)
    {
      TravelSeg* tvl = *tvIr;

      if(tvl->isAir())
      {
        airSegDest = static_cast<const AirSeg*>(tvl);
      }
      if(!airSegDest)
        continue;
      break;
    }
    if(!airSegDest)
      return false;

    if((boardCity != airSegOrig->boardMultiCity() &&
        boardCity != airSegOrig->origAirport()) ||
       (offCity != airSegDest->offMultiCity() &&
        offCity != airSegDest->destAirport())      )
      return false;
  }
  else
  if(_airSeg &&
       ((boardCity != _airSeg->boardMultiCity() && boardCity != _airSeg->origAirport()) ||
        (offCity != _airSeg->offMultiCity() && offCity != _airSeg->destAirport())   )     )
    return false;

  return true;
}

bool
RBDByCabinUtil::isBookingCodeSelected(const BookingCode& bookCode) const
{
  if (!_diag)
    return true;

  const std::string& diagRbd = _trx.diagnostic().diagParamMapItem(Diagnostic::BOOKING_CODE);
  if(!diagRbd.empty() && diagRbd != bookCode)
    return false;

  return true;
}

bool
RBDByCabinUtil::isCabinSelected()
{
  if (!_diag)
    return false;

  const std::string& cabinChar = _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CABIN);
  if(!cabinChar.empty())
  {
    finishDiag();
    return true;
  }
  return false;
}

bool
RBDByCabinUtil::isSequenceSelected(const RBDByCabinInfo* rbdByCabin) const
{
  if (!_diag)
    return true;

  const std::string& sequenceNo = _trx.diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER);

  if( !sequenceNo.empty() &&
      (rbdByCabin->sequenceNo() !=
       (uint32_t)atoi(_trx.diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER).c_str())))
    return false;

  return true;
}

bool
RBDByCabinUtil::isFlightNumberOrEquipmentCoded(const RBDByCabinInfo* rbdByCabin) const
{
  if (rbdByCabin->flightNo1() != 0 || !rbdByCabin->equipmentType().empty())
    return true;

  return false;
}

bool
RBDByCabinUtil::isFareClassOrBasisSelected(const PaxTypeFare& ptf) const
{
  if (!_diag)
    return true;

  typedef std::map<std::string, std::string>::const_iterator DiagParamMapVecIC;

  DiagParamMapVecIC endD = _trx.diagnostic().diagParamMap().end();
  DiagParamMapVecIC beginD = _trx.diagnostic().diagParamMap().find(Diagnostic::FARE_CLASS_CODE);

  size_t len = 0;

  // process /FCY26
  if (beginD != endD)
  {
    len = ((*beginD).second).size();
    if (len != 0)
    {
      if (((*beginD).second).substr(0, len) != ptf.fareClass())
        return false;
    }
  }

  // process /FBY26
  beginD = _trx.diagnostic().diagParamMap().find(Diagnostic::FARE_BASIS_CODE);

  if (beginD != endD)
  {
    len = ((*beginD).second).size();
    if (len != 0)
    {
      if (((*beginD).second).substr(0, len) != ptf.createFareBasis(&_trx, false))
        return false;
    }
  }
  return true;
}

void
RBDByCabinUtil::finishDiag()
{
  if (!_diag)
    return;

  _diag->flushMsg();
}

} // end namespace tse
