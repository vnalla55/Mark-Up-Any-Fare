//----------------------------------------------------------------------------
//  File:      Diag875Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 875 Service Fee - OC Fees
//
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
#include "Diagnostic/Diag875Collector.h"

#include "Common/FallbackUtil.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DBAccess/EmdInterlineAgreementInfo.h"
#include "DBAccess/MerchCarrierPreferenceInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "ServiceFees/ServiceFeesGroup.h"


#include <iomanip>
#include <iostream>

using namespace std;

namespace tse
{

void
Diag875Collector::printGroupCodesInTheRequest(PricingTrx& trx,
                                              const std::vector<ServiceGroup>& input,
                                              const std::vector<ServiceGroup>& vecInv,
                                              const std::vector<ServiceGroup>& vecInvTkt,
                                              ItinBoolMap& roundTrip,
                                              ItinBoolMap& international,
                                              bool noGroupCode) const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "*************** OC SUB CODE SERVICE ANALYSIS ******************\n";
  dc << "GROUP REQUESTED : ";
  if (!input.empty())
  {
    std::vector<ServiceGroup>::const_iterator it = input.begin();
    std::vector<ServiceGroup>::const_iterator itE = input.end();
    for (int i = 0; it != itE; ++it, i++)
    {
      if (i > 14)
      {
        i = 0;
        dc << "\n                  ";
      }
      dc << setw(3) << (*it);
    }
    dc << "\n";
  }
  else
    dc << "NO GROUP CODE IN THE ENTRY\n";

  const std::vector<ServiceFeesGroup*>& groups = trx.itin().front()->ocFeesGroup();
  if (groups.empty() && vecInvTkt.empty() && vecInv.empty())
  {
    dc << "GROUP ACTIVE    : NOT FOUND\n";
  }
  else
  {
    if (!groups.empty())
    {
      dc << "GROUP ACTIVE    : ";
      std::vector<ServiceFeesGroup*>::const_iterator it = groups.begin();
      std::vector<ServiceFeesGroup*>::const_iterator itE = groups.end();
      for (; it != itE; ++it)
      {
        dc << setw(3) << (*it)->groupCode();
      }
      dc << "\n";
    }

    if (!vecInvTkt.empty())
    {
      dc << "GROUP NOT ACTIVE: ";
      std::vector<ServiceGroup>::const_iterator it = vecInvTkt.begin();
      std::vector<ServiceGroup>::const_iterator itE = vecInvTkt.end();
      for (; it != itE; ++it)
      {
        dc << setw(3) << (*it);
      }
      dc << "\n";
    }
    if (!vecInv.empty())
    {
      dc << "INVALID GROUP   : ";
      std::vector<ServiceGroup>::const_iterator it = vecInv.begin();
      std::vector<ServiceGroup>::const_iterator itE = vecInv.end();
      for (int i = 0; it != itE; ++it, i++)
      {
        if (i > 14)
        {
          i = 0;
          dc << "\n                  ";
        }
        dc << setw(3) << (*it);
      }
      dc << "\n";
    }
  }

  dc << "TKT DATE        : " << trx.ticketingDate().dateToSqlString();
  if (trx.getOptions()->isOCHistorical())
  {
    dc << "                 "
       << "PCC SAME DATE : N";
  }

  dc << "\n";

  if ((1 < trx.itin().size()) && (trx.getOptions()->isCarnivalSumOfLocal()))
  {
    for (const auto itin : trx.itin())
    {
      if (itin)
      {
        dc << "JOURNEY TYPE    : ";

        if (roundTrip.getValForKey(itin))
        {
          dc << "RT ";
        }
        else
        {
          dc << "OW ";
        }

        if (international.getValForKey(itin))
        {
          dc << "INTL ";
        }
        else
        {
          dc << "DOM  ";
        }

        dc << "\n";
      }
    }
  }
  else
  {
    dc << "JOURNEY TYPE    : ";

    if (roundTrip.getValForKey(trx.itin().size() ? trx.itin()[0] : nullptr))
    {
      dc << "RT ";
    }
    else
    {
      dc << "OW ";
    }

    if (international.getValForKey(trx.itin().size() ? trx.itin()[0] : nullptr))
    {
      dc << "INTL ";
    }
    else
    {
      dc << "DOM  ";
    }

    dc << "\n";
  }
}

