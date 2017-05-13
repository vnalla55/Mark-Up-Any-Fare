//----------------------------------------------------------------------------
//  File:        Diag877Collector.cpp
//  Authors:
//  Created:
//
//  Description: Diagnostic 877 Service Fee - OC Fees S7
//  Updates:
//
//
//  Copyright Sabre 2009
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

#include "Diagnostic/Diag877Collector.h"

#include "Common/ServiceFeeUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/AncRequest.h"
#include "DataModel/FarePath.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/SvcFeesCurrencyInfo.h"
#include "DBAccess/SvcFeesCxrResultingFCLInfo.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"


#include <algorithm>
#include <iomanip>
#include <iostream>

using namespace std;

namespace
{
static const size_t TAX_ON_OC_BUFF_SIZE = 15;
}

namespace tse
{
void
Diag877Collector::S7DetailsDiagBuilder::buildFullInfo(const char* headerContext,
                                                      bool addAsterixIfFieldPresent)
{
  addHeader(headerContext)
      .addUpToStopoverTime()
      .addCabin()
      .addRBD198(addAsterixIfFieldPresent)
      .addUpgradeCabin()
      .addUpgradeRBD198()
      .addT171(addAsterixIfFieldPresent)
      .addT173(addAsterixIfFieldPresent)
      .addRuleTariffIndicator()
      .addRuleTariff()
      .addRule()
      .add1stCommonPart()
      .addT186(addAsterixIfFieldPresent)
      .add2ndCommonPart();
}

Diag877Collector::S7DetailsDiagBuilder&
Diag877Collector::S7DetailsDiagBuilder::addHeader(const char* headerContext)
{
  char buff[8];
  sprintf(buff, "%d", _s7->seqNo());

  _diag.setf(ios::left, ios::adjustfield);
  _dc << "------------- OPTIONAL SERVICE S7 DETAILED " << headerContext << "INFO --------------\n"
      << " SUBCODE : " << setw(3) << _s7->serviceSubTypeCode() << "           "
      << " VENDOR : " << setw(5) << _s7->vendor() << "   "
      << "SERVICE : ";
  _diag.displayFltTktMerchInd(_s7->fltTktMerchInd());
  _dc << "\n";

  _dc << " SEQ NBR : " << setw(7) << buff << "\n";

  _dc << " SRV EFF DATE : " << setw(10) << _s7->effDate().dateToSqlString() << "     ";
  if (ServiceFeeUtil::isStartDateSpecified(*_s7))
  {
    _dc << " TVL DATE START : ";
    std::ostringstream osM;
    if (!_s7->tvlStartYear())
      osM << "XXXX";
    else
      osM << setw(4) << _s7->tvlStartYear();
    osM << "-" << setw(2) << std::setfill('0') << _s7->tvlStartMonth() << "-";
    if (!_s7->tvlStartDay())
      osM << "XX\n";
    else
      osM << setw(2) << std::setfill('0') << _s7->tvlStartDay();
    _dc << setw(10) << osM.str().substr(0, 10) << "\n";
  }
  else
  {
    _dc << " TRVL EFF DATE  : ";
    if (_s7->ticketEffDate().isEmptyDate())
      _dc << "1980-01-01\n";
    else
      _dc << setw(10) << _s7->ticketEffDate().dateToSqlString() << "\n";
  }
  _dc << " SRV DISC DATE: " << setw(10) << _s7->discDate().dateToSqlString() << "     ";
  if (ServiceFeeUtil::isStopDateSpecified(*_s7))
  {
    _dc << " TVL DATE STOP  : ";
    std::ostringstream osM;
    if (!_s7->tvlStopYear())
      osM << "XXXX";
    else
      osM << setw(4) << _s7->tvlStopYear();
    osM << "-" << setw(2) << std::setfill('0') << _s7->tvlStopMonth() << "-";
    if (!_s7->tvlStopDay())
      osM << "XX\n";
    else
      osM << setw(2) << std::setfill('0') << _s7->tvlStopDay();
    _dc << setw(10) << osM.str().substr(0, 10) << "\n";
  }
  else
  {
    _dc << " TRVL DISC DATE : ";
    if (_s7->ticketDiscDate().isEmptyDate())
      _dc << "1980-01-01\n";
    else
      _dc << setw(10) << _s7->ticketDiscDate().dateToSqlString() << "\n";
  }

  _dc << "  \n";
  return *this;
}

Diag877Collector::S7DetailsDiagBuilder&
Diag877Collector::S7DetailsDiagBuilder::addUpToStopoverTime()
{
  _dc << " PAX TYPE : " << _s7->psgType() << "\n";

  _dc << "      AGE MIN  : ";
  if (_s7->minAge() == 0)
    _dc << "  ";
  else
    _dc << setw(2) << _s7->minAge();

  _dc << "      MAX : ";
  if (_s7->maxAge() != 0)
    _dc << setw(2) << _s7->maxAge();
  _dc << "\n";

  _dc << "  OCCUR FIRST  : ";
  if (_s7->firstOccurence() == 0)
    _dc << "   ";
  else
    _dc << setw(3) << _s7->firstOccurence();

  _dc << "     LAST: ";
  if (_s7->lastOccurence() != 0)
    _dc << setw(3) << _s7->lastOccurence();
  _dc << "\n";

  _dc << "   FF STATUS   : ";
  if (_s7->frequentFlyerStatus() != 0)
    _dc << _s7->frequentFlyerStatus();
  _dc << "\n";

  _dc << " PRIVATE IND   : " << setw(2) << _s7->publicPrivateInd() << "  "
      << " TOUR CODE   : " << _s7->tourCode() << "\n";

  _dc << " ACC CODE T172 : ";
  if (_s7->serviceFeesAccountCodeTblItemNo() != 0)
    _dc << setw(7) << _s7->serviceFeesAccountCodeTblItemNo();
  _dc << "\n";

  _dc << " TKT DSGN T173 : ";
  if (_s7->serviceFeesTktDesigTblItemNo() != 0)
    _dc << setw(7) << _s7->serviceFeesTktDesigTblItemNo();
  _dc << "\n";

  _dc << " SECURITY T183 : ";
  if (_s7->serviceFeesSecurityTblItemNo() != 0)
    _dc << setw(7) << _s7->serviceFeesSecurityTblItemNo();
  _dc << "\n";

  _dc << "SECTOR/PORTION : " << setw(2) << _s7->sectorPortionInd() << "   GEO FTW IND : ";
  if (_s7->fromToWithinInd() != BLANK)
    _dc << _s7->fromToWithinInd();
  _dc << "\n";

  _dc << "     LOC1 TYPE : " << setw(2) << _s7->loc1().locType() << "   LOC1 : " << setw(8)
      << _s7->loc1().loc() << "  LOC1 ZONE : ";
  if (!_s7->loc1ZoneTblItemNo().empty() && _s7->loc1ZoneTblItemNo() != "0000000")
    _dc << setw(8) << _s7->loc1ZoneTblItemNo();
  _dc << "\n";

  _dc << "     LOC2 TYPE : " << setw(2) << _s7->loc2().locType() << "   LOC2 : " << setw(8)
      << _s7->loc2().loc() << "  LOC2 ZONE : ";
  if (!_s7->loc2ZoneTblItemNo().empty() && _s7->loc2ZoneTblItemNo() != "0000000")
    _dc << setw(8) << _s7->loc2ZoneTblItemNo();
  _dc << "\n";

  _dc << "VIA  LOC  TYPE : " << setw(2) << _s7->viaLoc().locType() << "   LOC  : " << setw(8)
      << _s7->viaLoc().loc() << "  LOC  ZONE : ";
  if (!_s7->viaLocZoneTblItemNo().empty() && _s7->viaLocZoneTblItemNo() != "0000000")
    _dc << setw(8) << _s7->viaLocZoneTblItemNo();
  _dc << "\n";

  _dc << "STOP/CONN/DEST : ";
  if (_s7->stopCnxDestInd() != BLANK)
    _diag.displayStopCnxDestInd(_s7->stopCnxDestInd());
  _dc << "\n";

  _dc << " STOPOVER TIME : ";
  if (!_s7->stopoverTime().empty() || _s7->stopoverUnit() != BLANK)
  {
    if (_s7->stopoverTime().empty())
      _dc << "   ";
    else
      _dc << setw(3) << _s7->stopoverTime();

    if (_s7->stopoverUnit() != BLANK)
      _dc << " " << _s7->stopoverUnit();
  }
  _dc << "\n";

  return *this;
}

Diag877Collector::S7DetailsDiagBuilder&
Diag877Collector::S7DetailsDiagBuilder::addCabin(bool allignRight)
{
  _dc << (allignRight ? "         CABIN : " : "CABIN : ");
  if (_s7->cabin() != BLANK)
    _dc << _s7->cabin();
  _dc << "\n";
  return *this;
}

Diag877Collector::S7DetailsDiagBuilder&
Diag877Collector::S7DetailsDiagBuilder::addRBD198(bool addAsterixIfFieldPresent, bool allignRight)
{
  _dc << (allignRight ? "      RBD T198 : " : "RBD T198 : ");
  if (_s7->serviceFeesResBkgDesigTblItemNo() != 0)
    _dc << setw(addAsterixIfFieldPresent ? 0 : 7) << _s7->serviceFeesResBkgDesigTblItemNo()
        << (addAsterixIfFieldPresent ? " *" : "");

  _dc << "\n";
  return *this;
}

Diag877Collector::S7DetailsDiagBuilder&
Diag877Collector::S7DetailsDiagBuilder::addUpgradeCabin()
{
  _dc << " UPGRADE CABIN : ";
  if (_s7->upgradeCabin() != BLANK)
    _dc << _s7->upgradeCabin();
  _dc << "\n";
  return *this;
}

Diag877Collector::S7DetailsDiagBuilder&
Diag877Collector::S7DetailsDiagBuilder::addUpgradeRBD198()
{
  _dc << " UPGR RBD T198 : ";
  if (_s7->upgrdServiceFeesResBkgDesigTblItemNo() != 0)
    _dc << setw(7) << _s7->upgrdServiceFeesResBkgDesigTblItemNo();
  _dc << "\n";
  return *this;
}

Diag877Collector::S7DetailsDiagBuilder&
Diag877Collector::S7DetailsDiagBuilder::addT171(bool addAsterixIfFieldPresent)
{
  _dc << "CXR/RESULT FARE CLASS T171 : ";
  if (_s7->serviceFeesCxrResultingFclTblItemNo() != 0)
    _dc << setw(addAsterixIfFieldPresent ? 0 : 7) << _s7->serviceFeesCxrResultingFclTblItemNo()
        << (addAsterixIfFieldPresent ? " *" : "");

  _dc << "\n";
  return *this;
}

Diag877Collector::S7DetailsDiagBuilder&
Diag877Collector::S7DetailsDiagBuilder::addT173(bool addAsterixIfFieldPresent)
{
  _dc << "FARE TICKT DESIGNATOR T173 : ";
  if (_s7->resultServiceFeesTktDesigTblItemNo() != 0)
    _dc << setw(addAsterixIfFieldPresent ? 0 : 7) << _s7->resultServiceFeesTktDesigTblItemNo()
        << (addAsterixIfFieldPresent ? " *" : "");

  _dc << "\n";
  return *this;
}

Diag877Collector::S7DetailsDiagBuilder&
Diag877Collector::S7DetailsDiagBuilder::addRuleTariffIndicator()
{
  _dc << " RULETARIFF IND : ";
  if (!_s7->ruleTariffInd().empty())
    _dc << setw(3) << _s7->ruleTariffInd();
  _dc << "\n";
  return *this;
}

Diag877Collector::S7DetailsDiagBuilder&
Diag877Collector::S7DetailsDiagBuilder::addRuleTariff(bool allignRight)
{

  _dc << (allignRight ? "    RULE TARIFF : " : "RULE TARIFF : ");
  if (_s7->ruleTariff() == (uint16_t) - 1)
    _dc << "   ";
  else
    _dc << setw(3) << _s7->ruleTariff();
  return *this;
}

Diag877Collector::S7DetailsDiagBuilder&
Diag877Collector::S7DetailsDiagBuilder::addRule()
{
  _dc << "     RULE : " << setw(4) << _s7->rule() << "   FARE IND : ";
  _diag.displaySourceForFareCreated(_s7->fareInd());
  return *this;
}

Diag877Collector::S7DetailsDiagBuilder&
Diag877Collector::S7DetailsDiagBuilder::add1stCommonPart()
{
  _dc << "TRAVEL TIME IND : " << setw(5) << _s7->timeApplication()
      << "    DOW : " << _s7->dayOfWeek() << "\n";

  _dc << "          START : ";
  if (_s7->startTime() != 0)
  {
    _dc << setw(4) << _s7->startTime();
  }
  else
    _dc << "    ";

  _dc << "    STOP : ";
  if (_s7->stopTime() != 0)
    _dc << setw(4) << _s7->stopTime();

  _dc << "\n";

  _dc << "ADV/PURCHASE RSV: " << setw(3) << _s7->advPurchPeriod() << " " << setw(2)
      << _s7->advPurchUnit() << "             "
      << "ADV/TICKET ISSUE : ";
  if (_s7->advPurchTktIssue() != BLANK)
    _dc << _s7->advPurchTktIssue();
  _dc << "\n";

  _dc << "      EQUIPMENT : " << _s7->equipmentCode() << "\n";
  return *this;
}

Diag877Collector::S7DetailsDiagBuilder&
Diag877Collector::S7DetailsDiagBuilder::addT186(bool addAsterixIfFieldPresent)
{
  _dc << "CXR/FLIGHT APPL T186 : ";
  if (_s7->carrierFltTblItemNo() != 0)
    _dc << setw(addAsterixIfFieldPresent ? 0 : 7) << _s7->carrierFltTblItemNo()
        << (addAsterixIfFieldPresent ? " *" : "");

  _dc << "\n";
  return *this;
}

Diag877Collector::S7DetailsDiagBuilder&
Diag877Collector::S7DetailsDiagBuilder::add2ndCommonPart()
{
  _dc << " NOT AVAIL/NO CHARGE : ";
  _diag.displayNotAvailNoCharge(_s7->notAvailNoChargeInd());
  _dc << "       CURRENCY T170 : ";
  if (_s7->serviceFeesCurrencyTblItemNo() != 0)
    _dc << setw(7) << _s7->serviceFeesCurrencyTblItemNo();
  _dc << "\n";

  _dc << "         AND/OR : ";
  if (_s7->andOrInd() != BLANK)
    _dc << _s7->andOrInd();
  _dc << "\n";

  _dc << "FF MILEAGE FEE  : ";
  if (_s7->applicationFee() != 0)
    _dc << setw(8) << _s7->applicationFee();
  _dc << "\n";

  _dc << "FEE APPLICATION : ";
  if (_s7->frequentFlyerMileageAppl() != BLANK)
    _diag.displayFFMileageAppl(_s7->frequentFlyerMileageAppl());
  _dc << "\n";

  _dc << " REFUND/REISSUE : ";
  _diag.displayRefundReissue(_s7->refundReissueInd());
  _dc << " FORM OF REFUND : ";
  _diag.displayFormOfTheRefund(_s7->formOfFeeRefundInd());
  _dc << "     COMMISSION : ";
  if (_s7->commissionInd() != BLANK)
    _dc << _s7->commissionInd();
  _dc << "\n";

  _dc << "      INTERLINE : ";
  if (_s7->interlineInd() != BLANK)
    _dc << _s7->interlineInd();
  _dc << "\n";

  _dc << "   COLLECT/SUBSTRACT : " << setw(2) << _s7->collectSubtractInd();
  _dc << "  NET/SELL : ";
  if (_s7->netSellingInd() != BLANK)
    _dc << _s7->netSellingInd();
  _dc << "\n";

  _dc << "            TAX APPL : " << setw(2) << _s7->taxInclInd();
  _dc << "                AVAIL CHECK : ";
  if (_s7->availabilityInd() != BLANK)
    _dc << _s7->availabilityInd();
  _dc << "\n";
  _dc << "      TAX EXEMPT IND : " << setw(1) << _s7->taxExemptInd() << "\n";

  _dc << "RULE BUSTER SEGMENTS : ";
  if (_s7->segCount() != 0)
    _dc << setw(8) << _s7->segCount();
  else
    _dc << "        ";
  _dc << "RULE BUSTER FARE CLASS: " << _s7->ruleBusterFcl() << "\n";

  _dc << "TEXT T196 : ";
  if (_s7->taxTblItemNo() != 0)
    _dc << setw(7) << _s7->taxTblItemNo();
  _dc << "\n";
  return *this;
}

void
Diag877Collector::printS7Banner()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "*************** OC OPTIONAL SERVICE ANALYSIS ******************\n";
  return;
}

