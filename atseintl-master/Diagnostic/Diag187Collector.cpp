//----------------------------------------------------------------------------
//
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
#include "Common/CabinType.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/BookingCodeExceptionSegment.h"
#include "DBAccess/RBDByCabinInfo.h"
#include "Diagnostic/Diag187Collector.h"
#include "Diagnostic/DiagnosticUtil.h"

#include <iostream>
#include <iomanip>

namespace tse
{
namespace
{
const std::string title = "*****************  RBD BY CABIN DIAGNOSTIC  *****************\n";
const std::string title1 = "*-----------------------------------------------------------*\n";
const std::string diag_name = " RBD BY CABIN FOR CARRIER: ";
const std::string city_pair = "  CITY PAIR: ";

const std::string text_qualif1 = " AVAILABLE QUALIFIERS:\n";
const std::string text_qualif2 = " VN - VENDOR  CX - CARRIER    SQ - SEQUENCE\n";
const std::string text_qualif3 = " CB - CABIN   CP - CITY PAIR  DDZONE - ZONE DETAILED INFO\n";

const std::string header1 = " VN     CXR    SEQ     GI     EFF DATE      FIRST TKT DATE\n";
const std::string header2 = "                              DISC          LAST\n";
const std::string next_item = "-------------------------------------------------------------\n";
const std::string divider = "*************************************************************\n";
const std::string footer = "*********************** END   DIAG 187 ***********************\n";
const std::string flt1 = " FLIGHT 1: ";
const std::string flt2 = " FLIGHT 2: ";
const std::string equp = " EQUIPMENT: ";
const std::string loc1 = " GEO LOC1: ";
const std::string loc2 = " GEO LOC2: ";
const std::string zone_divider = "                - - - - - - - - - - - - - - - - - - - - - -\n";
const std::string zone_header  = "                VENDOR LOCTYPE  LOCATION  DIRECTN INCL/EXCL\n";
const std::string cabin       = " CABIN: ";

const std::string sourceCall_ItinAnalyzerService = "BLANK MESSAGE";

const std::string header1_1 = " VN   CXR  SEQ      GI  EFF DATE  FIRST TKT DATE    STATUS\n";

const std::string source_data = " SOURCE DATA: ";
const std::string tkt_date = " TRANSACTION DATE: ";
const std::string source_airseg_cxr = "    CXR: ";
const std::string source_airseg_rbd = "    RBD: ";
const std::string source_airseg_cpa = "  MARKET: ";
const std::string source_airseg_tvlDate = "  TRAVEL ON: ";
const std::string source_airseg_status_flight = "FLIGHT SEGMENT";
const std::string source_airseg_status_open = "OPEN SEGMENT";

const std::string source_multiple_rbds = " RBDS : \n";
const std::string source_fare_class = "FARE CLASS: ";
const std::string source_fare_market = "  FARE MARKET: ";
const std::string source_T999_item = "    T999 ITEM: ";
const std::string source_T999_seq = "   SEQ: ";
const std::string source_T999_restr = "   RESTR TAG: ";
}

void
Diag187Collector::printHeader(const RBDByCabinCall& sourceCall)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << divider << title;
  switch(sourceCall)
  {
  case ITIN_SHP_PYA:
    dc << "        GET CABIN FOR SHOPPING ORIGIN BASED RT PRICING\n";
    break;
  case ITIN_SHP_SVC:
    dc << "         GET CABIN FOR SOP PREMIUM IN ITINANALYZER\n";
    break;
  case SOLD_OUT:
    dc << "          GET CABIN FOR SOLD OUT IN ITINANALYZER\n";
    break;
  case DSS_RSP:
    dc << "            GET CABINS FOR RBD RETURNED BY DSS\n";
    break;
  case AVAIL_RSP:
    dc << "          GET CABINS FOR RBD RETRIEVED FROM ASV2\n";
    break;
  case CONTENT_SVC:
    dc << "          GET CABIN FOR ALL RBDS FOR OPEN SEGMENT\n";
    break;
  case CONTENT_SVC_FAKE:
    dc << "            GET CABIN FOR RBD FOR AVFAKE REQUEST\n";
    break;
  case FAMILY_LOGIC:
    dc << "        GET CABIN FOR FAMILY LOGIC IN ITINANALYZER\n";
    break;
  case OPTIONAL_SVC:
    dc << "     GET CABIN FOR T198 RBD IN ANCILLARY S7 VALIDATION\n";
    break;
  case ANC_BAG_RQ:
    dc << "    GET CABIN FOR FLIGHT RBD IN ANCILLARY/BAGGAGE REQUEST\n";
    break;
  case PRICING_RQ:
    dc << "         GET CABIN FOR THE RBD IN ITINERARY REQUEST\n";
    break;
  case T999_VAL:
    dc << "         GET CABIN FOR T999 REBOOKED RBD FOR WPNCS\n";
    break;
  case RBD_CAT31:
    dc << "       GET CABIN FOR CAT31 RBD HIERARCHY PROCESSING\n";
    break;
  case PREFERRED_CABIN:
    dc << "      GET CABIN FOR PREFERRED RBD IN SHOPPING REQUEST\n";
    break;
  case SHP_HANDLER:
    dc << "        GET CABIN FOR FLIGHT RBD IN SHOPPING REQUEST\n";
    break;
  case SHP_WPNI:
    dc << "          GET CABIN FOR ALL RBD IN SHOPPING WPNI.C\n";
    break;
  case RBD_VAL:
    dc << "          GET CABIN FOR WPNC/WPNCS IN PRICE BY CABIN\n";
    break;
  case NO_CALL:
    dc << "             GET ALL VALID RBD CABIN FOR CARRIER\n";
    break;
  default:
    break;
  }
  dc << divider;
}