void
Diag875Collector::printJourneyDestination(const LocCode& point) const
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "-------------- JOURNEY DEST/TURNAROUND: " << point << " ------------------\n";
}

void
Diag875Collector::displayActiveCxrGroupHeader()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "\n-------------- CARRIER MERCH ACTIVATION DATA ----------------- \n"
     << " CXR  ACTIVE        ACTIVE GROUP\n";
}

void
Diag875Collector::displayMrktDrivenCxrHeader()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "\n--------------- MERCH CARRIER PREFERENCE DATA ----------------\n"
     << " CXR  VENDOR  ALTPROCESS  GROUP   SECTORPORTION  CONCURRENCE \n";
}

void
Diag875Collector::displayMrktDrivenCxrData(std::vector<MerchCarrierPreferenceInfo*>& mCxrPrefVec)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  std::vector<MerchCarrierPreferenceInfo*>::const_iterator it = mCxrPrefVec.begin();
  std::vector<MerchCarrierPreferenceInfo*>::const_iterator itE = mCxrPrefVec.end();

  for (; it != itE; ++it)
  {
    dc << " ";
    dc << setw(3) << (*it)->carrier() << "    ";
    dc << setw(4) << (*it)->prefVendor() << "      ";
    dc << setw(1) << (*it)->altProcessInd() << "         ";
    dc << setw(2) << (*it)->groupCode() << "           ";
    dc << setw(1) << (*it)->sectorPortionInd() << "             ";
    dc << setw(1) << (*it)->concurrenceInd() << "      ";
    dc << "\n";
  }
}

void
Diag875Collector::displayMrktDrivenCxrNoData()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << " NO DATA FOUND"
     << "\n";
}

void
Diag875Collector::displayCxrNoMAdata(const CarrierCode cxr)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << " " << setw(3) << cxr << "  NO MERCH ACTIVATION DATA "
     << "\n";
}

void
Diag875Collector::displayActiveCxrGroup(const CarrierCode cxr,
                                        bool active,
                                        const std::string& group)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << " " << setw(3) << cxr << "    ";
  if (!active)
    dc << "N";
  else
    dc << "Y";
  dc << "           ";
  dc << group << "\n";
}

void
Diag875Collector::displayNoActiveCxrGroupForOC()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "\n------------NO CARRIERS ACTIVE FOR OC PROCESSING-------------- \n";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag875Collector::printTravelPortionHeader
//
// Description:  This method will print the Header info
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag875Collector::printTravelPortionHeader(TravelSeg& seg1,
                                           TravelSeg& seg2,
                                           std::set<tse::CarrierCode> marketingCxr,
                                           std::set<tse::CarrierCode> operatingCxr,
                                           const FarePath& farePath,
                                           const PricingTrx& trx) const
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
      dc << " -------TKT1------\n";
    else if (farePath.itin()->getMultiTktItinOrderNum() == 2)
      dc << " -------TKT2------\n";
    else
      dc << " ----SINGLE TKT---\n";
  }
  else
    dc << " ------------\n";

  std::set<CarrierCode>::iterator cxrSt, cxrEn;
  if (!marketingCxr.empty())
  {
    cxrSt = marketingCxr.begin();
    cxrEn = marketingCxr.end();
    dc << "MARKETING  CXR     : ";
    for (; cxrSt != cxrEn; ++cxrSt)
      dc << setw(3) << *cxrSt;
    dc << "\n";
  }

  if (!operatingCxr.empty())
  {
    cxrSt = operatingCxr.begin();
    cxrEn = operatingCxr.end();

    dc << "OPERATING  CXR     : ";
    for (; cxrSt != cxrEn; ++cxrSt)
      dc << setw(3) << *cxrSt;
    dc << "\n";
  }
  dc << "REQUESTED PAX TYPE : " << farePath.paxType()->paxType();
  if (!farePath.paxType()->psgTktInfo().empty())
  {
    dc << " -";
    dc.setf(ios::right, ios::adjustfield);
    int pos = 28;
    for (PaxType::TktInfo* tki : farePath.paxType()->psgTktInfo())
    {
      if (pos >= 60)
      {
        dc << "\n" << setw(26) << " ";
        pos = 26;
      }
      dc << setw(4) << (std::string("T") + tki->tktRefNumber());
      pos += 4;
    }
  }
  dc << "\n-------------------------------------------------------------- \n"
     << "                  S5 RECORDS DATA PROCESSING\n";

  return;
}