void
Diag877Collector::printTaxBanner()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "******************* SERVICE FEE DIAGNOSTIC ********************\n";
  dc << "*************** OC OPTIONAL SERVICE ANALYSIS ******************\n";
  dc << "*************************** TAXES *****************************\n";
  return;
}

void
Diag877Collector::printS7NotFound(const SubCodeInfo& subcode)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  displayVendor(subcode.vendor());

  dc << setw(3) << subcode.serviceSubTypeCode() << "  ";
  displayFltTktMerchInd(subcode.fltTktMerchInd());
  dc << "                         "
     << "DATA NOT FOUND\n";

  return;
}

void
Diag877Collector::printS7PortionOfTravel(const TravelSeg& seg1,
                                         const TravelSeg& seg2,
                                         const FarePath& farePath,
                                         const PricingTrx& trx)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  dc << "------------ PORTION OF TRAVEL : ";
  if (!seg1.boardMultiCity().empty())
    dc << seg1.boardMultiCity();
  else
    dc << seg1.origAirport();

  dc << " " << setw(2) << seg1.pnrSegment();
  dc << " - ";

  if (!seg2.offMultiCity().empty())
    dc << seg2.offMultiCity();
  else
    dc << seg2.destAirport();

  dc << " " << setw(2) << seg2.pnrSegment();

  if (trx.getRequest() && trx.getRequest()->multiTicketActive() && farePath.itin())
  {
    if (farePath.itin()->getMultiTktItinOrderNum() == 1)
      dc << " ----TKT1----\n";
    else if (farePath.itin()->getMultiTktItinOrderNum() == 2)
      dc << " ----TKT2----\n";
    else
      dc << " -SINGLE TKT-\n";
  }
  else
    dc << " ------------\n";

  return;
}