void
Diag187Collector::printAirSegData(const AirSeg& seg, const DateTime& date, const DateTime& tktDate, bool& open)
{
  if (!_active)
    return;
  std::vector<BookingCode> bks;
  printAirSegData(seg, seg.carrier(), seg.getBookingCode(), date, tktDate, open, bks);
}

void
Diag187Collector::printAirSegData(const AirSeg& seg,
                                  const DateTime& date,
                                  const std::vector<BookingCode>& bks,
                                  const DateTime& tktDate)
{
  if (!_active)
    return;
  bool open = false;
  printAirSegData(seg, seg.carrier(), seg.getBookingCode(), date, tktDate, open, bks);
}

void
Diag187Collector::printAirSegData(const AirSeg& seg,
                                  const CarrierCode& carrier,
                                  const BookingCode& bookingCode,
                                  const DateTime& date,
                                  const DateTime& tktDate,
                                  bool& open,
                                  const std::vector<BookingCode>& bks)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << source_data
     << ((seg.segmentType() != Open) ? source_airseg_status_flight : source_airseg_status_open)
     << "  " << tkt_date << std::setw(10) << tktDate.dateToString(DDMMMYYYY, '\0') << "\n"
     << source_airseg_cxr << carrier
     << source_airseg_cpa;

  if((!seg.origin() && seg.destination()) ||
     (seg.origin() && !seg.destination())   )
  {
    if(!seg.origin())
     dc << "   ";
    else
     dc << seg.origin()->loc();

    dc << "-";
    if(!seg.destination())
      dc << "   ";
    else
      dc << seg.destination()->loc();
  }
  else
  if(!seg.origin() && !seg.destination())
    dc << " EMPTY ";
  else
    dc << seg.origin()->loc() << "-" << seg.destination()->loc();

  dc << source_airseg_tvlDate << std::setw(10) << date.dateToString(DDMMMYYYY, '\0') << "\n";
  dc << source_airseg_rbd;

  printBookingCode(bookingCode, bks);
  dc << "\n";
}

