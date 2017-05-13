//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "Diagnostic/SvcFeesDiagCollector.h"

#include "DBAccess/SvcFeesAccCodeInfo.h"
#include "DBAccess/SvcFeesFareIdInfo.h"
#include "DBAccess/SvcFeesFeatureInfo.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "DBAccess/SvcFeesTktDesignatorInfo.h"

namespace tse
{

const std::string SvcFeesDiagCollector::_passed = "   PASS\n";
const std::string SvcFeesDiagCollector::_failed = "   FAIL\n";

SvcFeesDiagCollector&
  SvcFeesDiagCollector::operator<<(const SvcFeesAccCodeInfo& accCode)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << " " << std::setw(7) << accCode.seqNo() << "  " << std::setw(20) << accCode.accountCode();
  }
  return *this;
}

SvcFeesDiagCollector&
  SvcFeesDiagCollector::operator<< (const SvcFeesTktDesignatorInfo& tktDes)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << "   " << std::setw(7) << tktDes.seqNo() << "           " << std::setw(11)
       << tktDes.tktDesignator();
  }
  return *this;
}

void
SvcFeesDiagCollector::printValidationStatus(bool status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << (status ? _passed : _failed);
}

void
SvcFeesDiagCollector::printAccountCodeTable172Header(int itemNo)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << " *....................................................*\n";
  dc << " *  ACC CODE T172 ITEM NO : " << itemNo << " DETAIL INFO\n";
  dc << " *....................................................*\n";
  dc << " SEQ NO   ACCOUNT CODE           STATUS\n";
}

void
SvcFeesDiagCollector::printTktDesignatorTable173Header(int itemNo)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " .....................................................\n" // 53 dots
     << " * TKT DESIGNATOR T173 ITEM NO: " << std::setw(7) << itemNo << "  DETAIL INFO *\n"
     << " .....................................................\n" // 53 dots
     << "  SEQ NO           TKT DSGN CODE   STATUS\n";
}

void
SvcFeesDiagCollector::printTktDesignatorFailInfo(StatusT173 status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  dc << "                  ";
  switch (status)
  {
  case FAIL_NO_INPUT:
    dc << "INPUT DESIGNATOR MISSING";
    break;
  case FAIL_ON_BLANK:
    dc << "     DESIGNATOR IS EMPTY";
    break;
  case FAIL_NO_OUTPUT:
    dc << "OUTPUT DESIGNATOR MISSING";
    break;
  default:
    dc << "UNKNOWN STATUS";
    break;
  }
  dc << "\n";
}

void
SvcFeesDiagCollector::printSecurityTable183Header(int itemNo)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n";
  dc << " * SECURITY T183 ITEM NO : " << std::setw(7) << itemNo << " DETAIL INFO       * \n";
  dc << " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n";
  dc << " SEQ NO TVL GDS DUTY GEO LOC TYPE CODE VIEW STATUS \n";
}

void
SvcFeesDiagCollector::printSecurityTable183EmptyMsg(VendorCode vc)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " ***   NO SECURITY DATA FOR VENDOR " << std::setw(5) << vc << "   ***\n";
}

void
SvcFeesDiagCollector::printSecurityTable183Info(const SvcFeesSecurityInfo* feeSec,
                                                const StatusT183Security status)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " " << std::setw(7) << feeSec->seqNo() << " " << std::setw(1) << feeSec->travelAgencyInd() << "  "
     << std::setw(2) << feeSec->carrierGdsCode() << "  ";
  dc << std::setw(3) << feeSec->dutyFunctionCode() << "  ";
  dc << std::setw(2) << feeSec->loc().locType() << std::setw(5) << feeSec->loc().loc() << " ";
  dc << std::setw(2) << feeSec->code().locType() << std::setw(7) << feeSec->code().loc();

  dc << "  " << std::setw(1) << feeSec->viewBookTktInd() << "  ";

  switch (status)
  {
  case PASS_T183:
    dc << " PASS";
    break;
  case FAIL_TVL_AGENCY:
    dc << " FAIL TRVL";
    break;
  case FAIL_CXR_GDS:
    dc << " FAIL GDS";
    break;
  case FAIL_DUTY:
    dc << " FAIL DUTY";
    break;
  case FAIL_GEO:
    dc << " FAIL GEO SPEC";
    break;
  case FAIL_TYPE_CODE:
    dc << " FAIL TYPE CODE";
    break;
  case FAIL_VIEW_IND:
    dc << " FAIL VIEW BOOK";
    break;
  default:
    dc << " UNKNOWN STATUS";
  }
  dc << " \n";
}