void
Diag877Collector::printS7GroupCxrService_old(const FarePath& farePath,
                                         const ServiceGroup group,
                                         const CarrierCode cxr,
                                         bool cxrInd)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "**************************************************************\n";
  dc << " GROUP    : " << setw(3) << group << "\n";
  dc << " CXR      : " << setw(3) << cxr;
  if (cxrInd)
    dc << " - MARKETING";
  else
    dc << " - OPERATING";
  dc << "       ";
  dc << "REQUESTED PAX TYPE : " << farePath.paxType()->paxType();
  if (!farePath.paxType()->psgTktInfo().empty())
  {
    int pos = 59;
    dc.setf(ios::right, ios::adjustfield);
    for (PaxType::TktInfo* info : farePath.paxType()->psgTktInfo())
    {
      if (pos >= 60)
      {
        dc << "\n" << setw(34) << " ";
        pos = 35;
      }
      dc << setw(4) << (std::string("T") + info->tktRefNumber());
      pos += 4;
    }
  }
  dc << "\n-------------------------------------------------------------- \n"
     << "                  S7 RECORDS DATA PROCESSING\n";

  return;
}

void
Diag877Collector::printS7GroupCxrService(const FarePath& farePath,
                                         const ServiceGroup group,
                                         const CarrierCode cxr,
                                         const std::string& carrierStrategyType)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "**************************************************************\n";
  dc << " GROUP    : " << setw(3) << group << "\n";
  dc << " CXR      : " << setw(3) << cxr;
  dc << " - " << carrierStrategyType;
  dc << "       ";
  dc << "REQUESTED PAX TYPE : " << farePath.paxType()->paxType();
  if (!farePath.paxType()->psgTktInfo().empty())
  {
    int pos = 59;
    dc.setf(ios::right, ios::adjustfield);
    for (PaxType::TktInfo* info : farePath.paxType()->psgTktInfo())
    {
      if (pos >= 60)
      {
        dc << "\n" << setw(34) << " ";
        pos = 35;
      }
      dc << setw(4) << (std::string("T") + info->tktRefNumber());
      pos += 4;
    }
  }
  dc << "\n-------------------------------------------------------------- \n"
     << "                  S7 RECORDS DATA PROCESSING\n";

  return;
}

