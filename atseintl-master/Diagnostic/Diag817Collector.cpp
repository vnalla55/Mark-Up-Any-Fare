//----------------------------------------------------------------------------
//  File:        Diag817Collector.C
//  Authors:     Piotr Badarycz
//  Created:     Oct 2008
//
//  Description: Basic tax diagnostic
//
//  Updates:
//          10/1/2008 - VR - Intitial Development
//
//  Copyright Sabre 2008
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
#include "Diagnostic/Diag817Collector.h"

#include "Common/FallbackUtil.h"
#include "Common/Money.h"
#include "Common/RexPricingPhase.h"
#include "Common/TrxUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "Taxes/LegacyTaxes/CalculationDetails.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

#include <iomanip>
#include <sstream>

namespace
{
static const std::string ANCILLARY_TAXES_REQUESTED = "OC";
static const uint32_t MAX_MUM_OF_ANCILLARY_TAXES = 15;
static const std::string DETAILED_TAXES_REQUESTED = "DC";
using std::setw;

constexpr int WIDTH = 20;
constexpr int AMT_WIDTH = 13;
}

namespace tse
{
FALLBACK_DECL(ssdsp1508fix);
FALLBACK_DECL(dontFilterFailedTaxesFor817Diag);
FALLBACK_DECL(displayTaxInPaymentPrecision);

class BSRCollectionResults;

void
Diag817Collector::buildDiag817Header(const TaxResponse& taxResponse,
                                     TaxDisplayHeaderType taxDisplayHeaderType,
                                     bool utc_display = false)
{

  if ((!_active) || (taxDisplayHeaderType != DIAGNOSTIC_HEADER))
    return;

  uint32_t taxResponseSize;

  taxResponseSize = static_cast<uint32_t>(taxResponse.taxItemVector().size() +
                                          taxResponse.changeFeeTaxItemVector().size());

  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);
  DiagCollector& dc(*this);

  if (!utc_display)
  {
    dc << "\n"
       << "***************************************************************\n";

    if ((typeid(*(taxResponse.farePath())) == typeid(NetFarePath)) &&
        TrxUtil::isCat35TFSFEnabled(*pricingTrx))
      dc << "NET FARE PATH  \n";
    dc << " ---- TAX OUT VECTOR  " << taxResponseSize << " ITEMS --- PSGR "
       << taxResponse.paxTypeCode() << "----        \n"
       << "***************************************************************\n";
    if(pricingTrx->isValidatingCxrGsaApplicable())
    {
      dc << "VALIDATING CARRIER: " << taxResponse.validatingCarrier() << "\n"
         << "***************************************************************\n";
    }
    dc << "  CODE TYP  TXAMT    TXTTL   TXFARE BOARD OFF CARRIER SEQNO SPN\n"
       << "                      \n";
  }
  else
  {
    dc  << "\n"
        << "***************************************************************\n"
        << " ---- TAX OUT VECTOR  " << taxResponseSize << " ITEMS\n"
        << "***************************************************************\n";
    if(pricingTrx->isValidatingCxrGsaApplicable())
    {
      dc << "VALIDATING CARRIER: " << taxResponse.validatingCarrier() << "\n"
         << "***************************************************************\n";
    }
    dc << "   CODE   SEQNO     UT CONFIG NAME\n \n";
  }
}

void
Diag817Collector::buildDiag817OcHeader(const PaxTypeCode& pax)
{
  if (_active)
  {
    (static_cast<DiagCollector&>(*this))
        << "\n"
        << "***************************************************************\n"
        << " ---- ANCILLARY TAXES OUT VECTOR ---- PSGR " << pax << " ----\n"
        << "***************************************************************\n"
        << "  CODE TYP  TXAMT    SEQNO\n \n";
  }
}

void
Diag817Collector::buildDiag817DcHeader(const TaxCode& taxCode,
                                       const uint32_t& taxResponseNum,
                                       const CarrierCode& cxr)
{
  if (_active)
  {
    (static_cast<DiagCollector&>(*this))
        << "\n"
        << "***************************************************************\n"
        << " ---- DETAILED TAXES CALCULATION --- TAX " << taxCode << " NR" << taxResponseNum
        << " ----\n"
        << "***************************************************************\n";
    PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);
    if(pricingTrx->isValidatingCxrGsaApplicable())
    {
      (static_cast<DiagCollector&>(*this)) << "VALIDATING CARRIER: " << cxr << "\n"
         << "***************************************************************\n";
    }
  }
}