void
SvcFeesDiagCollector::printT189DetailInfo(const SvcFeesFareIdInfo* info, bool printSeparator)
{
  if (!_active)
    return;

  char buff[8];
  sprintf(buff, "%d", info->seqNo());
  char buffR[6];

  snprintf(buffR, 6, "%d", info->routing());

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  //         012345678901234567890123456789012345678901234567890123456789012 - 63
  dc << " VENDOR : " << std::setw(5) << info->vendor() << "   "
     << "SEQ NBR : " << std::setw(7) << buff << "   "
     << "ITEM NO : " << std::setw(7) << info->itemNo() << "\n";
  dc << " SOURCE : " << info->source() << "\n";

  dc << "\nFARE INFORMATION\n";
  dc << " APPL  : ";
  if (info->fareApplInd() != ' ')
    dc << info->fareApplInd();
  dc << "\n";
  dc << " OW/RT : ";
  if (info->owrt() == ONE_WAY_MAY_BE_DOUBLED)
    dc << "1 - ONE WAY MAY BE DOUBLED";
  else if (info->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    dc << "2 - ROUND TRIP MAY NOT BE HALVED";
  else if (info->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
    dc << "3 - ONE WAY MAY NOT BE DOUBLED";
  else
    dc << "  ";
  dc << "\n";

  dc << " RULETARIFF IND : ";
  if (!info->ruleTariffInd().empty())
    dc << std::setw(3) << info->ruleTariffInd();
  dc << "\n";
  dc << " RULE TARIFF : " << std::setw(3) << info->ruleTariff();

  dc << "   RULE NUMBER : ";
  if (!info->rule().empty())
    dc << std::setw(4) << info->rule();
  dc << "\n";
  dc << " FARE CLASS  : ";
  if (!info->fareClass().empty())
    dc << std::setw(8) << info->fareClass();
  dc << "\n";
  dc << " FARE TYPE : ";
  if (!info->fareType().empty())
    dc << std::setw(4) << info->fareType();
  dc << "\n";

  dc << " PAX TYPE : ";
  if (!info->paxType().empty())
    dc << info->paxType();
  dc << "\n";

  dc << " ROUTING : " << std::setw(5) << buffR << "\n";

  dc << " PRIME RBD1 : ";
  if (!info->bookingCode1().empty())
    dc << std::setw(2) << info->bookingCode1();
  dc << "\n PRIME RBD2 : ";
  if (!info->bookingCode2().empty())
    dc << std::setw(2) << info->bookingCode2();
  dc << "\n";

  dc << "\n FARE RANGE AMOUNT\n";
  displayMinAmount(info);
  displayMaxAmount(info);
  if(printSeparator)
    dc << "*------------------------------*\n";
}

void
SvcFeesDiagCollector::displayMinAmount(const SvcFeesFareIdInfo* info)
{
  DiagCollector& dc = (DiagCollector&)*this;

  dc << " MIN 1 : ";
  if (info->minFareAmt1() != 0.0)
  {
    dc.unsetf(std::ios::left);
    dc.setf(std::ios::right, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc.precision(info->noDec1());
    dc << std::setw(10) << info->minFareAmt1();
    dc.unsetf(std::ios::right);
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << std::setw(3) << info->cur1();
  }
  else
    dc << "             ";
  dc << "   MIN 2 : ";
  if (info->minFareAmt2() != 0.0)
  {
    dc.unsetf(std::ios::left);
    dc.setf(std::ios::right, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc.precision(info->noDec2());
    dc << std::setw(10) << info->minFareAmt2();
    dc.unsetf(std::ios::right);
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << std::setw(3) << info->cur2() << "\n";
  }
  else
    dc << "\n";
}

void
SvcFeesDiagCollector::displayMaxAmount(const SvcFeesFareIdInfo* info)
{
  DiagCollector& dc = (DiagCollector&)*this;

  dc << " MAX 1 : ";
  if (info->maxFareAmt1() != 0.0)
  {
    dc.unsetf(std::ios::left);
    dc.setf(std::ios::right, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc.precision(info->noDec1());
    dc << std::setw(10) << info->maxFareAmt1();
    dc.unsetf(std::ios::right);
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << std::setw(3) << info->cur1();
  }
  else
    dc << "             ";
  dc << "   MAX 2 : ";
  if (info->maxFareAmt2() != 0.0)
  {
    dc.unsetf(std::ios::left);
    dc.setf(std::ios::right, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc.precision(info->noDec2());
    dc << std::setw(10) << info->maxFareAmt2();
    dc.unsetf(std::ios::right);
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << std::setw(3) << info->cur2() << "\n";
  }
  else
    dc << "\n";
}

void
SvcFeesDiagCollector::printT166DetailInfo(const SvcFeesFeatureInfo* info)
{
  if (!_active)
    return;

  char buff[8];
  sprintf(buff, "%d", info->seqNo());
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  //         012345678901234567890123456789012345678901234567890123456789012 - 63
  dc << " VENDOR : " << std::setw(5) << info->vendor() << "   "
     << "SEQ NBR : " << std::setw(7) << buff << "   "
     << "ITEM NO : " << std::setw(7) << info->itemNo() << "\n";

  dc << " CARRIER  : " << std::setw(3) << info->carrier() << "\n";
  dc << " SERVICE  : ";
  displayFltTktMerchInd(info->fltTktMerchInd());
  dc << "\n";
  dc << " SERVICE CODE : " << info->serviceTypeCode()
     << "     SUB CODE : " << info->serviceSubTypeCode() << "\n";

  dc << " SEGMENT APPLICATION : \n";
  dc << " 1: ";
  displaySegmentApplication(info->segmentAppl1());

  dc << "  2: ";
  displaySegmentApplication(info->segmentAppl2());

  dc << "  3: ";
  displaySegmentApplication(info->segmentAppl3());
  dc << "\n";

  dc << " 4: ";
  displaySegmentApplication(info->segmentAppl4());

  dc << "  5: ";
  displaySegmentApplication(info->segmentAppl5());

  dc << "  6: ";
  displaySegmentApplication(info->segmentAppl6());
  dc << "\n";

  dc << " 7: ";
  displaySegmentApplication(info->segmentAppl7());
  dc << "  8: ";
  displaySegmentApplication(info->segmentAppl8());
  dc << "  9: ";
  displaySegmentApplication(info->segmentAppl9());
  dc << "\n";
  dc << "10: ";
  displaySegmentApplication(info->segmentAppl10());
  dc << "\n";
}

void
SvcFeesDiagCollector::displaySegmentApplication(Indicator ind)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  if (ind == 'F')
    dc << "FOR FREE    ";
  else if (ind == 'C')
    dc << "FOR CHARGE  ";
  else if (ind == 'N')
    dc << "NOT OFFERED ";
  else if (ind == 'A')
    dc << "NOT APPL    ";
  else
    dc << "            ";
}
}