void
Diag877Collector::printS7GroupService(const FarePath& farePath, const ServiceGroup group)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "**************************************************************\n";
  dc << " GROUP    : " << setw(3) << group << "\n";
  dc << " REQUESTED PAX TYPE : " << farePath.paxType()->paxType();
  if (!farePath.paxType()->psgTktInfo().empty())
  {
    int pos = 59;
    dc.setf(ios::right, ios::adjustfield);
    for (PaxType::TktInfo* info : farePath.paxType()->psgTktInfo())
    {
      if (pos >= 60)
      {
        dc << "\n" << setw(34) << " ";
        pos = 35;
      }
      dc << setw(4) << (std::string("T") + info->tktRefNumber());
      pos += 4;
    }
  }
  dc << "\n-------------------------------------------------------------- \n"
     << "                  S7 RECORDS DATA PROCESSING\n";

  return;
}

void
Diag877Collector::printS7CommonHeader()
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "V SCODE SERVICE   SEQ NUM  PAX    AMOUNT  STATUS\n";
}

void
Diag877Collector::printS7OptionalFeeInfo(const OptionalServicesInfo* info,
                                         const OCFees& ocFees,
                                         const StatusS7Validation& status,
                                         const bool markAsSelected)
{
  if (!_active)
    return;
  char buff[8];
  sprintf(buff, "%d", info->seqNo());

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  displayVendor(info->vendor());

  dc << setw(3) << info->serviceSubTypeCode() << "  ";
  displayFltTktMerchInd(info->fltTktMerchInd());

  dc << " " << setw(7) << buff << "  " << setw(3) << info->psgType();
  if (ocFees.optFee())
  {
    displayAmount(ocFees);
  }
  else
    dc << "            ";

  displayStatus(status);

  if (markAsSelected)
    dc << " F";

  dc << "\n";
}

void
Diag877Collector::printS7DetailInfo(const OptionalServicesInfo* info, const PricingTrx& trx)
{
  if (!_active)
    return;

  S7DetailsDiagBuilder diagBuilder(*this, info);
  diagBuilder.buildFullInfo("", false);
}

void
Diag877Collector::printS7DetailInfoPadis(const OptionalServicesInfo* info,
                                         const OCFees& ocFees,
                                         const std::vector<SvcFeesResBkgDesigInfo*>& padisData,
                                         const DateTime& travelDate)
{
  if (!_active)
    return;

  PricingTrx* trx = dynamic_cast<PricingTrx*>(_trx);

  if (!trx)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  dc.setf(ios::left, ios::adjustfield);
  dc << "*- - - - - - - - - - - - - - - - - - - - - - - - -  *\n";
  dc << "* UPGRD RBD T198 ITEM NO : ";
  dc << setw(7) << info->upgrdServiceFeesResBkgDesigTblItemNo();
  dc << "   DETAIL INFO    *\n";
  dc << "*- - - - - - - - - - - - - - - - - - - - - - - - - -*\n";
  dc << "O/D      SEQ   PADIS TRANSLATION                     \n";

  for (const SvcFeesResBkgDesigInfo* padis : padisData)
  {
    dc << setw(9)
       << ocFees.travelStart()->origin()->loc() + "-" + ocFees.travelEnd()->destination()->loc();
    dc << setw(6) << padis->seqNo();

    std::set<BookingCode> bookingCodeSet;
    ServiceFeeUtil::fill(bookingCodeSet, padis);

    std::map<BookingCode, std::string> padisCodeDescriptionMap;
    std::map<BookingCode, std::string> padisAbbreviatedDescriptionMap;

    ServiceFeeUtil::getTranslatedPadisDescription(*trx,
                                                  info->carrier(),
                                                  travelDate,
                                                  *padis,
                                                  padisCodeDescriptionMap,
                                                  padisAbbreviatedDescriptionMap);

    bool isFirstLine = true;

    for (const BookingCode bookingCode : bookingCodeSet)
    {
      if (isFirstLine)
        isFirstLine = false;
      else
        dc << std::string(15, ' ');

      dc << setw(6) << bookingCode;

      if (padisCodeDescriptionMap.find(bookingCode) == padisCodeDescriptionMap.end())
        dc << "UNKNOWN\n";
      else
        dc << padisCodeDescriptionMap[bookingCode] << " /"
           << padisAbbreviatedDescriptionMap[bookingCode] << "/ \n";
    }
  }

  dc << "*- - - - - - - - - - - - - - - - - - - - - - - - -  *\n";
}

void
Diag877Collector::printSvcFeeCurTable170Header(const int itemNo)
{
  if (!shouldCollectInRequestedContext(PROCESSING_T170))
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n";
  dc << " * SVC FEE CUR T170 ITEM NO : " << setw(7) << itemNo << " DETAIL INFO    * \n";
  dc << " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n\n";
  dc << " MATCH EXACT POS   : \n";
  dc << " V SEQ-NO  POSLTYPE POSLOC CUR FEE-AMNT     DEC    STATUS \n";
}

void
Diag877Collector::printStartStopTimeHeader(bool isErrorFromDOW)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n";
  dc << " * START TIME/STOP TIME VALIDATION                   * \n";
  dc << " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n";
  if (isErrorFromDOW)
    dc << "    FAILED DUE TO DOW VALIDATION \n\n";
  else
    dc << "    FAILED DUE TO START TIME/STOP TIME VALIDATION \n\n";
}

void
Diag877Collector::printSvcFeeCurInfoDetail(const SvcFeesCurrencyInfo& svcInfo, bool status)
{
  if (!shouldCollectInRequestedContext(PROCESSING_T170))
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  displayVendor(svcInfo.vendor(), true);

  dc << " " << setw(6) << svcInfo.seqNo() << "  ";

  if (!svcInfo.posLoc().isNull())
  {
    dc << setw(4) << svcInfo.posLoc().locType() << "     " << setw(6) << svcInfo.posLoc().loc()
       << " ";
  }
  else
    dc << "         "
       << "       ";

  dc << setw(3) << svcInfo.currency() << " ";

  dc.setf(ios::fixed, ios::floatfield);
  dc.precision(svcInfo.noDec());
  dc << setw(12) << svcInfo.feeAmount() << " " << setw(6) << svcInfo.noDec() << " ";

  if (status)
    dc << "PASS";
  else
    dc << "FAIL";

  dc << " \n";
}

void
Diag877Collector::printSvcFeeCurInfo170Label(bool isNullHeader)
{
  if (!shouldCollectInRequestedContext(PROCESSING_T170))
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  if (isNullHeader)
    dc << " DEFAULT POS EMPTY : \n";
  else
    dc << " DEFAULT POS FIRST : \n";
}

void
Diag877Collector::printS7OptionalServiceStatus(StatusS7Validation status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << " \n                              S7 STATUS : ";
  displayStatus(status);
  dc << "\n";
}

void
Diag877Collector::printS7RecordValidationFooter(const OptionalServicesInfo& info,
                                                const PricingTrx& trx)
{
}

