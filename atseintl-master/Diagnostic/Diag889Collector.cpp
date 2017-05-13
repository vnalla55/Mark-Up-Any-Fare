//----------------------------------------------------------------------------
//  File:        Diag889Collector.cpp
//  Authors:
//  Created:
//
//  Description: Diagnostic 889 Branded Fares - T189
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
#include "Diagnostic/Diag889Collector.h"

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Money.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/MiscFareTag.h"
#include "DBAccess/SvcFeesFareIdInfo.h"

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

namespace tse
{
namespace
{
ConfigurableValue<bool>
isSoftPassDisabled("TN_PATH", "DISABLE_SOFT_PASS", false);
}
void
Diag889Collector::printT189Banner()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  //         012345678901234567890123456789012345678901234567890123456789012 - 63
  dc << "*************** BRANDED FARES T189 ANALYSIS ***************\n";
  if (isSoftPassDisabled.getValue())
  {
    dc << "SOFT PASSES DISABLED IN TN_SHOPPING\n";
  }
  return;
}

void
Diag889Collector::printPaxTypeFareCommonHeader()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "  GI V RULE FARE CLS TRF O O    AMT   CUR PAX PAX ROUT FARE\n"
     << "                     NUM R I              REQ ACT      TYPE\n"
     << "***********************************************************\n";
}

Diag889Collector&
Diag889Collector::
operator<<(const PaxTypeFare& paxFare)
{
  if (!_active)
    return *this;

  DiagCollector& dc(*this);

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << std::setw(2) << cnvFlags(paxFare);

  std::string gd;
  globalDirectionToStr(gd, paxFare.fare()->globalDirection());

  dc << std::setw(3) << gd << std::setw(2)
     << (paxFare.vendor() == Vendor::ATPCO ? "A" : (paxFare.vendor() == Vendor::SITA ? "S" : "?"))
     << std::setw(5) << paxFare.ruleNumber() << std::setw(9) << paxFare.fareClass() << std::setw(4)
     << paxFare.fareTariff();

  dc << std::setw(2) << DiagnosticUtil::getOwrtChar(paxFare);

  dc << std::setw(2);

  if (paxFare.directionality() == FROM)
    dc << "O";
  else if (paxFare.directionality() == TO)
    dc << "I";
  else
    dc << " ";

  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << std::setw(8) << Money(paxFare.fareAmount(), paxFare.currency()) << " ";

  if (!paxFare.isFareClassAppSegMissing())
  {
    if (paxFare.fcasPaxType().empty())
      dc << "*** ";
    else
      dc << std::setw(4) << paxFare.fcasPaxType();
  }
  else
  {
    dc << "UNK";
  }

  std::string actualPaxType = paxFare.actualPaxType() ? paxFare.actualPaxType()->paxType() : "N/A";

  dc << std::setw(4) << actualPaxType;

  dc << std::setw(4) << paxFare.routingNumber() << " ";

  if (!paxFare.isFareClassAppMissing())
  {
    dc << std::setw(4) << paxFare.fcaFareType();
  }
  else
  {
    dc << "UNK";
  }

  dc << std::endl;
  return *this;
}

void
Diag889Collector::printSeqNoAndStatus(const SvcFeesFareIdInfo* svcFeesFareIdInfo, StatusT189 status)
{
  if (!_active)
    return;

  printItemNoAndSeqNo(svcFeesFareIdInfo, true);
  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);
  dc << " ";
}