void
Diag817Collector::displayDiag817(std::vector<TaxItem*>& itemVector,
                                 DiagCollector& dc,
                                 uint32_t& taxResponseNum,
                                 bool utc_display)
{
  TaxResponse::TaxItemVector::const_iterator taxItemI;

  for (taxItemI = itemVector.begin(); taxItemI != itemVector.end(); taxItemI++)
  {
    const TaxItem* taxItem = *taxItemI;

    if (!fallback::dontFilterFailedTaxesFor817Diag(_trx))
    {
      // Filtering out taxes with non-zero failCode
      // Trying to keep this change small now, only for certain tax code that was seen
      // on diagnostic, while it should not been there.
      // Subject to be changed to (taxItem->failCode() != TaxDiagnostic::NONE) later.
      if (taxItem->failCode() == TaxDiagnostic::NO_TAX_ADDED)
        continue;
    }

    Money paymentCurrency(taxItem->paymentCurrency());
    Money taxCurrency(taxItem->taxCur());

    if (fallback::displayTaxInPaymentPrecision(_trx))
    {
      dc.precision(paymentCurrency.noDec());
    }

    dc << setw(2) << taxResponseNum << " ";

    dc << taxItem->taxCode() << " ";
    if (taxItem->taxCode().size() < 3)
      dc << " ";

    if (!utc_display)
    {
      dc << taxItem->taxType();
      if (taxItem->taxType() != 'P')
      {
        if (!fallback::displayTaxInPaymentPrecision(_trx))
        {
          dc.precision(taxCurrency.noDec());
        }
        dc << setw(8) << taxItem->taxAmt() << taxItem->taxCur();
      }
      else
      {
        if (taxItem->specialPercentage() > EPSILON)
          dc << setw(8) << taxItem->specialPercentage() * 100 << "   ";
        else
          dc << setw(8) << taxItem->taxAmt() * 100 << "   ";
      }

      if (!fallback::displayTaxInPaymentPrecision(_trx))
      {
        dc.precision(paymentCurrency.noDec());
      }
      dc << setw(8) << taxItem->taxAmount() << " ";
      dc << setw(8) << taxItem->taxableFare() << "  ";
      dc << taxItem->taxLocalBoard() << " " << taxItem->taxLocalOff() << "  ";
      dc << taxItem->carrierCode() << " ";
      dc << setw(8) << taxItem->seqNo() << " ";
      dc << setw(4) << taxItem->specialProcessNo();
      dc << "\n";
    }
    else
    {
      dc << setw(8) << taxItem->seqNo() << " ";
      dc << taxItem->specConfigName();
      dc << "\n";
    }
    taxResponseNum++;
  } // End TaxResponse For Loop
}

void
Diag817Collector::buildDiag817(const TaxResponse& taxResponse)
{
  DiagCollector& dc(*this);
  uint32_t taxResponseNum = 1;

  const std::string utcParamName = "UT";
  const std::string& utc_str = dc.rootDiag()->diagParamMapItem(utcParamName);

  bool utc_display = (dc.rootDiag()->diagParamMapItemPresent(utcParamName) && utc_str.empty());
  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);

  if (!utc_str.empty() && pricingTrx)
  {

    DataHandle dataHandle(pricingTrx->getRequest()->ticketingDT());
    std::vector<TaxSpecConfigReg*> tscv = dataHandle.getTaxSpecConfig(utc_str);
    std::vector<TaxSpecConfigReg*>::const_iterator cit;
    for (cit = tscv.begin(); cit != tscv.end(); ++cit)
    {
      dc << "*** UNIVERSAL TAX CONFIGURATION : " << utc_str << "  ***\n";
      dc << "EFFECTIVE DATE: " << (*cit)->effDate().dateToIsoExtendedString() << "\n";
      dc << "DISCONTINUE DATE: " << (*cit)->discDate().dateToIsoExtendedString() << "\n \n";
      dc << "PARAMETER NAME : VALUE\n \n";
      std::vector<TaxSpecConfigReg::TaxSpecConfigRegSeq*>::const_iterator sit;
      for (sit = (*cit)->seqs().begin(); sit < (*cit)->seqs().end(); ++sit)
      {
        dc << setw(14) << (*sit)->paramName() << " : " << (*sit)->paramValue() << "\n";
      }
    }

    if (tscv.empty())
    {
      dc << "*** NO SUCH UNIVERSAL TAX CONFIGURATION :\n     " << utc_str << " \n";
    }
    return;
  }

  if (utc_str.empty())
    buildDiag817Header(taxResponse, DIAGNOSTIC_HEADER, utc_display);

  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc.setf(std::ios::right, std::ios::adjustfield);

  std::vector<TaxItem*> taxItemVector = taxResponse.taxItemVector();

  std::vector<TaxItem*> changeFeeTaxItemVector;

  changeFeeTaxItemVector = taxResponse.changeFeeTaxItemVector();

  if (taxItemVector.empty() && changeFeeTaxItemVector.empty())
  {
    dc << " T A X E S   N O T   A P P L I C A B L E \n";
    return;
  }

  if (!taxItemVector.empty())
  {
    displayDiag817(taxItemVector, dc, taxResponseNum, utc_display);
  }

  if (!changeFeeTaxItemVector.empty())
  {
    displayDiag817(changeFeeTaxItemVector, dc, taxResponseNum, utc_display);
  }
}