void
Diag875Collector::printS5CommonHeader()
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "V CXR   SERVICE  IND GRP  SUBGRP SUBCODE STATUS\n";
}

void
Diag875Collector::printS5SubCodeInfo_old(const SubCodeInfo* info,
                                     bool cxrInd,
                                     const StatusS5Validation& status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  dc.setf(ios::left, ios::adjustfield);
  displayVendor(info->vendor());
  dc << " " << setw(3) << info->carrier();
  if (cxrInd)
    dc << "M  ";
  else
    dc << "O  ";
  displayFltTktMerchInd(info->fltTktMerchInd());

  dc << " " << setw(1) << info->industryCarrierInd() << "  " << setw(2) << info->serviceGroup()
     << "     " << setw(2) << info->serviceSubGroup() << "     " << setw(3)
     << info->serviceSubTypeCode() << "     ";

  displayStatus(status);
}

void
Diag875Collector::printS5SubCodeInfo(const SubCodeInfo* info,
                                     char carrierStrategyType,
                                     const StatusS5Validation& status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  dc.setf(ios::left, ios::adjustfield);
  displayVendor(info->vendor());
  dc << " " << setw(3) << info->carrier();
  dc << carrierStrategyType << "  ";
  displayFltTktMerchInd(info->fltTktMerchInd());

  dc << " " << setw(1) << info->industryCarrierInd() << "  " << setw(2) << info->serviceGroup()
     << "     " << setw(2) << info->serviceSubGroup() << "     " << setw(3)
     << info->serviceSubTypeCode() << "     ";

  displayStatus(status);
}

std::string
Diag875Collector::getCarrierListAsString(const std::set<tse::CarrierCode>& carriers, const std::string& separator) const
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
Diag875Collector::printDetailInterlineEmdProcessingStatusS5Info(bool isValidationPassed, const std::set<tse::CarrierCode>& failedCarriers)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << " S5 STATUS : ";
  if(isValidationPassed)
  {
    dc << "I-EMD PROCESSED\n";
  }
  else
  {
    dc << "I-EMD NOT PROCESSED " << getCarrierListAsString(failedCarriers, "/") << "\n";
  }
}

void
Diag875Collector::printDetailInterlineEmdProcessingS5Info(const tse::NationCode& nation,
                                                          const tse::CrsCode& gds,
                                                          const tse::CarrierCode& validatingCarrier,
                                                          const std::set<tse::CarrierCode>& marketingCarriers,
                                                          const std::set<tse::CarrierCode>& operatingCarriers
                                                          )
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "--------- INTERLINE EMD AGREEMENT PROCESSING DETAILS ---------\n"
     << " NATION             : " << nation << "\n"
     << " GDS                : " << gds << "\n"
     << " VALIDATING CXR     : " << validatingCarrier << "\n"
     << " MARKETING  CXR     : " << getCarrierListAsString(marketingCarriers, " ") << "\n"
     << " OPERATING  CXR     : " << getCarrierListAsString(operatingCarriers, " ") << "\n";
}

void
Diag875Collector::printNoInterlineDataFoundInfo()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "--------- NO INTERLINE EMD AGREEMENT DATA FOUND ---------\n";
}

void
Diag875Collector::printDetailInterlineEmdAgreementInfo(const std::vector<EmdInterlineAgreementInfo*>& eiaList) const
{
    if (!_active)
      return;

    DiagCollector& dc = (DiagCollector&)*this;
    dc.setf(ios::left, ios::adjustfield);
    dc << " ALLOWED    CXR     : ";
    std::vector<EmdInterlineAgreementInfo*>::const_iterator it = eiaList.begin();
    std::vector<EmdInterlineAgreementInfo*>::const_iterator itE = eiaList.end();

    const int maxCarriersPerLine = 13;
    for (int counter = 0; it != itE; ++it)
    {
        if (counter == maxCarriersPerLine)
        {
            counter = 0;
            dc << "\n                      ";
        }
        dc << setw(3) << (*it)->getParticipatingCarrier();
        ++counter;
    }
    dc << "\n";
}