void
Diag889Collector::displayStatus(StatusT189 status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  // Currently max length for error msg is 20 !!
  switch (status)
  {
  case PASS_T189:
    dc << "PASS";
    break;
  case FAIL_FARECLASS:
    dc << "FAIL FARECLASS";
    break;
  case FAIL_FARETYPE:
    dc << "FAIL FARETYPE";
    break;
  case FAIL_PAXTYPE:
    dc << "FAIL PAXTYPE";
    break;
  case FAIL_ROUTING:
    dc << "FAIL ROUTING";
    break;
  case FAIL_PRIME_RBD:
    dc << "FAIL PRIME RBD";
    break;
  case FAIL_RANGE1_CURR:
    dc << "FAIL RANGE1 CURRENCY";
    break;
  case FAIL_RANGE1_DECIMAL:
    dc << "FAIL RANGE1 DECIMAL";
    break;
  case FAIL_RANGE1_MIN:
    dc << "FAIL RANGE1 MIN AMOUNT";
    break;
  case FAIL_RANGE1_MAX:
    dc << "FAIL RANGE1 MAX AMOUNT";
    break;
  case FAIL_RANGE2_CURR:
    dc << "FAIL RANGE2 CURRENCY";
    break;
  case FAIL_RANGE2_DECIMAL:
    dc << "FAIL RANGE2 DECIMAL";
    break;
  case FAIL_RANGE2_MIN:
    dc << "FAIL RANGE2 MIN AMOUNT";
    break;
  case FAIL_RANGE2_MAX:
    dc << "FAIL RANGE2 MAX AMOUNT";
    break;
  case APPL_NEGATIVE:
    dc << "APPL NEGATIVE";
    break;
  case FAIL_OWRT:
    dc << "FAIL OWRT";
    break;
  case FAIL_SOURCE:
    dc << "FAIL SOURCE";
    break;
  case FAIL_RULE_TARIFF:
    dc << "FAIL RULE TARIFF";
    break;
  case FAIL_RULE:
    dc << "FAIL RULE";
    break;
  case FAIL_SEC_T189:
    dc << "FAIL SEC T189";
    break;
  case SOFTPASS_RBD:
    dc << "SOFTPASS RBD";
    break;
  case FAIL_RULE_TARIFF_IND:
    dc << "FAIL RULE TARIFF IND";
    break;
  case NO_STATUS:
    dc << " ";
    break;
  default:
    dc << "UNKNOWN STATUS";
    break;
  }
}

void
Diag889Collector::printTravelInfo(const FareMarket* fm)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "PORTION OF TRAVEL  : ";
  if (!fm->boardMultiCity().empty())
    dc << setw(3) << fm->boardMultiCity();
  dc << " - ";
  if (!fm->offMultiCity().empty())
    dc << setw(3) << fm->offMultiCity();

  dc << "   GOV CARRIER  : ";
  if (!fm->governingCarrier().empty())
    dc << setw(3) << fm->governingCarrier();
  dc << "\n";
  dc << "***********************************************************\n";
}

void
Diag889Collector::printSeparator()
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;

  dc << "-----------------------------------------------------------\n";
}

void
Diag889Collector::displayT189Secondary(const SvcFeesFareIdInfo* t189Secondary)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  dc << "S T189 ITEM NO : ";
  if (t189Secondary->itemNo() > 0)
    dc << setw(7) << t189Secondary->itemNo();

  dc << " SEQ NO  : ";
  if (t189Secondary->seqNo() > 0)
    dc << setw(7) << t189Secondary->seqNo();

  dc << "\n";
}

void
Diag889Collector::printDetailInfo(const SvcFeesFareIdInfo* svcFeesFareIdInfo, StatusT189 status)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  printSeparator();
  printT189DetailInfo(svcFeesFareIdInfo, false);

  dc << "\n";
}

void
Diag889Collector::printDetailBrandProgram(const BrandProgram* brandPr)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "PROGRAM CODE : ";
  if (!brandPr->programCode().empty())
  {
    std::string tmp = brandPr->programCode();
    boost::to_upper(tmp);
    dc << setw(11) << tmp;
  }

  dc << " VENDOR : ";
  if (!brandPr->vendorCode().empty())
    dc << setw(5) << brandPr->vendorCode();

  dc << "\n";

  dc << "BRAND DATA SOURCE: ";
  switch (brandPr->dataSource())
  {
  case BRAND_SOURCE_S8:
    dc << "S8";
    break;
  case BRAND_SOURCE_CBAS:
    dc << "CB";
    break;
  default:
    dc << "NA";
    break;
  }

  dc << "\n";
}