void
Diag877Collector::displayStatus(StatusS7Validation status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  // Currently max length for error msg is 15
  switch (status)
  {
  case PASS_S7:
    dc << "PASS";
    break;
  case PASS_S7_DEFER:
    dc << "PASS/DEFER";
    break;
  case FAIL_S7_INPUT_TVL_DATE:
    dc << "FAIL IN TVL DATE";
    break;
  case FAIL_S7_TVL_DATE:
    dc << "FAIL TVL DATE";
    break;
  case FAIL_S7_INPUT_PSG_TYPE:
    dc << "FAIL INPUT PSG TYPE";
    break;
  case FAIL_S7_ACCOUNT_CODE:
    dc << "FAIL ACCOUNT CODE";
    break;
  case FAIL_S7_INPUT_TKT_DESIGNATOR:
    dc << "FAIL INPUT TKT DESIGN";
    break;
  case FAIL_S7_OUTPUT_TKT_DESIGNATOR:
    dc << "FAIL OUTPUT TKT DESIGN";
    break;
  case FAIL_S7_SECUR_T183:
    dc << "FAIL SECUR T183";
    break;
  case FAIL_S7_UPGRADE:
    dc << "FAIL UPGRADE CHECK";
    break;
  case FAIL_S7_UPGRADE_T198:
    dc << "FAIL UPGRADE T198";
    break;
  case FAIL_S7_SECTOR_PORTION:
    dc << "FAIL SECTOR PORTION";
    break;
  case FAIL_S7_FROM_TO_WITHIN:
    dc << "FAIL GEO FTW IND";
    break;
  case FAIL_S7_INTERMEDIATE_POINT:
    dc << "FAIL VIA LOC";
    break;
  case FAIL_S7_STOP_CNX_DEST:
    dc << "FAIL STOP CNX DEST";
    break;
  case FAIL_S7_CABIN:
    dc << "FAIL CABIN";
    break;
  case FAIL_S7_RBD_T198:
    dc << "FAIL RBD T198";
    break;
  case FAIL_S7_SFC_T170:
    dc << "FAIL SVC FEE CUR T170";
    break;
  case FAIL_S7_FREQ_FLYER_STATUS:
    dc << "FAIL FF STATUS";
    break;
  case FAIL_S7_EQUIPMENT:
    dc << "FAIL EQUIPMENT";
    break;
  case FAIL_S7_TOURCODE:
    dc << "FAIL TOUR CODE";
    break;
  case FAIL_S7_START_STOP_TIME:
    dc << "FAIL START/STOP TIME";
    break;
  case FAIL_S7_DOW:
    dc << "FAIL DOW";
    break;
  case FAIL_S7_ADVPUR:
    dc << "FAIL ADV PUR";
    break;
  case FAIL_S7_NOT_AVAIL_NO_CHANGE:
    dc << "SERVICE NOT AVAILABLE";
    break;
  case FAIL_S7_DEFER_BAGGAGE_RULE:
    dc << "FAIL DEFER BAG RULE";
    break;
  case PASS_S7_FREE_SERVICE:
    dc << "PASS SERVICE IS FREE";
    break;
  case PASS_S7_NOT_AVAIL:
    dc << "PASS SERVICE NOT AVAL";
    break;
  case FAIL_S7_RESULT_FC_T171:
    dc << "FAIL RESLT FC STATUS";
    break;
  case FAIL_S7_RULE_TARIFF_IND:
    dc << "FAIL RULE TARIFF IND";
    break;
  case FAIL_S7_RULE_TARIFF:
    dc << "FAIL RULE TARIFF";
    break;
  case FAIL_S7_RULE:
    dc << "FAIL RULE";
    break;
  case FAIL_S7_FARE_IND:
    dc << "FAIL FARE IND";
    break;
  case FAIL_S7_MILEAGE_FEE:
    dc << "MILEAGE FEE NOT APPL";
    break;
  case FAIL_S7_CXR_FLT_T186:
    dc << "FAIL CXR/FLT T186";
    break;
  case FAIL_S7_COLLECT_SUBTRACT:
    dc << "FAIL COLLECT/SUBTRACT";
    break;
  case FAIL_S7_NET_SELL:
    dc << "FAIL NET/SELL";
    break;
  case FAIL_S7_FEE_APPL:
    dc << "FEE APPLICATION";
    break;
  case FAIL_S7_ADV_PURCHASE_TKT_IND:
    dc << "ADVANCE PURCHASE TKT IND";
    break;
  case SOFT_PASS_S7:
    dc << "SOFT PASS";
    break;
  case FAIL_S7_AND_OR_IND:
    dc << "FAIL AND/OR IND";
    break;
  case FAIL_S7_BAGGAGE_WEIGHT_UNIT:
    dc << "FAIL BAG WEIGHT UNIT";
    break;
  case FAIL_S7_FEE_APPLICATION:
    dc << "FAIL FEE APPLICATION";
    break;
  case FAIL_S7_BAGGAGE_OCCURRENCE:
    dc << "FAIL BAG OCCURRENCE";
    break;
  case FAIL_S7_FREE_BAG_PIECES:
    dc << "FAIL FREE BAG PIECES";
    break;
  case FAIL_S7_TAX_APPLICATION:
    dc << "FAIL TAX APPLICATION";
    break;
  case FAIL_S7_AVAILABILITY:
    dc << "FAIL AVAILABILITY";
    break;
  case FAIL_S7_RULE_BUSTER_RMFC:
    dc << "FAIL RULE BUSTER RMFC";
    break;
  case FAIL_S7_PAX_MIN_MAX_AGE:
    dc << "FAIL PAX MIN MAX AGE";
    break;
  case FAIL_S7_PAX_OCCURRENCE:
    dc << "FAIL PAX OCCURRENCE";
    break;
  case FAIL_S7_BTA:
    dc << "FAIL BTA";
    break;
  case FAIL_S7_INTERLINE_IND:
    dc << "FAIL INTERLINE IND";
    break;
  default:
    dc << "UNKNOWN STATUS";
    break;
  }
}

void
Diag877Collector::displayStopCnxDestInd(Indicator ind)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << ind;
  switch (ind)
  {
  case 'C':
    dc << " - CONNECTION";
    break;
  case 'S':
    dc << " - STOPOVER";
    break;
  case 'F':
    dc << " - NOT FARE BREAK";
    break;
  case 'P':
    dc << " - NOT FARE BREAK OR STOPOVER";
    break;
  case 'T':
    dc << " - STOPOVER WITH GEO";
    break;
  default:
    break;
  }
}

void
Diag877Collector::displaySourceForFareCreated(Indicator ind)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  if (ind == '1')
    dc << "19/22";
  else if (ind == '2')
    dc << "25";
  else if (ind == '3')
    dc << "35";

  dc << "\n";
}

void
Diag877Collector::displayNotAvailNoCharge(Indicator ind)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  if (ind == 'X')
    dc << "X- SERVICE CODE IS NOT AVAILABLE";
  else if (ind == 'F')
    dc << "F- FREE SERVICE - NO EMD ISSUED";
  else if (ind == 'E')
    dc << "E- FREE SERVICE - EMD ISSUED";
  else if (ind == 'G')
    dc << "G- FREE SERVICE - NO EMD AND NO BKG REQ";
  else if (ind == 'H')
    dc << "H- FREE SERVICE - EMD ISSUED AND NO BKG REQ";
  else if (ind == 'D')
    dc << "D- DEFER BAGGAGE RULE FOR MARKETING CARRIER/DEFER TO THE MOST SIGNIFICANT CARRIER";
  dc << "\n";
}

