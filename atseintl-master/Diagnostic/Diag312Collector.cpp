//----------------------------------------------------------------------------
//  File:        Diag312Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 312 (Surcharges Rule)
//
//  Updates:
//          08/19/2004  VK - create.
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

#include "Diagnostic/Diag312Collector.h"

#include "Common/DateTime.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/SurchargeData.h"
#include "DBAccess/CarrierFlight.h"
#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/SurchargesInfo.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"

#include <iomanip>
#include <iostream>

using namespace std;

namespace tse
{
void
Diag312Collector::diag312Collector(PricingTrx& trx,
                                   const PaxTypeFare& paxTypeFare,
                                   const SurchargesInfo* srr)
{
  if (_active)
  {
    Diag312Collector& dc = *this;

    writeHeader(paxTypeFare, *srr);

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << "CATEGORY 12 RULE DATA:\n\n";

    if (srr->unavailTag() == 'X') // dataUnavailable
    {
      dc << " DATA UNAVAILABLE" << '\n';
    }
    else if (srr->unavailTag() == 'Y') // textOnly
    {
      dc << " TEXT DATA ONLY" << '\n';
    }
    // 994
    dc << " OVERRIDE TABLE 994 NUMBER: ";
    dc << setw(7) << srr->overrideDateTblItemNo() << '\n';

    dc << " SURCHARGE TYPE:            ";
    dc << setw(3) << srr->surchargeType() << '\n';

    dc << " EQUIPMENT TYPE:            ";
    dc << setw(3) << srr->equipType() << '\n';

    dc << " TRAVEL PORTION:            ";
    dc << setw(15) << formatTravelPortion(srr->tvlPortion()) << '\n';

    dc << " SURCHARGE APPLIES TO       ";
    dc << setw(27) << formatSurchargeApplication(srr->surchargeAppl(), paxTypeFare.vendor())
       << '\n';

    dc << " GEO TABLE 995 NUMBER:      ";
    dc << setw(7) << srr->geoTblItemNo() << '\n';

    bool ddgeo = (srr->sectorPortion() == 'S' || srr->sectorPortion() == 'P') &&
                 (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) ==
                      RuleConst::DIAGNOSTIC_INCLUDE_GEO ||
                  trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO");
    bool dd986 = srr->carrierFltTblItemNo() &&
                 (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "986" ||
                  trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO");

    dc << " GEO TABLE 995 NUMBER BTW:  ";
    dc << setw(7) << srr->geoTblItemNoBtw() << '\n';
    if (ddgeo)
    {
      dc << std::setw(28) << " ";
      RuleUtil::diagGeoTblItem(srr->geoTblItemNoBtw(), srr->vendor(), trx, *this);
      dc << '\n';
    }

    dc << " GEO TABLE 995 NUMBER AND:  ";
    dc << setw(7) << srr->geoTblItemNoAnd() << '\n';
    if (ddgeo)
    {
      dc << std::setw(28) << " ";
      RuleUtil::diagGeoTblItem(srr->geoTblItemNoAnd(), srr->vendor(), trx, *this);
      dc << '\n';
    }

    dc << " SECTOR/PORTION:            ";
    dc << srr->sectorPortion() << '\n';
    dc << " TBL986:                    ";
    dc << srr->carrierFltTblItemNo() << '\n';
    if (dd986)
    {
      const CarrierFlight* table986 =
          trx.dataHandle().getCarrierFlight(srr->vendor(), srr->carrierFltTblItemNo());

      std::vector<CarrierFlightSeg*>::const_iterator carrierFlightI = table986->segs().begin();
      std::vector<CarrierFlightSeg*>::const_iterator carrierFlightEndI = table986->segs().end();
      for (; carrierFlightI != carrierFlightEndI; carrierFlightI++)
      {
        dc << std::setw(28) << " ";
        CarrierFlightSeg* item = *carrierFlightI;
        if (item->flt2() != 0)
        {
          dc << "BETWEEN " << item->marketingCarrier() << item->flt1() << " AND "
             << item->marketingCarrier() << item->flt2();
        }
        else
        {
          dc << item->marketingCarrier();
          if (item->flt1() != -1)
            dc << item->flt1();
        }
        if (!item->operatingCarrier().empty())
        {
          dc << " OPER " << item->operatingCarrier();
        }
        dc << '\n';
      }
    }

    // date application

    if (!srr->startYear() && !srr->startMonth() && !srr->startDay() && !srr->stopYear() &&
        !srr->stopMonth() && !srr->stopDay())
    {
      dc << " START DATA:                ";
      dc << setw(7) << "NONE" << '\n';
      dc << " STOP DATA:                 ";
      dc << setw(7) << "NONE" << '\n';
    }
    else
    {
      dc.setf(std::ios::right, std::ios::adjustfield);

      dc << " START DATA:                ";
      if (srr->startYear())
      {
        dc << setw(4) << setfill('0') << srr->startYear();
      }
      else
      {
        dc << "0000";
      }
      dc << setw(1) << "/";
      dc << setw(2) << setfill('0') << srr->startMonth();
      dc << setw(1) << "/";

      if (srr->startDay())
      {
        dc << setw(2) << setfill('0') << srr->startDay() << '\n';
      }
      else
      {
        dc << setw(2) << "00" << '\n';
      }
      dc << " STOP DATA:                 ";
      if (srr->stopYear())
      {
        dc << setw(4) << setfill('0') << srr->stopYear();
      }
      else
      {
        dc << "0000";
      }
      dc << setw(1) << "/";
      dc << setw(2) << setfill('0') << srr->stopMonth();
      dc << setw(1) << "/";
      if (srr->stopDay())
      {
        dc << setw(2) << setfill('0') << srr->stopDay() << '\n';
      }
      else
      {
        dc << setw(2) << "00" << '\n';
      }
    }
    //
    dc.setf(std::ios::right, std::ios::adjustfield);

    dc << setfill(' ');

    dc << " DAY OF WEEK:               ";
    if (!srr->dow().empty())
    {
      dc << setw(8) << srr->dow() << '\n';
    }
    else
    {
      dc << "NONE" << '\n';
    }

    // tod application

    dc << " TIME APPLICATION:          ";
    dc << formatTimeApplication(srr->todAppl()) << '\n';
    // time
    if (srr->startTime() == -1 && srr->stopTime() == -1)
    {
      dc << " START TIME:                ";
      dc << "NONE" << '\n';
      dc << " STOP TIME:                 ";
      dc << "NONE" << '\n';
    }
    else
    {
      dc << " START TIME:                ";
      dc << std::setw(2) << std::setfill('0') << srr->startTime() / 60 << ":" << std::setw(2)
         << std::setfill('0') << srr->startTime() % 60 << '\n';

      dc << " STOP TIME:                 ";
      dc << std::setw(2) << std::setfill('0') << srr->stopTime() / 60 << ":" << std::setw(2)
         << std::setfill('0') << srr->stopTime() % 60 << '\n';
    }

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << " BOOKING CODE:              ";
    dc << setw(3) << srr->bookingCode() << '\n';

    if (srr->surchargeCur1().empty() && srr->surchargeCur2().empty() &&
        srr->surchargePercentAppl() == RuleConst::FARE)
    {
      dc << " SURCHARGE PERCENT:         ";
      dc << setw(8) << srr->surchargePercent() << '\n';
    }
    dc << "************************************************************\n";
    dc << '\n';
  }
}

void
Diag312Collector::writeHeader(const PaxTypeFare& paxTypeFare, const SurchargesInfo& srr)
{
  Diag312Collector& dc = *this;

  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "*            ATSE CATEGORY 12 SURCHARGES RULE              *\n";
  dc << "************************************************************\n";
  dc << setw(3) << paxTypeFare.fareMarket()->origin()->loc();
  dc << "-";
  dc << setw(4) << paxTypeFare.fareMarket()->destination()->loc();
  dc << setw(3) << paxTypeFare.fare()->carrier();
  dc << setw(4) << paxTypeFare.actualPaxType()->paxType();
  dc << setw(11) << paxTypeFare.createFareBasis(nullptr);
  dc << std::setw(8) << Money(paxTypeFare.fareAmount(), paxTypeFare.currency()) << " ";
  dc << std::setw(10) << "    REC3-";
  dc << setw(8) << srr.itemNo();
  dc << '\n';
}

void
Diag312Collector::displaySurchargeData(PricingTrx& trx,
                                       const SurchargeData* surchargeData,
                                       const SurchargesInfo& surchargeInfo,
                                       bool paperTktSurchargeMayApply)
{
  if (_active)
  {
    Diag312Collector& dc = *this;

    dc << "    SURCHARGE RULE - ADD SURCHARGE\n";
    dc << "    SEGMENT        - ";
    dc << surchargeData->brdAirport() << "-" << surchargeData->offAirport() << '\n';
    dc << "    APPLY TO       - ";
    dc << surchargeData->fcBrdCity() << "-" << surchargeData->fcOffCity() << '\n';
    dc << "    CURRENCY       - " << surchargeData->currSelected() << '\n';
    dc << "    AMOUNT         - ";
    dc << setw(8) << Money(surchargeData->amountSelected(), surchargeData->currSelected()) << '\n';

    if (surchargeInfo.surchargeType() == RuleConst::OTHER && paperTktSurchargeMayApply)
    {
      dc << "    PAPER TICKET SURCHARGE APPLIES\n";
    }
    dc << "************************************************************\n";
  }
}

void
Diag312Collector::displaySideTripSurchargeData(const SurchargeData* surchargeData,
                                               const LocCode& city)
{
  if (_active)
  {
    Diag312Collector& dc = *this;

    dc << "    SURCHARGE RULE      - ADD SIDE TRIP SURCHARGE\n";
    dc << "    AIRPORT CODE        - " << city << '\n';
    dc << "    CURRENCY            - " << surchargeData->currSelected() << '\n';
    dc << "    AMOUNT              - ";
    dc << setw(8) << Money(surchargeData->amountSelected(), surchargeData->currSelected()) << '\n';
    dc << "************************************************************\n";
  }
}

std::string
Diag312Collector::formatSurchargeApplication(const Indicator& surrAppl, const VendorCode& vendor)
    const
{
  if (isalpha(surrAppl)) // negative surcharges
    return formatNegativeSurchargeApplication(surrAppl);

  if (vendor == Vendor::ATPCO)
  {
    switch (surrAppl)
    {
    case ' ':
      return "ANY PASSENGER";
    case '1':
      return "CHILD";
    case '2':
      return "ADULT AND CHILD AND INFANT";
    case '3':
      return "ADULT AND CHILD DISCOUNT AND INFANT DISCOUNT";
    case '4':
      return "ADULT";
    default:
    {
      return "UNKNOWN";
    }
    }
  }
  else // SITA
  {
    switch (surrAppl)
    {
    case ' ':
      return "ANY PASSENGER";
    case '1':
      return "CHILD";
    case '2':
      return "ADULT AND CHILD";
    case '3':
      return "ADULT AND CHILD DISCOUNT";
    case '4':
      return "ADULT";
    case '5':
      return "ADULT AND DISC CHILD AND INFANT";
    case '6':
      return "ADULT AND CHILD AND FREE INFANT";
    case '7':
      return "ADULT AND DISC CHILD AND FREE INF";
    case '8':
      return "INFANT";
    default:
    {
      return "UNKNOWN";
    }
    }
  }
}

std::string
Diag312Collector::formatNegativeSurchargeApplication(const Indicator& surrAppl) const
{
  switch (surrAppl)
  {
  case 'A':
    return "ANY PASSENGER";
  case 'B':
    return "CHILD AND INFANT";
  case 'C':
    return "ADULT, CHILD AND INFANT";
  case 'D':
    return "ADULT AND CHILD DISCOUNT AND INFANT DISCOUNT";
  case 'E':
    return "ADULT";
  case 'G':
    return "ADULT AND CHILD AND FREE INFANT";
  case 'H':
    return "ADULT AND DISC CHILD AND FREE INF";
  case 'I':
    return "INFANT";
  default:
  {
    return "UNKNOWN";
  }
  }
}

std::string
Diag312Collector::formatTravelPortion(const Indicator& tvlPortion) const
{
  switch (tvlPortion)
  {
  case ' ':
  case '1':
    return "PER COMPONENT";
  case '2':
    return "ROUNDTRIP";
  case '3':
    return "PER TRANSFER";
  case '4':
    return "PER TICKET";
  case '5':
    return "PER COUPON";
  case '6':
    return "PER DIRECTION";
  default:
    return "UNKNOWN";
  }
}

std::string
Diag312Collector::formatTimeApplication(const Indicator& timeAppl) const
{
  switch (timeAppl)
  {
  case ' ':
    return "NONE";
  case 'R':
    return "RANGE";
  case 'D':
    return "DAILY";
  default:
    return "UNKNOWN";
  }
}
}
