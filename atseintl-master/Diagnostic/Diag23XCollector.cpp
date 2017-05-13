//----------------------------------------------------------------------------
//  File:        Diag23XCollector.C
//  Authors:     Artur Krezel
//  Created:     Apr 2007
//
//  Description: Diagnostic 231 & 233 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2007
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

#include "Diagnostic/Diag23XCollector.h"

#include "Common/ExchangeUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/Money.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/ExcItin.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexBaseRequest.h"
#include "Rules/RuleUtil.h"

#include <boost/logic/tribool.hpp>

#include <iomanip>

namespace tse
{
FALLBACK_DECL(excDiscDiag23XImprovements);
FALLBACK_DECL(diag23XVarianceFix);

Diag23XCollector&
Diag23XCollector::operator << ( RexBaseTrx& trx )
{
  if (!_active)
    return *this;

  _printType = (trx.excTrxType() == PricingTrx::AF_EXC_TRX) ? "ORIGINAL " : "EXCHANGE ";

  Diag23XCollector& dc = *this;
  dc << _printType << "TICKET\n";
  dc << "CONVERTED CURRENCY: " << trx.exchangeItin()[0]->calculationCurrency() << '\n';
  dc << _printType
      << "PAX TYPE: " << (trx.exchangePaxType() ? trx.exchangePaxType()->paxType() : "") << '\n';
  dc << "\n";

  if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    RexPricingTrx& rtrx = static_cast<RexPricingTrx&>(trx);

    if (rtrx.isPlusUpCalculationNeeded())
    {
      if (!boost::indeterminate(rtrx.excTktNonRefundable()))
      {
        dc << "ALL FARE COMPONENTS ";

        if (rtrx.excTktNonRefundable())
          dc << "NON";

        dc << "REFUNDABLE:\n  SKIPPING NONREFUNDABLE EXC TKT PART EXTRACTION\n";
        dc << "  NONREFUNDABLE FARES: XAN, XBN, XPN, XPV, EIP, SIP\n";
        dc << "  EXC TOTAL FARE CALC AMT C5E: " << trx.totalFareCalcAmount() << "\n";
      }
    }
  }

  dc << " \n";
  _trx = &trx;

  return *this;
}

void
Diag23XCollector::printFareAmount(const FareCompInfo& fc)
{
  *this << "FARE CALC FARE AMOUNT: ";
  if (_trx != nullptr)
  {
    ExcItin& excItin = *(_trx->exchangeItin().front());

    if (excItin.calculationCurrency() != excItin.calcCurrencyOverride())
    {
      Money fareAmtOverridenCurr(fc.tktFareCalcFareAmt(), excItin.calcCurrencyOverride());
      *this << fareAmtOverridenCurr << " ";

      Money fareAmtOriginalCurr(fc.fareCalcFareAmt(), excItin.calculationCurrency());
      *this << fareAmtOriginalCurr;
    }
    else
    {
      Money fareAmt(fc.tktFareCalcFareAmt(), excItin.calculationCurrency());
      *this << fareAmt;
    }
    *this << '\n';

    const RexBaseRequest* request = static_cast<RexBaseRequest*>(_trx->getRequest());
    if (fallback::excDiscDiag23XImprovements(_trx))
    {
      if (request->excDiscounts().isPPEntry())
      {
        uint32_t segmentNo = fc.fareMarket()->travelSeg().front()->pnrSegment();
        const Percent* percent = request->excDiscounts().getPercentage(segmentNo, false);
        if (percent && *percent != 0.0)
          *this << "MARKUP PERCENT " << std::to_string(-1.0 * (*percent)) << "\n";
      }
      else if (request->excDiscounts().isPAEntry())
      {
        uint32_t segmentNo = fc.fareMarket()->travelSeg().front()->pnrSegment();
        const DiscountAmount* da = request->excDiscounts().getAmount(segmentNo);
        if (da && da->amount)
          *this << "MARKUP AMOUNT " << Money(-1.0 * da->amount, da->currencyCode).toString()
                << "\n";
      }
    }
    else
    {
      const uint32_t segmentNo = fc.fareMarket()->travelSeg().front()->pnrSegment();
      const auto getName = [](auto value){ return value < 0 ? "MARKUP" : "DISCOUNT"; };
      const Percent* percent = request->excDiscounts().getPercentage(segmentNo, false);
      if (percent && *percent != 0.0)
      {
        *this << getName(*percent) << " PERCENT " << std::abs(*percent) << "\n";
      }
      else
      {
        const DiscountAmount* da = request->excDiscounts().getAmount(segmentNo);
        if (da && da->amount)
        {
          *this << getName(da->amount) << " AMOUNT "
                << Money(std::abs(da->amount), da->currencyCode) << "\n";
        }
      }
    }
  }
  else
  {
    *this << '\n';
  }
}


