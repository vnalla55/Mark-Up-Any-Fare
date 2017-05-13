//----------------------------------------------------------------------------
//  File:        Diag876Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 876 Service Fee - OC Fees S6
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
#include "Diagnostic/Diag876Collector.h"

#include "Common/FallbackUtil.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/OptionalServicesConcur.h"
#include "DBAccess/SubCodeInfo.h"
#include "ServiceFees/ServiceFeesGroup.h"

#include <iomanip>
#include <iostream>

namespace tse
{

bool
Diag876Collector::shouldDisplay(const OptionalServicesConcur& concur) const
{
  if (_trx)
  {
    std::string filter = _trx->diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER);
    if (!filter.empty())
    {
      uint32_t nSeqFilter = std::atoi(filter.c_str());
      if (nSeqFilter != concur.seqNo())
        return false;
    }
  }
  return true;
}

bool
Diag876Collector::shouldDisplay(const SubCodeInfo& sci) const
{
  if (_trx)
  {
    std::string filter = _trx->diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);
    if (!filter.empty() && filter != sci.carrier())
      return false;

    filter = _trx->diagnostic().diagParamMapItem(Diagnostic::SRV_GROUP);
    if (!filter.empty() && filter != sci.serviceGroup())
      return false;

    filter = _trx->diagnostic().diagParamMapItem(Diagnostic::SRV_CODE);
    if (!filter.empty() && filter != sci.serviceSubTypeCode())
      return false;

    if (sci.fltTktMerchInd() == BAGGAGE_ALLOWANCE || sci.fltTktMerchInd() == CARRY_ON_ALLOWANCE ||
        sci.fltTktMerchInd() == BAGGAGE_EMBARGO || sci.fltTktMerchInd() == BAGGAGE_CHARGE)
      return false;
  }
  return true;
}

bool
Diag876Collector::shouldDisplay(const TravelSeg* begin, const TravelSeg* end) const
{
  if (_trx && begin && end)
  {
    return _trx->diagnostic().shouldDisplay(
        begin->origAirport(), begin->boardMultiCity(), end->destAirport(), end->offMultiCity());
  }
  return true;
}

bool
Diag876Collector::shouldDisplay(const ServiceFeesGroup* srvFeesGrp) const
{
  if (_trx)
  {
    std::string filter = _trx->diagnostic().diagParamMapItem(Diagnostic::SRV_GROUP);
    if (!filter.empty() && filter != srvFeesGrp->groupCode())
      return false;
  }
  return true;
}

bool
Diag876Collector::shouldDisplay(const CarrierCode& cxr) const
{
  if (_trx)
  {
    std::string filter = _trx->diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);
    if (!filter.empty() && cxr != filter)
      return false;
  }
  return true;
}

bool
Diag876Collector::isDdInfo()
{
  if (!_trx)
    return false;
  return _trx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO";
}

void
Diag876Collector::printHeader(const std::set<CarrierCode>& marketingCarriers,
                              const std::set<CarrierCode>& operatingCarriers,
                              const TravelSeg* begin,
                              const TravelSeg* end,
                              bool needValidation,
                              const FarePath& farePath)
{
  if (!_active || !shouldDisplay(begin, end))
    return;

  *this << "***********     OPTIONAL SERVICES S6 VALIDATION     ***********\n";
  *this << " MARKETING CARRIERS: ";

  std::set<CarrierCode>::const_iterator it = marketingCarriers.begin();
  std::set<CarrierCode>::const_iterator ie = marketingCarriers.end();
  for (; it != ie; it++)
    *this << *it << " ";

  *this << "\n OPERATING CARRIERS: ";
  for (it = operatingCarriers.begin(), ie = operatingCarriers.end(); it != ie; it++)
    *this << *it << " ";

  *this << "\n  PORTION OF TRAVEL: ";
  if (begin->boardMultiCity().empty())
    *this << std::setw(3) << begin->origAirport();
  else
    *this << std::setw(3) << begin->boardMultiCity();

  *this << " - ";

  if (end->offMultiCity().empty())
    *this << std::setw(3) << end->destAirport();
  else
    *this << std::setw(3) << end->offMultiCity();

  if (_trx && _trx->getRequest() && _trx->getRequest()->multiTicketActive() && farePath.itin())
  {
    if (farePath.itin()->getMultiTktItinOrderNum() == 1)
      *this << "      TKT1\n";
    else if (farePath.itin()->getMultiTktItinOrderNum() == 2)
      *this << "      TKT2\n";
  }
  *this << "\n      ";
  if (needValidation)
    *this << "** S6 VALIDATION IS NEEDED\n";
  else
    *this << "** S6 VALIDATION IS NOT NEEDED\n";

  addStarLine();
}