void
Diag187Collector::printAirSegData(const PaxTypeFare& ptf,
                                  const DateTime& date,
                                  const std::vector<BookingCode>& bks,
                                  const DateTime& tktDate)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << source_data
     << source_airseg_status_flight
     << "  " << tkt_date << std::setw(10) << tktDate.dateToString(DDMMMYYYY, '\0') << "\n"
     << source_airseg_cxr << ptf.fareMarket()->governingCarrier()
     << source_airseg_cpa;

  if((!ptf.fareMarket()->travelSeg().front()->origin() && ptf.fareMarket()->travelSeg().back()->destination()) ||
     (ptf.fareMarket()->travelSeg().front()->origin() && !ptf.fareMarket()->travelSeg().back()->destination())   )
  {
    if(!ptf.fareMarket()->travelSeg().front()->origin())
     dc << "   ";
    else
     dc << ptf.fareMarket()->travelSeg().front()->origin()->loc();

    dc << "-";
    if(!ptf.fareMarket()->travelSeg().back()->destination())
      dc << "   ";
    else
      dc << ptf.fareMarket()->travelSeg().back()->destination()->loc();
  }
  else
  if(!ptf.fareMarket()->travelSeg().front()->origin() &&
     !ptf.fareMarket()->travelSeg().back()->destination())
    dc << " EMPTY ";
  else
    dc << ptf.fareMarket()->travelSeg().front()->origin()->loc() << "-" 
       << ptf.fareMarket()->travelSeg().back()->destination()->loc();

  dc << source_airseg_tvlDate << std::setw(10) << date.dateToString(DDMMMYYYY, '\0') << "\n";
  dc << source_airseg_rbd;

  printBookingCode(CarrierCode(), bks);

  dc << "  " << source_fare_class << std::setw(8) << ptf.fareClass()  << "\n";
}

void
Diag187Collector::printAirSegData(const AirSeg& seg,
                                  const BookingCode& bookingCode,
                                  const DateTime& date,
                                  const PaxTypeFare& paxTypeFare,
                                  const uint32_t& itemNo,
                                  const BookingCodeExceptionSegment& bceSegment,
                                  const DateTime& tktDate,
                                  bool& open)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << source_data
     << ((seg.segmentType() != Open) ? source_airseg_status_flight : source_airseg_status_open)
     << "  " << tkt_date << std::setw(10) << tktDate.dateToString(DDMMMYYYY, '\0') << "\n"
     << source_airseg_cxr << seg.carrier()
     << source_airseg_cpa;

  if((!seg.origin() && seg.destination()) ||
     (seg.origin() && !seg.destination())   )
  {
    if(!seg.origin())
     dc << "   ";
    else
     dc << seg.origin()->loc();

    dc << "-";
    if(!seg.destination())
      dc << "   ";
    else
      dc << seg.destination()->loc();
  }
  else
  if(!seg.origin() && !seg.destination())
    dc << " EMPTY ";
  else
    dc << seg.origin()->loc() << "-" << seg.destination()->loc();

  dc << source_airseg_tvlDate << std::setw(10) << date.dateToString(DDMMMYYYY, '\0') << "\n";
  dc << source_airseg_rbd;

  const std::vector<BookingCode> bks;
  printBookingCode(bookingCode, bks);
  dc << "\n";
  dc << "    "
     << source_fare_class << std::setw(8) << paxTypeFare.fareClass() << "  "
     << source_fare_market << paxTypeFare.fareMarket()->travelSeg().front()->origin()->loc() << "-"
     << paxTypeFare.fareMarket()->travelSeg().back()->destination()->loc() << "\n";
  dc << source_T999_item  << itemNo
     << source_T999_seq   << bceSegment.segNo() << "   "
     << source_T999_restr << bceSegment.restrictionTag() << "\n";
}

void
Diag187Collector::printRecordHeader()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << next_item
     << header1 << header2 << next_item;
}

void
Diag187Collector::printRecordHeaderShort()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << next_item
     << header1_1 << next_item;
}

void
Diag187Collector::printBookingCode(const BookingCode& bookingCode,
                                   const std::vector<BookingCode>& bks)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  if(bks.size() > 0)
  {
    for (const auto& rbd : bks)
    {
      dc << rbd << " ";
    }
  }
  else
    dc << bookingCode;
}

void
Diag187Collector::printCurrentRBD(const BookingCode& bookingCode)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << next_item
     << "     GET CABIN FOR RBD:  " << bookingCode << "\n";
}

void
Diag187Collector::printReqCabinNotMatch(const Indicator& cabin,
                                        const BookingCode& bookingCode)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << "\n  *** RBD: " << bookingCode << "  IS NOT IN THE REQUESTED CABIN: " << cabin << "\n";
}