namespace
{
const char* const
getFareMatchingDate(const uint16_t phase)
{
  switch (phase)
  {
  case PREVIOUS_EXCHANGE_DATE:
    return "PREVIOUS EXCHANGE DATE";
  case ORIGINAL_TICKET_DATE:
    return "ORIGINAL TICKET DATE";
  case REISSUE_TICKET_DATE:
    return "REISSUE TICKET DATE";
  case COMMENCE_DATE:
    return "COMMENCE DATE";
  case ONE_DAY_BEFORE_PREV_EXC_DATE:
    return "ONE DAY BEFORE PREV EXC DATE";
  case TWO_DAYS_BEFORE_PREV_EXC_DATE:
    return "TWO DAYS BEFORE PREV EXC DATE";
  case ONE_DAY_AFTER_PREV_EXC_DATE:
    return "ONE DAY AFTER PREV EXC DATE";
  case TWO_DAYS_AFTER_PREV_EXC_DATE:
    return "TWO DAYS AFTER PREV EXC DATE";
  case ONE_DAY_BEFORE_ORIG_TKT_DATE:
    return "ONE DAY BEFORE ORIG TKT DATE";
  case TWO_DAYS_BEFORE_ORIG_TKT_DATE:
    return "TWO DAYS BEFORE ORIG TKT DATE";
  case ONE_DAY_AFTER_ORIG_TKT_DATE:
    return "ONE DAY AFTER ORIG TKT DATE";
  case TWO_DAYS_AFTER_ORIG_TKT_DATE:
    return "TWO DAYS AFTER ORIG TKT DATE";
  case ONE_DAY_BEFORE_REISSUE_TKT_DATE:
    return "ONE DAY BEFORE REISSUE TKT DATE";
  case TWO_DAYS_BEFORE_REISSUE_TKT_DATE:
    return "TWO DAYS BEFORE REISSUE TKT DATE";
  case ONE_DAY_AFTER_REISSUE_TKT_DATE:
    return "ONE DAY AFTER REISSUE TKT DATE";
  case TWO_DAYS_AFTER_REISSUE_TKT_DATE:
    return "TWO DAYS AFTER REISSUE TKT DATE";
  case ONE_DAY_BEFORE_COMMENCE_DATE:
    return "ONE DAY BEFORE COMMENCE DATE";
  case TWO_DAYS_BEFORE_COMMENCE_DATE:
    return "TWO DAYS BEFORE COMMENCE DATE";
  case ONE_DAY_AFTER_COMMENCE_DATE:
    return "ONE DAY AFTER COMMENCE DATE";
  case TWO_DAYS_AFTER_COMMENCE_DATE:
    return "TWO DAYS AFTER COMMENCE DATE";
  case FARE_COMPONENT_DATE:
    return "FARE COMPONENT DATE";
  }
  return "";
}

} // namespace