void
Diag876Collector::printNoOCFeesFound(const CarrierCode& cxr, const ServiceGroup& grp)
{
  if (!_active)
    return;

  *this << "NO OCFEES FOUND FOR CARRIER: " << cxr << " AND GROUP CODE: " << grp << "\n";
  addStarLine();
}

void
Diag876Collector::printS6(const OptionalServicesConcur& concur, const char* msg)
{
  if (_active && shouldDisplay(concur))
  {
    *this << "  S6 RECORD:\n";
    if (isDdInfo())
    {
      *this << "   VENDOR               - " << concur.vendor() << "\n";
      *this << "   CARRIER              - " << concur.carrier() << "\n";
      *this << "   CREATEDATE           - " << concur.createDate().dateToString(MMDDYYYY, "/")
            << "\n";
      *this << "   EXPIREDATE           - " << concur.expireDate().dateToString(MMDDYYYY, "/")
            << "\n";
      *this << "   EFFDATE              - " << concur.effDate().dateToString(MMDDYYYY, "/") << "\n";
      *this << "   DISCDATE             - " << concur.discDate().dateToString(MMDDYYYY, "/")
            << "\n";
      *this << "   SEQ NO               - " << concur.seqNo() << "\n";
      *this << "   SERVICE TYPE CODE    - " << concur.serviceTypeCode() << "\n";
      *this << "   SERVICE SUBTYPE CODE - " << concur.serviceSubTypeCode() << "\n";
      *this << "   SERVICE GROUP        - " << concur.serviceGroup() << "\n";
      *this << "   SERVICE SUBGROUP     - " << concur.serviceSubGroup() << "\n";
      *this << "   ASSEESSED CARRIER    - " << concur.accessedCarrier() << "\n";
      *this << "   MKGOPER FARE OWNER   - " << concur.mkgOperFareOwner() << "\n";
      *this << "   CONCUR               - " << concur.concur() << "\n";
    }
    else
    {
      //         1         2         3         4         5         6
      // 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
      //   CARRIER:    AA       SEQ: 12345678
      //   SRVTYPE:    OC       SRVGROUP:    AA
      //   SRVSUBTYPE: ABC      SRVSUBGROUP: 0CE
      //   ASSESED CXR: AA  MKGOPER: Y   CONCUR: Y
      *this << std::setw(11) << "CARRIER:" << std::setw(6) << concur.carrier();
      *this << std::setw(11) << "SEQ:" << std::setw(11) << concur.seqNo() << "\n";

      *this << "   SRVTYPE:" << std::setw(6) << concur.serviceTypeCode();
      *this << std::setw(16) << "SRVGROUP:" << std::setw(6) << concur.serviceGroup() << "\n";

      *this << "   SRVSUBTYPE:" << std::setw(4) << concur.serviceSubTypeCode();
      *this << std::setw(18) << "SRVSUBGROUP:" << std::setw(3) << concur.serviceSubGroup() << "\n";

      *this << std::setw(16) << "ASSESED CXR:" << std::setw(3) << concur.accessedCarrier();
      *this << std::setw(10) << "MKGOPER:" << std::setw(2) << concur.mkgOperFareOwner();
      *this << std::setw(10) << "CONCUR:" << std::setw(2) << concur.concur() << "\n";
    }
    if (msg)
      *this << std::setw(57) << msg << "\n";
  }
}