// ----------------------------------------------------------------------------
// <PRE>
//
// @function Diag875Collector::printNoGroupCodeProvided
//
// Description:  SHOPPING does not provide group code
//
// </PRE>
// ----------------------------------------------------------------------------

void
Diag875Collector::printNoGroupCodeProvided()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << " *** NOT PROCESSED - NO GROUP CODE PROVIDED ***\n";
}

void
Diag875Collector::printNoOptionsRequested()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << " *** NOT PROCESSED - REQUESTED OPTIONS = 0 ***\n";
}

//----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag875Collector::printS5NotFound
//
// Description:  This method will print msg then S5 is not found
//
// </PRE>
// ----------------------------------------------------------------------------

void
Diag875Collector::printS5NotFound_old(const ServiceGroup group, const CarrierCode cxr, bool cxrInd)
    const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "  " << setw(3) << cxr;
  if (cxrInd)
    dc << "M";
  else
    dc << "O";
  dc << "               " << setw(3) << group << "                   DATA NOT FOUND\n";
}

void
Diag875Collector::printS5NotFound(const ServiceGroup group, const CarrierCode cxr, char carrierStrategyType)
    const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "  " << setw(3) << cxr;
  dc << carrierStrategyType;
  dc << "               " << setw(3) << group << "                   DATA NOT FOUND\n";
}

void
Diag875Collector::printPccIsDeactivated(const PseudoCityCode& pcc) const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << " *** NOT PROCESSED - AGENCY PCC " << pcc << " IS NOT ACTIVE ***\n";
}

void
Diag875Collector::printCanNotCollect(const StatusS5Validation status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << " *** NOT PROCESSED - ";
  displayStatus(status);
}

void
Diag875Collector::printS5SubCodeStatus(StatusS5Validation status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "                              S5 STATUS : ";
  displayStatus(status);
}

void
Diag875Collector::displayStatus(StatusS5Validation status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  // Currently max length for error msg is 15
  switch (status)
  {
  case PASS_S5:
    dc << "PASS";
    break;

  case OC_NOT_ACTIVE_SHOPPING:
    dc << "OC IS NOT ACTIVATED ***";
    break;

  case OC_NOT_ACTIVE_YET:
    dc << "TICKETING DATE BEFORE OC ACTIVATION DATE";
    break;

  case ANCILLARY_NOT_ACTIVE:
    dc << "ANCILLARY FEES ARE NOT ACTIVATED";
    break;

  case ANCILLARY_WP_DISPLAY_NOT_ACTIVE:
    dc << "WP*AE ANCILLARY FEES ARE NOT ACTIVATED";
    break;

  case ANCILLARY_R7_NOT_ACTIVE:
    dc << "POST TKT ANCILLARY FEES NOT ACTIVATED";
    break;

  case FAIL_S5_TKT_DATE:
    dc << "FAIL TKT DATE";
    break;

  case FAIL_S5_FLIGHT_RELATED:
    dc << "SVC NOT PROCESSED";
    break;

  case FAIL_S5_INDUSTRY_INDICATOR:
    dc << "FAIL INDUSTRY IND";
    break;

  case FAIL_S5_DISPLAY_CATEGORY:
    dc << "FAIL DISPLAY CAT";
    break;

  case FAIL_S5_EMD_TYPE:
    dc << "FAIL EMD TYPE";
    break;

  case FAIL_S5_EMD_AGREEMENT_CHECK:
    dc << "I-EMD NOT PROCESSED";
    break;

  case FAIL_S6:
    dc << "FAIL S6";
    break;

  case FAIL_S7:
    dc << "FAIL S7";
    break;

  default:
    dc << "UNKNOWN STATUS";
    break;
  }
  dc << "\n";
}