void
Diag817Collector::displayInfo()
{
  DiagCollector& dc(*this);

  const std::string utcParamName = "UT";
  const std::string& utc_str = dc.rootDiag()->diagParamMapItem(utcParamName);

  bool utc_display = (dc.rootDiag()->diagParamMapItemPresent(utcParamName) && utc_str.empty());
  if (!utc_display)
  {
    dc << " \n * FOR UTC INQUIRY USE UT PARAMETER : /*817/UT *\n";
  }
  else
  {
    dc << " \n * TO DISPLAY CONFIG PARAMS AND VALUES : /*817/UTCONFIGNAME *\n";
  }
  dc << " * FOR DETAILED CALCULATION OF TAXINQUIRY USE DC PARAMETER: \n";
  dc << "   /*817/DCTAXCODE --- E.G.: /*817/DCUS1 --- \n";
  dc << " * FOR DETAILED CALCULATION OF TAXITEM NR USE NR PARAMETER: \n";
  dc << "   /*817/DCTAXCODE/NRTAXITEMNR --- E.G.: /*817/DCUS1/NR10 --- \n";
}

void
Diag817Collector::buildDiag817Oc(const TaxResponse& taxResponse)
{
  DiagCollector& dc(*this);
  uint32_t taxResponseNum = 0;

  buildDiag817OcHeader(taxResponse.paxTypeCode());

  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc.setf(std::ios::right, std::ios::adjustfield);
  std::vector<ServiceFeesGroup*>::const_iterator sfgIter =
      taxResponse.farePath()->itin()->ocFeesGroup().begin();
  std::vector<ServiceFeesGroup*>::const_iterator sfgIterEnd =
      taxResponse.farePath()->itin()->ocFeesGroup().end();
  for (; sfgIter != sfgIterEnd; ++sfgIter)
  {
    ServiceFeesGroup* sfg = *sfgIter;
    std::map<const FarePath*, std::vector<OCFees*> >& ocFeesMap = sfg->ocFeesMap();

    const FarePath* farePath = taxResponse.farePath();
    std::vector<OCFees*> ocFees = ocFeesMap[const_cast<FarePath*>(farePath)];
    std::vector<OCFees*>::const_iterator iOcFees = ocFees.begin();
    std::vector<OCFees*>::const_iterator iEOcFees = ocFees.end();

    for (; iOcFees != iEOcFees; ++iOcFees)
    {
      std::vector<OCFees::TaxItem>::const_iterator iTaxItem = (*iOcFees)->getTaxes().begin();
      std::vector<OCFees::TaxItem>::const_iterator iEndTaxItem = (*iOcFees)->getTaxes().end();

      for (; iTaxItem != iEndTaxItem; iTaxItem++)
      {
        taxResponseNum++;

        if (taxResponseNum == MAX_MUM_OF_ANCILLARY_TAXES + 1)
          dc << "****************************************************************\n"
             << "NOT PROCESSED:\n";

        Money moneyPayment(iTaxItem->getCurrencyPub());
        dc.precision(2);

        dc << setw(2) << taxResponseNum << " ";
        dc << setw(3) << iTaxItem->getTaxCode() << " ";
        dc << iTaxItem->getTaxType();
        if (iTaxItem->getTaxType() != 'P')
          dc << setw(8) << iTaxItem->getTaxAmountPub() << iTaxItem->getCurrencyPub();
        else
          dc << setw(8) << iTaxItem->getTaxAmountPub() * 100 << "   ";

        dc << setw(7) << iTaxItem->getSeqNo() << "\n";
      }

      if (taxResponseNum)
        return;
    }
  }

  if (taxResponseNum == 0)
  {
    dc.clear();
    dc << " A N C I L L A R Y   T A X E S   N O T   A P P L I C A B L E\n";
  }
}