void
Diag876Collector::printS5(const SubCodeInfo& sci, const char* msg)
{
  if (_active && shouldDisplay(sci))
  {
    *this << "S5 RECORD:\n";
    if (isDdInfo())
    {
      *this << " VENDOR               - " << sci.vendor() << "\n";
      *this << " CARRIER              - " << sci.carrier() << "\n";
      *this << " SERVICE TYPE CODE    - " << sci.serviceTypeCode() << "\n";
      *this << " SERVICE SUBTYPE CODE - " << sci.serviceSubTypeCode() << "\n";
      *this << " SERVICE GROUP        - " << sci.serviceGroup() << "\n";
      *this << " SERVICE SUBGROUP     - " << sci.serviceSubGroup() << "\n";
      *this << " CONCUR               - " << sci.concur() << "\n";
      *this << " SERVICE              - ";
      displayFltTktMerchInd(sci.fltTktMerchInd());
      *this << "\n";
    }
    else
    {
      //         1         2         3         4         5         6
      // 1234567890123456789012345678901234567890123456789012345678901234567890
      // CARRIER:    AA       VENDOR:     ABCD       CONCUR:      1
      // SRVTYPE:    OC       SRVGROUP:    AA
      // SRVSUBTYPE: ABC      SRVSUBGROUP: 0CE
      *this << " CARRIER:" << std::setw(6) << sci.carrier();
      *this << std::setw(14) << "VENDOR:" << std::setw(9) << sci.vendor();
      *this << std::setw(12) << "CONCUR:" << std::setw(7) << sci.concur() << "\n";

      *this << " SRVTYPE:" << std::setw(6) << sci.serviceTypeCode();
      *this << std::setw(16) << "SRVGROUP:" << std::setw(6) << sci.serviceGroup() << "\n";

      *this << " SRVSUBTYPE:" << std::setw(4) << sci.serviceSubTypeCode();
      *this << std::setw(18) << "SRVSUBGROUP:" << std::setw(3) << sci.serviceSubGroup() << "\n";

      *this << " SERVICE: ";
      displayFltTktMerchInd(sci.fltTktMerchInd());
      *this << "\n";
    }
    if (msg)
      *this << msg << "\n";
  }
}

void
Diag876Collector::printS5Concur(Indicator i)
{
  if (!_active)
    return;

  if (i == '2')
    *this << std::setw(61) << "*** S5 CONCUR NOT ALLOWED ***\n";
  else if (i == 'X')
    *this << std::setw(61) << "*** NO CONCURRENCE IS REQUIRED ***\n";
}

void
Diag876Collector::printS6Found(bool found)
{
  if (!_active)
    return;

  if (found)
    *this << std::setw(61) << "** S6 FOUND FOR S5**";
  *this << "\n";
  // addStarLine();
}

void
Diag876Collector::printS6ValidationHeader(bool marketing)
{
  if (!_active)
    return;

  if (marketing)
    *this << "MARKETING CARRIER THAT CAN BE ASSESED:\n";
  else
    *this << "OPERATING CARRIER THAT CAN BE ASSESED:\n";
  //         1         2         3         4         5         6
  // 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
  // CX VENDR TYPE GROP SUBTYPE SUBGROP CONCUR               STATUS
  *this << " CX VENDR TYPE GROP SUBTYPE SUBGROP CONCUR               STATUS\n";
  addStarLine();
}
void
Diag876Collector::printS6Validation(const SubCodeInfo& sci, const char* status)
{
  if (_active && shouldDisplay(sci))
  {
    //         1         2         3         4         5         6
    // 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
    // CX VENDR TYPE GROP SUBTYPE SUBGROP CONCUR               STATUS
    // AA  MMGR   OC   BG     ABC      BG      1                 PASS
    *this << std::setw(3) << sci.carrier();
    *this << std::setw(6) << sci.vendor();
    *this << std::setw(5) << sci.serviceTypeCode();
    *this << std::setw(5) << sci.serviceGroup();
    *this << std::setw(8) << sci.serviceSubTypeCode();
    *this << std::setw(8) << sci.serviceSubGroup();
    *this << std::setw(7) << sci.concur();
    if (status)
      *this << std::setw(21) << status;
    *this << "\n";
  }
}
void
Diag876Collector::printS6ValidationNoPass()
{
  if (!_active)
    return;
  *this << std::setw(61) << "*** NO S5 PASS S6 VALIDATION ***\n";
}

void
Diag876Collector::printNoS6InDB()
{
  if (!_active)
    return;
  *this << std::setw(62) << "** NO S6 RECORDS FOUND IN DATABASE **\n";
}
void
Diag876Collector::printS6PassSeqNo(const SubCodeInfo& sci, const OptionalServicesConcur& concur)
{
  if (_active && shouldDisplay(sci))
    *this << std::setw(44) << concur.carrier() << " ON SEQ " << std::setw(10) << concur.seqNo()
          << "\n";
}

void
Diag876Collector::addStarLine(int LineLength)
{
  if (_active)
    *this << std::setfill('*') << std::setw(LineLength) << "" << std::setfill(' ') << "\n";
}
}