void
Diag187Collector::printCabinNotFoundForRBD(const BookingCode& bookingCode)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << "\n  *** SEQUENCE MATCHED FIXED DATA: RBD - " << bookingCode << " NOT FOUND\n";
}

void
Diag187Collector::printRBDinRQNotMatchRBDinSeg(const BookingCode& bkgRQ,
                                               const BookingCode& bkgSeg)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << next_item
     << "  REQUESTED RBD: " << bkgRQ
     << "  NOT MATCH PROCESSED RBD: " << bkgSeg << "\n";
}

void
Diag187Collector::printAncBagRBDNotFiledInCxrRbdByCabin(const RBDByCabinInfo& rbdInfo,
                                                        const BookingCode& bookingCode)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << "  PROCESSED RBD: " << bookingCode
     << "\n  NOT FILED IN ATPCO RBDBYCABIN FOR CXR: " << rbdInfo.carrier()
     << "  IN SEQ: " << rbdInfo.sequenceNo() << "\n";
}

void
Diag187Collector::printDefaultEconomyCabin()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << "\n DEFAULT ECONOMY CABIN ASSIGNED\n";
}

void
Diag187Collector::printCabinFoundForRBD(const BookingCode& bookingCode,
                                        const Indicator& cabin,
                                        const CarrierCode& carrier,
                                        bool defaultCabin)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );
  CabinType cabinType;

  dc << " ** RESULT: FOUND CABIN: " << cabin << " - " << cabinType.printNameAnswer(cabin)
     << " FOR RBD: " << bookingCode << " **\n";
  dc << " **         ";
/*  if(carrier == INDUSTRY_CARRIER && !defaultCabin)
   printStatus(PASS_YY);
  else
  if(!defaultCabin)
   printStatus(PASS_CXR);
  else
   printStatus(PASS_DEFAULT);
*/
  lineSkip( 1 );
}

void
Diag187Collector::printFailFoundCabinForRBD(const BookingCode& bookingCode,
                                            const CarrierCode& carrier)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << "\n  *** CABIN NOT FOUND FOR RBD : " << bookingCode << "  CARRIER : " <<  carrier << "\n";
}

void
Diag187Collector::printRBDByCabinNotFoundForCarrier(const CarrierCode& carrier)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << next_item << "  *** CABIN RECORD NOT FOUND FOR CARRIER : " <<  carrier << "\n";
}

void
Diag187Collector::printFailStatus(const RBDByCabinStatus& status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );
  if(status == SKIP_SEQ)
  {
    dc << "          *** STATUS: SKIP - FLIGHT NO/EQUIPMENT PRESENT ***\n";
  }
  else
  {
    dc << "                                     *** STATUS: ";
    printStatus(status);
  }
  dc << next_item;
}

void
Diag187Collector::printFailStatusShort(const RBDByCabinStatus& status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );
  printStatus(status);
}

void
Diag187Collector::printStatus(const RBDByCabinStatus& status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  switch (status)
  {
  case PASS_CXR:
    dc << "PASS CXR RBD BY CABIN **\n";
    break;
  case PASS_YY:
    dc << "PASS IATA RESO 728 YY RBD BY CABIN **\n";
    break;
  case PASS_DEFAULT:
    dc << "PASS DEFAULT Y CABIN **\n";
    break;
  case PASS_SEQ:
    dc << "PASS\n";
    break;
  case FAIL_TRAVEL_DATE:
    dc << "FAIL TVL\n";
    break;
  case FAIL_TICKET_DATE:
    dc << "FAIL TKT\n";
    break;
  case FAIL_GEO_LOC:
    dc << "FAIL GEO\n";
    break;
  case FAIL_GLOBAL_IND:
    dc << "FAIL GI\n";
    break;
  case FAIL_EQUP:
    dc << "FAIL EQUP\n";
    break;
  case FAIL_FLIGHT:
    dc << "FAIL FLIGHT\n";
    break;
  case SKIP_SEQ:
    dc << "SKIP SEQ\n";
    break;
  default:
    dc << "UNKNOWN\n";
    break;
  }
}