void
Diag817Collector::buildDiag817Dc(const TaxResponse& taxResponse)
{
  DiagCollector& dc(*this);

  TaxResponse::TaxItemVector::const_iterator taxItemI;

  if (taxResponse.taxItemVector().empty())
  {
    dc << " T A X E S   N O T   A P P L I C A B L E \n";
    return;
  }

  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc.setf(std::ios::right, std::ios::adjustfield);

  const CarrierCode cxr = taxResponse.validatingCarrier();

  uint32_t taxResponseNum = 1;
  for (taxItemI = taxResponse.taxItemVector().begin();
       taxItemI != taxResponse.taxItemVector().end();
       taxItemI++, taxResponseNum++)
  {
    const TaxItem* taxItem = *taxItemI;

    uint32_t taxResponseNumRequested;
    try
    {
      taxResponseNumRequested =
          boost::lexical_cast<uint32_t>(dc.rootDiag()->diagParamMapItem("NR"));
    }
    catch (boost::bad_lexical_cast&) { taxResponseNumRequested = 0; }
    bool isTaxResponseNumMatched =
        taxResponseNumRequested == 0 || taxResponseNumRequested == taxResponseNum;

    std::string taxCodeRequested = dc.rootDiag()->diagParamMapItem("DC");
    bool isTaxCodeMatched = taxCodeRequested.empty() || taxCodeRequested == taxItem->taxCode();

    if (isTaxResponseNumMatched && isTaxCodeMatched)
    {
      buildDiag817DcHeader(taxItem->taxCode(), taxResponseNum, cxr);

      if (taxItem->isCalculationDetailsSet())
      {
        const CalculationDetails& details = taxItem->calculationDetails();

        const TaxTypeCode taxType = taxItem->taxType();
        if (taxType == Tax::PERCENTAGE)
        {
          dc << setw(WIDTH) << "PERCENT TAX:" << setw(AMT_WIDTH) << taxItem->taxAmt() << "\n";
          if (!details.isTaxOnTax || taxItem->taxOnTaxExcl() != YES)
          {
            if (!taxItem->taxOnChangeFee())
            {
              if (details.earlyPlusUpAmount != 0)
                dc << setw(WIDTH) << "EARLYPLUSUP:" << setw(AMT_WIDTH) << details.earlyPlusUpAmount
                   << details.calculationCurrency << "\n";
              dc << setw(WIDTH) << "FARE IN CALCCUR:" << setw(AMT_WIDTH)
                 << details.fareInCalculationCurrencyAmount << details.calculationCurrency << "\n";

              if (details.calculationCurrency != details.baseFareCurrency)
                printExchange(details.calculationToBaseFareResults);

              dc << setw(WIDTH) << "FARE IN BASEFARECUR:" << setw(AMT_WIDTH)
                 << details.fareInBaseFareCurrencyAmount << details.baseFareCurrency << "\n";

              if (details.baseFareCurrency != taxItem->paymentCurrency())
                printExchange(details.baseFareToPaymentResults);

              dc << setw(WIDTH) << "FARE IN PAYMENTCUR:" << setw(AMT_WIDTH)
                 << details.fareInPaymentCurrencyAmount << taxItem->paymentCurrency() << "\n";

              if (details.isInternationalRounded)
                dc << setw(WIDTH) << "INTERNATIONAL ROUND:" << setw(AMT_WIDTH)
                   << details.internationalRoundedAmount << taxItem->paymentCurrency() << "\n";
            }
            else
            {
              // foreach penalty: penalty source, intermediate, target
            }
          }
          if (details.isTaxOnTax)
          {
            std::vector<std::pair<TaxCode, MoneyAmount> >::const_iterator taxableTax;
            for (taxableTax = details.taxableTaxes.begin();
                 taxableTax != details.taxableTaxes.end();
                 taxableTax++)
              dc << setw(WIDTH - 4) << "TAX " << setw(3) << taxableTax->first << ":"
                 << setw(AMT_WIDTH) << taxableTax->second << taxItem->paymentCurrency() << "\n";

            dc << setw(WIDTH) << "TAXABLE TAX SUM:" << setw(AMT_WIDTH)
               << details.taxableTaxSumAmount << taxItem->paymentCurrency() << "\n";
            if (taxItem->taxOnTaxExcl() == YES)
              dc << setw(WIDTH) << "FAREEXCLUDED:" << setw(AMT_WIDTH) << taxItem->taxOnTaxExcl()
                 << "\n";
            else
              dc << setw(WIDTH) << "FAREWITHTAXES:" << setw(AMT_WIDTH) << taxItem->taxableFare()
                 << taxItem->paymentCurrency() << "\n";
          }
        }
        else
        {
          dc << setw(WIDTH) << "FIXED TAX:" << setw(AMT_WIDTH) << taxItem->taxAmt()
             << taxItem->taxCur() << "\n";
          if (taxItem->taxCur() != taxItem->paymentCurrency())
          {
            dc << setw(WIDTH) << "EXCHANGERATE:" << setw(AMT_WIDTH) << taxItem->exchangeRate1()
               << "\n";
            if (taxItem->intermediateCurrency() != "")
            {
              dc << setw(WIDTH) << "INTERMEDIATE:" << setw(AMT_WIDTH)
                 << taxItem->intermediateAmount() << taxItem->intermediateCurrency() << "\n";
              dc << setw(WIDTH) << "EXCHANGERATE:" << setw(AMT_WIDTH) << taxItem->exchangeRate2()
                 << "\n";
            }
          }
        }
        if (details.taxToAdjustAmount != 0)
        {
          dc << setw(WIDTH) << "TAX TO ADJUST:" << setw(AMT_WIDTH) << details.taxToAdjustAmount
             << taxItem->paymentCurrency() << "\n";
        }
        if (details.plusedUpAmount != details.taxToAdjustAmount)
          dc << setw(WIDTH) << "PLUS UP:" << setw(AMT_WIDTH)
             << details.taxToAdjustAmount - details.plusedUpAmount << taxItem->paymentCurrency()
             << "\n";
        if (details.bookletAdjustedAmount != details.plusedUpAmount)
          dc << setw(WIDTH) << "WITH BOOKLET:" << setw(AMT_WIDTH) << details.bookletAdjustedAmount
             << taxItem->paymentCurrency() << "\n";
        if (details.minTaxAdjustedAmount != details.bookletAdjustedAmount)
          dc << setw(WIDTH) << "MIN TAX:" << setw(AMT_WIDTH) << details.minTaxAdjustedAmount
             << taxItem->paymentCurrency() << "\n";
        if (details.adjustedTaxAmount != details.minTaxAdjustedAmount)
          dc << setw(WIDTH) << "MAX TAX:" << setw(AMT_WIDTH) << details.adjustedTaxAmount
             << taxItem->paymentCurrency() << "\n";
        else if (details.adjustedTaxAmount != 0)
          dc << setw(WIDTH) << "ADJUSTED TAX:" << setw(AMT_WIDTH) << details.adjustedTaxAmount
             << taxItem->paymentCurrency() << "\n";
        if (details.taxRangeAmount != details.adjustedTaxAmount)
          dc << setw(WIDTH) << "TAXRANGEAMT:" << setw(AMT_WIDTH) << details.taxRangeAmount
             << taxItem->paymentCurrency() << "\n";
        if (details.isSpecialRounded)
          dc << setw(WIDTH) << "SPECIAL ROUNDING"
             << "\n";
        else
        {
          dc << setw(WIDTH) << "ROUNDINGRULE:" << setw(AMT_WIDTH)
             << roundingToString(details.roundingUnit, details.roundingNoDec, details.roundingRule)
             << "\n";
        }
      }
      else
        dc << "SPECIAL TAX PROCESSING"
           << "\n";
      dc << setw(WIDTH) << "FINAL TAXAMOUNT:" << setw(AMT_WIDTH) << taxItem->taxAmount()
         << taxItem->paymentCurrency() << "\n";
    }
  } // End TaxResponse For Loop
}