void
Diag875Collector::printDetailS5Info_old(const SubCodeInfo* info, bool cxrInd)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "------------------ SUB CODE DETAILED INFO --------------------\n"
     << " SUB CODE : " << setw(5) << info->serviceSubTypeCode() << " VN : ";
  displayVendor(info->vendor(), true);
  dc << " CXR : " << setw(3) << info->carrier();
  if (cxrInd)
    dc << "M ";
  else
    dc << "O ";

  dc << " EFF DATE : " << info->effDate().dateToSqlString() << "\n"
     << " SERVICE  : ";

  displayFltTktMerchInd(info->fltTktMerchInd());

  dc << "                  "
     << " DISC DATE: " << info->discDate().dateToSqlString() << "\n";
  dc << "  \n";
  dc << " GROUP    : " << setw(3) << info->serviceGroup() << "   "
     << " SUBGROUP : " << setw(3) << info->serviceSubGroup() << " DESCR1 : " << setw(3)
     << info->description1() << " DESCR2 : " << setw(3) << info->description2() << "\n";
  dc << "  \n";

  dc << " INDUSTRY/CXR : " << info->industryCarrierInd() << "   "
     << " CONCUR : ";
  displayConcurInd(info->concur());

  dc << " RFIC : " << setw(1) << info->rfiCode() << " "
     << " SSR :" << setw(4) << info->ssrCode() << "\n";

  dc << " DISPLAY CAT  : " << setw(2) << info->displayCat() << " "
     << " BOOKING : ";
  displayBookingInd(info->bookingInd());

  dc << " SSIM : " << setw(1) << info->ssimCode() << " "
     << " EMD : " << setw(1) << info->emdType() << "\n";

  dc << " COMMERCIAL : " << info->commercialName() << "\n";
  dc << " TEXT T196  : " << setw(8) << info->taxTextTblItemNo() << "\n";
  dc << " PICTURE NUM: " << setw(8) << info->pictureNo() << "\n";
  dc << " CONSUMPTION AT ISSUANCE : " << setw(1) << info->consumptionInd() << "\n";
}

void
Diag875Collector::printDetailS5Info(const SubCodeInfo* info, char carrierStrategyType)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "------------------ SUB CODE DETAILED INFO --------------------\n"
     << " SUB CODE : " << setw(5) << info->serviceSubTypeCode() << " VN : ";
  displayVendor(info->vendor(), true);
  dc << " CXR : " << setw(3) << info->carrier();
  dc << carrierStrategyType << " ";
  dc << " EFF DATE : " << info->effDate().dateToSqlString() << "\n"
     << " SERVICE  : ";

  displayFltTktMerchInd(info->fltTktMerchInd());

  dc << "                  "
     << " DISC DATE: " << info->discDate().dateToSqlString() << "\n";
  dc << "  \n";
  dc << " GROUP    : " << setw(3) << info->serviceGroup() << "   "
     << " SUBGROUP : " << setw(3) << info->serviceSubGroup() << " DESCR1 : " << setw(3)
     << info->description1() << " DESCR2 : " << setw(3) << info->description2() << "\n";
  dc << "  \n";

  dc << " INDUSTRY/CXR : " << info->industryCarrierInd() << "   "
     << " CONCUR : ";
  displayConcurInd(info->concur());

  dc << " RFIC : " << setw(1) << info->rfiCode() << " "
     << " SSR :" << setw(4) << info->ssrCode() << "\n";

  dc << " DISPLAY CAT  : " << setw(2) << info->displayCat() << " "
     << " BOOKING : ";
  displayBookingInd(info->bookingInd());

  dc << " SSIM : " << setw(1) << info->ssimCode() << " "
     << " EMD : " << setw(1) << info->emdType() << "\n";

  dc << " COMMERCIAL : " << info->commercialName() << "\n";
  dc << " TEXT T196  : " << setw(8) << info->taxTextTblItemNo() << "\n";
  dc << " PICTURE NUM: " << setw(8) << info->pictureNo() << "\n";
  dc << " CONSUMPTION AT ISSUANCE : " << setw(1) << info->consumptionInd() << "\n";
}

void
Diag875Collector::displayConcurInd(Indicator ind)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  if (ind == '1')
    dc << "FLIGHT   ";
  else if (ind == '2')
    dc << "RULE     ";
  else
    dc << "X        ";
}