void
Diag187Collector::printHeader(const std::string& carrierReq,
                              const std::string& cityPair,
                              const DateTime& tktDate)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << title << title1 << diag_name;
  if(carrierReq.empty())
   dc << "ALL";
  else
   dc << std::setw(3) << carrierReq;

  if(!cityPair.empty())
  {
    const std::string boardCity(cityPair.substr(0, 3));
    const std::string offCity(cityPair.substr(3, 3));

    dc << city_pair << boardCity << "-" << offCity;
  }
  dc << std::endl;
  dc << tkt_date << std::setw(10) << tktDate.dateToString(DDMMMYYYY, '\0') << "\n";
  dc << title1 << text_qualif1 << text_qualif2 << text_qualif3;
  dc << divider;
  dc << header1 << header2 << next_item;
}

void
Diag187Collector::printRbdByCabinInfoShort(const RBDByCabinInfo* rbdByCabin)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << " " << std::setw(4) << rbdByCabin->vendor() << " ";
  dc << std::setw(3) << rbdByCabin->carrier() << "  ";
  dc << std::setw(8) << rbdByCabin->sequenceNo() << " ";
  dc << std::setw(2) <<rbdByCabin->globalDir() << "  ";

  dc << std::setw(10) << rbdByCabin->effDate().dateToString(DDMMMYYYY, '\0') << " ";

  if( rbdByCabin->firstTicketDate().isEmptyDate())
    dc << "      N/A      ";
  else
    dc << std::setw(10) << rbdByCabin->firstTicketDate().dateToString(DDMMMYYYY, '\0') << "     ";
}

