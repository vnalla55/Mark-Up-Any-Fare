//----------------------------------------------------------------------------
//
//  File:  PricingDetailResponseFormatter.cpp
//  Description: See PricingDetailResponseFormatter.h file
//  Created:  May 18, 2005
//  Authors:  Andrea Yang
//
//  Copyright Sabre 2005
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

#include "Xform/PricingDetailResponseFormatter.h"

#include "Common/ErrorResponseException.h"
#include "Common/FareCalcUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Message.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "Common/XMLConstruct.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/CurrencyDetail.h"
#include "DataModel/PaxDetail.h"
#include "DataModel/PlusUpDetail.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/SurchargeDetail.h"
#include "DataModel/TaxBreakdown.h"
#include "DataModel/TaxDetail.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareCalcConfig.h"
#include "FareCalc/FcDispItem.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"
#include "Xform/PricingResponseXMLTags.h"

#include <iomanip>
#include <string>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackWpdfEnhancedRuleDisplay);

static Logger
logger("atseintl.Xform.PricingDetailResponseFormatter");

void PricingDetailResponseFormatter::formatResponse(const ErrorResponseException& ere, std::string& response)
{
  XMLConstruct construct;
  construct.openElement("PricingDTLResponse");

  construct.openElement(xml2::MessageInformation);
  construct.addAttributeChar(xml2::MessageType, Message::TYPE_ERROR);
  construct.addAttributeShort(xml2::MessageFailCode, Message::errCode(ere.code()));
  construct.addAttribute(xml2::MessageText, ere.message());
  construct.closeElement();

  construct.closeElement();
  response = construct.getXMLData();
}

std::string
PricingDetailResponseFormatter::formatResponse(PricingDetailTrx& pricingDetailTrx,
                                               FareCalcConfig* fcConfigArg)
{
  LOG4CXX_DEBUG(logger, "In formatResponse");
  pricingDetailTrx.response().setf(std::ios::fixed, std::ios::floatfield);
  pricingDetailTrx.response().setf(std::ios::right, std::ios::adjustfield);

  bool canadianPointOfSale = isCanadianPointOfSale(pricingDetailTrx);

  const FareCalcConfig* const fcConfigPtr =
      (fcConfigArg == nullptr ? FareCalcUtil::getFareCalcConfig(pricingDetailTrx) : fcConfigArg);
  if (fcConfigPtr == nullptr)
  {
    pricingDetailTrx.response() << "UNABLE TO ACCESS FARE CALC CONFIGURATION" << std::endl;
    LOG4CXX_ERROR(logger, "Could not access fare calc configuration");
  }
  else if (pricingDetailTrx.response().str().empty()) // no error messages
  {
    const FareCalcConfig& fcConfig = *fcConfigPtr;
    if (pricingDetailTrx.ticketingAgent().vendorCrsCode() == AXESS_MULTIHOST_ID) // JAL/AXESS agent
    {
      pricingDetailTrx.response() << "VD " << std::endl;
    }
    bool isDetailedWQ = (pricingDetailTrx.billing()->actionCode().find("WQ") == 0);
    pricingDetailTrx.response() << (isDetailedWQ ? "WQ$DF" : "WPDF");
    if (pricingDetailTrx.selectionChoice() > 0)
    {
      pricingDetailTrx.response() << pricingDetailTrx.selectionChoice();
    }
    pricingDetailTrx.response() << std::endl;

    std::vector<PaxDetail*>::iterator paxIter = pricingDetailTrx.paxDetails().begin();
    std::vector<PaxDetail*>::iterator paxIterEnd = pricingDetailTrx.paxDetails().end();
    int paxTypeCount = 0;
    for (; paxIter != paxIterEnd && paxTypeCount < 4; paxIter++)
    {
      if (paxTypeCount > 0)
      {
        pricingDetailTrx.response() << std::endl;
      }
      addPassengerTypeLine(pricingDetailTrx, **paxIter);
      addFareLine(pricingDetailTrx, **paxIter, fcConfig);
      addTaxLine(pricingDetailTrx, **paxIter, fcConfig);
      addTotalLine(pricingDetailTrx, **paxIter, fcConfig);
      addFareCalcLine(pricingDetailTrx, **paxIter);
      addXTLines(pricingDetailTrx, **paxIter, fcConfig);
      addBSRLine(pricingDetailTrx, **paxIter);
      addTrafficDocumentIssuedInLine(pricingDetailTrx);
      addFareCalcHeaderLine(pricingDetailTrx, **paxIter, fcConfig);
      addSegmentDetails(pricingDetailTrx, **paxIter, fcConfig);
      addNonSpecificTotal(pricingDetailTrx, **paxIter);
      addPlusUps(pricingDetailTrx, **paxIter);
      addDifferentials(pricingDetailTrx, **paxIter);
      if (canadianPointOfSale)
        addConsolidatorPlusUp(pricingDetailTrx, **paxIter);
      addBreakPointTotal(pricingDetailTrx, **paxIter);
      if (!canadianPointOfSale)
        addConsolidatorPlusUp(pricingDetailTrx, **paxIter);
      addTaxHeaderLine(pricingDetailTrx, **paxIter);
      addTaxDetails(pricingDetailTrx, **paxIter);
      addIATARateInfo(pricingDetailTrx, **paxIter);
      addFareBSRInfo(pricingDetailTrx, **paxIter);
      addTaxBSRInfo(pricingDetailTrx, **paxIter);
      addPUTripType(pricingDetailTrx, **paxIter);

      ++paxTypeCount;
    }
  }

  // Attaching MSG elements
  std::string tmpResponse = pricingDetailTrx.response().str();
  LOG4CXX_DEBUG(logger,
                "PricingDetailResponseFormatter::formatResponse before XML:\n" << tmpResponse);

  unsigned int lastPos = 0;
  int recNum = 2;
  const int BUF_SIZE = 256;
  char tmpBuf[BUF_SIZE];

  XMLConstruct construct;
  construct.openElement("PricingDTLResponse");

  // Clobber the trailing newline
  while (1)
  {
    lastPos = tmpResponse.rfind("\n");
    if (lastPos > 0 && lastPos == (tmpResponse.length() - 1))
      tmpResponse.replace(lastPos, 1, "\0");
    else
      break;
  }
  char* pHolder = nullptr;
  int tokenLen = 0;
  for (char* token = strtok_r((char*)tmpResponse.c_str(), "\n", &pHolder); token != nullptr;
       token = strtok_r(nullptr, "\n", &pHolder), recNum++)
  {
    tokenLen = strlen(token);
    if (tokenLen == 0)
      continue;

    construct.openElement(xml2::MessageInformation);
    construct.addAttribute(xml2::MessageType, "X");

    sprintf(tmpBuf, "%06d", recNum + 1);
    construct.addAttribute(xml2::MessageFailCode, tmpBuf);
    construct.addAttribute(xml2::MessageText, token);
    construct.closeElement();
  }
  construct.closeElement();

  LOG4CXX_DEBUG(logger,
                "PricingDetailResponseFormatter::formatResponse XML output:\n"
                    << construct.getXMLData());
  LOG4CXX_DEBUG(logger, "Finished in formatResponse");

  return construct.getXMLData();
}

void
PricingDetailResponseFormatter::addPassengerTypeLine(PricingDetailTrx& pricingDetailTrx,
                                                     const PaxDetail& paxDetail)
{
  LOG4CXX_DEBUG(logger, "In addPassengerTypeLine");
  pricingDetailTrx.response() << "PSGR TYPE " << paxDetail.paxType() << std::endl;
  LOG4CXX_DEBUG(logger, "Finished in addPassengerTypeLine");
}

