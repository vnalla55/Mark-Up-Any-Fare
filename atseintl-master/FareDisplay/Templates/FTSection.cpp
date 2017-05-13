//-------------------------------------------------------------------
//
//  File:        FTSection.cpp
//  Description: This class abstracts a display template.  It maintains
//               all the data and methods necessary to describe
//               and realize the Tax  Display that
//               appear on the Sabre greenscreen.)
//
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/FTSection.h"

#include "Common/BSRCollectionResults.h"
#include "Common/BSRCurrencyConverter.h"
#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/FareDisplaySurcharge.h"
#include "Common/FareDisplayTax.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "FareDisplay/Templates/ElementField.h"
#include "FareDisplay/Templates/ElementFilter.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"

#include <sstream>
#include <string>

namespace tse
{
namespace
{
static const Indicator TRUE_CHAR = 'T';
static const std::string OW = " OW";
static const std::string RT = " RT";
static const std::string HR = " HR";
static const std::string FARE = "FARE";
static const std::string FT_TAX = "TAX";
static const std::string FT_SUP = "SUP";
static const std::string FT_SURCHG = "SURCHG";
static const std::string FEE = "FEE";
static const std::string TOTAL = "TOTAL";
static const std::string NO_TAX_APPLIES = "*NO TAX APPLIES*";
static const std::string NO_TAX_DISPLAY_WITH_NUC_CURRENCY = "*NO TAX DISPLAY WITH NUC CURRENCY*";
static const std::string NO_TAX_FOR_HR_FARES = "*NO TAX FOR HR FARES*";
}

static Logger
logger("atseintl.FareDisplay.Template.FTSection");

void
FTSection::buildDisplay()
{
  _trx.response().clear();

  // Make sure we have some fares
  if (_trx.allPaxTypeFare().empty())
  {
    LOG4CXX_ERROR(logger, "buildDisplay:: allPaxTypeFare is empty. Going back ");
    return;
  }

  PaxTypeFare& paxTypeFare = *_trx.allPaxTypeFare().front();
  FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();
  if (fareDisplayInfo == nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "buildDisplay:: No FareDisplayInfo object in PaxTypeFare "
                      << paxTypeFare.fareClass());
    return;
  }

  // ------------------------------------
  // Display surcharge info line
  //-------------------------------------
  if (_trx.getOptions()->applySurcharges() == YES &&
      _trx.getOptions()->halfRoundTripFare() != TRUE_CHAR)
  {
    _trx.response() << SURCHARGE_INFO << std::endl;
    _trx.response() << SPACE << std::endl;
  }

  // ------------------------------------
  // Display 2nd Line: OW    FARE
  // -------------------------------------
  if (_trx.itin().empty())
  {
    LOG4CXX_ERROR(logger, "buildDisplay:: Trx::itin is empty.");
    return;
  }