void
Diag889Collector::printBrand(const BrandInfo* brand)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  printSeparator();

  std::string tmp;

  dc << "BRAND CODE : ";
  if (!brand->brandCode().empty())
  {
    tmp = brand->brandCode();
    boost::to_upper(tmp);
    dc << tmp;
  }
  dc << "\n";

  dc << "BRAND NAME : ";
  if (!brand->brandName().empty())
  {
    tmp = brand->brandName();
    boost::to_upper(tmp);
    dc << setw(31) << tmp;
  }

  dc << " TIER : ";
  if (brand->tier() > 0)
    dc << setw(7) << brand->tier();
  dc << "\n";
}

void
Diag889Collector::printT189NotFound(const BrandInfo* brand)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "BRAND   : ";
  if (!brand->brandCode().empty())
  {
    std::string tmp = brand->brandCode();
    boost::to_upper(tmp);
    dc << tmp;
  }

  dc << "  P T189 ITEMNO : NOT FOUND\n";
}

void
Diag889Collector::printBrandProgram(const BrandProgram* brandPr)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "PROGRAM : ";
  if (!brandPr->programCode().empty())
  {
    std::string tmp = brandPr->programCode();
    boost::to_upper(tmp);
    dc << setw(11) << tmp;
  }
  dc << "\n";
}

void
Diag889Collector::printFailGlobalDirection(const BrandProgram& brandPr,
                                           const PaxTypeFare& paxTypeFare)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  std::string progGlobalDir, ptfGlobalDir;
  globalDirectionToStr(progGlobalDir, brandPr.globalDirection());
  globalDirectionToStr(ptfGlobalDir, paxTypeFare.globalDirection());

  dc << "PROGRAM GLOBAL DIR : " << progGlobalDir << "  ";
  dc << "DOES NOT MATCH FARE GLOBAL DIR : " << ptfGlobalDir;
  dc << "\n";
}

void
Diag889Collector::printFailDirectionality(const BrandProgram& brandPr,
                                          const PaxTypeFare& paxTypeFare,
                                          bool useDirectionality,
                                          Direction programDirection)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "PROGRAM DIR : " << std::setw(10);

  if (brandPr.direction() == "OT")
    dc << "OUTBOUND";
  else if (brandPr.direction() == "IN")
    dc << "INBOUND";
  else
    dc << " ";

  if (useDirectionality)
  {
    dc << "/ " << std::setw(16);
    if (programDirection == Direction::BOTHWAYS)
      dc << "BOTH";
    else if (programDirection == Direction::ORIGINAL)
      dc << "ORIGINAL (FROM)";
    else
      dc << "REVERSED (TO)";
  }

  dc << "DOES NOT MATCH FARE DIR : ";
  switch (paxTypeFare.directionality())
  {
  case FROM:
    dc << "FROM";
    break;
  case TO:
    dc << "TO";
    break;
  case BOTH:
    dc << "BOTH";
    break;

  default:
    dc << "UNKNOWN";
  }
  dc << "\n";
}

void
Diag889Collector::printBrandT189Item(const SvcFeesFareIdInfo* svcFeesFareIdInfo,
                                     const BrandInfo* brand)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "BRAND   : ";
  if (!brand->brandCode().empty())
  {
    std::string tmp = brand->brandCode();
    boost::to_upper(tmp);
    dc << setw(11) << tmp;
  }

  dc << "\n";
}

void
Diag889Collector::printBlankLine()
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;

  dc << "                                                           \n";
}

void
Diag889Collector::printSize(const char* txt, int size)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc << txt << size;
  dc << "\n";
}

void
Diag889Collector::printDataNotFound()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  dc << "BRAND DATA NOT FOUND\n";
  return;
}

void
Diag889Collector::printDataNotFound(const FareMarket& fareMarket)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  dc << "FARE MARKET " << fareMarket.origin()->loc() << "-" << fareMarket.destination()->loc()
     << " BRAND DATA NOT FOUND\n";
  return;
}