/* public */
Diag817Collector&
Diag817Collector::operator <<(const TaxResponse& taxResponse)
{
  if (!_active)
    return *this;

  RexPricingPhase phase(_trx);
  if (phase.isExcPhase())
  {
    *this << "- BEGIN EXC ITIN DIAGNOSTIC -";
  }
  else if (phase.isNewPhase())
  {
    if (!fallback::ssdsp1508fix(_trx))
    {
      if (_rootDiag->diagParamMapItem(Diagnostic::ITIN_TYPE) == "ALL")
        *this << " \n";

      *this << "- BEGIN NEW ITIN DIAGNOSTIC -";
    }
    else
    {
      *this << " \n"
            << "- BEGIN NEW ITIN DIAGNOSTIC -";
    }
  }

  if (_rootDiag->diagParamMapItemPresent(ANCILLARY_TAXES_REQUESTED))
    buildDiag817Oc(taxResponse);
  else
  {
    if (_rootDiag->diagParamMapItemPresent(DETAILED_TAXES_REQUESTED))
      buildDiag817Dc(taxResponse);
    else
      buildDiag817(taxResponse);
  }
  return *this;
}

const std::string
Diag817Collector::roundingToString(const RoundingFactor& roundingFactor,
                                   const CurrencyNoDec& roundingFactorNoDec,
                                   const RoundingRule& roundingRule) const
{
  std::stringstream unit;

  unit << std::fixed << std::setprecision(roundingFactorNoDec) << roundingFactor;

  switch (roundingRule)
  {
  case DOWN:
    return "DOWN TO " + unit.str();
  case UP:
    return "UP TO " + unit.str();
  case NEAREST:
    return "NEAREST " + unit.str();
  case NONE:
    return "NONE";
  default:
    return "EMPTY";
  }
}