void
PricingDetailResponseFormatter::addFareLine(PricingDetailTrx& pricingDetailTrx,
                                            const PaxDetail& paxDetail,
                                            const FareCalcConfig& fcConfig)
{
  LOG4CXX_DEBUG(logger, "In addFareLine");

  DateTime& ticketingDate = pricingDetailTrx.ticketingDate();

  pricingDetailTrx.response() << "FARE  " << paxDetail.baseCurrencyCode();
  if (fcConfig.fareTaxTotalInd() == MIX_FARECALC3)
  {
    pricingDetailTrx.response() << " ";
  }

  pricingDetailTrx.response().precision(noCurrencyDec(paxDetail.baseCurrencyCode(), ticketingDate));

  const SellingFareData& sellingFare = paxDetail.sellingFare();
  if (!fallback::fixed::fallbackWpdfEnhancedRuleDisplay() 
    && !sellingFare.fareCalculation().empty())
    pricingDetailTrx.response() << std::setw(BASE_FARE_WIDTH) << sellingFare.baseFareAmount();
  else
    pricingDetailTrx.response() << std::setw(BASE_FARE_WIDTH) << paxDetail.baseFareAmount();

  if (!paxDetail.equivalentCurrencyCode().empty() &&
      paxDetail.baseCurrencyCode() != paxDetail.equivalentCurrencyCode())
  {
    int32_t amountWidth;
    if (fcConfig.fareTaxTotalInd() == VERTICAL_FARECALC2)
    {
      pricingDetailTrx.response() << std::endl << "EQUIV ";
      amountWidth = VERT_EQUIV_WIDTH;
    }
    else if (fcConfig.fareTaxTotalInd() == MIX_FARECALC3)
    {
      pricingDetailTrx.response() << " EQUIV ";
      amountWidth = MIXED_EQUIV_WIDTH;
    }
    else
    {
      pricingDetailTrx.response() << "  EQUIV  ";
      amountWidth = HORIZ_EQUIV_WIDTH;
    }
    pricingDetailTrx.response() << paxDetail.equivalentCurrencyCode();

    pricingDetailTrx.response().precision(
        noCurrencyDec(paxDetail.equivalentCurrencyCode(), ticketingDate));

  if (!fallback::fixed::fallbackWpdfEnhancedRuleDisplay() 
    && !sellingFare.fareCalculation().empty())
    pricingDetailTrx.response() << std::setw(amountWidth) << sellingFare.equivalentAmount();
  else
    pricingDetailTrx.response() << std::setw(amountWidth) << paxDetail.equivalentAmount();
  }
  pricingDetailTrx.response() << "\n";
  LOG4CXX_DEBUG(logger, "Finished in addFareLine");
}

