//----------------------------------------------------------------------------
//  File:        Diag888Collector.cpp
//  Authors:
//  Created:
//
//  Description: Diagnostic 888- S8 Branded Fares - programs, fares, services
//  Updates:
//
//
//  Copyright Sabre 2013
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
#include "Diagnostic/Diag888Collector.h"

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/BrandedFare.h"
#include "DBAccess/BrandedFareSeg.h"
#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/TaxText.h"


#include <algorithm>
#include <iomanip>
#include <iostream>

using namespace std;

namespace tse
{
void
Diag888Collector::printS8Banner()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  //         012345678901234567890123456789012345678901234567890123456789012 - 63
  dc << "*************** BRANDED FARES - S8 ANALYSIS ******************\n";
  return;
}

void
Diag888Collector::printS8NotFound(const VendorCode& vendor, const CarrierCode& cxrCode)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  displayVendor(vendor);

  dc << setw(3) << cxrCode << "     DATA NOT FOUND\n";
  return;
}

void
Diag888Collector::printS8NotProcessed()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "   DATA NOT PROCESSED\n";
  return;
}

void
Diag888Collector::printS8FareMarket(const FareMarket& market)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  dc << "------------ FARE MARKET : ";
  dc << market.origin()->loc() << " - " << market.destination()->loc();
  dc << "   CXR - " << market.governingCarrier();
  dc << " -------------\n";
  return;
}

void
Diag888Collector::printS8CommonHeader()
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  //         012345678901234567890123456789012345678901234567890123456789012 - 63
  dc << "V CXR   SEQ      PAX  PROGRAM      BRANDS\n";
}

void
Diag888Collector::printS8BrandedFaresContent(const BrandedFare* info)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  displayVendor(info->vendor());

  dc << setw(3) << info->carrier() << "  ";

  dc << setw(7) << info->seqNo() << "  " << setw(3) << info->psgType();

  dc << "  " << setw(10) << info->programCode();
  dc << "    " << setw(2) << info->segCount() << "\n";
}