Diag23XCollector&
Diag23XCollector::operator<<(const FareCompInfo& fc)
{
  if (!_active)
    return *this;

  Diag23XCollector& dc = *this;

  dc << "******************* FARE COMPONENT MATCHING ******************\n";
  dc << "FARE COMPONENT NUMBER: " << fc.fareCompNumber() << '\n';
  dc << "FARE BASIS CODE: " << fc.fareBasisCode() << '\n';
  printFareAmount(fc);

  if (fallback::excDiscDiag23XImprovements(_trx))
  {
    if (fc.discounted())
      dc << "MANUAL DISCOUNT APPLIED\n";
  }

  dc << "HIP INCLUDED: " << (fc.hipIncluded() ? "Y": "N") << '\n';
  dc << "MILEAGE SURCHARGE PERCENTAGE: " << fc.mileageSurchargePctg() << '\n';
  dc << "VCTR USED FOR MATCH: " << (fc.hasVCTR() ? "Y" : "N") << '\n';
  dc << "MATCHED DATE: " << getFareMatchingDate(fc.fareMatchingPhase()) << '\n';

  if (fc.secondaryFMprocessed())
  {
    dc << "ORIGINAL FARE MARKET NOT MATCHED" << '\n';

    if (fc.getMatchedFares().empty())
    {
      dc << *fc.fareMarket();
      dc << "SECONDARY FARE MARKET NOT MATCHED" << '\n';
      dc << *fc.secondaryFareMarket();
    }
    else
    {
      dc << *fc.secondaryFareMarket();
      dc << "SECONDARY FARE MARKET MATCHED" << '\n';
      dc << *fc.fareMarket();
    }
  }
  else
  {
    dc << *fc.fareMarket();
  }

  return *this;
}

void
Diag23XCollector::printSeparator()
{
  if (_active)
  {
    if (fallback::diag23XVarianceFix(_trx))
    {
      ((DiagCollector&)*this)
          << "  GI V RULE  FARE BASIS  TRF O O      AMT CUR FAR PAX   CNV    VAR\n"
          << "                         NUM R I              TPE TPE   AMT   \n"
          << "- -- - ---- ------------ --- - - -------- --- --- --- -------- ----- \n";
    }
    else
    {
      ((DiagCollector&)*this)
          << "  GI V RULE  FARE BASIS  TRF O O      AMT CUR FAR PAX   CNV      VAR\n"
          << "                         NUM R I              TPE TPE   AMT   \n"
          << "- -- - ---- ------------ --- - - -------- --- --- --- -------- ------- \n";
    }
  }
}

Diag23XCollector&
Diag23XCollector::operator<<(const PaxTypeFare& paxFare)
{
  if (!_active)
    return *this;

  Diag23XCollector& dc = *this;

  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << std::setw(2) << cnvFlags(paxFare);

  std::string gd;
  globalDirectionToStr(gd, paxFare.fare()->globalDirection());

  dc << std::setw(3) << gd << std::setw(2) << Vendor::displayChar(paxFare.vendor());
  dc << std::setw(5) << paxFare.ruleNumber();
  std::string fareBasis = paxFare.createFareBasis(_trx, false);
  if (fareBasis.size() > 12)
    fareBasis = fareBasis.substr(0, 12) + "*"; // Cross-of-lorraine?
  dc << std::setw(13) << fareBasis << std::setw(4) << paxFare.tcrRuleTariff();

  dc << std::setw(2) << DiagnosticUtil::getOwrtChar(paxFare);

  if (paxFare.directionality() == FROM)
    dc << std::setw(2) << "O";
  else if (paxFare.directionality() == TO)
    dc << std::setw(2) << "I";
  else
    dc << "  ";

  dc << std::setw(8) << Money(paxFare.fareAmount(), paxFare.currency()) << " ";

  if (!paxFare.isFareClassAppMissing())
  {
    dc << std::setw(4) << paxFare.fcaFareType();
  }
  else
  {
    dc << "UNK";
  }

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

  printAmountWithVariance(paxFare);

  return *this;
}

void
Diag23XCollector::printConvertedAmount(const MoneyAmount& amt, int width)
{
  setf(std::ios::right, std::ios::adjustfield);
  setf(std::ios::fixed, std::ios::floatfield);

  Money convertedAmount(amt, _trx->exchangeItin()[0]->calculationCurrency());
  *this << std::setw(width) << std::setprecision(convertedAmount.noDec(_trx->ticketingDate()))
        << convertedAmount.value();
}