void
Diag877Collector::displayRefundReissue(Indicator ind)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  if (ind == 'Y')
    dc << "Y - REFUNDABLE";
  else if (ind == 'N')
    dc << "N - NON-REFUNDABLE";
  else if (ind == 'R')
    dc << "R - REUSE TOWARDS FUTURE";
  dc << "\n";
}

void
Diag877Collector::displayFFMileageAppl(Indicator ind)
{
  if (!_active)
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
    dc << " - PER ITEM";
    break;
  case '4':
    dc << " - PER TRAVEL";
    break;
  case '5':
    dc << " - PER TICKET";
    break;
  default:
    break;
  }
}

void
Diag877Collector::displayFormOfTheRefund(Indicator ind)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  if (ind == '1')
    dc << "ORIGINAL FOP";
  else if (ind == '2')
    dc << "ELECTRONIC VOUCHER";
  dc << "\n";
}

void
Diag877Collector::printRBDTable198Header(const int itemNo)
{
  if (!shouldCollectInRequestedContext(PROCESSING_RBD))
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << " *- - - - - - *- - - - - - - - - - - - - - - - - - - - - - -* \n";
  dc << " * SEG INFO   * RBD T198 ITEM NO : " << setw(7) << itemNo << " DETAIL INFO      * \n";
  dc << " *- - - - - - *- - - - - - - - - - - - - - - - - - - - - - -* \n";
  dc << " ORIG DEST RBD SEQ NO MKT/OP CXR RBD 1 2 3 4 5  STATUS \n";
}

void
Diag877Collector::printRBDTable198Info(const BookingCode& bookingCode,
                                       const LocCode& origAirport,
                                       const LocCode& destAirport,
                                       const SvcFeesResBkgDesigInfo* info,
                                       StatusT198 status)
{
  if (!shouldCollectInRequestedContext(PROCESSING_RBD))
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << " " << setw(3) << origAirport << "  " << setw(3) << destAirport << "  " << setw(2)
     << bookingCode << "  " << setw(7) << info->seqNo() << "  " << setw(1) << info->mkgOperInd()
     << "    " << setw(2) << info->carrier() << "      " << setw(2) << info->bookingCode1()
     << setw(2) << info->bookingCode2() << setw(2) << info->bookingCode3() << setw(2)
     << info->bookingCode4() << setw(2) << info->bookingCode5() << " ";
  switch (status)
  {
  case PASS_T198:
    dc << "PASS";
    break;
  case FAIL_RBD_NOT_PROCESSED:
    dc << "NOT PROCESSED";
    break;
  case FAIL_ON_RBD_CXR:
    dc << "FAIL ON CXR";
    break;
  case FAIL_ON_RBD_CODE:
    dc << "FAIL ON RBD";
    break;
  default:
    break;
  }
  dc << "\n";
}

void
Diag877Collector::printResultingFareClassTable171Header(const int itemNo)
{
  if (!shouldCollectInRequestedContext(PROCESSING_T171))
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  dc << " *- - - - - - - - - - -*- - - - - - - - - - - - - - - - - - - - -* \n";
  dc << " * FARE INFO           * RBD T171 ITEM NO : " << setw(7) << itemNo
     << " DETAIL INFO  * \n";
  dc << " *- - - - - - - - - - -*- - - - - - - - - - - - - - - - - - - - -* \n";

  dc << " CXR  TYPE  FARE BASIS  SEQ NO  CXR  FARE CLASS  FARE TYPE  STATUS \n";
}

void
Diag877Collector::formatResultingFareClassTable171Info(const SvcFeesCxrResultingFCLInfo& info,
                                                       const std::string& fareBasis,
                                                       const FareType& fcaFareType,
                                                       const CarrierCode& carrier,
                                                       bool useFareCarrier)
{
  DiagCollector& dc = (DiagCollector&)*this;

  dc << " " << setw(3) << carrier << "  " << setw(4) << fcaFareType << "  " << setw(10) << fareBasis
     << " " << setw(7) << info.seqNo() << " " << setw(3) << info.carrier() << "  " << setw(11)
     << info.resultingFCL() << " " << setw(4) << info.fareType() << "       ";
}

CarrierCode
Diag877Collector::getCarrierCode(const PaxTypeFare& ptf, bool useFareCarrier)
{
  return useFareCarrier ? ptf.carrier() : ptf.fareMarket()->governingCarrier();
}

std::string
Diag877Collector::getFareBasis(bool isAncillaryRequest,
                               const TravelSeg* ptfTravelSeg,
                               const PaxTypeFare& ptf)
{
  PricingTrx* trx = static_cast<PricingTrx*>(_trx);

  std::string fareBasis =
      (!isAncillaryRequest)
          ? ServiceFeeUtil::getFareBasisRoot(*trx, ptf.createFareBasis(*trx, false))
          : ServiceFeeUtil::getFareBasisRoot(*trx, ptf.fare()->fareInfo()->fareClass().c_str());

  return ptfTravelSeg && !ptfTravelSeg->specifiedFbc().empty() ? ptfTravelSeg->specifiedFbc()
                                                               : fareBasis;
}

void
Diag877Collector::printResultingFareClassTable171Info(const PaxTypeFare& ptf,
                                                      const SvcFeesCxrResultingFCLInfo& info,
                                                      StatusT171 status,
                                                      bool useFareCarrier)
{
  if (!shouldCollectInRequestedContext(PROCESSING_T171))
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  const TravelSeg* ptfTravelSeg = TravelSegUtil::lastAirSeg(ptf.fareMarket()->travelSeg());

  dc.setf(ios::left, ios::adjustfield);

  PricingTrx* trx = static_cast<PricingTrx*>(_trx);
  AncRequest* ancReq = dynamic_cast<AncRequest*>(trx->getRequest());

  CarrierCode carrier = getCarrierCode(ptf, useFareCarrier);
  std::string fareBasis(getFareBasis(ancReq != nullptr, ptfTravelSeg, ptf));
  formatResultingFareClassTable171Info(
      info, fareBasis, ptf.fcaFareType(), carrier, useFareCarrier);

  switch (status)
  {
  case PASS_T171:
    dc << "PASS";
    break;
  case FAIL_ON_CXR:
    dc << "FAIL ON CXR";
    break;
  case FAIL_ON_FARE_CLASS:
    dc << "FAIL ON FARE CLASS";
    break;
  case FAIL_NO_FARE_TYPE:
    dc << "FAIL ON FARE TYPE";
    break;
  case SOFTPASS_FARE_CLASS:
    dc << "SOFT ON FARE CLASS";
    break;
  case SOFTPASS_FARE_TYPE:
    dc << "SOFT ON FARE TYPE";
    break;
  default:
    break;
  }
  dc << "\n";
}