void
Diag889Collector::printCompleteBrandProgram(const BrandProgram* brandPr)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);
  std::string tmp;

  dc << "PROGRAM CODE : ";
  if (!brandPr->programCode().empty())
  {
    tmp = brandPr->programCode();
    boost::to_upper(tmp);
    dc << setw(11) << tmp;
  }
  dc << "\n";

  dc << "PROGRAM NAME : ";
  if (!brandPr->programName().empty())
  {
    tmp = brandPr->programName();
    boost::to_upper(tmp);
    dc << setw(31) << tmp;
  }
  dc << "\n";

  dc << "PROGRAM DESC : ";
  if (!brandPr->programDescription().empty())
  {
    tmp = brandPr->programDescription();
    boost::to_upper(tmp);
    dc << setw(31) << tmp;
  }
  dc << "\n";

  dc << "VENDOR : ";
  if (!brandPr->vendorCode().empty())
    dc << setw(5) << brandPr->vendorCode();
  dc << "\n";

  std::vector<PaxTypeCode>::const_iterator paxTypeCodeBeg = brandPr->passengerType().begin();
  dc << "PSG TYPE : ";
  for (; paxTypeCodeBeg != brandPr->passengerType().end(); paxTypeCodeBeg++)
  {
    if (!(*paxTypeCodeBeg).empty())
      dc << setw(4) << (*paxTypeCodeBeg);
  }
  dc << "\n";

  dc << "DIRECTION : ";
  if (!brandPr->direction().empty())
    dc << setw(3) << brandPr->direction();
  dc << "\n";

  std::string tmpStr;
  bool rc = globalDirectionToStr(tmpStr, brandPr->globalDirection());
  dc << "GLOBAL DIRECTION : ";
  if (rc)
    dc << tmpStr;
  dc << "\n";

  dc << "ORIGINLOC : ";
  if (!brandPr->originLoc().empty())
    dc << setw(3) << brandPr->originLoc();
  dc << "\n";

  dc << "SEQ NO  : ";
  if (brandPr->sequenceNo() > 0)
    dc << setw(7) << brandPr->sequenceNo();
  dc << "\n";

  dc << "EFFECTIVE DATE : ";
  tmpStr = brandPr->effectiveDate().dateToString(YYYYMMDD, "-");
  if (!tmpStr.empty())
    dc << tmpStr;
  dc << "\n";

  dc << "DISCONTINUE DATE : ";
  tmpStr = brandPr->discontinueDate().dateToString(YYYYMMDD, "-");
  if (!tmpStr.empty())
    dc << tmpStr;
  dc << "\n";

  BrandProgram::AccountCodes accountCodes = brandPr->accountCodes();
  std::vector<std::string>::const_iterator accountCodesBeg = accountCodes.begin();
  dc << "ACCOUNT CODE : ";
  for (; accountCodesBeg != accountCodes.end(); accountCodesBeg++)
  {
    dc << (*accountCodesBeg);
  }

  dc << "\n";
}

void
Diag889Collector::printCompleteBrand(const BrandInfo* brand)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  printSeparator();
  std::string tmp;

  dc << "BRAND CODE : ";
  if (!brand->brandCode().empty())
  {
    tmp = brand->brandCode();
    boost::to_upper(tmp);
    dc << tmp;
  }
  dc << "\n";

  dc << "BRAND NAME : ";
  if (!brand->brandName().empty())
  {
    tmp = brand->brandName();
    boost::to_upper(tmp);
    dc << setw(31) << tmp;
  }
  dc << "\n";

  dc << "TIER NUMBER : ";
  if (brand->tier() > 0)
    dc << setw(7) << brand->tier();
  dc << "\n";

  dc << "PRIMARYFAREIDTABLE : ";
  if (brand->primaryFareIdTable() > 0)
    dc << setw(7) << brand->primaryFareIdTable();
  dc << "\n";

  dc << "SECONDARYFAREIDTABLE : ";
  if (brand->secondaryFareIdTable() > 0)
    dc << setw(7) << brand->secondaryFareIdTable();
  dc << "\n";
}