void
Diag888Collector::printS8DetailContent(PricingTrx& trx, const BrandedFare* info)
{
  if (!_active)
    return;
  const std::string& suppTable = trx.diagnostic().diagParamMapItem(Diagnostic::TABLE_DETAILS);
  const std::string& sequenceNo = trx.diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER);

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  //         012345678901234567890123456789012345678901234567890123456789012 - 63
  dc << "--------------- BRANDED FARES S8 DETAILED INFO ---------------\n"
     << " CARRIER: " << setw(3) << info->carrier() << "           "
     << " VENDOR : " << setw(5) << info->vendor() << "   "
     << " SOURCE : " << setw(1) << info->source();
  dc << "\n";

  dc << " SEQ NBR : " << setw(7) << info->seqNo() << "\n";

  dc << " S8 EFF DATE : " << setw(10) << info->effDate().dateToSqlString() << "     ";
  dc << " TVL DATE START : ";
  if (isStartDateSpecified(*info))
  {
    std::ostringstream osM;
    if (!info->tvlFirstYear())
      osM << "XXXX";
    else
      osM << setw(4) << info->tvlFirstYear();
    osM << "-" << setw(2) << std::setfill('0') << info->tvlFirstMonth() << "-";
    if (!info->tvlFirstDay())
      osM << "XX\n";
    else
      osM << setw(2) << std::setfill('0') << info->tvlFirstDay();
    dc << setw(10) << osM.str().substr(0, 10) << "\n";
  }
  else
    dc << "1980-01-01\n";

  dc << " S8 DISC DATE: " << setw(10) << info->discDate().dateToSqlString() << "     ";

  dc << " TVL DATE STOP  : ";
  if (isStopDateSpecified(*info))
  {
    std::ostringstream osM;
    if (!info->tvlLastYear())
      osM << "XXXX";
    else
      osM << setw(4) << info->tvlLastYear();
    osM << "-" << setw(2) << std::setfill('0') << info->tvlLastMonth() << "-";
    if (!info->tvlLastDay())
      osM << "XX\n";
    else
      osM << setw(2) << std::setfill('0') << info->tvlLastDay();
    dc << setw(10) << osM.str().substr(0, 10) << "\n";
  }
  else
    dc << "\n";

  dc << "  \n";
  dc << "     PAX TYPE  : " << info->psgType() << "\n";

  dc << " PRIVATE IND   : " << setw(2) << info->publicPrivateInd() << "\n";

  dc << " ACC CODE T172 : ";
  if (info->svcFeesAccountCodeTblItemNo() > 0)
    dc << setw(7) << info->svcFeesAccountCodeTblItemNo();
  dc << "\n";

  dc << " SECURITY T183 : ";
  if (info->svcFeesSecurityTblItemNo() > 0)
    dc << setw(7) << info->svcFeesSecurityTblItemNo();
  dc << "\n";

  dc << "DIRECTIONALITY : " << setw(2) << info->directionality() << "\n";

  dc << "     LOC1 TYPE : " << setw(2) << info->locKey1().locType() << "   LOC1 : " << setw(8)
     << info->locKey1().loc() << "  LOC1 ZONE : ";
  if (!info->loc1ZoneTblItemNo().empty() && info->loc1ZoneTblItemNo() != "0000000")
    dc << setw(8) << info->loc1ZoneTblItemNo();
  dc << "\n";

  dc << "     LOC2 TYPE : " << setw(2) << info->locKey2().locType() << "   LOC2 : " << setw(8)
     << info->locKey2().loc() << "  LOC2 ZONE : ";
  if (!info->loc2ZoneTblItemNo().empty() && info->loc2ZoneTblItemNo() != "0000000")
    dc << setw(8) << info->loc2ZoneTblItemNo();
  dc << "\n";

  std::string gd;
  globalDirectionToStr(gd, info->globalInd());

  dc << " GLOBAL IND : ";
  if (info->globalInd() != GlobalDirection::NO_DIR)
    dc << setw(3) << gd;

  dc << "         MATRIX : ";
  if (info->oneMatrix() != BLANK)
    dc << info->oneMatrix();
  dc << "\n";

  dc << "  PROGRAM CODE : ";
  if (!info->programCode().empty())
    dc << setw(11) << info->programCode();
  dc << "\n";

  dc << "  PROGRAM TEXT : ";
  if (!info->programText().empty())
    dc << info->programText();
  dc << "\n";

  dc << "ANCILLARY T166 : ";
  if (info->svcFeesFeatureTblItemNo() > 0)
    dc << setw(7) << info->svcFeesFeatureTblItemNo();
  dc << "\n";

  dc << "TEXT T196 : ";
  if (info->taxTextTblItemNo() > 0)
    dc << setw(7) << info->taxTextTblItemNo();
  dc << "\n";

  //  supporting tables detailed info if requested
  //
  if (!sequenceNo.empty() && suppTable == "YES")
  {
    if (info->svcFeesAccountCodeTblItemNo() > 0)
    {
      printAccountCodeTable172Header(info->svcFeesAccountCodeTblItemNo());
      const std::vector<SvcFeesAccCodeInfo*>& accCodeInfos =
          getSvcFeesAccCodeInfo(trx, info->svcFeesAccountCodeTblItemNo());

      for (const auto accCodeInfo : accCodeInfos)
      {
        *this << *accCodeInfo;
      }
      dc << "\n";
    }
    if (info->svcFeesSecurityTblItemNo() > 0)
    {
      printSecurityTable183Header(info->svcFeesSecurityTblItemNo());
      const std::vector<SvcFeesSecurityInfo*>& securityInfo =
          getSecurityInfo(trx, info->vendor(), info->svcFeesSecurityTblItemNo());

      std::vector<SvcFeesSecurityInfo*>::const_iterator secI = securityInfo.begin();
      const std::vector<SvcFeesSecurityInfo*>::const_iterator secEnd = securityInfo.end();
      for (; secI != secEnd; ++secI)
      {
        printSecurityTable183Info(*secI, NO_VAL);
      }
    }

    if (info->svcFeesFeatureTblItemNo() > 0)
    {
      dc << "*-----------------------------------------------------*\n";
      dc << "*   FEE FEATURE T166 ITEM NO : " << setw(7) << info->svcFeesFeatureTblItemNo()
         << "                *\n";
      dc << "*-----------------------------------------------------*\n";

      const std::vector<SvcFeesFeatureInfo*>& t166Info =
          getT166Info(trx, info->vendor(), info->svcFeesFeatureTblItemNo());

      std::vector<SvcFeesFeatureInfo*>::const_iterator t166Id = t166Info.begin();
      const std::vector<SvcFeesFeatureInfo*>::const_iterator t166IdEnd = t166Info.end();
      for (; t166Id != t166IdEnd; ++t166Id)
      {
        printT166DetailInfo(*t166Id);
      }
    }
  }
  //
  dc << "-------------------------------------------------------------\n";
  dc << " BRAND SEGMENTS : ";
  if (info->segCount() != 0)
    dc << setw(3) << info->segCount() << "\n";

  if (info->segCount() != 0)
  {
    std::vector<BrandedFareSeg*>::const_iterator it = info->segments().begin();
    std::vector<BrandedFareSeg*>::const_iterator itE = info->segments().end();
    for (; it != itE; ++it)
    {
      //              012345678901234567890123456789012345678901234567890123456789012 - 63
      dc << "-------------------------------------------------------------\n";
      dc << " SEG NBR: " << setw(6) << (*it)->segNo() << "\n";
      dc << " TIER: " << setw(4) << (*it)->tier() << "    BRAND NAME:" << setw(31)
         << (*it)->brandName() << "\n";
      dc << " FARE ID T189 : ";
      if ((*it)->svcFeesFareIdTblItemNo() > 0)
        dc << setw(7) << (*it)->svcFeesFareIdTblItemNo();
      dc << "\n";
      dc << "    TEXT T196 : ";
      if ((*it)->taxTextTblItemNo() > 0)
        dc << setw(7) << (*it)->taxTextTblItemNo();
      dc << "\n";

      //  supporting tables detailed info if requested
      //
      if (!sequenceNo.empty() && suppTable == "YES")
      {
        if ((*it)->svcFeesFareIdTblItemNo() > 0)
        {
          dc << "*-----------------------------------------------------*\n";
          dc << "*   FARE IDENTIFICATION T189 ITEM NO : " << setw(7)
             << (*it)->svcFeesFareIdTblItemNo() << "        *\n";
          dc << "*-----------------------------------------------------*\n";

          const std::vector<SvcFeesFareIdInfo*>& fareIdInfo =
              getFareIdInfo(trx, info->vendor(), (*it)->svcFeesFareIdTblItemNo());

          std::vector<SvcFeesFareIdInfo*>::const_iterator fareId = fareIdInfo.begin();
          const std::vector<SvcFeesFareIdInfo*>::const_iterator fareIdEnd = fareIdInfo.end();
          for (; fareId != fareIdEnd; ++fareId)
          {
            printT189DetailInfo(*fareId);
          }
        }
        if ((*it)->taxTextTblItemNo() > 0)
        {
          //                   012345678901234567890123456789012345678901234567890123456789012 - 63
          dc << "*-----------------------------------------------------*\n";
          dc << "*   TEXT TABLE T196 ITEM NO : " << setw(7) << (*it)->taxTextTblItemNo()
             << "                 *\n";
          dc << "*-----------------------------------------------------*\n";

          const TaxText* taxText =
              trx.dataHandle().getTaxText(info->vendor(), (*it)->taxTextTblItemNo());

          if (taxText)
          {
            std::vector<std::string>::const_iterator itT = taxText->txtMsgs().begin();
            std::vector<std::string>::const_iterator itTE = taxText->txtMsgs().end();
            for (; itT != itTE; ++itT)
            {
              dc << "  " << *itT << "\n";
            }
          }
          else
            dc << "  DATA NOT FOUND"
               << "\n";
          dc << "\n";
        }
      }
    }
  }
}