  std::string trip = " OW";
  if (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    trip = " RT";

  // Get totalfare and appy percentage
  const MoneyAmount fareAmount = paxTypeFare.nucFareAmount();

  // -----------------------------
  // Display: "*NO TAX FOR HR FARES*"
  // -----------------------------
  if (_trx.getOptions()->halfRoundTripFare() == TRUE_CHAR)
  {
    addNoTaxAppliesLine(paxTypeFare, HR, fareAmount / 2, fareAmount / 2, NO_TAX_FOR_HR_FARES);
    return;
  }

  // ----------------------------------------
  // Display: "*NO TAX DISPLAY WITH NUC CURRENCY*"
  // ----------------------------------------
  Itin* itin = _trx.itin().front();
  _displayCurrency = itin->calculationCurrency();
  if (_displayCurrency == NUC)
  {
    displayMsgForNUC(paxTypeFare, fareAmount, paxTypeFare.owrt());
    return;
  }

  // ------------------------------
  // Populate two vector of FTTax
  // ------------------------------
  std::vector<FTTax*> ftTaxVecOW;
  std::vector<FTTax*> ftTaxVecRT;

  populateSurchargesInTaxVec(paxTypeFare, ftTaxVecOW, ftTaxVecRT);
  populateTaxVec(paxTypeFare, ftTaxVecOW, ftTaxVecRT);

  LOG4CXX_DEBUG(logger,
                "TaxItems Seperated: OW= " << ftTaxVecOW.size() << " ,RT= " << ftTaxVecRT.size());

  // ----------------------------------
  // If both vectors are empty no tax applies. Go back
  // ----------------------------------
  if (ftTaxVecOW.empty() && ftTaxVecRT.empty())
  {
    displayMsgForNOTAXApplies(paxTypeFare, fareAmount, paxTypeFare.owrt());
    return;
  }

  Indicator owJourneyType = _trx.getOptions()->oneWayFare();
  Indicator rtJourneyType = _trx.getOptions()->roundTripFare();
  switch (paxTypeFare.owrt())
  {
  // ------------------------
  // Both OW and RT Tax
  // ------------------------
  case ONE_WAY_MAY_BE_DOUBLED:
  {
    if (ftTaxVecOW.empty() == true)
      addNoTaxAppliesLine(paxTypeFare, OW, fareAmount, fareAmount, NO_TAX_APPLIES);
    else
      addTaxLine(paxTypeFare, OW, fareAmount, getTotalAmount(ftTaxVecOW), ftTaxVecOW);

    _trx.response() << SPACE << std::endl;

    const MoneyAmount actualFareAmount = fareAmount * 2;
    if (ftTaxVecRT.empty() == true)
      addNoTaxAppliesLine(paxTypeFare, RT, actualFareAmount, actualFareAmount, NO_TAX_APPLIES);
    else
      addTaxLine(paxTypeFare, RT, actualFareAmount, getTotalAmount(ftTaxVecRT), ftTaxVecRT);
    break;
  }

  // -------------------
  // RT Fare
  // -------------------
  case ROUND_TRIP_MAYNOT_BE_HALVED:
  {
    if (ftTaxVecRT.empty() == true || owJourneyType == TRUE_CHAR)
      addNoTaxAppliesLine(paxTypeFare, RT, fareAmount, fareAmount, NO_TAX_APPLIES);
    else
      addTaxLine(paxTypeFare, RT, fareAmount, getTotalAmount(ftTaxVecRT), ftTaxVecRT);
    break;
  }

  // -------------------
  // OW Fare
  // -------------------
  case ONE_WAY_MAYNOT_BE_DOUBLED:
  {
    if (ftTaxVecOW.empty() == true || rtJourneyType == TRUE_CHAR)
      addNoTaxAppliesLine(paxTypeFare, OW, fareAmount, fareAmount, NO_TAX_APPLIES);
    else
      addTaxLine(paxTypeFare, OW, fareAmount, getTotalAmount(ftTaxVecOW), ftTaxVecOW);
    break;
  }
  }

  if (isSupplementFeeTrailer)
  {
    // Assign text
    static std::string fdHeaderMsg =
        TrxUtil::supplementHdrMsgFareDisplayData(_trx); // Trailer message read from config

    if (fdHeaderMsg.empty())
      return;

    replace(fdHeaderMsg.begin(), fdHeaderMsg.end(), '_', ' ');

    _trx.response() << fdHeaderMsg << std::endl;
  }
}

void
FTSection::addToStream(std::ostringstream& oss, int16_t size, const std::string& str)
{
  oss << std::setfill(' ') << std::setw(size);
  oss.setf(std::ios::right, std::ios::adjustfield);
  oss << str;
}

void
FTSection::addNoTaxAppliesLine(PaxTypeFare& ptFare,
                               const std::string& trip,
                               const MoneyAmount fare,
                               const MoneyAmount total,
                               const std::string& str)
{
  std::ostringstream& oss = _trx.response();
  addToStream(oss, 3, trip);
  addToStream(oss, 9, FARE);
  addToStream(oss, 4 + str.size(), str);
  addToStream(oss, 59 - (16 + str.size()), TOTAL);
  oss << std::endl;

  addToStream(oss, 12, formatMoneyAmount(fare));
  addToStream(oss, 47, formatMoneyAmount(total));

  // CurrencyCode displayCurrency ;
  // FareDisplayUtil::getDisplayCurrency ( *_trx, displayCurrency ) ;
  addToStream(oss, 4, _displayCurrency);
  oss << std::endl;
}

bool
FTSection::populateTaxVec(const PaxTypeFare& paxTypeFare,
                          std::vector<FTTax*>& ftTaxVecOW,
                          std::vector<FTTax*>& ftTaxVecRT)
{
  const FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();

  if (!fareDisplayInfo)
    return false;

  Indicator owrt = paxTypeFare.owrt();

  const std::vector<TaxRecord*>& taxRecordVector =
      (owrt == ONE_WAY_MAYNOT_BE_DOUBLED) ? fareDisplayInfo->taxRecordVector()
                                          : fareDisplayInfo->taxRecordOWRTFareTaxVector();

  // if ( taxRecordVector.empty() && taxRecordVectorOWRT.empty() )
  //	return false ;

  // ---------------------------------
  // get OW Tax Amount and fill it up in FT Vector
  // ---------------------------------
  for (const auto taxRecord : taxRecordVector)
  {
    LOG4CXX_DEBUG(logger, "populateTaxVec: Getting OW tax of " << taxRecord->taxCode());
    if (FareDisplayTax::shouldBypassTax(taxRecord->taxCode()))
      continue;

    MoneyAmount owTaxAmount = 0;
    if (!FareDisplayTax::getOWTax(_trx,
                                  paxTypeFare,
                                  taxRecord->taxCode(),
                                  owTaxAmount,
                                  (owrt == ROUND_TRIP_MAYNOT_BE_HALVED)))
      continue;

    const std::vector<TaxItem*>& taxItemVector = (owrt == ONE_WAY_MAYNOT_BE_DOUBLED)
                                                     ? fareDisplayInfo->taxItemVector()
                                                     : fareDisplayInfo->taxItemOWRTFareTaxVector();

    const Indicator feeInd = getFeeInd(taxItemVector, taxRecord->taxCode());
    populateFTTax((owrt == ROUND_TRIP_MAYNOT_BE_HALVED ? ftTaxVecRT : ftTaxVecOW),
                  *taxRecord,
                  feeInd,
                  owTaxAmount);
  }

  // ---------------------------------
  // get RT Tax Amount and fill it up in FT Vector
  // ---------------------------------
  if (owrt == ONE_WAY_MAYNOT_BE_DOUBLED)
    return true;

  for (const auto taxRecordOWRT : fareDisplayInfo->taxRecordVector())
  {
    LOG4CXX_DEBUG(logger, "populateTaxVec: Getting RT tax of " << taxRecordOWRT->taxCode());
    if (FareDisplayTax::shouldBypassTax(taxRecordOWRT->taxCode()))
      continue;

    std::vector<FTTax*>::const_iterator it = ftTaxVecRT.begin();
    for (; it != ftTaxVecRT.end(); it++)
    {
      if ((*it)->taxCode() == taxRecordOWRT->taxCode())
        break;
    }

    MoneyAmount rtTaxAmount = 0;
    if (!FareDisplayTax::getRTTax(_trx, paxTypeFare, taxRecordOWRT->taxCode(), rtTaxAmount))
      continue;
    const Indicator feeInd = getFeeInd(fareDisplayInfo->taxItemVector(), taxRecordOWRT->taxCode());
    if (it == ftTaxVecRT.end())
    {
      // item wasn't previously found in taxRecordOWRTVector
      populateFTTax(ftTaxVecRT, *taxRecordOWRT, feeInd, rtTaxAmount);
    }
    else
    {
      // same OW tax found before in taxRecordOWRTVector - but
      // RT tax takes precedence over OW for
      // ROUNDTRIP; just replace amount & fee indicator
      (*it)->taxAmount() = rtTaxAmount;
      (*it)->feeInd() = ((feeInd == 'Y') ? true : false);
    }
  }

  return true;
}

bool
FTSection::populateFTTax(std::vector<FTTax*>& ftTaxVector,
                         const TaxRecord& taxRecord,
                         const Indicator feeInd,
                         MoneyAmount taxAmount)
{
  FTTax* ftTax(nullptr);

  _trx.dataHandle().get(ftTax);
  ftTax->taxCode() = taxRecord.taxCode();
  ftTax->taxAmount() = taxAmount;

  ftTax->feeInd() = ((feeInd == 'Y') ? true : false);
  ftTax->surchgInd() = false;

  ftTaxVector.push_back(ftTax);
  return true;
}

bool
FTSection::populateSurchargesInTaxVec(const PaxTypeFare& paxTypeFare,
                                      std::vector<FTTax*>& ftTaxVecOW,
                                      std::vector<FTTax*>& ftTaxVecRT)
{
  MoneyAmount owSurcharge(0), rtSurcharge(0);

  if (_trx.getOptions()->applySurcharges() == NO)
    return true;

  FareDisplaySurcharge::getTotalOWSurcharge(_trx, paxTypeFare, owSurcharge);
  FareDisplaySurcharge::getTotalRTSurcharge(_trx, paxTypeFare, rtSurcharge);

  // Add one way surcharge to OW vec
  if (owSurcharge > 0)
  {
    FTTax* owTax(nullptr);
    _trx.dataHandle().get(owTax);
    owTax->surchgInd() = true;
    owTax->feeInd() = false;
    owTax->taxAmount() = owSurcharge;

    ftTaxVecOW.push_back(owTax);
  }

  // Add round trip surcharge to RT vec
  if (rtSurcharge > 0)
  {
    FTTax* rtTax(nullptr);
    _trx.dataHandle().get(rtTax);
    rtTax->surchgInd() = true;
    rtTax->feeInd() = false;
    rtTax->taxAmount() = rtSurcharge;
    ftTaxVecRT.push_back(rtTax);
  }
  return true;
}

MoneyAmount
FTSection::getTotalAmount(const std::vector<FTTax*>& ftTaxVec)
{
  return std::accumulate(ftTaxVec.cbegin(),
                         ftTaxVec.cend(),
                         static_cast<MoneyAmount>(0),
                         [](const MoneyAmount& init, const FTTax* const tax) -> MoneyAmount
                         { return init + tax->taxAmount(); });
}

void
FTSection::addTaxLine(PaxTypeFare& ptFare,
                      const std::string& trip,
                      const MoneyAmount fareAmount,
                      const MoneyAmount totalTaxAmount,
                      const std::vector<FTTax*> ftTaxVec)
{

  if (ftTaxVec.empty())
    return;

  bool firstLine = true;
  bool sendToResponse = false;
  bool isNewLine = false;
  bool isTotalWritten = false;
  int16_t count = 0;

  std::ostringstream line1;
  std::ostringstream line2;

  // Add " OW" or " RT"
  addToStream(line1, 3, trip);
  addToStream(line2, 3, EMPTY_STRING());

  for (FTTax* const ftTax : ftTaxVec)
  {
    sendToResponse = false;

    // -------------------
    // Write FARE
    // --------------------
    if (firstLine)
    {
      addToStream(line1, 9, FARE);
      addToStream(line2, 9, formatMoneyAmount(fareAmount));
      firstLine = false;
    }
    else if (isNewLine)
    {
      isNewLine = false;

      addToStream(line1, 12, EMPTY_STRING());
      addToStream(line2, 12, EMPTY_STRING());
    }

    std::string freeOrTax = ((ftTax->feeInd()) ? FEE : FT_TAX);

    TaxCode taxCodeStr = ftTax->taxCode();

    if (taxCodeStr.substr(0, 2) == "YZ" && isdigit(taxCodeStr[2]))
    {
      freeOrTax = FT_SUP;
      isSupplementFeeTrailer = true;
    }

    if (ftTax->surchgInd() == true)
    {
      addToStream(line1, 9, FT_SURCHG);
    }
    else if (ftTax->taxCode().length() == 2)
    {
      addToStream(line1, 9, ftTax->taxCode() + " " + freeOrTax);
    }
    else
    {
      addToStream(line1, 9, ftTax->taxCode() + freeOrTax);
    }
    addToStream(line2, 9, formatMoneyAmount(ftTax->taxAmount()));

    // Count = 3 mean we are writing 4 the tax
    if (++count < 4)
      continue;

    isNewLine = true;
    sendToResponse = true;

    // Fillup with space - if required
    int16_t spaceToFillup = (4 - count) * 9;
    if (spaceToFillup)
    {
      addToStream(line1, spaceToFillup, EMPTY_STRING());
      addToStream(line2, spaceToFillup, EMPTY_STRING());
    }

    if (!isTotalWritten)
    {
      isTotalWritten = true;

      // Write Total Line
      addToStream(line1, 11, TOTAL);
      addToStream(line2, 11, formatMoneyAmount(fareAmount + totalTaxAmount));

      // Write Currency
      addToStream(line1, 4, EMPTY_STRING());
      addToStream(line2, 4, _displayCurrency);
    }
    else
    {
      // Write 15 blank space instead of TOTAL anc CURRENCY
      addToStream(line1, 15, EMPTY_STRING());
      addToStream(line2, 15, EMPTY_STRING());
    }

    // Reset the count.
    count = 0;

    line1 << std::endl;
    line2 << std::endl;

    LOG4CXX_DEBUG(logger, "\"" << line1.str() << "\" Size: " << line1.str().length());
    LOG4CXX_DEBUG(logger, "\"" << line2.str() << "\" Size: " << line2.str().length());

    _trx.response() << line1.str();
    _trx.response() << line2.str();

    line1.str(EMPTY_STRING());
    line2.str(EMPTY_STRING());
  }

  if (!sendToResponse)
  {

    // Did we write TOTAL and CURRENCY ?
    if (!isTotalWritten)
    {
      // No. Write it.
      isTotalWritten = true;

      // Fillup with space - if required
      int16_t spaceToFillup = (4 - count) * 9;
      if (spaceToFillup)
      {
        addToStream(line1, spaceToFillup, EMPTY_STRING());
        addToStream(line2, spaceToFillup, EMPTY_STRING());
      }

      // Write Total
      addToStream(line1, 11, TOTAL);
      addToStream(line2, 11, formatMoneyAmount(fareAmount + totalTaxAmount));

      // Write Currency
      addToStream(line1, 4, EMPTY_STRING());
      addToStream(line2, 4, _displayCurrency);
    }

    line1 << std::endl;
    line2 << std::endl;

    LOG4CXX_DEBUG(logger, "\"" << line1.str() << "\" Size: " << line1.str().length());
    LOG4CXX_DEBUG(logger, "\"" << line2.str() << "\" Size: " << line2.str().length());

    _trx.response() << line1.str();
    _trx.response() << line2.str();
  }
}

// ----------------------
// formatMoneyAmount
// This method  only formats the amount
// ----------------------
std::string
FTSection::formatMoneyAmount(const MoneyAmount amount)
{
  ElementField field;
  ElementFilter::currencyCode(field, _trx);
  field.moneyValue() = amount;
  ElementFilter::formatMoneyAmount(field, _trx);
  return std::string(field.strValue());
}

// ------------------------
// displayMsgForNUC
// ------------------------
void
FTSection::displayMsgForNUC(PaxTypeFare& ptFare,
                            const MoneyAmount fareAmount,
                            const Indicator& owRT)
{
  if (owRT == ONE_WAY_MAY_BE_DOUBLED || owRT == ONE_WAY_MAYNOT_BE_DOUBLED)
    addNoTaxAppliesLine(ptFare, OW, fareAmount, fareAmount, NO_TAX_DISPLAY_WITH_NUC_CURRENCY);

  if (owRT == ONE_WAY_MAY_BE_DOUBLED)
    _trx.response() << SPACE << std::endl;

  if (owRT == ONE_WAY_MAY_BE_DOUBLED)
    addNoTaxAppliesLine(
        ptFare, RT, fareAmount * 2, fareAmount * 2, NO_TAX_DISPLAY_WITH_NUC_CURRENCY);
  else if (owRT == ROUND_TRIP_MAYNOT_BE_HALVED)
    addNoTaxAppliesLine(ptFare, RT, fareAmount, fareAmount, NO_TAX_DISPLAY_WITH_NUC_CURRENCY);
}

// ------------------------
// displayMsgForNOTAX
// ------------------------
void
FTSection::displayMsgForNOTAXApplies(PaxTypeFare& ptFare,
                                     const MoneyAmount fareAmount,
                                     const Indicator& owRT)
{
  if (owRT == ONE_WAY_MAY_BE_DOUBLED || owRT == ONE_WAY_MAYNOT_BE_DOUBLED)
    addNoTaxAppliesLine(ptFare, OW, fareAmount, fareAmount, NO_TAX_APPLIES);

  if (owRT == ONE_WAY_MAY_BE_DOUBLED)
    _trx.response() << SPACE << std::endl;

  if (owRT == ONE_WAY_MAY_BE_DOUBLED)
    addNoTaxAppliesLine(ptFare, RT, fareAmount * 2, fareAmount * 2, NO_TAX_APPLIES);
  else if (owRT == ROUND_TRIP_MAYNOT_BE_HALVED)
    addNoTaxAppliesLine(ptFare, RT, fareAmount, fareAmount, NO_TAX_APPLIES);
}

Indicator
FTSection::getFeeInd(const std::vector<TaxItem*>& taxItemVector, const TaxCode& taxCode)
{
  Indicator ind = 'Y';
  for (const auto taxItem : taxItemVector)
  {
    if (taxItem->taxCode() == taxCode)
    {
      ind = taxItem->feeInd();
      break;
    }
  }

  return ind;
}
} // tse namespace