void
Diag877Collector::printCarrierFlightT186Header(const int itemNo)
{
  if (!shouldCollectInRequestedContext(PROCESSING_T186))
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -* \n";
  dc << " * CXR/FLIGHT APPL T186 ITEM NO : " << setw(7) << itemNo << " DETAIL INFO       * \n";
  dc << " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -* \n";
  dc << " NO      MARKET CXR  OPERAT CXR  FLT1   FLT2   STATUS \n";
}

void
Diag877Collector::printCarrierFlightApplT186Info(const CarrierFlightSeg& info, StatusT186 status)
{
  if (!shouldCollectInRequestedContext(PROCESSING_T186))
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << " " << setw(7) << info.orderNo() << "     " << setw(3) << info.marketingCarrier()
     << "         " << setw(3) << info.operatingCarrier() << "     ";
  if (info.flt1() == -1)
  {
    dc << "****          ";
  }
  else
  {
    dc << setw(4) << info.flt1() << "   ";

    if (info.flt2() == 0)
      dc << "       ";
    else
      dc << setw(4) << info.flt2() << "   ";
  }
  switch (status)
  {
  case PASS_T186:
    dc << "PASS";
    break;
  case FAIL_ON_MARK_CXR:
    dc << "FAIL ON MARK CXR";
    break;
  case FAIL_ON_OPER_CXR:
    dc << "FAIL ON OPER CXR";
    break;
  case FAIL_ON_FLIGHT:
    dc << "FAIL ON FLIGHT NO";
    break;
  case SOFTPASS_FLIGHT:
    dc << "SOFTPASS ON FLTNO";
    break;
  default:
    break;
  }
  dc << "\n";
}

void
Diag877Collector::printNoCxrFligtT186Data()
{
  if (!shouldCollectInRequestedContext(PROCESSING_T186))
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "   CARRIER/FLIGHT REQUIREMENTS ARE NOT PROVIDED\n";
}

void
Diag877Collector::printS7SoftMatchedFields(const OptionalServicesInfo* info, const OCFees& ocFees)
{
  if (!_active)
    return;

  static const string ResFare = " * CXR/RESULT FARE CLASS";
  static const string CxrFlight = " * CXR/FLIGHT APPL T186";
  static const string StopCnx = " * STOP/CONN/DEST";
  static const string StopOverTime = " * STOPOVER TIME";
  static const string StopOverUnit = " * STOPOVER UNIT";
  static const string RuleTariff = " * RULE TARIFF";
  static const string StartTime = " * START TIME";
  static const string StopTime = " * STOP TIME";
  static const string Equipment = " * EQUIPMENT";
  static const string Rbd = " * RBD T198";
  static const string Cabin = " * CABIN";
  static const string Rule = " * RULE";
  static const string FriquentFs = " * FF STATUS";

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n";
  dc << " SOFT MATCH :";

  if (!ocFees.isAnyS7SoftPass())
    dc << " N/A\n";
  else
  {
    int i = 13;
    if (ocFees.softMatchS7Status().isSet(OCFees::S7_FREQFLYER_SOFT))
      displaySoftMatchField(FriquentFs, i);

    if (ocFees.softMatchS7Status().isSet(OCFees::S7_TIME_SOFT))
    {
      if (info->stopCnxDestInd() != BLANK)
        displaySoftMatchField(StopCnx, i);

      if (!info->stopoverTime().empty())
        displaySoftMatchField(StopOverTime, i);

      if (info->stopoverUnit() != BLANK)
        displaySoftMatchField(StopOverUnit, i);
    }

    if (ocFees.softMatchS7Status().isSet(OCFees::S7_CABIN_SOFT))
      displaySoftMatchField(Cabin, i);

    if (ocFees.softMatchS7Status().isSet(OCFees::S7_RBD_SOFT) && !ocFees.softMatchRBDT198().empty())
      displaySoftMatchField(Rbd, i);

    if (ocFees.softMatchS7Status().isSet(OCFees::S7_RESULTING_FARE_SOFT) &&
        !ocFees.softMatchResultingFareClassT171().empty())
      displaySoftMatchField(ResFare, i);

    if (ocFees.softMatchS7Status().isSet(OCFees::S7_RULETARIFF_SOFT))
      displaySoftMatchField(RuleTariff, i);

    if (ocFees.softMatchS7Status().isSet(OCFees::S7_RULE_SOFT))
      displaySoftMatchField(Rule, i);

    if (ocFees.softMatchS7Status().isSet(OCFees::S7_TIME_SOFT))
    {
      if (info->startTime() != 0)
        displaySoftMatchField(StartTime, i);

      if (info->stopTime() != 0)
        displaySoftMatchField(StopTime, i);
    }

    if (ocFees.softMatchS7Status().isSet(OCFees::S7_EQUIPMENT_SOFT))
      displaySoftMatchField(Equipment, i);

    if (ocFees.softMatchS7Status().isSet(OCFees::S7_CARRIER_FLIGHT_SOFT) &&
        !ocFees.softMatchCarrierFlightT186().empty())
      displaySoftMatchField(CxrFlight, i);

    if (i > 13)
      dc << "\n";
  }
  dc << " *- - - - - -  - - - - - - - - - - - - - - - - - - - - - - -*\n";
}

void
Diag877Collector::displaySoftMatchField(const std::string& softMatch, int& i)
{
  if (i + softMatch.size() >= 63)
  {
    i = 13;
    *this << "\n             ";
  }

  *this << softMatch;
  i += softMatch.size();
}