void
Diag187Collector::printRbdByCabinInfo(const PricingTrx& trx,
                                      const RBDByCabinInfo* rbdByCabin,
                                      bool skip)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  std::string zoneDetail;
  if(trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ZONE")
    zoneDetail = "ZONE";
  if(skip && trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO")
    zoneDetail = "ZONE";

  const std::string& cabinReq = trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CABIN);

  dc << " " << std::setw(4) << rbdByCabin->vendor() << "   ";
  dc << std::setw(3) << rbdByCabin->carrier() << "   ";
  dc << std::setw(8) << rbdByCabin->sequenceNo() << "   ";
  dc << std::setw(2) <<rbdByCabin->globalDir() << "   ";

  dc << std::setw(10) << rbdByCabin->effDate().dateToString(DDMMMYYYY, '\0')
     << "    "
     << std::setw(10) << rbdByCabin->firstTicketDate().dateToString(DDMMMYYYY, '\0') << "\n"
     << "                              "
     << std::setw(10) << rbdByCabin->discDate().dateToString(DDMMMYYYY, '\0')
     << "    "
     << std::setw(10) << rbdByCabin->lastTicketDate().dateToString(DDMMMYYYY, '\0') << "\n";

  dc << flt1;
  if(rbdByCabin->flightNo1() == 0)
    dc << "       ";
  else
    dc << std::setw(4) << rbdByCabin->flightNo1() << "   ";

  dc << equp;
  if(rbdByCabin->equipmentType().empty())
    dc << "\n";
  else
    dc << std::setw(3) << rbdByCabin->equipmentType() << "\n";

  dc << flt2;
  if(rbdByCabin->flightNo2() == 0)
    dc << "\n";
  else
    dc << std::setw(4) << rbdByCabin->flightNo2() << "\n";

  dc << loc1;
  if(!rbdByCabin->locKey1().isNull())
  {
    dc << std::setw(2) << rbdByCabin->locKey1().locType();
    if(rbdByCabin->locKey1().locType() == 'Z')
      dc << std::setw(8) << rbdByCabin->locKey1().loc() << " \n";
    else
      dc << std::setw(3) << rbdByCabin->locKey1().loc() << " \n";

    if(!zoneDetail.empty() && zoneDetail == "ZONE" && rbdByCabin->locKey1().locType() == 'Z')
      displayZoneDetail(trx, rbdByCabin->vendor(), rbdByCabin->locKey1().loc());
  }
  else
   dc << " \n";

  dc << loc2;
  if(!rbdByCabin->locKey2().isNull())
  {
    dc << std::setw(2) << rbdByCabin->locKey2().locType();
    if(rbdByCabin->locKey2().locType() == 'Z')
      dc << std::setw(8) << rbdByCabin->locKey2().loc() << " \n";
    else
      dc << std::setw(3) << rbdByCabin->locKey2().loc() << " \n";
    if(!zoneDetail.empty() && zoneDetail == "ZONE" && rbdByCabin->locKey2().locType() == 'Z')
    {
      if(rbdByCabin->locKey1().locType() == 'Z' &&
         rbdByCabin->locKey1().loc() == rbdByCabin->locKey2().loc())
        dc << "                SAME ZONE AS ABOVE\n";
      else
        displayZoneDetail(trx, rbdByCabin->vendor(), rbdByCabin->locKey2().loc() );
    }
  }
  else
   dc << " \n";

  lineSkip( 1 );

  // Display Cabin
  std::vector<BookingCode> vectorR;
  std::vector<BookingCode> vectorF;
  std::vector<BookingCode> vectorJ;
  std::vector<BookingCode> vectorC;
  std::vector<BookingCode> vectorW;
  std::vector<BookingCode> vectorY;

  for (BookingCodeCabinMap::const_iterator it(rbdByCabin->bookingCodeCabinMap().begin()),
                                        itend(rbdByCabin->bookingCodeCabinMap().end());
         it != itend; ++it)
  {
    if(!cabinReq.empty() && (cabinReq.at(0) != it->second))
      continue;

    switch(it->second)
    {
      case 'R':
        vectorR.push_back(it->first);
        break;
      case 'F':
        vectorF.push_back(it->first);
        break;
      case 'J':
        vectorJ.push_back(it->first);
        break;
      case 'C':
        vectorC.push_back(it->first);
        break;
      case 'W':
        vectorW.push_back(it->first);
        break;
      case 'Y':
        vectorY.push_back(it->first);
        break;
      default: 
        dc << " INVALID CABIN - " << it->second 
           << " - FOUND FOR CARRIER " << rbdByCabin->carrier() << "\n";
        lineSkip( 1 );
    }
  }
  if(vectorR.empty() && vectorF.empty() && vectorJ.empty() &&
     vectorC.empty() && vectorW.empty() && vectorY.empty() )
  {
    if(!cabinReq.empty())
      dc << " CABIN " << cabinReq << " REQUESTED - CABIN NOT FOUND FOR CARRIER "
         << rbdByCabin->carrier() << "\n";
    else
      dc << " NO CABIN AND RBD DATA FOUND FOR CARRIER " << rbdByCabin->carrier() << "\n";
   dc << next_item;
   return;
  }

  if(!vectorR.empty())
    displayCabin('R', vectorR);
  if(!vectorF.empty())
    displayCabin('F', vectorF);
  if(!vectorJ.empty())
    displayCabin('J', vectorJ);
  if(!vectorC.empty())
    displayCabin('C', vectorC);
  if(!vectorW.empty())
    displayCabin('W', vectorW);
  if(!vectorY.empty())
    displayCabin('Y', vectorY);
  if(!skip)
  dc << next_item;
}

void
Diag187Collector::displayZoneDetail(const PricingTrx& trx, const VendorCode& vendor,
                                    const Zone& zone)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  Zone zoneNo = zone;

  // if the zone is less than seven characters, then
  // pad the front of it with '0' characters
  if (zoneNo.size() < 7)
  {
    const int diff = 7 - int(zoneNo.size());
    for (int n = 6; n >= 0; --n)
    {
      if (n - diff < 0)
      {
        zoneNo[n] = '0';
      }
      else
      {
        zoneNo[n] = zoneNo[n - diff];
      }
    }
  }

  const ZoneInfo* zoneInfo = getZone(trx, vendor, zoneNo);

  if (zoneInfo == nullptr)
  {
    dc << "  ZONE: " << std::setw(8) << zone
       << "  VENDOR: " << std::setw(4) << vendor
       << "  NOT FOUND\n";
    return;
  }

  // Zode decode and display
  dc << zone_divider;
  dc << zone_header;
  dc << zone_divider;

  std::vector<std::vector<ZoneInfo::ZoneSeg> >::const_iterator i = zoneInfo->sets().begin();
  for (; i != zoneInfo->sets().end(); ++i)
  {
    std::vector<ZoneInfo::ZoneSeg>::const_iterator j = i->begin();
    for (; j != i->end(); ++j)
    {
      dc << "                 "
         << std::setw(4) << zoneInfo->vendor()
         << "  ";
      displayLocType(j->locType());
      dc << "  "
         << std::setw(8) << j->loc()
         << "  ";
      displayDirectionalInd(j->directionalQualifier());
      dc << "  ";
      displayExclIncl(j->inclExclInd());
    }
  }
}

