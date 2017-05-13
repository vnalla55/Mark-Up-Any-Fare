//----------------------------------------------------------------------------
//  File:        Diag865Collector.cpp
//  Authors:
//  Created:
//
//  Description: Diagnostic 865
//               NegotiatedFareRule commissions processing for IT/BT ticketing.
//
//  Updates:
//          11/12/2004  VK - create.
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

#include "Diagnostic/Diag865Collector.h"

#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/Money.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Currency.h"
#include "DBAccess/NegFareRest.h"
#include "Rules/Commissions.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleConst.h"

#include <iomanip>
#include <iostream>

namespace tse
{

void
Diag865Collector::displayRequestData(PricingTrx& trx,
                                     const FarePath& farePath,
                                     const Commissions& comm,
                                     const CollectedNegFareData& collectedNegFareData)
{
  if (_active)
  {
    Diag865Collector& dc = *this;

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << "***************************************************************\n";
    dc << "*  ATSE REQUEST DATA FOR NEGOTIATED FARE COMMISSION PROCESS   *\n";
    dc << "***************************************************************\n";
    dc << std::setw(35) << "  ABACUS USER"
       << ": " << formatFlag(trx.getRequest()->ticketingAgent()->abacusUser()) << '\n';
    dc << std::setw(35) << "  AXESS USER"
       << ": " << formatFlag(trx.getRequest()->ticketingAgent()->axessUser()) << '\n';
    dc << std::setw(35) << "  CWT USER"
       << ": " << formatFlag(trx.getRequest()->ticketingAgent()->cwtUser()) << '\n';
    dc << std::setw(35) << "  NET REMIT METHOD"
       << ": " << collectedNegFareData.bspMethod() << '\n';
    dc << std::setw(35) << "  TICKETING ENTRY"
       << ": " << formatFlag(comm.ticketingEntry()) << '\n';
    dc << std::setw(35) << "  PRINT SELLING CATEGORY 35 FARE"
       << ": " << formatFlag(trx.getOptions()->isCat35Sell()) << '\n';
    dc << std::setw(35) << "  PRINT NET CATEGORY 35 FARE"
       << ": " << formatFlag(trx.getOptions()->isCat35Net()) << '\n';
    dc << std::setw(35) << "  CREDIT CARD FOP"
       << ": " << formatFlag(trx.getRequest()->isFormOfPaymentCard()) << '\n';
    dc << std::setw(35) << "  CASH FOP"
       << ": " << formatFlag(trx.getRequest()->isFormOfPaymentCash()) << '\n';
    dc << std::setw(35) << "  OTHER FOP"
       << ": " << formatFlag(trx.getRequest()->isFormOfPaymentCheck() ||
                             trx.getRequest()->isFormOfPaymentGTR()) << '\n';
    dc << std::setw(35) << "  BASE FARE CURRENCY"
       << ": " << comm.baseFareCurrency() << '\n';
    dc << std::setw(35) << "  PAYMENT CURRENCY"
       << ": " << comm.paymentCurrency() << '\n';

    std::string commTypeString = "NONE", commAmountString;

    if (!trx.getRequest()->ticketingAgent()->agentCommissionType().empty())
    {
      if (trx.getRequest()->ticketingAgent()->agentCommissionType()[0] ==
          Commissions::PERCENT_COMM_TYPE)
      {
        commTypeString = "PERCENT";
        commAmountString = formatPercent(trx.getRequest()->ticketingAgent()->commissionPercent());
      }
      else
      {
        commTypeString = "AMOUNT";
        commAmountString = formatAmount(trx.getRequest()->ticketingAgent()->commissionAmount(),
                                        comm.paymentCurrency());
      }
    }

    dc << std::setw(35) << "  AGENT COMMISSION TYPE"
       << ": " << commTypeString << '\n';
    dc << std::setw(35) << "  AGENT COMMISSION VALUE"
       << ": " << (!commAmountString.empty() ? commAmountString : "");
    dc << "\n \n";
  }
}

void
Diag865Collector::displayFareComponentHeader()
{
  if (_active)
  {
    Diag865Collector& dc = *this;

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << "***************************************************************\n";
    dc << "*         COMMISSION ANALYSIS - FARE COMPONENT LEVEL          *\n";
  }
}

void
Diag865Collector::displayFarePathHeader(const FarePath& farePath)
{
  if (_active)
  {
    Diag865Collector& dc = *this;

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << "***************************************************************\n";
    dc << "*            COMMISSION ANALYSIS - FARE PATH LEVEL            *\n";
    dc << "***************************************************************\n";
    std::vector<PricingUnit*>::const_iterator puIter = farePath.pricingUnit().begin();

    for (; puIter != farePath.pricingUnit().end(); ++puIter)
    {
      if (!(*puIter)->isSideTripPU())
      {
        dc << *(*puIter);
      }
    }
    dc << "***************************************************************\n";
    dc.setf(std::ios::left, std::ios::adjustfield);

    displayPaxTypeLine(farePath.paxType()->paxType());
  }
}

void
Diag865Collector::displayCommissionData(const Commissions& comm,
                                        const FareUsage* fareUsage,
                                        const FarePath& farePath,
                                        const NegFareRest* negFareRest,
                                        const NegPaxTypeFareRuleData* negPaxTypeFare)
{
  if (_active)
  {
    Diag865Collector& dc = *this;
    const PaxTypeFare* paxTypeFare = fareUsage->paxTypeFare();

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << "***************************************************************\n";
    dc << " " << paxTypeFare->fareMarket()->origin()->loc() << " - ";
    dc << paxTypeFare->fare()->carrier() << " - ";
    dc << paxTypeFare->fareMarket()->destination()->loc() << "  ";
    dc << paxTypeFare->fare()->fareClass() << '\n';
    dc << "***************************************************************\n";

    displayPaxTypeLine(
        farePath.paxType()->paxType(), getFareBox(negFareRest), paxTypeFare->fcaDisplayCatType());
    dc << " \n";
    dc << std::setw(35) << "  COMMISSION PERCENT"
       << ": " << formatPercent(negFareRest->commPercent()) << '\n';
    dc << std::setw(35) << "  COMMISSION AMOUNT 1"
       << ": " << formatAmount(negFareRest->commAmt1(), negFareRest->cur1()) << '\n';
    dc << std::setw(35) << "  COMMISSION AMOUNT 2"
       << ": " << formatAmount(negFareRest->commAmt2(), negFareRest->cur2()) << '\n';
    dc << std::setw(35) << "  NET GROSS INDICATOR"
       << ": " << negFareRest->netGrossInd() << '\n';
    dc << " \n";

    displayCommissionData(paxTypeFare->nucFareAmount(),
                          negPaxTypeFare->nucNetAmount(),
                          paxTypeFare->mileageSurchargeAmt(),
                          comm.netMileageSurchargeAmt(),
                          (farePath.isRollBackSurcharges() ? fareUsage->stopOverAmtUnconverted()
                                                           : fareUsage->stopOverAmt()),
                          (farePath.isRollBackSurcharges() ? fareUsage->transferAmtUnconverted()
                                                           : fareUsage->transferAmt()),
                          (farePath.isRollBackSurcharges() ? fareUsage->surchargeAmtUnconverted()
                                                           : fareUsage->surchargeAmt()),
                          _totalSellAmtCalCurr,
                          comm.totalSellAmt(),
                          _totalNetAmtCalCurr,
                          comm.totalNetAmt(),
                          comm.calculationCurrency(),
                          comm.paymentCurrency());
    dc << "\n \n";
  }
}

void
Diag865Collector::displayCommissionData(Commissions& comm, const FarePath& farePath)
{
  if (_active)
  {
    Diag865Collector& dc = *this;
    const CollectedNegFareData* cNegFareData = farePath.collectedNegFareData();

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << " \n";
    dc << std::setw(35) << "  COMMISSION PERCENT"
       << ": " << formatPercent(cNegFareData->comPercent()) << '\n';
    dc << std::setw(35) << "  COMMISSION AMOUNT"
       << ": " << formatAmount(cNegFareData->comAmount(), cNegFareData->currency()) << '\n';
    dc << std::setw(35) << "  NET GROSS INDICATOR"
       << ": " << cNegFareData->indNetGross() << '\n';
    dc << " \n";

    MoneyAmount surchargeTotalAmt = cNegFareData->totalSellingMileageCharges() +
                                    cNegFareData->otherSurchargeTotalAmt() +
                                    cNegFareData->cat12SurchargeTotalAmt();

    MoneyAmount totalSellingAmt = cNegFareData->totalSellingAmt() + surchargeTotalAmt;
    MoneyAmount totalNetAmt = cNegFareData->netTotalAmt() + surchargeTotalAmt;

    totalNetAmt = cNegFareData->netTotalAmt() + cNegFareData->totalMileageCharges() +
                  cNegFareData->otherSurchargeTotalAmt() + cNegFareData->cat12SurchargeTotalAmt();

    displayCommissionData(cNegFareData->totalSellingAmt(),
                          cNegFareData->netTotalAmt(),
                          cNegFareData->totalSellingMileageCharges(),
                          cNegFareData->totalMileageCharges(),
                          cNegFareData->cat8SurchargeTotalAmt(),
                          cNegFareData->cat9SurchargeTotalAmt(),
                          cNegFareData->cat12SurchargeTotalAmt(),
                          totalSellingAmt,
                          comm.totalSellAmt(),
                          totalNetAmt,
                          comm.totalNetAmt(),
                          comm.calculationCurrency(),
                          comm.paymentCurrency());
    dc << "\n \n";
  }
}

void
Diag865Collector::displayPaxTypeLine(const PaxTypeCode& paxTypeCode,
                                     const std::string& fareBox,
                                     const Indicator& displayType)
{
  Diag865Collector& dc = *this;

  dc << "  REQUESTED PAXTYPE: " << std::setw(9) << paxTypeCode;
  dc << "TEXT: " << std::setw(9) << fareBox;
  dc << "DISPLAY TYPE: " << displayType << '\n';
}

void
Diag865Collector::displayPaxTypeLine(const PaxTypeCode& paxTypeCode)
{
  Diag865Collector& dc = *this;

  dc << "  REQUESTED PAXTYPE: " << paxTypeCode << '\n';
}

void
Diag865Collector::displayCommissionData(MoneyAmount sellingAmt,
                                        MoneyAmount netAmt,
                                        MoneyAmount milageSurcharge,
                                        MoneyAmount milageNetSurcharge,
                                        MoneyAmount cat8Charge,
                                        MoneyAmount cat9Charge,
                                        MoneyAmount cat12Charge,
                                        MoneyAmount totalSellAmt,
                                        MoneyAmount totalSellAmtInPayCurr,
                                        MoneyAmount totalNetAmt,
                                        MoneyAmount totalNetAmtInPayCurr,
                                        const CurrencyCode& calcCurr,
                                        const CurrencyCode& paymentCurr)
{
  Diag865Collector& dc = *this;

  dc << std::setw(35) << "  SELLING CATEGORY 35 FARE AMT"
     << ": " << formatAmount(sellingAmt, calcCurr) << '\n';
  dc << std::setw(35) << "  EXTRA MILAGE SURCHARGE"
     << ": " << formatAmount(milageSurcharge, calcCurr) << '\n';
  dc << std::setw(35) << "  CAT8  STOPOVER CHARGE"
     << ": " << formatAmount(cat8Charge, calcCurr) << '\n';
  dc << std::setw(35) << "  CAT9  TRANSFER CHARGE"
     << ": " << formatAmount(cat9Charge, calcCurr) << '\n';
  dc << std::setw(35) << "  CAT12 SURCHARGE"
     << ": " << formatAmount(cat12Charge, calcCurr) << '\n';
  dc << std::setw(35) << "  TOTAL SELL AMT"
     << ": " << formatAmount(totalSellAmt, calcCurr) << '\n';
  dc << std::setw(35) << "  TOTAL SELL AMT IN PAYMENT CURR"
     << ": " << formatAmount(totalSellAmtInPayCurr, paymentCurr);
  dc << "\n \n";
  dc << std::setw(35) << "  NET CATEGORY 35 FARE AMT"
     << ": " << formatAmount(netAmt, calcCurr) << '\n';
  dc << std::setw(35) << "  EXTRA MILAGE SURCHARGE"
     << ": " << formatAmount(milageNetSurcharge, calcCurr) << '\n';
  dc << std::setw(35) << "  CAT8  STOPOVER CHARGE"
     << ": " << formatAmount(cat8Charge, calcCurr) << '\n';
  dc << std::setw(35) << "  CAT9  TRANSFER CHARGE"
     << ": " << formatAmount(cat9Charge, calcCurr) << '\n';
  dc << std::setw(35) << "  CAT12 SURCHARGE"
     << ": " << formatAmount(cat12Charge, calcCurr) << '\n';
  dc << std::setw(35) << "  TOTAL NET AMT"
     << ": " << formatAmount(totalNetAmt, calcCurr) << '\n';
  dc << std::setw(35) << "  TOTAL NET AMT IN PAYMENT CURR"
     << ": " << formatAmount(totalNetAmtInPayCurr, paymentCurr);
}

void
Diag865Collector::displayCommissionApplication(const Commissions& comm,
                                               MoneyAmount amount,
                                               CommissionCalcMethod method,
                                               bool netFareCommission)
{
  if (_active)
  {
    Diag865Collector& dc = *this;
    std::string formattedAmount = formatAmount(amount, comm.paymentCurrency());

    dc.setf(std::ios::left, std::ios::adjustfield);
    if (netFareCommission)
      dc << "  NET COMMISSION APPLICATION\n";
    else
      dc << "  COMMISSION APPLICATION\n";
    dc << std::setw(35) << "  AMOUNT"
       << ": ";

    if (method == AMOUNT)
    {
      dc << formattedAmount;
    }
    dc << '\n';

    dc << std::setw(35) << "  NET  * COMM PCT"
       << ": ";

    if (method == NET_TIMES_COMM_PCT)
    {
      dc << formattedAmount;
    }
    dc << '\n';

    dc << std::setw(35) << "  SELL * COMM PCT"
       << ": ";

    if (method == SELL_TIMES_COMM_PCT)
    {
      dc << formattedAmount;
    }
    dc << '\n';

    if (netFareCommission)
    {
      dc << "\n \n"; // other commCalc methods not applicable to net fares.
      return;
    }
    dc << std::setw(35) << "  SELL - NET PLUS SELL * COMM PCT"
       << ": ";

    if (method == MARKUP_PLUS_SELL_TIMES_COMM_PCT)
    {
      dc << formattedAmount;
    }
    dc << '\n';

    dc << std::setw(35) << "  SELL - NET PLUS COMM AMOUNT"
       << ": ";

    if (method == MARKUP_PLUS_COMM_AMT)
    {
      dc << formattedAmount;
    }
    dc << '\n';

    dc << std::setw(35) << "  SELL - NET DIFFERENCE"
       << ": ";

    if (method == MARKUP)
    {
      dc << formattedAmount;
    }
    dc << "\n \n";
  }
}

void
Diag865Collector::displayFinalCommission(const Commissions& comm)
{
  if (_active)
  {
    Diag865Collector& dc = *this;

    dc.setf(std::ios::left | std::ios::fixed, std::ios::adjustfield | std::ios::floatfield);

    dc << "***************************************************************\n";
    dc << "*     FINAL NEGOTIATED FARE COMMISSION - FARE PATH LEVEL      *\n";
    dc << "***************************************************************\n";
    dc << std::setw(35) << "  CALCULATED COMMISSION AMOUNT"
       << ": " << formatAmount(comm.commAmount(), comm.paymentCurrency()) << '\n';
    dc << std::setw(35) << "  TICKET COMMISSION PERCENT"
       << ": " << formatPercent(comm.commPercent(), true) << '\n';

    if (!comm.ticketingEntry() && comm.isNetTktingWithItBtFare())
    { // pricing entry..display the net fare commissions
      dc << std::setw(35) << "  CALCULATED NET COMMISSION AMOUNT"
         << ": " << formatAmount(comm.netCommAmount(), comm.paymentCurrency()) << '\n';
      dc << std::setw(35) << "  TICKET NET COMMISSION PERCENT"
         << ": " << formatPercent(comm.netCommPercent(), true) << '\n';
    }
    dc << "***************************************************************\n \n";
    if (comm.errorCommission())
    {
      dc << "\n UNABLE TO OVERRIDE NEGOTIATED DATA FOR COMMISSION\n";
    }
  }
}

void
Diag865Collector::displayFailInfo()
{
  if (_active)
  {
    Diag865Collector& dc = *this;

    dc << "    CAT35 RULE - FAIL:\n";
    dc << " SKIP CAT35 COMMISSIONS - TOTAL NET AMOUNT EXCEEDS SELLING\n";
  }
}

std::string
Diag865Collector::getFareBox(const NegFareRest* negFareRest) const
{
  std::string fareBox;

  if (negFareRest->noSegs() == NegotiatedFareRuleUtil::ONE_SEGMENT)
  {
    fareBox = negFareRest->fareBoxText1();
  }
  else if (negFareRest->noSegs() == NegotiatedFareRuleUtil::TWO_SEGMENTS)
  {
    if (negFareRest->couponInd1() == NegotiatedFareRuleUtil::PSG_COUPON)
    {
      fareBox = negFareRest->fareBoxText1();
    }
    else if (negFareRest->couponInd2() == NegotiatedFareRuleUtil::PSG_COUPON)
    {
      fareBox = negFareRest->fareBoxText2();
    }
  }
  return fareBox;
}

std::string
Diag865Collector::formatAmount(MoneyAmount amount, const CurrencyCode& currencyCode) const
{
  CurrencyNoDec noDec = 2;

  if (currencyCode != NUC)
  {
    DataHandle dataHandle;
    const Currency* currency = nullptr;
    currency = dataHandle.getCurrency( currencyCode );

    if (currency)
    {
      noDec = currency->noDec();
    }
  }
  std::ostringstream os;

  os.setf(std::ios::left | std::ios::fixed, std::ios::adjustfield | std::ios::floatfield);
  os.precision(noDec);

  os << std::setw(7) << amount << "   " << currencyCode;

  return os.str();
}

std::string
Diag865Collector::formatPercent(Percent percent, bool zeroNA) const
{
  std::ostringstream os;

  if (percent == RuleConst::PERCENT_NO_APPL || (zeroNA && percent == 0))
  {
    os << "NOT APPLICABLE";
  }
  else
  {
    os.setf(std::ios::left | std::ios::fixed, std::ios::adjustfield | std::ios::floatfield);
    os.precision(2);
    os << percent;
  }

  return os.str();
}

std::string
Diag865Collector::formatFlag(bool value) const
{
  return (value ? "Y" : "N");
}
}
