//----------------------------------------------------------------------------
//  File:        Diag870Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 870 Service Fee - OB Fees
//
//  Updates:
//          02/26/09 - OB Fees
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

#include "Diagnostic/Diag870Collector.h"

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FarePath.h"
#include "DBAccess/TicketingFeesInfo.h"


#include <iomanip>
#include <iostream>

using namespace std;

namespace tse
{

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag870Collector::printSolutionHeader
//
// Description:  This method will print the Fare Path record Header info
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag870Collector::printSolutionHeader(PricingTrx& trx,
                                      const CarrierCode& cxr,
                                      const FarePath& farePath,
                                      bool international,
                                      bool roundTrip,
                                      const LocCode& furthestPoint)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  dc << "********************** OB FEES ANALYSIS ***********************\n"
     << "VALIDATING CXR : ";

  dc.setf(ios::left, ios::adjustfield);

  dc << cxr << "                  REQUESTED PAX TYPE : " << farePath.paxType()->paxType() << "\n";
  dc << "TRAVEL DATE    : " << farePath.itin()->travelDate().dateToSqlString() << "\n";
  dc << "TKT DATE       : " << trx.ticketingDate().dateToSqlString() << "\n";
  dc << "JOURNEY TYPE   : ";

  if (roundTrip)
    dc << "RT ";
  else
    dc << "OW ";

  if (international)
    dc << "INTL ";
  else
    dc << "DOM  ";

  dc << "  JOURNEY DEST/TURNAROUND: " << furthestPoint;
  dc << "\n-------------------------------------------------------------- \n"
     << "                  S4 RECORDS DATA PROCESSING\n";

  return;
}

void
Diag870Collector::printS4CommonHeader()
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "  SVC    SEQ NUM   PAX    AMOUNT      FOP BIN    STATUS\n";
}

void
Diag870Collector::printS4GeneralInfo(const TicketingFeesInfo* obFee, StatusS4Validation status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  char buff[8];
  sprintf(buff, "%d", obFee->seqNo());

  dc.setf(ios::left, ios::adjustfield);
  dc << " " << obFee->serviceTypeCode() << obFee->serviceSubTypeCode() << "   ";

  dc.unsetf(ios::left);
  dc.setf(ios::right, ios::adjustfield);
  dc << setw(7) << buff << "   ";
  dc << setw(3) << obFee->paxType() << "  ";

  dc.setf(ios::fixed, ios::floatfield);

  if (obFee->feePercent() > 0.0)
  {
    dc.precision(obFee->feePercentNoDec());
    dc << setw(7) << obFee->feePercent() << setw(3) << "PCT"
       << "    ";
  }
  else
  {
    dc.precision(obFee->noDec());
    dc << setw(7) << obFee->feeAmount() << setw(3) << obFee->cur() << "    ";
  }
  dc.unsetf(ios::right);
  dc.setf(ios::left, ios::adjustfield);

  dc << setw(7) << obFee->fopBinNumber() << "    ";
  displayStatus(status);
}