const ZoneInfo*
Diag187Collector::getZone(const PricingTrx& trx,
                          const VendorCode& vendor,
                          const Zone& zoneNo)
{
  DataHandle& dataHandle = trx.dataHandle();
  const ZoneInfo* zoneInfo = dataHandle.getZone(vendor, zoneNo, RESERVED, trx.ticketingDate());
  if (zoneInfo == nullptr && vendor != ATPCO_VENDOR_CODE)
    zoneInfo = dataHandle.getZone(ATPCO_VENDOR_CODE, zoneNo, RESERVED, trx.ticketingDate());
  return zoneInfo;
}

void
Diag187Collector::displayLocType(const Indicator& locType)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << std::setw(7);
  switch (locType)
  {
   case LOCTYPE_CITY:
     dc << "CITY";
     break;
   case LOCTYPE_STATE:
     dc << "STATE";
     break;
   case LOCTYPE_NATION:
     dc << "NATION";
     break;
   case LOCTYPE_SUBAREA:
     dc << "SUBAREA";
     break;
   case LOCTYPE_AREA:
     dc << "AREA";
     break;
   case LOCTYPE_ZONE:
     dc << "ZONE";
     break;
   case LOCTYPE_AIRPORT:
     dc << "AIRPORT";
     break;
   case LOCTYPE_PCC:
     dc << "PCC";
     break;
   case LOCTYPE_PCC_ARC:
     dc << "PCC ARC";
     break;
   case LOCTYPE_USER:
     dc << "USER";
     break;
   case LOCTYPE_FMSZONE:
     dc << "FMSZONE";
     break;
   default:
     dc << "NONE";
  }
}

void
Diag187Collector::displayDirectionalInd(const Indicator& dir)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << std::setw(7);
    switch (dir)
    {
    case 'B':
      dc << "BETWEEN";
      break;
    case 'A':
      dc << "AND";
      break;
    case 'T':
      dc << "TO";
      break;
    case 'F':
      dc << "FROM";
      break;
    case 'N':
      dc << "NONE";
      break;
    default:
      dc << "UNKWN";
    }
}

void
Diag187Collector::displayExclIncl(const Indicator& ei)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << std::setw(7);
  switch (ei)
  {
   case 'I':
     dc << "INCLUDE\n";
     break;
   case 'E':
     dc << "EXCLUDE\n";
     break;
    default:
     dc << "UNKWN  \n";
  }
}

void
Diag187Collector::displayCabin(Indicator cab, std::vector<BookingCode>& rbdVector)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  if(!rbdVector.empty())
  {
    CabinType cabinType;
    dc << cabin;
    dc << cab << " - " << cabinType.printNameAnswer(cab) << " \n";
    dc << "   " << cabinType.getCabinNumberFromAlphaAnswer(cab);

    std::ostringstream oss;
    for (const auto& rbd : rbdVector)
    {
      oss << rbd << " ";
    }
    if(oss.str().size() <= 48)
      dc << "        " << oss.str() << "\n";
    else
    {
      std::string value = oss.str();
      while(!value.empty())
      {
        if(value.size() > 48)
        {
          std::string msg = value.substr(0, 48);
          dc << "        " << msg << "\n";
          dc << "    ";
          value.erase(0, 48);
          msg.clear();
        }
        else
        {
          dc << "        " << value << "\n";
          value.clear();
        }
      }
    }
  }
}

void
Diag187Collector::printFooter()
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  dc << footer;
}
} // namespace tse