void
Diag888Collector::displayVendor(const VendorCode& vendor, bool isDetailDisp)
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

bool
Diag888Collector::isStartDateSpecified(const BrandedFare& S8Info)
{
  return S8Info.tvlFirstYear() || S8Info.tvlFirstMonth() || S8Info.tvlFirstDay();
}

bool
Diag888Collector::isStopDateSpecified(const BrandedFare& S8Info)
{
  return S8Info.tvlLastYear() || S8Info.tvlLastMonth() || S8Info.tvlLastDay();
}

const std::vector<SvcFeesAccCodeInfo*>&
Diag888Collector::getSvcFeesAccCodeInfo(PricingTrx& trx, int itemNo) const
{
  return trx.dataHandle().getSvcFeesAccountCode(ATPCO_VENDOR_CODE, itemNo);
}
const std::vector<SvcFeesSecurityInfo*>&
Diag888Collector::getSecurityInfo(PricingTrx& trx, VendorCode vc, int itemNo)
{
  return trx.dataHandle().getSvcFeesSecurity(vc, itemNo);
}

const std::vector<SvcFeesFareIdInfo*>&
Diag888Collector::getFareIdInfo(PricingTrx& trx, VendorCode vc, long long itemNo)
{
  return trx.dataHandle().getSvcFeesFareIds(vc, itemNo);
}

const std::vector<SvcFeesFeatureInfo*>&
Diag888Collector::getT166Info(PricingTrx& trx, VendorCode vc, long long itemNo)
{
  return trx.dataHandle().getSvcFeesFeature(vc, itemNo);
}

} // namespace