void
Diag889Collector::printBrandProgramNotFound()
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "PROGRAM : NOT FOUND\n";
}

void
Diag889Collector::printBrandProgramNotFound(const FareMarket& fareMarket)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "FARE MARKET " << fareMarket.origin()->loc() << "-" << fareMarket.destination()->loc()
     << " PROGRAM : NOT FOUND\n";
}

void
Diag889Collector::printBrandNotFound()
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "BRAND   : NOT FOUND";
  dc << " P T189 ITEMNO : NOT FOUND\n";
}

void
Diag889Collector::printFareMarketNotMatched(const FareMarket& fareMarket)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  dc << "FARE MARKET " << fareMarket.origin()->loc() << "-" << fareMarket.destination()->loc()
     << " NOT MATCHED\n";
  return;
}

void
Diag889Collector::printNoFaresFound(const FareMarket& fareMarket)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  dc << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->loc() << "-"
     << fareMarket.destination()->loc();
  dc << "\n";
  return;
}

void
Diag889Collector::printT189SecondaryItemNoAndSeqNo(const SvcFeesFareIdInfo* svcFeesFareIdInfo,
                                                   bool status)
{
  if (!_active)
    return;

  printItemNoAndSeqNo(svcFeesFareIdInfo, false);
  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);
  printT189SecondaryStatus(dc, status);
}

void
Diag889Collector::printT189NotExist(const FareMarket& fareMarket)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  dc << "FARE MARKET " << fareMarket.origin()->loc() << "-" << fareMarket.destination()->loc()
     << " NO FAREID T189 DATA EXIST\n";
  return;
}

void
Diag889Collector::printT189SecondaryDetailInfo(const SvcFeesFareIdInfo* info, bool status)
{
  if (!_active)
    return;

  char buff[8];
  sprintf(buff, "%d", info->seqNo());

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  dc << " VENDOR : " << setw(5) << info->vendor() << "   "
     << "SEQ NBR : " << setw(7) << buff << "   "
     << "ITEM NO : " << setw(7) << info->itemNo() << "\n";
  dc << " SOURCE : " << info->source() << "\n";

  dc << "\nFARE INFORMATION\n";

  dc << " PRIME RBD1 : ";
  if (!info->bookingCode1().empty())
    dc << setw(2) << info->bookingCode1();
  dc << "\n PRIME RBD2 : ";
  if (!info->bookingCode2().empty())
    dc << setw(2) << info->bookingCode2();
  dc << "\n STATUS : ";
  printT189SecondaryStatus(dc, status);
}

void
Diag889Collector::printFareNotValid()
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "FARE NOT VALID FOR PRICING - SEE DIAG 499/DDALLFARES\n";
}

void
Diag889Collector::printCarrierNotMatched(const FareMarket& fareMarket)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  dc << "FARE MARKET " << fareMarket.origin()->loc() << "-" << fareMarket.destination()->loc()
     << " CARRIER NOT MATCHED\n";
  return;
}

void
Diag889Collector::printBrandSizeAndCurrentBrand(const BrandInfo* brand, int size)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "NUMBER OF BRANDS : " << size << "\n";
  printCurrentBrand(brand);
}

void
Diag889Collector::printCurrentBrand(const BrandInfo* brand)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  if (!brand->brandCode().empty())
  {
    std::string tmp = brand->brandCode();
    boost::to_upper(tmp);
    dc << "CURRENT BRAND CODE : " << setw(11) << tmp << "\n";
  }
}

void
Diag889Collector::displayT189Status(StatusT189 status, bool displayDetails)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);
  if (displayDetails)
    dc << "STATUS : ";
  displayStatus(status);
  dc << "\n";
}