const std::string
Diag817Collector::roundingToString(const BSRCollectionResults& results, const uint32_t& roundingNo)
    const
{
  if (roundingNo == 1)
    return roundingToString(
        results.roundingFactor1(), results.roundingFactorNoDec1(), results.roundingRule1());
  else
    return roundingToString(
        results.roundingFactor2(), results.roundingFactorNoDec2(), results.roundingRule2());
}

void
Diag817Collector::printExchange(const BSRCollectionResults& results)
{
  DiagCollector& dc(*this);

  dc << setw(WIDTH) << "EXCHANGERATE:" << setw(AMT_WIDTH) << results.exchangeRate1() << "\n";
  bool roundingIntermediatePerformed =
      results.intermediateUnroundedAmount() != results.intermediateAmount();
  bool roundingTargetPerformed = results.targetUnroundedAmount() != results.convertedAmount();
  if (results.intermediateCurrency() != "")
  {
    if (roundingIntermediatePerformed)
    {
      dc << setw(WIDTH) << "UNROUNDED:" << setw(AMT_WIDTH) << results.intermediateUnroundedAmount()
         << results.intermediateCurrency() << "\n";
      dc << setw(WIDTH) << "ROUNDINGRULE:" << setw(AMT_WIDTH) << roundingToString(results, 1)
         << "\n";
    }
    dc << setw(WIDTH) << "INTERMEDIATE:" << setw(AMT_WIDTH) << results.intermediateAmount()
       << results.intermediateCurrency() << "\n";
    dc << setw(WIDTH) << "EXCHANGERATE:" << setw(AMT_WIDTH) << results.exchangeRate2() << "\n";
  }
  if (roundingTargetPerformed)
  {
    dc << setw(WIDTH) << "UNROUNDED:" << setw(AMT_WIDTH) << results.targetUnroundedAmount()
       << results.targetCurrency() << "\n";
    if (results.intermediateCurrency() != "")
      dc << setw(WIDTH) << "ROUNDINGRULE:" << setw(AMT_WIDTH) << roundingToString(results, 2)
         << "\n";
    else
      dc << setw(WIDTH) << "ROUNDINGRULE:" << setw(AMT_WIDTH) << roundingToString(results, 1)
         << "\n";
  }
}
}