void
PricingDetailResponseFormatter::addTaxLine(PricingDetailTrx& pricingDetailTrx,
                                           const PaxDetail& paxDetail,
                                           const FareCalcConfig& fcConfig)
{
  LOG4CXX_DEBUG(logger, "In addTaxLine");

  DateTime& ticketingDate = pricingDetailTrx.ticketingDate();

  bool bUseAslTax = false;
  if (!fallback::fixed::fallbackWpdfEnhancedRuleDisplay() 
    && !paxDetail.sellingFare().fareCalculation().empty())
    bUseAslTax = true;
  const std::vector<TaxDetail*>& taxDetails = (bUseAslTax) ? paxDetail.sellingFare().taxDetails() 
                                              : paxDetail.taxDetails();

  std::vector<TaxDetail*>::const_iterator taxIter = taxDetails.begin();
  std::vector<TaxDetail*>::const_iterator taxIterEnd = taxDetails.end();
  if (taxIter == taxIterEnd)
    return;

  CurrencyCode taxCur = paxDetail.equivalentCurrencyCode().empty()
                            ? paxDetail.baseCurrencyCode()
                            : paxDetail.equivalentCurrencyCode();

  LOG4CXX_INFO(logger, "WPDF FC Config Fare Tax Total Ind: " << fcConfig.fareTaxTotalInd());

  if (fcConfig.fareTaxTotalInd() == HORIZONTAL_FARECALC1)
  {
    // only do first tax for horizontal case
    pricingDetailTrx.response() << "TAX   ";
    if ((*taxIter)->publishedCurrencyCode().equalToConst("TE"))
    {
      pricingDetailTrx.response() << "TE\n";
    }
    else
    {
      uint16_t taxCount = 0;
      MoneyAmount taxSum(0.0);

      for (; taxIter != taxIterEnd; taxIter++)
      {
        taxSum += (*taxIter)->amount();
        ++taxCount;
      }
      taxIter = taxDetails.begin();
      pricingDetailTrx.response() << (*taxIter)->currencyCode();

      pricingDetailTrx.response().precision(
          noCurrencyDec((*taxIter)->currencyCode(), ticketingDate));

      pricingDetailTrx.response() << std::setw(HORIZ_TAX_WIDTH)
                                  << (pricingDetailTrx.segmentFeeApplied()
                                          ? paxDetail.totalTaxesPlusImposedSrg()
                                          : taxSum)
                                  << (taxCount > 1 ? "XT" : (*taxIter)->code().substr(0, 2))
                                  << "\n";
    }
  }
  else if (fcConfig.fareTaxTotalInd() == MIX_FARECALC3)
  {
    // mixed tax display
    const uint16_t maxMixedTaxes = 3;
    uint16_t taxCount = 0;
    MoneyAmount xtAmount = 0;

    pricingDetailTrx.response() << "TAX  ";
    taxIter = taxDetails.begin();
    for (; taxIter != taxIterEnd; taxIter++)
    {
      ++taxCount;
      if ((*taxIter)->publishedCurrencyCode().equalToConst("TE"))
      {

        if ((taxCount < maxMixedTaxes) || (taxDetails.size() == maxMixedTaxes))
        {
          if (fcConfig.taxExemptionInd() == '1')
          {
            pricingDetailTrx.response() << "              TE";
          }
          else
          {
            pricingDetailTrx.response() << " " << taxCur << " EXEMPT "
                                        << (*taxIter)->code().substr(0, 2);
          }
        }
      }
      else
      {
        taxCur = (*taxIter)->currencyCode();

        if ((taxCount < maxMixedTaxes) || (taxDetails.size() == maxMixedTaxes))
        {
          pricingDetailTrx.response() << " ";
          pricingDetailTrx.response() << (*taxIter)->currencyCode();

          pricingDetailTrx.response().precision(
              noCurrencyDec((*taxIter)->currencyCode(), ticketingDate));

          pricingDetailTrx.response() << std::setw(HORIZ_TAX_WIDTH) << (*taxIter)->amount()
                                      << (*taxIter)->code().substr(0, 2);
        }
        else
        {
          xtAmount += (*taxIter)->amount();
        }
      }
    }
    if (taxDetails.size() > maxMixedTaxes)
    {
      pricingDetailTrx.response() << " ";
      pricingDetailTrx.response() << taxCur;

      pricingDetailTrx.response().precision(noCurrencyDec(taxCur, ticketingDate));

      pricingDetailTrx.response() << std::setw(MIXED_TAX_WIDTH)
                                  << (pricingDetailTrx.segmentFeeApplied()
                                          ? paxDetail.totalTaxesPlusImposedSrg()
                                          : xtAmount) << "XT";
    }
    pricingDetailTrx.response() << std::endl;
  }
  else
  {
    // vertical tax display
    const uint16_t maxVerticalTaxes = 3;
    uint16_t taxCount = 0;
    MoneyAmount xtAmount = 0;

    taxIter = taxDetails.begin();
    for (; taxIter != taxIterEnd; taxIter++)
    {
      ++taxCount;
      if ((*taxIter)->publishedCurrencyCode().equalToConst("TE"))
      {
        if ((taxCount < maxVerticalTaxes) || (taxDetails.size() == maxVerticalTaxes))
        {
          if (fcConfig.taxExemptionInd() == '1')
          {
            pricingDetailTrx.response() << "TAX               TE\n";
          }
          else
          {
            pricingDetailTrx.response() << "TAX         EXEMPT " << (*taxIter)->code().substr(0, 2)
                                        << "\n";
          }
        }
      }
      else
      {
        taxCur = (*taxIter)->currencyCode();

        if ((taxCount < maxVerticalTaxes) || (taxDetails.size() == maxVerticalTaxes))
        {
          pricingDetailTrx.response() << "TAX   ";
          pricingDetailTrx.response() << (*taxIter)->currencyCode();

          pricingDetailTrx.response().precision(
              noCurrencyDec((*taxIter)->currencyCode(), ticketingDate));

          pricingDetailTrx.response() << std::setw(VERT_TAX_WIDTH) << (*taxIter)->amount()
                                      << (*taxIter)->code().substr(0, 2) << "\n";
        }
        else
        {
          xtAmount += (*taxIter)->amount();
        }
      }
    }
    if (taxDetails.size() > maxVerticalTaxes)
    {
      pricingDetailTrx.response() << "TAX   ";
      pricingDetailTrx.response() << taxCur;

      pricingDetailTrx.response().precision(noCurrencyDec(taxCur, ticketingDate));

      pricingDetailTrx.response() << std::setw(VERT_TAX_WIDTH)
                                  << (pricingDetailTrx.segmentFeeApplied()
                                          ? paxDetail.totalTaxesPlusImposedSrg()
                                          : xtAmount) << "XT"
                                  << "\n";
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in addTaxLine");
}

void
PricingDetailResponseFormatter::addTotalLine(PricingDetailTrx& pricingDetailTrx,
                                             const PaxDetail& paxDetail,
                                             const FareCalcConfig& fcConfig)
{
  LOG4CXX_DEBUG(logger, "In addTotalLine");

  // choose currency for total
  const CurrencyCode& totalCurrencyCode = paxDetail.equivalentCurrencyCode().empty()
                                              ? paxDetail.baseCurrencyCode()
                                              : paxDetail.equivalentCurrencyCode();

  pricingDetailTrx.response().precision(
      noCurrencyDec(totalCurrencyCode, pricingDetailTrx.ticketingDate()));

  int32_t totalWidth = 0;
  if (fcConfig.fareTaxTotalInd() == HORIZONTAL_FARECALC1)
    totalWidth = HORIZ_TOTAL_WIDTH;
  else if (fcConfig.fareTaxTotalInd() == VERTICAL_FARECALC2)
    totalWidth = VERT_TOTAL_WIDTH;
  else // mixed
    totalWidth = MIXED_TOTAL_WIDTH;

  bool bUseAslTotal = false;
  if (!fallback::fixed::fallbackWpdfEnhancedRuleDisplay() 
    && !paxDetail.sellingFare().fareCalculation().empty())
    bUseAslTotal = true;

  const MoneyAmount& totalPerPassenger = (bUseAslTotal) ? paxDetail.sellingFare().totalPerPassenger()
                                          : paxDetail.totalPerPassenger();

  pricingDetailTrx.response() << "TOTAL " << totalCurrencyCode << std::setw(totalWidth)
                              << (pricingDetailTrx.segmentFeeApplied()
                                      ? paxDetail.totalPerPaxPlusImposedSrg()
                                      : totalPerPassenger) << std::endl;
  LOG4CXX_DEBUG(logger, "Finished in addTotalLine");
}

void
PricingDetailResponseFormatter::addFareCalcLine(PricingDetailTrx& pricingDetailTrx,
                                                const PaxDetail& paxDetail)
{
  LOG4CXX_DEBUG(logger, "In addFareCalcLine");
  const char* cptr;

  if (!fallback::fixed::fallbackWpdfEnhancedRuleDisplay() 
    && !paxDetail.sellingFare().fareCalculation().empty())
    cptr = paxDetail.sellingFare().fareCalculation().c_str();
  else
    cptr = paxDetail.fareCalcLine().c_str();

  // skip leading white space
  while (*cptr && *cptr == ' ')
    cptr++;

  pricingDetailTrx.response() << cptr << "\n";
  LOG4CXX_DEBUG(logger, "Finished in addFareCalcLine");
}

void
PricingDetailResponseFormatter::addXTLines(PricingDetailTrx& pricingDetailTrx,
                                           const PaxDetail& paxDetail,
                                           const FareCalcConfig& fcConfig)
{
  LOG4CXX_DEBUG(logger, "In addXTLines");
  const int32_t XT_COLUMNS = 5;
  int32_t column = 1;
  bool findYZ = (pricingDetailTrx.segmentFeeApplied() ? true : false);
  // Only need the first passenger

  bool bUseAslTax = false;
  if (!fallback::fixed::fallbackWpdfEnhancedRuleDisplay() 
    && !paxDetail.sellingFare().fareCalculation().empty())
    bUseAslTax = true;
  const std::vector<TaxDetail*>& taxDetails = (bUseAslTax) ? paxDetail.sellingFare().taxDetails()
                                              : paxDetail.taxDetails();

  std::vector<TaxDetail*>::const_iterator taxIter = taxDetails.begin();
  std::vector<TaxDetail*>::const_iterator taxIterEnd = taxDetails.end();

  if (fcConfig.fareTaxTotalInd() != HORIZONTAL_FARECALC1)
  {
    if (taxDetails.size() <= 3)
      return;
    else
      taxIter += 2;
  }

  for (; taxIter != taxIterEnd; taxIter++)
  {
    if (column == 1)
    {
      pricingDetailTrx.response() << "XT";
    }
    pricingDetailTrx.response() << " ";
    if ((*taxIter)->publishedCurrencyCode().equalToConst("TE"))
    {
      // tax exempt
      if (fcConfig.taxExemptionInd() == '2')
      {
        pricingDetailTrx.response() << "EXEMPT ";
      }
      else
      {
        pricingDetailTrx.response() << "EX";
      }
      pricingDetailTrx.response() << (*taxIter)->code().substr(0, 2);
    }
    else
    {
      // normal tax

      // YZ tax first
      if (findYZ)
      {
        CurrencyCode curCode;
        std::vector<TaxBreakdown*>::const_iterator taxBdIter = paxDetail.taxBreakdowns().begin();
        std::vector<TaxBreakdown*>::const_iterator taxBdIterEnd = paxDetail.taxBreakdowns().end();
        MoneyAmount yzAmount = 0;
        for (; taxBdIter != taxBdIterEnd; ++taxBdIter)
        {
          if ((*taxBdIter)->code().substr(0, 2) == "YZ")
          {
            yzAmount += (*taxBdIter)->amount();
            curCode = (*taxBdIter)->currencyCode();
          }
        }
        if (fcConfig.taxCurCodeDisplayInd() == 'Y')
        {
          pricingDetailTrx.response() << curCode;
        }
        pricingDetailTrx.response().precision(
            noCurrencyDec(curCode, pricingDetailTrx.ticketingDate()));

        pricingDetailTrx.response() << yzAmount << "YZ ";

        findYZ = false;
      }

      if (fcConfig.taxCurCodeDisplayInd() == 'Y')
      {
        pricingDetailTrx.response() << (*taxIter)->currencyCode();
      }
      pricingDetailTrx.response().precision(
          noCurrencyDec((*taxIter)->currencyCode(), pricingDetailTrx.ticketingDate()));

      pricingDetailTrx.response() << (*taxIter)->amount() << (*taxIter)->code().substr(0, 2);

      if ((*taxIter)->code().equalToConst("XF"))
      {
        std::vector<TaxBreakdown*>::const_iterator taxBdIter = paxDetail.taxBreakdowns().begin();
        std::vector<TaxBreakdown*>::const_iterator taxBdIterEnd = paxDetail.taxBreakdowns().end();
        for (; taxBdIter != taxBdIterEnd; ++taxBdIter)
        {
          if ((*taxBdIter)->code().equalToConst("XF"))
          {
            pricingDetailTrx.response()
                << (*taxBdIter)->airportCode()
                << FcDispItem::convertAmount((*taxBdIter)->amountPublished(), 2, 0);
          }
        }
      }
    }

    ++column;
    if (column > XT_COLUMNS)
    {
      pricingDetailTrx.response() << "\n";
      column = 1;
    }
  }
  if (column != 1)
  {
    pricingDetailTrx.response() << "\n";
  }
  LOG4CXX_DEBUG(logger, "Finished in addXTLines");
}

void
PricingDetailResponseFormatter::extractExchangeRates(const CurrencyDetail& curDetail,
                                                     ExchRate& rateOne,
                                                     ExchRate& rateTwo)
{
  const std::string OVERRIDE_CONVERSION = "O";
  rateOne = curDetail.exchangeRateOne();
  rateTwo = curDetail.exchangeRateTwo();
  if (curDetail.conversionType() == OVERRIDE_CONVERSION)
  {
    rateOne = (curDetail.exchangeRateOne() == 0 ? 999 : 1 / curDetail.exchangeRateOne());
    rateTwo = (curDetail.exchangeRateTwo() == 0 ? 0 : 1 / curDetail.exchangeRateTwo());
  }
}

size_t
PricingDetailResponseFormatter::formatExchangeRate(const ExchRate& rate,
                                                   const CurrencyNoDec noDec,
                                                   const CurrencyCode& from,
                                                   const CurrencyCode& to,
                                                   char separator,
                                                   std::ostringstream& stream)
{
  std::string formattedRate;

  // noDec assumes rate < 10. Values >= 10 show too many decimals
  size_t rateLength = noDec + 2; // 1 for first digit and 1 for decimal point
  FareCalcUtil::doubleToStringTruncate(rate, formattedRate, noDec);
  stream << '1' << from << separator << formattedRate.substr(0, rateLength) << to;
  return 2 + from.size() + to.size() + rateLength;
}

size_t
PricingDetailResponseFormatter::formatBSR(const ExchRate& rate,
                                          const CurrencyNoDec noDec,
                                          const CurrencyCode& from,
                                          const CurrencyCode& to,
                                          char separator,
                                          std::ostringstream& stream)
{
  std::string formattedRate = FareCalcUtil::formatExchangeRate(rate, noDec);
  stream << '1' << from << separator << formattedRate.substr(0, formattedRate.size()) << to;
  return from.size() + to.size() + formattedRate.size();
}

void
PricingDetailResponseFormatter::addBSRLine(PricingDetailTrx& pricingDetailTrx,
                                           const PaxDetail& paxDetail)
{
  LOG4CXX_DEBUG(logger, "In addBSRLine");

  std::vector<CurrencyDetail*>::const_iterator curIter = paxDetail.fareBankerSellRates().begin();
  std::vector<CurrencyDetail*>::const_iterator curIterEnd = paxDetail.fareBankerSellRates().end();
  for (; curIter != curIterEnd; ++curIter)
  {
    const CurrencyDetail& curDetail = **curIter;
    ExchRate rateOne;
    ExchRate rateTwo;
    extractExchangeRates(curDetail, rateOne, rateTwo);

    pricingDetailTrx.response() << "RATE USED ";

    if (TrxUtil::isIcerActivated(pricingDetailTrx, pricingDetailTrx.ticketingDate()))
    {
      formatBSR(rateOne,
                curDetail.decimalPlacesExchangeRateOne(),
                curDetail.from(),
                curDetail.to(),
                '-',
                pricingDetailTrx.response());
    }
    else if (rateTwo > 0 && !curDetail.intermediateCurrency().empty())
    {
      formatExchangeRate(rateOne,
                         curDetail.decimalPlacesExchangeRateOne(),
                         curDetail.from(),
                         curDetail.intermediateCurrency(),
                         '-',
                         pricingDetailTrx.response());
      pricingDetailTrx.response() << ' ';
      formatExchangeRate(rateTwo,
                         curDetail.decimalPlacesExchangeRateTwo(),
                         curDetail.intermediateCurrency(),
                         curDetail.to(),
                         '-',
                         pricingDetailTrx.response());
    }
    else
    {
      formatExchangeRate(rateOne,
                         curDetail.decimalPlacesExchangeRateOne(),
                         curDetail.from(),
                         curDetail.to(),
                         '-',
                         pricingDetailTrx.response());
    }
    pricingDetailTrx.response() << ' ';
  }
  pricingDetailTrx.response() << std::endl;
  LOG4CXX_DEBUG(logger, "Finished in addBSRLine");
}

void
PricingDetailResponseFormatter::addTrafficDocumentIssuedInLine(PricingDetailTrx& pricingDetailTrx)
{
  LOG4CXX_DEBUG(logger, "In addTrafficDocumentIssuedInLine");
  pricingDetailTrx.response() << "/////////////// TRAFFIC DOCUMENT ISSUED IN "
                              << pricingDetailTrx.salesLocation() << " ///////////////\n";
  LOG4CXX_DEBUG(logger, "Finished in addTrafficDocumentIssuedInLine");
}

void
PricingDetailResponseFormatter::addFareCalcHeaderLine(PricingDetailTrx& pricingDetailTrx,
                                                      const PaxDetail& paxDetail,
                                                      const FareCalcConfig& fcConfig)
{
  LOG4CXX_DEBUG(logger, "In addFareCalcHeaderLine");
  pricingDetailTrx.response() << "PU -FARE BASIS-CUR  "
                              << (paxDetail.constructionCurrencyCode() == "NUC" ? "NUC" : "LSF")
                              << " AMT ELEMENT DESCRIPTION ----------- GI\n";
  LOG4CXX_DEBUG(logger, "Finished in addFareCalcHeaderLine");
}

void
PricingDetailResponseFormatter::addSegmentDetails(PricingDetailTrx& pricingDetailTrx,
                                                  const PaxDetail& paxDetail,
                                                  const FareCalcConfig& fcConfig)
{
  LOG4CXX_DEBUG(logger, "In addSegmentDetails");

  bool sideTrip = false;

  createFCMarketMap(paxDetail);

  std::vector<FareCalcDetail*>::const_iterator fcdIter = paxDetail.fareCalcDetails().begin();
  std::vector<FareCalcDetail*>::const_iterator fcdIterEnd = paxDetail.fareCalcDetails().end();
  for (; fcdIter != fcdIterEnd; ++fcdIter)
  {
    if ((*fcdIter)->isSideTripIncluded())
    {
      if (!sideTrip) // side trip has not been processed yet
      {
        _fCDstored = (*fcdIter);
        continue;
      }
      else if (_fCDstored)
      {
        analyzeFCwithSideTrip((*fcdIter));
      }
    }
    std::vector<SegmentDetail*>::iterator segIter = (*fcdIter)->segmentDetails().begin();
    std::vector<SegmentDetail*>::iterator segIterEnd = (*fcdIter)->segmentDetails().end();

    sideTrip = false;
    for (; segIter != segIterEnd; ++segIter)
    {

      addComponentPrefixInfo(**segIter, paxDetail.fareCalcLine(), pricingDetailTrx, paxDetail);
      addSurchargeInfo(**segIter, paxDetail, pricingDetailTrx, fcConfig);

      if ((*segIter)->isSideTripIndicator())
      {
        sideTrip = true;
      }
    }

    pricingDetailTrx.response() << std::setfill('0') << std::setw(2)
                                << (*fcdIter)->pricingUnitCount() << std::setfill(' ');
    addFareBreakPointInfo(**fcdIter, pricingDetailTrx, paxDetail, fcConfig, sideTrip);
  }
  LOG4CXX_DEBUG(logger, "Finished in addSegmentDetails");
}

void
PricingDetailResponseFormatter::addComponentPrefixInfo(const SegmentDetail& segmentDetail,
                                                       const std::string& calcLine,
                                                       PricingDetailTrx& pricingDetailTrx,
                                                       const PaxDetail& paxDetail)
{
  LOG4CXX_DEBUG(logger, "In addComponentPrefixInfo");

  std::ostringstream& response = pricingDetailTrx.response();
  DateTime& ticketingDate = pricingDetailTrx.ticketingDate();

  std::map<const SegmentDetail*, std::pair<size_t, size_t> >::iterator it =
      _marketPosInFCLine.find(&segmentDetail);

  // should always be true, but just to be sure
  if (it != _marketPosInFCLine.end())
  {
    size_t cityDeparturePos = it->second.second;
    size_t prevCityDeparurePos = it->second.first;
    size_t found;

    if (std::string::npos != (found = calcLine.find(" E/", prevCityDeparurePos)))
    {
      found += 1; // skip space
    }
    else if (std::string::npos != (found = calcLine.find("/E/", prevCityDeparurePos)))
    {
      found += 1; // skip "/"
    }

    if (found != std::string::npos && found < cityDeparturePos)
    {
      response << "                            " << calcLine.substr(found, 5)
               << "   TPM DEDUCTED-TRVL VIA " << segmentDetail.departureCity() << "\n";
    }

    if (std::string::npos != (found = calcLine.find(" T/", prevCityDeparurePos)))
    {
      found += 1; // skip space
    }
    else if (std::string::npos != (found = calcLine.find("/T/", prevCityDeparurePos)))
    {
      found += 1; // skip "/"
    }

    if (found != std::string::npos && found < cityDeparturePos)
    {
      response << "                            " << calcLine.substr(found, 5) << "   "
               << segmentDetail.departureCity() << " EXCLUDD FROM MILE CALC\n";
    }
  }

  if ((segmentDetail.stopover() == 'T') && segmentDetail.cityStopoverCharge() > 0.001)
  {
    pricingDetailTrx.response().precision(
        noCurrencyDec(paxDetail.constructionCurrencyCode(), ticketingDate));
    response << "               " << segmentDetail.stopoverPubCurrency() << std::setw(9)
             << segmentDetail.cityStopoverCharge() << " " << segmentDetail.arrivalCity()
             << "     STOPOVER SURCHARGE\n";
  }
  if ((segmentDetail.transfer() == 'T') && segmentDetail.transferCharge() > 0.001)
  {
    pricingDetailTrx.response().precision(
        noCurrencyDec(paxDetail.constructionCurrencyCode(), ticketingDate));
    response << "               " << segmentDetail.transferPubCurrency() << std::setw(9)
             << segmentDetail.transferCharge() << " " << segmentDetail.arrivalCity()
             << "     TRANSFER SURCHARGE\n";
  }
  LOG4CXX_DEBUG(logger, "Finished in addComponentPrefixInfo");
}

tse::CurrencyNoDec
PricingDetailResponseFormatter::noCurrencyDec(const tse::CurrencyCode& code,
                                              const DateTime& ticketingDate) const
{
  Money money(code);
  return money.noDec(ticketingDate);
}

void
PricingDetailResponseFormatter::addSurchargeInfo(const SegmentDetail& segmentDetail,
                                                 const PaxDetail& paxDetail,
                                                 PricingDetailTrx& pricingDetailTrx,
                                                 const FareCalcConfig& fcConfig)
{
  LOG4CXX_DEBUG(logger, "In addSurchargeInfo");
  std::vector<SurchargeDetail*>::const_iterator schgIter = segmentDetail.surchargeDetails().begin();
  std::vector<SurchargeDetail*>::const_iterator schgIterEnd =
      segmentDetail.surchargeDetails().end();
  std::string description;
  for (; schgIter != schgIterEnd; ++schgIter)
  {
    if ((*schgIter)->description() == "MISCELLANEOUS/OTHER")
    {
      description = "MISCELLANEOUS SURCHARGE";
    }
    else if ((*schgIter)->description() == "RESERVATION BOOKING DESIGNATOR (RBD)")
    {
      description = "RBD SURCHARGE";
    }
    else if ((*schgIter)->description().find("SURCH", 0) == std::string::npos)
    {
      description = (*schgIter)->description() + " SURCHARGE";
    }
    else
    {
      description = (*schgIter)->description();
    }
    pricingDetailTrx.response().precision(
        noCurrencyDec(paxDetail.constructionCurrencyCode(), pricingDetailTrx.ticketingDate()));

    if (segmentDetail.isSideTripIndicator())
    {
      pricingDetailTrx.response() << "              ";
      if (fcConfig.globalSidetripInd() == FC_ONE)
      {
        pricingDetailTrx.response() << PARENTH;
      }
      else if (fcConfig.globalSidetripInd() == FC_TWO)
      {
        pricingDetailTrx.response() << DISPLAY;
      }
      else if (fcConfig.globalSidetripInd() == FC_THREE)
      {
        pricingDetailTrx.response() << BRACKET;
      }
      else
      {
        pricingDetailTrx.response() << BLANK;
      }
    }
    else
      pricingDetailTrx.response() << "               ";

    LocCode origin, destination;
    if ((*schgIter)->destination().empty())
      destination = segmentDetail.arrivalCity();
    else
      destination = (*schgIter)->destination();
    if ((*schgIter)->origin().empty())
      origin = segmentDetail.departureCity();
    else
      origin = (*schgIter)->origin();
    pricingDetailTrx.response() << (*schgIter)->publishedCurrency() << std::setw(9)
                                << (*schgIter)->amount() << " " << origin << "-" << destination
                                << " " << description << std::endl;
  }
  LOG4CXX_DEBUG(logger, "Finished in addSurchargeInfo");
}

void
PricingDetailResponseFormatter::addFareBreakPointInfo(const FareCalcDetail& fareCalcDetail,
                                                      PricingDetailTrx& pricingDetailTrx,
                                                      const PaxDetail& paxDetail,
                                                      const FareCalcConfig& fcConfig,
                                                      const bool isSideTrip)
{
  LOG4CXX_DEBUG(logger, "In addFareBreakPointInfo");
  pricingDetailTrx.response().setf(std::ios::left, std::ios::adjustfield);
  pricingDetailTrx.response() << " " << std::setw(11)
                              << fareCalcDetail.fareBasisCode().substr(0, 11);
  if (isSideTrip)
  {
    if (fcConfig.globalSidetripInd() == FC_ONE)
    {
      pricingDetailTrx.response() << PARENTH;
    }
    else if (fcConfig.globalSidetripInd() == FC_TWO)
    {
      pricingDetailTrx.response() << DISPLAY;
    }
    else if (fcConfig.globalSidetripInd() == FC_THREE)
    {
      pricingDetailTrx.response() << BRACKET;
    }
    else
    {
      pricingDetailTrx.response() << BLANK;
    }
  }
  else
  {
    pricingDetailTrx.response() << " ";
  }
  pricingDetailTrx.response() << std::setw(4) << fareCalcDetail.fareComponentCurrencyCode();

  pricingDetailTrx.response().setf(std::ios::right, std::ios::adjustfield);
  pricingDetailTrx.response().precision(
      noCurrencyDec(paxDetail.constructionCurrencyCode(), pricingDetailTrx.ticketingDate()));
  pricingDetailTrx.response() << std::setw(8) << fareCalcDetail.fareAmount() << " ";
  if (fareCalcDetail.directionality() == "TO")
  {
    pricingDetailTrx.response() << fareCalcDetail.arrivalCity() << "-"
                                << fareCalcDetail.departureCity();
  }
  else
  {
    pricingDetailTrx.response() << fareCalcDetail.departureCity() << "-"
                                << fareCalcDetail.arrivalCity();
  }

  pricingDetailTrx.response() << " " << fareCalcDetail.trueGoverningCarrier() << " /"
                              << fareDescription(fareCalcDetail) << " ";

  if (fareCalcDetail.isRouting()) // is mileage routing
  {
    pricingDetailTrx.response() << std::setw(2) << fareCalcDetail.mileageSurchargePctg() << "M ";
  }
  else
  {
    pricingDetailTrx.response() << "RTG ";
  }
  if (!fareCalcDetail.hipOrigCity().empty())
  {
    pricingDetailTrx.response() << fareCalcDetail.hipOrigCity() << fareCalcDetail.hipDestCity();
  }
  else
  {
    pricingDetailTrx.response() << "      ";
  }
  if (!fareCalcDetail.constructedHipCity().empty())
  {
    pricingDetailTrx.response() << " C/" << fareCalcDetail.constructedHipCity() << " ";
  }
  else
  {
    pricingDetailTrx.response() << "       ";
  }
  pricingDetailTrx.response() << fareCalcDetail.globalIndicator() << std::endl;
  LOG4CXX_DEBUG(logger, "Finished in addFareBreakPointInfo");
}

void
PricingDetailResponseFormatter::addPlusUps(PricingDetailTrx& pricingDetailTrx,
                                           const PaxDetail& paxDetail)
{
  LOG4CXX_DEBUG(logger, "In addPlusUps");
  pricingDetailTrx.response().setf(std::ios::right, std::ios::adjustfield);
  pricingDetailTrx.response().precision(
      noCurrencyDec(paxDetail.constructionCurrencyCode(), pricingDetailTrx.ticketingDate()));

  std::vector<PlusUpDetail*>::const_iterator pupIter = paxDetail.plusUpDetails().begin();
  std::vector<PlusUpDetail*>::const_iterator pupIterEnd = paxDetail.plusUpDetails().end();
  for (; pupIter != pupIterEnd; ++pupIter)
  {
    const PlusUpDetail& pup = **pupIter;
    pricingDetailTrx.response() << "               ";

    const std::string& module = pup.message();

    pricingDetailTrx.response() << std::setw(3);
    if (module == "LCM")
    {
      pricingDetailTrx.response() << paxDetail.baseCurrencyCode().data();
    }
    else
    {
      pricingDetailTrx.response() << (pup.countryOfPayment().empty()
                                          ? paxDetail.baseCurrencyCode().data()
                                          : pup.countryOfPayment().data());
    }

    pricingDetailTrx.response() << " " << std::setw(8) << pup.amount() << " " << pup.origCity()
                                << "-" << pup.destCity() << " ";

    if (module == "DMC")
    {
      pricingDetailTrx.response() << "DMC-DIRECTIONAL MINIMUM";
    }
    else if (module == "COM")
    {
      pricingDetailTrx.response() << "COM-COUNTRY OF ORIGIN MIN";
    }
    else if (module == "CTM")
    {
      pricingDetailTrx.response() << "CTM-CIRCLE TRIP MINIMUM";
    }
    else if (module == "LCM")
    {
      pricingDetailTrx.response() << "COP-" << pup.countryOfPayment() << " CTRY OF PAYMENT MIN";
    }
    else if (module == "BHC")
    {
      pricingDetailTrx.response() << "BHC-OW BACKHAUL";
    }
    else if (module == "OJM")
    {
      pricingDetailTrx.response() << "HIGHEST RT OPEN JAW";
    }
    else if (module == "OSC")
    {
      pricingDetailTrx.response() << "OSC-OW SUBJOURNEY CHECK";
    }
    else if (module == "CPM")
    {
      pricingDetailTrx.response() << "CPM-COMMON POINT MINIMUM";
    }
    else if (module == "HRTC")
    {
      pricingDetailTrx.response() << "HIGHEST RT CHECK";
    }
    else
    {
      pricingDetailTrx.response() << module;
    }

    pricingDetailTrx.response() << std::endl;
    if (!pup.viaCity().empty())
    {
      pricingDetailTrx.response() << "                                    " << pup.origCity() << "-"
                                  << pup.destCity() << " CONSTRUCTED VIA " << pup.viaCity()
                                  << std::endl;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in addPlusUps");
}

void
PricingDetailResponseFormatter::addDifferentials(PricingDetailTrx& pricingDetailTrx,
                                                 const PaxDetail& paxDetail)
{
  pricingDetailTrx.response().precision(
      noCurrencyDec(paxDetail.constructionCurrencyCode(), pricingDetailTrx.ticketingDate()));
  LOG4CXX_DEBUG(logger, "In addDifferentials");
  std::vector<FareCalcDetail*>::const_iterator fcdIter = paxDetail.fareCalcDetails().begin();
  std::vector<FareCalcDetail*>::const_iterator fcdIterEnd = paxDetail.fareCalcDetails().end();
  for (; fcdIter != fcdIterEnd; fcdIter++)
  {
    if (*fcdIter == nullptr)
      continue;

    //        const FareCalcDetail& fareCalcDetail = **fcdIter;
    std::vector<DifferentialDetail*>::const_iterator diffIter =
        (*fcdIter)->differentialDetails().begin();
    std::vector<DifferentialDetail*>::const_iterator diffIterEnd =
        (*fcdIter)->differentialDetails().end();
    for (; diffIter != diffIterEnd; diffIter++)
    {
      if (*diffIter == nullptr)
        continue;
      const DifferentialDetail& diffDetail = **diffIter;
      pricingDetailTrx.response().setf(std::ios::left, std::ios::adjustfield);
      pricingDetailTrx.response() << "   " << std::setw(8) << diffDetail.fareClassHigh() << "    "
                                  << std::setw(3) << paxDetail.baseCurrencyCode();
      pricingDetailTrx.response().setf(std::ios::right, std::ios::adjustfield);
      pricingDetailTrx.response() << " " << std::setw(8) << diffDetail.amount() << " "
                                  << std::setw(3) << diffDetail.origCityHIP() << "-"
                                  << diffDetail.destCityHIP() << " DIFF " << std::setw(2)
                                  << diffDetail.mileagePctg() << "M H:"
                                  << (diffDetail.highOrigHIP().empty() ? diffDetail.origCityHIP()
                                                                       : diffDetail.highOrigHIP())
                                  << (diffDetail.highDestHIP().empty() ? diffDetail.destCityHIP()
                                                                       : diffDetail.highDestHIP())
                                  << "/" << diffDetail.fareClassHigh() << std::endl;
      pricingDetailTrx.response() << "                                             L:"
                                  << (diffDetail.lowOrigHIP().empty() ? diffDetail.origCityHIP()
                                                                      : diffDetail.lowOrigHIP())
                                  << (diffDetail.lowDestHIP().empty() ? diffDetail.destCityHIP()
                                                                      : diffDetail.lowDestHIP())
                                  << "/" << diffDetail.fareClassLow() << std::endl;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in addPlusUps");
}

void
PricingDetailResponseFormatter::addConsolidatorPlusUp(PricingDetailTrx& pricingDetailTrx,
                                                      const PaxDetail& paxDetail)
{
  LOG4CXX_DEBUG(logger, "In addConsolidatorPlusUp");
  if (Money::isZeroAmount(pricingDetailTrx.consolidatorPlusUpFareCalcAmount()))
    return;

  pricingDetailTrx.response().precision(
      noCurrencyDec(paxDetail.constructionCurrencyCode(), pricingDetailTrx.ticketingDate()));

  pricingDetailTrx.response() << "               "
                              << pricingDetailTrx.consolidatorPlusUpCurrencyCode() << " "
                              << std::setw(8) << pricingDetailTrx.consolidatorPlusUpFareCalcAmount()
                              << " PLUS UP" << std::endl;

  LOG4CXX_DEBUG(logger, "Finished in addConsolidatorPlusUp");
}

void
PricingDetailResponseFormatter::addBreakPointTotal(PricingDetailTrx& pricingDetailTrx,
                                                   const PaxDetail& paxDetail)
{
  LOG4CXX_DEBUG(logger, "In addBreakPointTotal");
  pricingDetailTrx.response().precision(
      noCurrencyDec(paxDetail.constructionCurrencyCode(), pricingDetailTrx.ticketingDate()));
  pricingDetailTrx.response() << "                   " << std::setw(8)
                              << paxDetail.constructionTotalAmount() << "    -    TOTAL "
                              << paxDetail.constructionCurrencyCode() << std::endl;
  LOG4CXX_DEBUG(logger, "Finished in addBreakPointTotal");
}

void
PricingDetailResponseFormatter::addTaxHeaderLine(PricingDetailTrx& pricingDetailTrx,
                                                 const PaxDetail& paxDetail)
{
  LOG4CXX_DEBUG(logger, "In addTaxHeaderLine");

  bool bUseAslTax = false;
  if (!fallback::fixed::fallbackWpdfEnhancedRuleDisplay() 
    && !paxDetail.sellingFare().fareCalculation().empty())
    bUseAslTax = true;
  const std::vector<TaxDetail*>& taxDetails = (bUseAslTax) ? paxDetail.sellingFare().taxDetails()
                                              : paxDetail.taxDetails();

  std::vector<TaxDetail*>::const_iterator taxIter = taxDetails.begin();
  std::vector<TaxDetail*>::const_iterator taxIterEnd = taxDetails.end();
  for (; taxIter != taxIterEnd; taxIter++)
  {
    if ((*taxIter)->publishedCurrencyCode() != "TE" && (*taxIter)->publishedCurrencyCode() != "OV")
    {
      pricingDetailTrx.response() << "-TAX AMT/" << (paxDetail.equivalentCurrencyCode().empty()
                                                         ? paxDetail.baseCurrencyCode()
                                                         : paxDetail.equivalentCurrencyCode())
                                  << "/-- CUR  CODES  DESCRIPTION------------------------\n";
      break;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in addTaxHeaderLine");
}

bool
PricingDetailResponseFormatter::taxExempt(PricingDetailTrx& pricingDetailTrx, const std::string& code, const PaxDetail& paxDetail)
{
  bool bUseAslTax = false;
  if (!fallback::fixed::fallbackWpdfEnhancedRuleDisplay() 
    && !paxDetail.sellingFare().fareCalculation().empty())
    bUseAslTax = true;
  const std::vector<TaxDetail*>& taxDetails = (bUseAslTax) ? paxDetail.sellingFare().taxDetails() 
                                              : paxDetail.taxDetails();

  std::vector<TaxDetail*>::const_iterator taxIter = taxDetails.begin();
  std::vector<TaxDetail*>::const_iterator taxIterEnd = taxDetails.end();
  for (; taxIter != taxIterEnd; taxIter++)
  {
    if ((*taxIter)->publishedCurrencyCode().equalToConst("TE") &&
        (*taxIter)->code().substr(0, 2) == code.substr(0, 2))
    {
      return true;
    }
  }
  return false;
}

void
PricingDetailResponseFormatter::addTaxDetails(PricingDetailTrx& pricingDetailTrx,
                                              const PaxDetail& paxDetail)
{
  LOG4CXX_DEBUG(logger, "In addTaxDetails");

  std::ostringstream& response = pricingDetailTrx.response();
  DateTime& ticketingDate = pricingDetailTrx.ticketingDate();

  std::map<std::string, std::vector<TaxBreakdown*> > taxBdMap;

  TaxBreakdownVecIC taxBdIter, lTaxBdIter, taxBdIterEnd, lTaxBdIterEnd;

  taxBdIter = paxDetail.taxBreakdowns().begin();
  taxBdIterEnd = paxDetail.taxBreakdowns().end();

  for (; taxBdIter != taxBdIterEnd; taxBdIter++)
  {
    if ((*taxBdIter)->publishedCurrencyCode() != "OV")
    {
      taxBdMap[(*taxBdIter)->code()].push_back(*taxBdIter);
    }
  }

  std::map<std::string, std::vector<TaxBreakdown*> >::iterator vecIterEnd;
  std::map<std::string, std::vector<TaxBreakdown*> >::const_iterator vecCIter;

  for (taxBdIter = paxDetail.taxBreakdowns().begin(); taxBdIter != taxBdIterEnd; taxBdIter++)
  {
    if (taxExempt(pricingDetailTrx, (*taxBdIter)->code(), paxDetail))
      continue;

    vecIterEnd = taxBdMap.end();
    vecCIter = taxBdMap.find((*taxBdIter)->code());
    if (vecCIter == vecIterEnd)
      continue; // this tax code already processed or not exists

    const std::vector<TaxBreakdown*>& tbdVec = vecCIter->second;
    const TaxBreakdown& firstTaxBreakdown = **tbdVec.begin();

    // calculate sum
    MoneyAmount sum = 0;
    lTaxBdIter = tbdVec.begin();
    lTaxBdIterEnd = tbdVec.end();
    for (; lTaxBdIter != lTaxBdIterEnd; lTaxBdIter++)
    {
      sum += (*lTaxBdIter)->amount();
    }

    // output summary line
    CurrencyCode currencyOfPayment = firstTaxBreakdown.currencyCode().empty()
                                         ? pricingDetailTrx.totalCurrencyCode()
                                         : firstTaxBreakdown.currencyCode();
    response.precision(noCurrencyDec(currencyOfPayment, ticketingDate));
    response.setf(std::ios::right, std::ios::adjustfield);
    response << " " << std::setw(11) << sum << "        " << firstTaxBreakdown.countryCode() << "/";

    response.setf(std::ios::left, std::ios::adjustfield);
    response << std::setw(3) << (*taxBdIter)->code() << " "
             << firstTaxBreakdown.description().substr(0, 36) << std::endl;

    lTaxBdIter = tbdVec.begin();
    lTaxBdIterEnd = tbdVec.end();

    // output breakdown lines

    for (; lTaxBdIter != lTaxBdIterEnd; lTaxBdIter++)
    {
      const TaxBreakdown& taxBreakdown = **lTaxBdIter;
      currencyOfPayment = taxBreakdown.currencyCode().empty() ? pricingDetailTrx.totalCurrencyCode()
                                                            : taxBreakdown.currencyCode();
      if (currencyOfPayment == "TBD" || taxBreakdown.publishedCurrencyCode() == "TBD")
      {
        LOG4CXX_WARN(logger, "Encountered TBD currency for tax " << (*taxBdIter)->code());
        continue;
      }

      response.setf(std::ios::right, std::ios::adjustfield);
      response.precision(noCurrencyDec(currencyOfPayment, ticketingDate));
      response << std::setw(15) << taxBreakdown.amount() << ":";
      if (taxBreakdown.type() == 'P')
      {
        response << taxBreakdown.currencyCode();
      }
      else
      {
        response << taxBreakdown.publishedCurrencyCode();
      }
      response.precision(noCurrencyDec(taxBreakdown.publishedCurrencyCode(), ticketingDate));
      response << std::setw(9) << taxBreakdown.amountPublished();

      if (!taxBreakdown.airportCode().empty())
      {
        response << " *" << taxBreakdown.airportCode() << "* ";
      }
      else
      {
        response << "       ";
      }

      if (!taxBreakdown.airlineCode().empty() && taxBreakdown.code().substr(0, 2) != "AY" &&
          taxBreakdown.code().substr(0, 2) != "ZP" && taxBreakdown.code().substr(0, 2) != "XF")
      {
        response << "*" << taxBreakdown.airlineCode() << "*\n";
      }
      else
      {
        response << "    \n";
      }
    } // lTaxBdIter

    taxBdMap.erase((*taxBdIter)->code()); // remove processed tax from the map
  } // taxBdIter

  LOG4CXX_DEBUG(logger, "Finished in addTaxDetails");
}

void
PricingDetailResponseFormatter::addIATARateInfo(PricingDetailTrx& pricingDetailTrx,
                                                const PaxDetail& paxDetail)
{
  LOG4CXX_DEBUG(logger, "In addIATARateInfo");
  if (paxDetail.fareIATARates().empty())
  {
    return;
  }
  pricingDetailTrx.response()
      << "-IATA RATES OF EXCHANGE USED IN FARE CALCULATION --------------\n";

  std::vector<CurrencyDetail*>::const_iterator curIter = paxDetail.fareIATARates().begin();
  std::vector<CurrencyDetail*>::const_iterator curIterEnd = paxDetail.fareIATARates().end();
  pricingDetailTrx.response().setf(std::ios::right, std::ios::adjustfield);

  std::string formattedRate;
  for (; curIter != curIterEnd; ++curIter)
  {
    const CurrencyDetail& curDetail = **curIter;
    FareCalcUtil::doubleToStringTruncate(
        curDetail.exchangeRateOne(), formattedRate, curDetail.decimalPlacesExchangeRateOne());
    pricingDetailTrx.response() << " " << std::setw(12) << formattedRate << "  " << curDetail.from()
                                << "-ROE  EFF ";
    if (!curDetail.effectiveDate().empty())
    {
      pricingDetailTrx.response() << curDetail.effectiveDate() << "*"
                                  << (!curDetail.discontinueDate().empty()
                                          ? curDetail.discontinueDate()
                                          : "INDEF");
    }
    pricingDetailTrx.response() << std::endl;
  }
  LOG4CXX_DEBUG(logger, "Finished in addIATARateInfo");
}

void
PricingDetailResponseFormatter::addFareBSRInfo(PricingDetailTrx& pricingDetailTrx,
                                               const PaxDetail& paxDetail)
{
  LOG4CXX_DEBUG(logger, "In addFareBSRInfo");
  if (paxDetail.fareBankerSellRates().empty() || paxDetail.equivalentCurrencyCode().empty() ||
      paxDetail.baseCurrencyCode() == paxDetail.equivalentCurrencyCode())
  {
    return;
  }
  pricingDetailTrx.response()
      << "-BSR USED IN FARE CALCULATION ---------------------------------\n ";

  int column = 1;
  std::vector<CurrencyDetail*>::const_iterator curIter = paxDetail.fareBankerSellRates().begin();
  std::vector<CurrencyDetail*>::const_iterator curIterEnd = paxDetail.fareBankerSellRates().end();
  for (; curIter != curIterEnd; ++curIter)
  {
    addBSR(pricingDetailTrx, pricingDetailTrx.response(), **curIter, column);
  }

  if (column == 2)
  {
    pricingDetailTrx.response() << std::endl;
  }

  LOG4CXX_DEBUG(logger, "Finished in addFareBSRInfo");
}

void
PricingDetailResponseFormatter::addTaxBSRInfo(PricingDetailTrx& pricingDetailTrx,
                                              const PaxDetail& paxDetail)
{
  LOG4CXX_DEBUG(logger, "In addTaxBSRInfo");
  if (paxDetail.taxBankerSellRates().empty())
  {
    return;
  }
  pricingDetailTrx.response()
      << "-BSR USED IN TAX CALCULATION ----------------------------------\n ";

  int column = 1;
  std::vector<CurrencyDetail*>::const_iterator curIter = paxDetail.taxBankerSellRates().begin();
  std::vector<CurrencyDetail*>::const_iterator curIterEnd = paxDetail.taxBankerSellRates().end();
  for (; curIter != curIterEnd; ++curIter)
  {
    addBSR(pricingDetailTrx, pricingDetailTrx.response(), **curIter, column);
  }

  if (column == 2)
  {
    pricingDetailTrx.response() << std::endl;
  }

  LOG4CXX_DEBUG(logger, "Finished in addTaxBSRInfo");
}

void
PricingDetailResponseFormatter::formatExchangeRateColumns(int& column,
                                                          size_t fieldSize,
                                                          std::ostringstream& response)
{
  const int columnWidth = 32;
  int numSpaces = columnWidth - fieldSize;

  if (column == 1)
  {
    for (int i = 0; i < numSpaces; ++i)
    {
      response << ' ';
    }
    column = 2;
  }
  else
  {
    response << std::endl << ' ';
    column = 1;
  }
}

void
PricingDetailResponseFormatter::addBSR(PricingDetailTrx& pricingDetailTrx,
                                       std::ostringstream& response,
                                       const CurrencyDetail& curDetail,
                                       int& column)
{
  LOG4CXX_DEBUG(logger, "In addBSR");

  ExchRate rateOne;
  ExchRate rateTwo;
  extractExchangeRates(curDetail, rateOne, rateTwo);
  size_t fieldSize;

  if (TrxUtil::isIcerActivated(pricingDetailTrx, pricingDetailTrx.ticketingDate()))
  {
    fieldSize = formatBSR(rateOne,
                          curDetail.decimalPlacesExchangeRateOne(),
                          curDetail.from(),
                          curDetail.to(),
                          ':',
                          response);
  }
  else if (rateTwo > 0.0 && !curDetail.intermediateCurrency().empty())
  {
    fieldSize = formatExchangeRate(rateOne,
                                   curDetail.decimalPlacesExchangeRateOne(),
                                   curDetail.from(),
                                   curDetail.intermediateCurrency(),
                                   ':',
                                   response);

    formatExchangeRateColumns(column, fieldSize, response);

    fieldSize = formatExchangeRate(rateTwo,
                                   curDetail.decimalPlacesExchangeRateTwo(),
                                   curDetail.intermediateCurrency(),
                                   curDetail.to(),
                                   ':',
                                   response);
  }
  else
  {
    fieldSize = formatExchangeRate(rateOne,
                                   curDetail.decimalPlacesExchangeRateOne(),
                                   curDetail.from(),
                                   curDetail.to(),
                                   ':',
                                   response);
  }

  formatExchangeRateColumns(column, fieldSize, response);

  LOG4CXX_DEBUG(logger, "Finished in addBSR");
}

namespace
{
struct _sortPus
{
  bool operator()(const FareCalcDetail* fcd1, const FareCalcDetail* fcd2)
  {
    return (!fcd1) || (!fcd2) || (fcd1->pricingUnitCount() < fcd2->pricingUnitCount());
  }
} sortPus;
};

//--------------------------------------------------------------------------
// PricingDetailResponseFormatter::addPUTripType
//--------------------------------------------------------------------------
void
PricingDetailResponseFormatter::addPUTripType(PricingDetailTrx& pricingDetailTrx,
                                              const PaxDetail& paxDetail)
{
  LOG4CXX_DEBUG(logger, "In addPUTripType");

  std::ostringstream& response = pricingDetailTrx.response();
  response << "-PRICING UNIT TRIP TYPE ---------------------------------------" << std::endl;

  uint16_t puCount = 0;

  std::vector<FareCalcDetail*> sortedFCDetails(paxDetail.fareCalcDetails().begin(),
                                               paxDetail.fareCalcDetails().end());
  std::sort(sortedFCDetails.begin(), sortedFCDetails.end(), sortPus);

  std::vector<FareCalcDetail*>::const_iterator fcdIter = sortedFCDetails.begin();
  std::vector<FareCalcDetail*>::const_iterator fcdIterEnd = sortedFCDetails.end();

  for (; fcdIter != fcdIterEnd; ++fcdIter)
  {
    const FareCalcDetail& fcDetail = **fcdIter;
    if (fcDetail.pricingUnitCount() > puCount)
    {
      puCount = fcDetail.pricingUnitCount();
      response << "  PU " << std::setfill('0') << std::setw(2) << puCount << std::setfill(' ')
               << " - " << (fcDetail.normalOrSpecialFare() == 'T' ? "SPECIAL " : "NORMAL  ");
      if (fcDetail.pricingUnitType().empty())
      {
        LOG4CXX_WARN(logger, "Empty pricing unit type for PU#" << puCount);
        response << "UNKNOWN" << std::endl;
        continue;
      }
      switch (fcDetail.pricingUnitType().c_str()[0])
      {
      case 'J':
        response << "OPEN JAW";
        break;
      case 'R':
        response << "ROUND TRIP";
        break;
      case 'C':
        response << "CIRCLE TRIP";
        break;
      case 'O':
        response << "ONE WAY";
        break;
      case 'W':
        response << "RW SINGLE FARE COMPONENT";
        break;
      case 'T':
        response << "CT SINGLE FARE COMPONENT";
        break;
      default:
        response << "UNKNOWN";
        break;
      }
      response << std::endl;
    }
  }
  LOG4CXX_DEBUG(logger, "Finished in addPUTripType");
}

//--------------------------------------------------------------------------
// @function PricingDetailResponseFormatter::addNonSpecificTotal
//
// Description: Add Non Specific Stopover/Transfer surcharges
//
// @param pricingDetailTrx - a valid Pricing Detail Trx
// @param paxDetail - passenger type information
//--------------------------------------------------------------------------
void
PricingDetailResponseFormatter::addNonSpecificTotal(PricingDetailTrx& pricingDetailTrx,
                                                    const PaxDetail& paxDetail)
{
  LOG4CXX_DEBUG(logger, "In addNonSpecificTotal");

  DateTime& ticketingDate = pricingDetailTrx.ticketingDate();

  if (paxDetail.stopoverCharges() > 0)
  {
    pricingDetailTrx.response().precision(
        noCurrencyDec(paxDetail.constructionCurrencyCode(), ticketingDate));
    pricingDetailTrx.response() << "               " << paxDetail.stopOverPubCurrency()
                                << std::setw(9) << paxDetail.stopoverCharges() << "   "
                                << paxDetail.stopoverCount() << "-S   STOPOVER SURCHARGE"
                                << std::endl;
  }
  if (paxDetail.transferCharges() > 0)
  {
    pricingDetailTrx.response().precision(
        noCurrencyDec(paxDetail.constructionCurrencyCode(), ticketingDate));
    pricingDetailTrx.response() << "               " << paxDetail.transferPubCurrency()
                                << std::setw(9) << paxDetail.transferCharges() << "   "
                                << paxDetail.transferCount() << "-S   TRANSFER SURCHARGE"
                                << std::endl;
  }
  LOG4CXX_DEBUG(logger, "Finished in addNonSpecificTotal");

  return;
}

//--------------------------------------------------------------------------
// @function PricingDetailResponseFormatter::analyzeFCwithSideTrip
//
// Description: Analyze Fare Component with the side-trip in it.
//              The FC could be split in two FareCalcDetail. One partial FCD
//              before FareCalcDetail with the side-trip, and another FCD after
//              side-trip. Need to combine them in one FCD for display purpose.
//
// @param fareCalcDetail - current Fare Calc Detail pointer
//
//--------------------------------------------------------------------------
void
PricingDetailResponseFormatter::analyzeFCwithSideTrip(FareCalcDetail* fcd)
{
  if ((fcd->departureCity() == _fCDstored->departureCity()) &&
      (fcd->departureAirport() == _fCDstored->departureAirport()) &&
      (fcd->governingCarrier() == _fCDstored->governingCarrier()) &&
      (fcd->baseCurrencyCode() == _fCDstored->baseCurrencyCode()) &&
      (fcd->arrivalCity() == _fCDstored->arrivalCity()) &&
      (fcd->arrivalAirport() == _fCDstored->arrivalAirport()) &&
      (fcd->fareAmount() == _fCDstored->fareAmount()) &&
      (fcd->fareBasisCode() == _fCDstored->fareBasisCode()) &&
      (fcd->cabinCode() == _fCDstored->cabinCode()) &&
      (fcd->arrivalCountry() == _fCDstored->arrivalCountry()) &&
      (fcd->roundTripFare() == _fCDstored->roundTripFare()) &&
      (fcd->oneWayFare() == _fCDstored->oneWayFare()) &&
      (fcd->typeOfFare() == _fCDstored->typeOfFare()) &&
      (fcd->pricingUnitCount() == _fCDstored->pricingUnitCount()) &&
      (fcd->pricingUnitType() == _fCDstored->pricingUnitType()) &&
      (fcd->globalIndicator() == _fCDstored->globalIndicator()) &&
      (fcd->directionality() == _fCDstored->directionality()) &&
      (fcd->discountCode() == _fCDstored->discountCode()) &&
      (fcd->publishedFareAmount() == _fCDstored->publishedFareAmount()) &&
      (fcd->normalOrSpecialFare() == _fCDstored->normalOrSpecialFare()) &&
      (fcd->mileageSurchargePctg() == _fCDstored->mileageSurchargePctg()) &&
      (fcd->fareComponentCurrencyCode() == _fCDstored->fareComponentCurrencyCode()) &&
      (fcd->segmentDetails().size() != fcd->segmentsCount() &&
       _fCDstored->segmentDetails().size() != _fCDstored->segmentsCount()))
  {
    std::vector<SegmentDetail*>::iterator segIter = _fCDstored->segmentDetails().begin();
    std::vector<SegmentDetail*>::iterator segIterEnd = _fCDstored->segmentDetails().end();

    for (; segIter != segIterEnd; ++segIter)
    {
      fcd->segmentDetails().push_back((*segIter));
    }
    _fCDstored = nullptr;
  }
}

//--------------------------------------------------------------------------
// @function PricingDetailResponseFormatter::isCanadianPointOfSale
//
// Description: Determine whether the agent is located in Canada or not
//
// @param pricingDetailTrx - a valid Pricing Detail Trx
// @return bool true if agent is in Canada
//
//--------------------------------------------------------------------------
bool
PricingDetailResponseFormatter::isCanadianPointOfSale(PricingDetailTrx& pricingDetailTrx)
{
  const Loc* pointOfSaleLocation;

  if (pricingDetailTrx.salesLocation().empty())
  {
    pointOfSaleLocation = pricingDetailTrx.ticketingAgent().agentLocation();
  }
  else
  {
    pointOfSaleLocation = pricingDetailTrx.dataHandle().getLoc(pricingDetailTrx.salesLocation(),
                                                               pricingDetailTrx.ticketingDate());
  }

  return (pointOfSaleLocation && LocUtil::isCanada(*pointOfSaleLocation));
}

void
PricingDetailResponseFormatter::createFCMarketMap(const PaxDetail& paxDetail)
{
  size_t found = 0;
  size_t prevFound = 0;
  // segments are in order from itinerary
  std::vector<FareCalcDetail*>::const_iterator fcdIter = paxDetail.fareCalcDetails().begin();
  std::vector<FareCalcDetail*>::const_iterator fcdIterEnd = paxDetail.fareCalcDetails().end();
  for (; fcdIter != fcdIterEnd; ++fcdIter)
  {
    std::vector<SegmentDetail*>::iterator segIter = (*fcdIter)->segmentDetails().begin();
    std::vector<SegmentDetail*>::iterator segIterEnd = (*fcdIter)->segmentDetails().end();
    for (; segIter != segIterEnd; ++segIter)
    {
      found = paxDetail.fareCalcLine().find((*segIter)->departureCity(), found);
      _marketPosInFCLine.insert(std::pair<const SegmentDetail*, std::pair<size_t, size_t> >(
          *segIter, std::pair<size_t, size_t>(prevFound, found)));
      prevFound = found;
    }
  }
}

std::string
PricingDetailResponseFormatter::fareDescription(const FareCalcDetail& fareCalcDetail) const
{
  std::string description;
  if (fareCalcDetail.pricingUnitType() == "W" || fareCalcDetail.pricingUnitType() == "T")
  {
    description = "RT";
  }
  else
  {
    description = (fareCalcDetail.isOneWayFare() ? "OW" : "HR");
  }

  return description;
}

} // namespace tse