void
Diag889Collector::printBrandFilter(const FareMarket& fareMarket,
                                   const BrandCode& brandCode,
                                   bool match)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  std::string tmp;

  if (!brandCode.empty())
  {
    tmp = brandCode;
    boost::to_upper(tmp);
  }

  dc << "FARE MARKET " << fareMarket.origin()->loc() << "-" << fareMarket.destination()->loc()
     << std::setw(13) << (match ? " MATCHED" : " NOT MATCHED") << " BRAND CODE : " << setw(11)
     << tmp << "\n";
}

void
Diag889Collector::printIsValidForBranding(const PaxTypeFare& paxTypeFare)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "FARE IS VALID FOR PBB :  " << (paxTypeFare.isValidForBranding() ? "YES" : "NO") << "\n";
}

void
Diag889Collector::printTable189Header()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "P/S T189 ITEMNO SEQ NUM  STATUS\n";
}

void
Diag889Collector::printItemNoAndSeqNo(const SvcFeesFareIdInfo* svcFeesFareIdInfo, bool primary)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << (primary ? "P" : "S") << "   ";

  if (svcFeesFareIdInfo->itemNo() > 0)
    dc << setw(7) << svcFeesFareIdInfo->itemNo() << "     ";

  if (svcFeesFareIdInfo->seqNo() > 0)
    dc << setw(7) << svcFeesFareIdInfo->seqNo() << (primary ? " " : "  ");
}

void
Diag889Collector::printT189SecondaryStatus(DiagCollector& dc, bool status)
{
  if (status)
    dc << "SOFTPASS SECOND RBD";
  else
    dc << "FAIL SECOND RBD";

  dc << "\n";
}

void
Diag889Collector::printNoProgramFound(const FareMarket& fareMarket)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "FARE MARKET " << fareMarket.origin()->loc() << "-" << fareMarket.destination()->loc()
     << " NO PROGRAMS FOUND\n";
}

void
Diag889Collector::printMatchBasedOnBookingCodeHeaderPrimary()
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "PRIME RBD     STATUS\n";
}

void
Diag889Collector::printMatchBasedOnBookingCodeHeaderSecondary()
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "SECOND RBD    STATUS\n";
}

void
Diag889Collector::printMatchBasedOnFareBasisCodeHeader()
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "FARE BASIS CODE      STATUS\n";
}

void
Diag889Collector::printMatchBasedOnExcludedFareBasisCodeHeader()
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "MATCHING EXCLUDED FARE BASIS CODES:\n";
}

void
Diag889Collector::printMatchBasedOnFareBasisCode(const FareBasisCode& fareBasisCode, bool isValid)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  std::string fareBasisCodeText = fareBasisCode;
  boost::to_upper(fareBasisCodeText);

  dc << setw(21) << fareBasisCodeText;

  if (isValid)
    dc << "MATCH\n";
  else
    dc << "NO MATCH\n";
}

void
Diag889Collector::printMatchBasedOnBookingCode(const BookingCode& bookingCode, bool isValid)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  std::string bookingCodeText = bookingCode;
  boost::to_upper(bookingCodeText);

  dc << setw(14) << bookingCodeText;

  std::string passFailText(isValid ? "PASS" : "FAIL");

  dc << passFailText << "\n";
}

void
Diag889Collector::printValidateCbasResult(PaxTypeFare::BrandStatus status)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "CBAS VALIDATION RESULT: ";

  switch (status)
  {
  case PaxTypeFare::BS_FAIL:
    dc << "FAIL";
    break;
  case PaxTypeFare::BS_HARD_PASS:
    dc << "HARD PASS";
    break;
  case PaxTypeFare::BS_SOFT_PASS:
    dc << "SOFT PASS";
    break;
  default:
    dc << "UNKNOWN";
    break;
  }

  dc << "\n";
}

void
Diag889Collector::printYyFareIgnored()
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  dc.setf(ios::left, ios::adjustfield);

  dc << "SKIPPED BRAND VALIDATION FOR YY FARE. STATUS SET TO FAIL\n";
}

} // namespace