void
Diag870Collector::printFistPartDetailedRequest(const TicketingFeesInfo* obFee)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  char buff[8];
  sprintf(buff, "%d", obFee->seqNo());

  dc << "------------------- OB FEES DETAILED INFO --------------------\n"
     << "SVC CODE      : " << obFee->serviceTypeCode() << obFee->serviceSubTypeCode() << "     "
     << "CXR CODE: " << setw(3) << obFee->carrier() << "         "
     << "PAX TYPE: " << setw(3) << obFee->paxType() << "\n";

  dc.setf(ios::left, ios::adjustfield);
  dc << "SEQ NO        : " << setw(7) << buff << "\n";

  dc << "FOP BIN       : " << setw(7) << obFee->fopBinNumber() << "\n";

  dc << "TKT EFF DATE  : " << obFee->ticketEffDate().dateToSqlString();
  dc << "     TKT DISC DATE : " << obFee->ticketDiscDate().dateToSqlString() << "\n";

  dc.setf(ios::left, ios::adjustfield);
  dc << "JOURNEY IND   : " << obFee->journeyInd() << "\n";
  dc << "     LOC1 TYPE: " << setw(2) << obFee->loc1().locType() << "  LOC1 : " << setw(8)
     << obFee->loc1().loc();

  dc << "  LOC1 ZONE : ";
  if (!obFee->loc1ZoneItemNo().empty() && obFee->loc1ZoneItemNo() != "0000000")
    dc << setw(8) << obFee->loc1ZoneItemNo();
  dc << "\n";

  dc << "     LOC2 TYPE: " << setw(2) << obFee->loc2().locType() << "  LOC2 : " << setw(8)
     << obFee->loc2().loc();

  dc << "  LOC2 ZONE : ";
  if (!obFee->loc2ZoneItemNo().empty() && obFee->loc2ZoneItemNo() != "0000000")
    dc << setw(8) << obFee->loc2ZoneItemNo();
  dc << "\n";

  dc << "WHOLLY WITHIN\n"
     << "     LOC TYPE : " << setw(2) << obFee->locWhlWithin().locType() << "  LOC  : " << setw(8)
     << obFee->locWhlWithin().loc();

  dc << "  LOC  ZONE : ";
  if (!obFee->locZoneWhlWithinItemNo().empty() && obFee->locZoneWhlWithinItemNo() != "0000000")
    dc << setw(8) << obFee->locZoneWhlWithinItemNo();
  dc << "\n";

  dc << "VIA  LOC TYPE : " << setw(2) << obFee->locVia().locType() << "  LOC  : " << setw(8)
     << obFee->locVia().loc();

  dc << "  LOC  ZONE : ";
  if (!obFee->locZoneViaItemNo().empty() && obFee->locZoneViaItemNo() != "0000000")
    dc << setw(8) << obFee->locZoneViaItemNo();
  dc << "\n";
  dc << "  \n";

  dc << "FARE IND      : " << obFee->fareInd() << "   PRIMARY CXR: " << obFee->primaryFareCarrier()
     << "    FARE BASIS: " << obFee->fareBasis() << "\n";
  dc.unsetf(ios::left);
  dc.setf(ios::right, ios::adjustfield);
  dc.setf(ios::fixed, ios::floatfield);
  dc.precision(obFee->noDec());
  dc << "FEE AMOUNT    : ";
  if (obFee->feeAmount() != 0.0)
  {
    dc << setw(7) << obFee->feeAmount() << setw(3) << obFee->cur();
  }

  dc << "\n";
  dc.precision(obFee->feePercentNoDec());
  dc << "FEE PERCENT   : ";
  if (obFee->feePercent() != 0.0)
  {
    dc << setw(7) << obFee->feePercent();
    dc << "   MAX FEE AMOUNT: ";
  }
  else
    dc << "    MAX FEE AMOUNT: ";
  dc.precision(obFee->maxFeeNoDec());
  if (obFee->maxFeeAmount() != 0.0)
  {
    dc << setw(7) << obFee->maxFeeAmount() << setw(3) << obFee->maxFeeCur();
  }
  dc << "\n";
  dc << "IATA INTERLINE: " << setw(1) << obFee->interline();
  dc << "   COMMISSION : " << setw(1) << obFee->commission();
  dc << "   REFUND/REISSUE: " << setw(1) << obFee->refundReissue() << "\n";
  dc << "NO CHARGE     : " << setw(1) << obFee->noCharge();
  dc << "   TAX INCLUDE: " << setw(1) << obFee->taxInclude() << "\n";
  dc.setf(ios::left, ios::adjustfield);
  dc << "ACC CODE T172 : " << setw(7) << obFee->svcFeesAccCodeTblItemNo() << " \n";
  dc << "TKT DSGN T173 : " << setw(7) << obFee->svcFeesTktDsgnTblItemNo() << " \n";
  dc << "SECURITY T183 : " << setw(7) << obFee->svcFeesSecurityTblItemNo();
  dc << "   PRIVATE IND: " << obFee->publicPrivateInd() << " \n";
}

void
Diag870Collector::printFinalPartDetailedRequest(StatusS4Validation status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "                                   S4 STATUS : ";
  displayStatus(status);
}