void
Diag875Collector::displayBookingInd(ServiceBookingInd ind)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  if (ind == "01")
    dc << "SSR      ";
  else if (ind == "02")
    dc << "AUX SEGM ";
  else if (ind == "03")
    dc << "ASK CXR  ";
  else if (ind == "04")
    dc << "NO BKG RQ";
  else
    dc << "         ";
}
void
Diag875Collector::displayVendor(const VendorCode& vendor, bool ind)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  if (vendor == "ATP")
  {
    if (ind)
      dc << "ATP ";
    else
      dc << "A";
  }
  else if (vendor == "MMGR")
  {
    if (ind)
      dc << "MM  ";
    else
      dc << "M";
  }
  else if (vendor == "USOC")
  {
    if (ind)
      dc << "USOC";
    else
      dc << "C";
  }
  else if (ind)
    dc << "    ";
  else
    dc << " ";
}
void
Diag875Collector::displayTicketDetail(const PricingTrx* ptrx)
{
  if (!_active)
    return;

  const AncillaryPricingTrx* trx = dynamic_cast<const AncillaryPricingTrx*>(ptrx);
  if (nullptr == trx)
    return;

  const AncRequest* req = dynamic_cast<const AncRequest*>(trx->getRequest());
  if (nullptr == req)
    return;

  if (!req->isPostTktRequest())
    return;
  //          1         2         3         4         5         6
  // 1234567890123456789012345678901234567890123456789012345678901234
  *this << "\n";
  *this << "-------------- TICKET DETAILS --------------------------------\n";
  *this << "NAME TICKET REF/NBR                FLIGHT SEGMENTS  GUARANTEED\n";
  for (Itin* itin : trx->itin())
  {
    std::vector<int> segIds;
    for (TravelSeg* ts : itin->travelSeg())
    {
      AirSeg* as = dynamic_cast<AirSeg*>(ts);
      if (!as || as->resStatus() != "OK")
        continue;
      segIds.push_back(as->pnrSegment());
    }
    int prev;
    std::stringstream segStr;
    std::vector<int>::iterator ss, st = segIds.begin();
    std::vector<int>::iterator se = segIds.end();
    for (; st != se; st = ss)
    {
      segStr << *st;
      // find next consecutive set
      for (ss = st + 1, prev = *st; ss != se && prev + 1 == *(ss); ss++)
        prev = *ss;
      // if this is range (more than to elements in the set
      if (ss - st > 1)
        segStr << "-" << prev;
      // will be next set
      if (ss != se)
        segStr << "/";
    }
    Indicator ind(' ');
    if (req->ancillNonGuaranteePerItin().find(itin) != req->ancillNonGuaranteePerItin().end())
      ind = req->ancillNonGuaranteePerItin().find(itin)->second ? 'N' : 'Y';

    // display from itin and pricing itins
    displayTicketDetail(req, itin, segStr.str(), ind);
    if (req->pricingItins().find(itin) != req->pricingItins().end())
    {
      std::vector<Itin*>::const_iterator it = req->pricingItins().find(itin)->second.begin();
      std::vector<Itin*>::const_iterator ie = req->pricingItins().find(itin)->second.end();
      for (; it != ie; it++)
        displayTicketDetail(req, *it, segStr.str(), ind);
    }
  }
}
void
Diag875Collector::displayTicketDetail(const AncRequest* req,
                                      const Itin* itin,
                                      const std::string& segStr,
                                      Indicator TktInd)
{
  if (req->paxTypesPerItin().find(itin) != req->paxTypesPerItin().end())
  {
    std::vector<PaxType*>::const_iterator it = req->paxTypesPerItin().find(itin)->second.begin();
    std::vector<PaxType*>::const_iterator ie = req->paxTypesPerItin().find(itin)->second.end();
    for (; it != ie; it++)
    {
      for (PaxType::TktInfo* tki : (*it)->psgTktInfo())
      {
        this->setf(ios::right, ios::adjustfield);
        *this << setw(4) << tki->psgNameNumber() << setw(5)
              << (std::string("T") + tki->tktRefNumber());
        this->setf(ios::left, ios::adjustfield);
        *this << "-" << setw(26) << tki->tktNumber();
        this->setf(ios::right, ios::adjustfield);
        *this << setw(14) << segStr << setw(12) << TktInd << "\n";
      }
    }
  }
}

void
Diag875Collector::displayGroupDescriptionEmpty(ServiceGroup sg) const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "            " << setw(3) << sg << "NOT PROCESSED - GROUP DESCRIPTION IS EMPTY\n";
}
}