void
Diag23XCollector::printAmountWithVariance(const PaxTypeFare& ptf)
{
  printConvertedAmount(ptf.nucFareAmount(), 8);
  if (_variance < -EPSILON)
    *this << std::setw(5) << "N/A";
  else
  {
    if (fallback::diag23XVarianceFix(_trx))
      *this << " " << std::setw(5) << std::setprecision(2) << _variance;
    else
      *this << " " << std::setw(7) << std::setprecision(4) << _variance;
  }

  *this << '\n';
}

Diag23XCollector&
Diag23XCollector::operator << ( const FareMarket& fareMarket )
{
  if (!_active)
    return *this;

  Diag23XCollector& dc = *this;

  if ( _trx->isIataFareSelectionApplicable() )
  {
    char tpmStatus = 'N';
    if ( fareMarket.isFirstCrossingAndHighTpm() )
      tpmStatus = 'B';
    else if ( fareMarket.isHighTPMGoverningCxr() )
      tpmStatus = 'Y';
    dc << "HIGHEST TPM GOVERNING CARRIER:  " << tpmStatus << std::endl;
  }

  dc << " \n" << FareMarketUtil::getFullDisplayString(fareMarket) << "\n \n";

  dc << "GEOTRAVELTYPE : ";
  dc << DiagnosticUtil::geoTravelTypeToString(fareMarket.geoTravelType());
  dc << "\n \n";

  const PaxTypeBucket* ptc = fareMarket.paxTypeCortege(_trx->paxType().front());

  if (ptc && !fareMarket.fareCompInfo()->getMatchedFares().empty())
  {
    dc << " \n";
    dc << "REQUESTED PAXTYPE : " << ptc->requestedPaxType()->paxType() << '\n';
    dc << " INBOUND CURRENCY : " << ptc->inboundCurrency() << '\n';
    dc << "OUTBOUND CURRENCY : " << ptc->outboundCurrency() << '\n';
    dc << " \n";
  }

  else
  {
    dc << " \n";
    dc << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->loc() << '-'
        << fareMarket.destination()->loc() << ". REQUESTED PAXTYPE : ";

    if (ptc)
      dc << ptc->requestedPaxType()->paxType();

    dc << '\n';
  }

  dc << "\n \n";

  return *this;
}

void
Diag23XCollector::parseQualifiers(RexBaseTrx& trx)
{
  _trx = &trx;

  std::map<std::string, std::string>::const_iterator e = trx.diagnostic().diagParamMap().end();

  std::map<std::string, std::string>::const_iterator i;

  i = trx.diagnostic().diagParamMap().find(Diagnostic::DIAG_VENDOR);
  if (i != e)
    if (!i->second.empty())
      _vendor = i->second;

  i = trx.diagnostic().diagParamMap().find(Diagnostic::DISPLAY_DETAIL);
  if (i != e)
  {
    _allFares = (i->second == "ALLFARES");
    _ddAllPax = (i->second == "ALLPAX");
  }

  i = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_TYPE);
  if (i != e)
    _fareType = i->second;
}

Diag23XCollector&
Diag23XCollector::operator<< (const FareCompInfo::MatchedFare& fare)
{
  if (!_active )
      return *this;
  _variance = fare.getVariance();
  (*this) << *fare.get();
  printNetAmountLine(*fare.get());
  return *this;
}

void
Diag23XCollector::printNetAmountLine(const PaxTypeFare& ptf)
{
  if (!ptf.isNegotiated() || !_trx->getRexOptions().isNetSellingIndicator())
    return;

  const NegPaxTypeFareRuleData* ruleData = ptf.getNegRuleData();
  if (!ruleData)
    return;

  *this << " NET AMOUNT: " << Money(ruleData->netAmount(), ptf.currency());
  *this << " CNV: ";
  printConvertedAmount(ruleData->nucNetAmount(), 0);
  *this << '\n';
}

} // tse