void
Diag870Collector::displayStatus(StatusS4Validation status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  // Currently max length for error msg is 15
  switch (status)
  {
  case PASS_S4:
    dc << "PASS";
    break;
  case FAIL_SVC_TYPE:
    dc << "FAIL SVC TYPE";
    break;
  case FAIL_PAX_TYPE:
    dc << "FAIL PAX TYPE";
    break;
  case FAIL_ACC_T172:
    dc << "FAIL ACC T172";
    break;
  case FAIL_TDSG_T173:
    dc << "FAIL TKTDS T173";
    break;
  case FAIL_SECUR_T183:
    dc << "FAIL SECUR T183";
    break;
  case FAIL_TKT_DATE:
    dc << "FAIL TKT DATE";
    break;
  case FAIL_TRV_DATE:
    dc << "FAIL TVL DATE";
    break;
  case FAIL_GEO_BTW:
    dc << "FAIL GEO BTW";
    break;
  case FAIL_FARE_BASIS:
    dc << "FAIL FARE BASIS";
    break;
  case FAIL_ON_GEO_LOC:
    dc << "FAIL GEO";
    break;
  case INTERNAL_ERROR:
    dc << "INTERNAL ERROR";
    break;
  case TJR_NOT_APPLY_TKTFEE:
    dc << "TJR OBFEES NOT APPLY SET TO Y";
    break;
  case OB_NOT_ACTIVE_YET:
    dc << "TICKETING DATE BEFORE OB ACTIVATION DATE";
    break;
  case VALIDATING_CXR_EMPTY:
    dc << "VALIDATING CXR EMPTY";
    break;
  case ALL_SEGS_OPEN:
    dc << "ALL SEGMENTS OPEN";
    break;
  case NOT_ALL_SEGS_CONFIRM:
    dc << "ALL SEGMENTS NOT CONFIRMED";
    break;
  case DUPLICATE_S4:
    dc << " DUPLICATE";
    break;
  case PASS_FOP:
    dc << " PASS FOP BIN";
    break;
  case PROCESSING_ERROR:
    dc << "ERROR";
    break;
  case FAIL_FOP_BIN:
    dc << "FAIL FOP BIN";
    break;
  case NON_CREDIT_CARD_FOP:
    dc << "NON CREDIT CARD FOP";
    break;
  case R_TYPE_NOT_REQUIRED:
    dc << "R TYPE NOT REQUIRED";
    break;
  case T_TYPE_NOT_REQUIRED:
    dc << "T TYPE NOT REQUIRED";
    break;
  case F_TYPE_NOT_REQUIRED:
    dc << "F TYPE NOT REQUIRED";
    break;
  case SKIP_TTYPE_NOT_EXCHANGE_REFUND:
    dc << "T90-T99 NOT EXCHANGE/REFUND";
    break;

  default:
    dc << "UNKNOWN STATUS";
    break;
  }
  dc << "\n";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag870Collector::printS4NotFound
//
// Description:  This method will print msg then S4 is not found
//
// </PRE>
// ----------------------------------------------------------------------------

void
Diag870Collector::printS4NotFound()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "          ********** NO S4 SEQUENCE FOUND **********\n";
}

void
Diag870Collector::printCanNotCollect(const StatusS4Validation status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << " *** NOT PROCESSED - ";
  displayStatus(status);
}

void
Diag870Collector::printCxrNotActive(const CarrierCode& cxr)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << " *** NOT PROCESSED - OB FEE PROCESSING NOT ACTIVE FOR " << cxr << " ***\n";
}

void
Diag870Collector::printFopBinNumber(const std::vector<FopBinNumber>& fopBinNumber)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  dc << "                  FOP BIN NUMBER -";
  string fop;
  for (FopBinNumber fop : fopBinNumber)
    dc << " " << fop;

  dc << "\n";
}

void
Diag870Collector::printFopBinInfo(const std::vector<FopBinNumber>& fopBinNumbers,
                                  const std::vector<Indicator>& cardTypes,
                                  const CurrencyCode& paymentCurrencyCode,
                                  const MoneyAmount& chargeAmount)
{
  if (!_active)
    return;

  if (2 != cardTypes.size() || 2 != fopBinNumbers.size())
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  printCardInfo(fopBinNumbers.front(), cardTypes.front());
  printCardInfo(fopBinNumbers.back(), cardTypes.back());
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "                  CHARGE AMOUNT  - ";
  dc << std::setw(8) << Money(chargeAmount, paymentCurrencyCode) << "\n";
}

void
Diag870Collector::printCardInfo(const FopBinNumber& fopBin, Indicator cardType)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "                  FOP BIN NUMBER - " << fopBin;
  switch (cardType)
  {
    case 'C': dc << " CREDIT\n"; break;
    case 'D': dc << " DEBIT\n"; break;
    default: dc << " CREDIT*\n"; break;
  }
}

void
Diag870Collector::printAmountFopResidualInd(const CurrencyCode& currency,
                                            const MoneyAmount mAmt,
                                            const bool chargeResInd)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "                  CHARGE AMOUNT  - ";
  dc << std::setw(8) << Money(mAmt, currency) << "\n";
  dc << "                  RESIDUAL IND   - " << (chargeResInd ? "TRUE" : "FALSE") << "\n";
}

void
Diag870Collector::printFopBinNotFound()
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc << " *** NO S4 SEQUENCE FOUND FOR BIN NUMBER"
     << "\n";
}

void
Diag870Collector::printOBFeesNotRequested()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "*** NOT PROCESSED - OB FEES NOT REQUESTED "
     << " \n";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag870Collector::operator <<
//
// Description:  This method will be the override base operator << to handle the
//         Tax Record Diagnostic Display.
//
// @param  TaxCodeData - Specific Tax Information
//
//
// </PRE>
// ----------------------------------------------------------------------------

Diag870Collector&
Diag870Collector::operator << (const FarePath& x)
{
  if (!_active)
    return *this;

  return *this;
}
} // namespace