void
Diag877Collector::displayAmount(const OCFees& ocFees)
{
  DiagCollector& dc = (DiagCollector&)*this;

  if (ocFees.feeAmount() != 0.0)
  {
    dc.unsetf(std::ios::left);
    dc.setf(std::ios::right, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc.precision(ocFees.feeNoDec());
    dc << setw(8) << ocFees.feeAmount();
    dc.unsetf(std::ios::right);
    dc.setf(ios::left, ios::adjustfield);
    dc << setw(3) << ocFees.feeCurrency() << " ";
  }
  else
  {
    if (ocFees.optFee()->notAvailNoChargeInd() == 'X')
    {
      dc.setf(ios::right, ios::adjustfield);
      dc << setw(11) << "NOTAVAIL"
         << " ";
    }
    else
    {
      dc.setf(ios::right, ios::adjustfield);
      dc << setw(11) << "FREE"
         << " ";
    }
  }
}

void
Diag877Collector::displayVendor(const VendorCode& vendor, bool isDetailDisp)
{
  DiagCollector& dc = (DiagCollector&)*this;
  if ("ATP" == vendor)
  {
    if (isDetailDisp)
      dc << " A";
    else
      dc << "A  ";
  }
  else if ("MMGR" == vendor)
  {
    if (isDetailDisp)
      dc << " M";
    else
      dc << "M  ";
  }
  else if ("USOC" == vendor)
  {
    if (isDetailDisp)
      dc << " C";
    else
      dc << "C  ";
  }
  else if (isDetailDisp)
    dc << "  ";
  else
    dc << "   ";
}

Diag877Collector&
Diag877Collector::operator << ( const TaxResponse& taxResponse )
{
  if (!_active)
    return *this;

  DiagCollector& dc = (DiagCollector&)*this;
  PricingTrx* trx = dynamic_cast<PricingTrx*>(_trx);

  if (!trx)
    return *this;

  /*
    877/SC0CD/TAX  (return passed 0CD subcodes with taxes)
    877/SGML/TAX (return passed subcodes with taxes within ML subgroup)
    877/SQ1000/TAX (return passed subcodes with taxes where sequence no is 1000)
    877/TAX/FMFRALON (return passed subcodes with taxes for market FRALON)
  */

  const std::string& subcodeParam = dc.rootDiag()->diagParamMapItem("SC");
  const std::string& subgroupParam = dc.rootDiag()->diagParamMapItem("SG");
  const std::string& sequenceParam = dc.rootDiag()->diagParamMapItem("SQ");
  const std::string& faremarketParam = dc.rootDiag()->diagParamMapItem("FM");

  CurrencyCode paymentCurrency = trx->getRequest()->ticketingAgent()->currencyCodeAgent();
  if (!trx->getOptions()->currencyOverride().empty())
  {
    paymentCurrency = trx->getOptions()->currencyOverride();
  }

  FarePath* farePath = const_cast<FarePath*>(taxResponse.farePath());

  printTaxBanner();

  std::map<std::string, std::vector<std::pair<ServiceFeesGroup*, OCFees*> > > ocMap;
  std::map<std::string, std::vector<std::pair<ServiceFeesGroup*, OCFees*> > >::iterator ocMapI;

  std::vector<ServiceFeesGroup*>::const_iterator sfgIter =
      taxResponse.farePath()->itin()->ocFeesGroup().begin();
  std::vector<ServiceFeesGroup*>::const_iterator sfgIterEnd =
      taxResponse.farePath()->itin()->ocFeesGroup().end();
  for (; sfgIter != sfgIterEnd; ++sfgIter)
  {
    ServiceFeesGroup* sfg = (*sfgIter);

    if (!subgroupParam.empty() && subgroupParam != sfg->groupCode())
      continue;

    std::map<const FarePath*, std::vector<OCFees*> >& ocFeesMap = sfg->ocFeesMap();

    std::vector<OCFees*> ocFees = ocFeesMap[farePath];

    std::vector<OCFees*>::iterator iOcFees = ocFees.begin();
    std::vector<OCFees*>::iterator iEOcFees = ocFees.end();

    if (!ocFees.empty())
    {

      for (; iOcFees != iEOcFees; ++iOcFees)
      {
        OCFees* ocFees = *iOcFees;

        std::string fareMarket = std::string(ocFees->travelStart()->origin()->loc()) + " - " +
                                 std::string(ocFees->travelEnd()->destination()->loc());

        std::string fareMarketShort = std::string(ocFees->travelStart()->origin()->loc()) +
                                      std::string(ocFees->travelEnd()->destination()->loc());

        if (!subcodeParam.empty() && subcodeParam != ocFees->optFee()->serviceSubTypeCode())
          continue;

        if (!sequenceParam.empty())
        {
          std::ostringstream seqNo;
          seqNo << ocFees->optFee()->seqNo();
          if (sequenceParam != seqNo.str())
            continue;
        }

        if (!faremarketParam.empty() && faremarketParam != fareMarketShort)
          continue;

        ocMapI = ocMap.find(fareMarket);
        if (ocMapI == ocMap.end())
        {
          std::vector<std::pair<ServiceFeesGroup*, OCFees*> > v;
          ocMap[fareMarket] = v;
          ocMapI = ocMap.find(fareMarket);
        }

        ocMapI->second.push_back(std::pair<ServiceFeesGroup*, OCFees*>(sfg, ocFees));
      }
    }
  }

  const bool isExemptAllTaxes = trx->getRequest()->isExemptAllTaxes();
  const bool isExemptSpecificTaxes = trx->getRequest()->isExemptSpecificTaxes();
  const std::vector<std::string>& taxIdExempted = trx->getRequest()->taxIdExempted();

  for (ocMapI = ocMap.begin(); ocMapI != ocMap.end(); ocMapI++)
  {
    dc << "------------ PORTION OF TRAVEL : " << ocMapI->first << " -----------------\n";
    ServiceFeesGroup* prev_group = nullptr;
    std::vector<std::pair<ServiceFeesGroup*, OCFees*> >& v = ocMapI->second;
    std::vector<std::pair<ServiceFeesGroup*, OCFees*> >::const_iterator vI;

    for (vI = v.begin(); vI != v.end(); vI++)
    {
      ServiceFeesGroup* sfg = vI->first;
      OCFees& ocFees = *(vI->second);
      if (sfg != prev_group)
      {
        printS7GroupService(*farePath, sfg->groupCode());
        printS7CommonHeader();
      }
      prev_group = sfg;

      printS7OptionalFeeInfo(ocFees.optFee(), ocFees, PASS_S7);

      const std::vector<OCFees::TaxItem>& taxVector = ocFees.getTaxes();
      std::vector<OCFees::TaxItem>::const_iterator taxVectorI;
      uint16_t i = 0;
      for (taxVectorI = taxVector.begin(); taxVectorI != taxVector.end(); ++taxVectorI)
      {

        if (isTaxExempted(
                taxVectorI->getTaxCode(), isExemptAllTaxes, isExemptSpecificTaxes, taxIdExempted))
          continue;

        dc << std::string(30, ' ');
        dc.unsetf(std::ios::left);
        dc.setf(std::ios::right, std::ios::adjustfield);
        dc.setf(std::ios::fixed, std::ios::floatfield);
        dc.precision(taxVectorI->getNumberOfDec());
        dc << setw(8) << taxVectorI->getTaxAmount();
        dc.unsetf(std::ios::right);
        dc.setf(ios::left, ios::adjustfield);
        dc << setw(3) << paymentCurrency << " ";
        dc << setw(3) << taxVectorI->getTaxCode() << "\n";
        ++i;
        if (i == TAX_ON_OC_BUFF_SIZE)
          break;
      }
    }
  }
  return *this;
}

bool
Diag877Collector::isTaxExempted(const std::string& taxCode,
                                const bool isExemptAllTaxes,
                                const bool isExemptSpecificTaxes,
                                const std::vector<std::string>& taxIdExempted)
{
  if (isExemptAllTaxes)
    return true;

  if (isExemptSpecificTaxes)
  {

    if (taxIdExempted.empty())
      return true;

    std::vector<std::string>::const_iterator taxIdExemptedI = taxIdExempted.begin();
    std::vector<std::string>::const_iterator taxIdExemptedEndI = taxIdExempted.end();

    for (; taxIdExemptedI != taxIdExemptedEndI; taxIdExemptedI++)
    {
      if (taxCode.compare(0, taxIdExemptedI->size(), *taxIdExemptedI) == 0)
        return true;
    }
  }

  return false;
}

bool
Diag877Collector::shouldCollectInRequestedContext(Diag877Collector::ProcessingContext context) const
{
  return _active && rootDiag()->diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO";
}

} // namespace
