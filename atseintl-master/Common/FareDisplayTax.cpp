//----------------------------------------------------------------------------
//
//  File:            FareDisplayTax.cpp
//  Created:     12/21/2005
//  Authors:     Partha Kumar Chakraborti
//
//  Description:    Common functions required for Tax Calculation of ATSE fare display
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
#include "Common/FareDisplayTax.h"

#include "Common/ErrorResponseException.h"
#include "Common/FareDisplayUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/TaxCodeReg.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"

namespace tse
{
static Logger
logger("atseintl.Common.FareDisplayTax");

namespace FareDisplayTax
{
const std::string
TAX_CODE_US1("US1");
const std::string
TAX_CODE_US2("US2");
const std::string
TAX_CODE_YZ1("YZ1");
const std::string
TAX_CODE_UO2("UO2");
const std::string
TAX_CODE_NZ1("NZ1");

// -----------------------------------------
// getTaxCodeForFQ:
// Fare displayed in FQ/RDentry includes some specific taxes.
// This method returns TaxCode which need to be added with FQ entry.
// -----------------------------------------
void
getTaxCodeForFQ(const FareDisplayTrx& trx,
                std::set<TaxCode>& taxCode,
                const GlobalDirection globalDirection,
                const FareMarket* fareMarket)
{

  if (trx.getRequest()->requestType() == FARE_TAX_REQUEST)
    return;

  const bool isRoundTheWorld =
      trx.isSameCityPairRqst() &&
      (globalDirection == GlobalDirection::RW || globalDirection == GlobalDirection::CT);

  // Add YZ1 for all customers.  Tax Service will check Tax Nation table to process YZ1 tax.
  taxCode.insert(TAX_CODE_YZ1);

  // Only add US1, US2 or YZ1 for USA, UO2 for Australian tax  and  NZ1  for NewZealand tax  with
  // FQ.
  if (LocUtil::isUS(*trx.getRequest()->ticketingAgent()->agentLocation()))
  {
    //		taxCode.insert(TAX_CODE_YZ1) ;
    // Add US1 only if the faremarket origin is in US
    if (fareMarket && (!LocUtil::isUS(*fareMarket->origin()) ||
                       LocUtil::isUSTerritoryOnly(*fareMarket->origin())))
      return;
    taxCode.insert(TAX_CODE_US1);
    // Add US2 only if the faremarket origin and destination are in US including Hawaii and Alaska
    // and if the fare is not RoundTheWorld
    if (LocUtil::isUS(*fareMarket->destination()) &&
        !LocUtil::isUSTerritoryOnly(*fareMarket->origin()) &&
        !LocUtil::isUSTerritoryOnly(*fareMarket->destination()) && !isRoundTheWorld)
      taxCode.insert(TAX_CODE_US2);
  }
  else if (trx.getRequest()->ticketingAgent()->agentLocation()->nation() == AUSTRALIA &&
           !isRoundTheWorld)
    taxCode.insert(TAX_CODE_UO2);
  else if (trx.getRequest()->ticketingAgent()->agentLocation()->nation() == NEW_ZEALAND &&
           !isRoundTheWorld)
    taxCode.insert(TAX_CODE_NZ1);
}

static void
getTotalTax(const FareDisplayTrx& trx,
            const PaxTypeFare& paxTypeFare,
            const std::set<TaxCode>& taxCode,
            MoneyAmount& owTaxAmount,
            MoneyAmount& rtTaxAmount)
{
  for (const TaxCode& tc : taxCode)
  {
    MoneyAmount tempOWAmount = 0;
    MoneyAmount tempRTAmount = 0;
    getOWTax(trx, paxTypeFare, tc, tempOWAmount);
    getRTTax(trx, paxTypeFare, tc, tempRTAmount);
    owTaxAmount += tempOWAmount;
    rtTaxAmount += tempRTAmount;
  }
}

//-----------------------------------------------------------------------------
// getTotalTax
// This method is not for FT entry. Normally FQ and RD uses this method. FQ / RD entries add some
// specific
// taxes with base fare. This method gets taxCode to be added and collects tax based on it.
//-----------------------------------------------------------------------------
void
getTotalTax(const FareDisplayTrx& trx,
            const PaxTypeFare& paxTypeFare,
            MoneyAmount& owTaxAmount,
            MoneyAmount& rtTaxAmount)
{
  owTaxAmount = 0;
  rtTaxAmount = 0;

  if (trx.getRequest()->requestType() == FARE_TAX_REQUEST)
    return;

  std::set<TaxCode> taxCode;
  getTaxCodeForFQ(trx, taxCode, paxTypeFare.globalDirection(), paxTypeFare.fareMarket());
  if (taxCode.empty() == true)
    return;

  getTotalTax(trx, paxTypeFare, taxCode, owTaxAmount, rtTaxAmount);
}

bool
getOWTax(const FareDisplayTrx& trx,
         const PaxTypeFare& paxTypeFare,
         const TaxCode& taxCode,
         MoneyAmount& owTaxAmount,
         bool overrideFareOWRTCheck)
{
  owTaxAmount = 0;
  if (taxCode.empty() == true)
    return false;

  const FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();

  if (!fareDisplayInfo)
    return false;

  const Indicator owrt = paxTypeFare.owrt();

  if (!overrideFareOWRTCheck)
    if (owrt != ONE_WAY_MAY_BE_DOUBLED && owrt != ONE_WAY_MAYNOT_BE_DOUBLED)
      return false;

  // Take reference of tax Vectors in new meaningfull names in order to avoid confusion
  const std::vector<TaxItem*>& taxItemVector = (owrt == ONE_WAY_MAYNOT_BE_DOUBLED)
                                                   ? fareDisplayInfo->taxItemVector()
                                                   : fareDisplayInfo->taxItemOWRTFareTaxVector();

  if (taxItemVector.empty())
  {
    LOG4CXX_DEBUG(logger, "Leaving getOWTax: taxItemVector is empty");
    return false;
  }

  // -------------------------------------
  // Get OW tax amount
  // -------------------------------------
  for (const auto taxItem : taxItemVector)
  {
    if (taxCode != taxItem->taxCode())
      continue;

    LOG4CXX_DEBUG(logger,
                  "getOWTax " << taxItem->taxCode() << "  ,taxType: " << taxItem->taxType() << ", "
                              << taxItem->taxAmount());

    owTaxAmount += taxItem->taxAmount();
  }

  LOG4CXX_DEBUG(logger, "Leaving getOWTax = " << owTaxAmount);

  return true;
}

bool
getRTTax(const FareDisplayTrx& trx,
         const PaxTypeFare& paxTypeFare,
         const TaxCode& taxCode,
         MoneyAmount& rtTaxAmount)
{
  rtTaxAmount = 0;
  if (taxCode.empty() == true)
    return false;

  const FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();

  if (!fareDisplayInfo)
    return false;

  const Indicator owrt = paxTypeFare.owrt();

  if (owrt != ROUND_TRIP_MAYNOT_BE_HALVED && owrt != ONE_WAY_MAY_BE_DOUBLED)
    return false;

  // Take reference of tax Vectors in new meaningfull names in order to avoid confusion
  const std::vector<TaxItem*>& taxItemVectorOWRT = fareDisplayInfo->taxItemVector();

  if (taxItemVectorOWRT.empty())
  {
    LOG4CXX_DEBUG(logger, "Leaving getRTTax: taxItemVectorOWRT is empty");
    return false;
  }

  for (const auto taxItem : taxItemVectorOWRT)
  {
    if (taxCode != taxItem->taxCode())
      continue;

    LOG4CXX_DEBUG(logger,
                  "getRTTax : " << taxItem->taxCode() << "  ,taxType: " << taxItem->taxType()
                                << ", " << taxItem->taxAmount());

    rtTaxAmount += taxItem->taxAmount();
  }

  LOG4CXX_DEBUG(logger, "Leaving getRTTax = " << rtTaxAmount);

  return true;
}

// ---------------------------------------------
// getTotalOWTax
// Desc: Returns total of all OW taxes for a specific PaxTypeFare.
// ---------------------------------------------
bool
getTotalOWTax(const FareDisplayTrx& trx, const PaxTypeFare& paxTypeFare, MoneyAmount& owTaxAmount)
{
  owTaxAmount = 0;

  const FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();

  if (!fareDisplayInfo)
    return false;

  Indicator owrt = paxTypeFare.owrt();

  const std::vector<TaxRecord*>& taxRecordVector =
      (owrt == ONE_WAY_MAYNOT_BE_DOUBLED) ? fareDisplayInfo->taxRecordVector()
                                          : fareDisplayInfo->taxRecordOWRTFareTaxVector();

  for (const auto taxRecord : taxRecordVector)
  {
    if (shouldBypassTax(taxRecord->taxCode()))
      continue;

    MoneyAmount tempAmount = 0;
    getOWTax(trx, paxTypeFare, taxRecord->taxCode(), tempAmount);
    owTaxAmount += tempAmount;
  }

  return true;
}

// ---------------------------------------------
// getTotalRTTax
// Desc: Returns total of all RT taxes for a specific PaxTypeFare.
// ---------------------------------------------
bool
getTotalRTTax(const FareDisplayTrx& trx, const PaxTypeFare& paxTypeFare, MoneyAmount& rtTaxAmount)
{
  rtTaxAmount = 0;

  Indicator owrt = paxTypeFare.owrt();

  if (owrt == ONE_WAY_MAYNOT_BE_DOUBLED)
    return true;

  const FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();
  if (!fareDisplayInfo)
    return false;

  for (const auto taxRecord : fareDisplayInfo->taxRecordVector())
  {
    if (shouldBypassTax(taxRecord->taxCode()))
      continue;

    MoneyAmount tempAmount = 0;
    getRTTax(trx, paxTypeFare, taxRecord->taxCode(), tempAmount);
    rtTaxAmount += tempAmount;
  }

  // need to additionally process elements from taxRecordVectorOWRT
  for (const auto taxRecord : fareDisplayInfo->taxRecordOWRTFareTaxVector())
  {
    if (shouldBypassTax(taxRecord->taxCode()))
      continue;

    std::vector<TaxRecord*>::const_iterator it = fareDisplayInfo->taxRecordVector().begin();
    for (; it != fareDisplayInfo->taxRecordVector().end(); ++it)
      if ((*it)->taxCode() == taxRecord->taxCode())
        break;

    if (it != fareDisplayInfo->taxRecordVector().end())
      continue;

    MoneyAmount owTaxAmount = 0;
    if (!getOWTax(trx, paxTypeFare, taxRecord->taxCode(), owTaxAmount, true))
      continue;

    rtTaxAmount += owTaxAmount;
  }

  return true;
}

bool
shouldCalculateTax(FareDisplayTrx& trx, const FareMarket* fareMarket)
{
  // For FT entry allow.
  if (trx.getRequest()->requestType() == FARE_TAX_REQUEST)
    return true;

  // For FQ entry with FL qualifier - Allow
  if (trx.getRequest()->requestType() == FARE_DISPLAY_REQUEST &&
      trx.getOptions()->displayBaseTaxTotalAmounts() == TRUE_INDICATOR)
    return true;

  // ----------------------------
  // For BA carrier
  // ----------------------------
  if (fareMarket->governingCarrier().equalToConst("BA"))
    return true;

  // ----------------------------
  // For US Location
  // ----------------------------
  if (LocUtil::isUS(*trx.getRequest()->ticketingAgent()->agentLocation()))
  {
    if (LocUtil::isUSTerritoryOnly(*trx.getRequest()->ticketingAgent()->agentLocation()))
      return false;

    return true;
  }

  // ----------
  // Australia
  // ----------
  if (trx.getRequest()->ticketingAgent()->agentLocation()->nation() == AUSTRALIA)
    return true;

  // ----------
  // New Zealand
  // ----------
  if (trx.getRequest()->ticketingAgent()->agentLocation()->nation() == NEW_ZEALAND)
    return true;

  return false;
}

// ----------------------
// shouldBypassTax
// ----------------------
bool
shouldBypassTax(const TaxCode& taxCode)
{
  const int16_t size = 4;
  const std::string taxList[size] = { std::string("AY"), std::string("ZP"), std::string("YQ"),
                                      std::string("YR") };

  for (const auto& elem : taxList)
    if (!strncmp(elem.c_str(), taxCode.c_str(), 2))
    {
      LOG4CXX_DEBUG(logger, "ByPassing ..." << taxCode);
      return true;
    }

  return false;
}

static void
addFareUsage(const FareDisplayTrx& trx,
             FarePath* farePath,
             PricingUnit* pricingUnit,
             TravelSeg* travelSeg)
{
  FareUsage* fareUsage(nullptr);
  trx.dataHandle().get(fareUsage);
  if (!fareUsage)
    throw ErrorResponseException(
        ErrorResponseException::SYSTEM_EXCEPTION,
        "FareDisplayTax::addFareUsage - ERROR-UNABLE TO ALLOCATE MEMORY FOR FAREUSAGE");

  fareUsage->travelSeg().push_back(travelSeg);
  pricingUnit->fareUsage().push_back(fareUsage);
}

static void
createPricingUnit(const FareDisplayTrx& trx, FarePath* farePath, std::vector<TravelSeg*>& travelSeg)
{
  if (!trx.itin().front()->travelSeg().empty())
  {
    // -------------------
    // Build Pricing Unit
    // -------------------
    PricingUnit* pricingUnit(nullptr);
    trx.dataHandle().get(pricingUnit);

    if (!pricingUnit)
      throw ErrorResponseException(
          ErrorResponseException::SYSTEM_EXCEPTION,
          "FareDisplayTax::createPricingUnit - ERROR-UNABLE TO ALLOCATE MEMORY FOR PRICING UNIT");

    addFareUsage(trx, farePath, pricingUnit, trx.itin().front()->travelSeg().front());

    if (trx.itin().front()->travelSeg().size() == 2)
      addFareUsage(trx, farePath, pricingUnit, trx.itin().front()->travelSeg().back());

    farePath->pricingUnit().push_back(pricingUnit);
  }
}

bool
initializeFarePath(const FareDisplayTrx& trx, FarePath* farePath)
{
  // Create an itin
  Itin* itin(nullptr);
  trx.dataHandle().get(itin);
  if (!itin)
  {
    LOG4CXX_ERROR(logger,
                  "FareDisplayTax::initializeFarePath - ERROR-UNABLE TO ALLOCATE MEMORY FOR ITIN");
    return false;
  }
  itin->duplicate(*(trx.itin().front()), trx.dataHandle());
  // Add a return travel seg, if one doesn't exist already
  if (itin->travelSeg().size() == 1)
  {
    TravelSeg* tvlSeg = itin->travelSeg().front();
    AirSeg* airSeg = dynamic_cast<AirSeg*>(tvlSeg);
    if (airSeg == nullptr)
    {
      LOG4CXX_DEBUG(
          logger,
          "FareDisplayTax::initializeFarePath - ERROR DYNAMIC CASTING TRAVELSEG TO AIRSEG");
      return false;
    }
    AirSeg* returnSeg(nullptr);
    trx.dataHandle().get(returnSeg);
    returnSeg->segmentOrder() = 1;
    returnSeg->departureDT() = airSeg->departureDT().addDays(2);
    returnSeg->arrivalDT() = returnSeg->departureDT().addDays(2);
    returnSeg->origAirport() = airSeg->destAirport();
    returnSeg->origin() = airSeg->destination();
    returnSeg->destAirport() = airSeg->origAirport();
    returnSeg->destination() = airSeg->origin();

    itin->travelSeg().push_back(returnSeg);
  }

  farePath->itin() = itin;
  itin->farePath().push_back(farePath);

  // Build a Fare Usage and populate it with travelSegs from Itin
  if (farePath->pricingUnit().empty())
  {
    createPricingUnit(trx, farePath, trx.itin().front()->travelSeg());

    /*All paxTypeFare.nucFareAmount has already been convered to calculation
    * currency, hence set the origination currency to calculation currency
    * for correct tax calculations*/
    farePath->itin()->originationCurrency() = trx.itin().front()->calculationCurrency();
    farePath->baseFareCurrency() = trx.itin().front()->calculationCurrency();
    farePath->calculationCurrency() = trx.itin().front()->calculationCurrency();
  }
  return true;
}

static bool
processOWRTForFT(const FareDisplayTrx& trx, FarePath& farePath, const PaxTypeFare& paxTypeFare)
{

  if (farePath.itin()->travelSeg().size() != 2)
  {
    LOG4CXX_DEBUG(logger,
                  "processOWRTForFT: Unexpected number of travel segments= "
                      << farePath.itin()->travelSeg().size());
    return false;
  }

  // ----------------------------------------------
  // Populate Travel Segments
  // ----------------------------------------------i
  TravelSeg* tvlSeg = farePath.itin()->travelSeg()[0];
  TravelSeg* returnTvlSeg = farePath.itin()->travelSeg()[1];
  AirSeg* airSeg = dynamic_cast<AirSeg*>(tvlSeg);
  AirSeg* returnSeg = dynamic_cast<AirSeg*>(returnTvlSeg);
  if (airSeg == nullptr || returnSeg == nullptr)
  {
    LOG4CXX_DEBUG(
        logger,
        "processOWRTForFT:  ERROR: Unable to dynamic cast from TravelSeg to AirSeg. FareClass: "
            << paxTypeFare.fareClass());
    return false;
  }

  airSeg->furthestPoint() = true;
  airSeg->setMarketingCarrierCode(paxTypeFare.carrier());
  airSeg->setOperatingCarrierCode(paxTypeFare.carrier());

  LOG4CXX_DEBUG(logger,
                "processOWRTForFT:  FareClass: "
                    << paxTypeFare.fareClass() << ", owrt: " << paxTypeFare.owrt()
                    << ", No of TravelSeg: " << farePath.itin()->travelSeg().size());

  returnSeg->setMarketingCarrierCode(paxTypeFare.carrier());
  returnSeg->setOperatingCarrierCode(paxTypeFare.carrier());
  // Double the TotalNucAmount in the FarePath if the ONE_WAY_MAY_BE_DOUBLED  .
  if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
  {
    farePath.setTotalNUCAmount(paxTypeFare.nucFareAmount() * 2);
  }
  // ------------------------------------------------------
  // Populate Pricing Units and fareUsages
  // -----------------------------------------------------------

  if (!farePath.pricingUnit()[0]->fareUsage().empty())
  {
    FareUsage* fareUsage = farePath.pricingUnit()[0]->fareUsage()[0];
    fareUsage->paxTypeFare() = const_cast<PaxTypeFare*>(&paxTypeFare);
    if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
      fareUsage->isRoundTrip() = true;
  }

  if (farePath.pricingUnit()[0]->fareUsage().size() == 2)
  {
    FareUsage* returnFareUsage = farePath.pricingUnit()[0]->fareUsage()[1];
    returnFareUsage->paxTypeFare() = const_cast<PaxTypeFare*>(&paxTypeFare);
  }

  LOG4CXX_DEBUG(
      logger,
      "Leaving processOWRTForFT: FarePath::totalNucAmount =" << farePath.getTotalNUCAmount());

  return true;
}

bool
populateFarePath(const FareDisplayTrx& trx, const PaxTypeFare* paxTypeFare, FarePath* fp)
{
  if (!paxTypeFare->isValid())
  {
    LOG4CXX_DEBUG(
        logger,
        "FareDisplayTax:populateFarePath::Do not populate farepath for invalid paxtypefare");
    return false;
  }

  fp->itin()->ticketingCarrier() = paxTypeFare->carrier();
  fp->setTotalNUCAmount(paxTypeFare->nucFareAmount());
  fp->paxType() = const_cast<PaxType*>(paxTypeFare->actualPaxType());

  // Default to 1st ITIN PAX TYPE
  if (fp->paxType() == nullptr)
  {
    fp->paxType() = trx.paxType().front(); // default paxType
  }
  /////  Populate bookingCode  in Travel segment

  for (auto ts : fp->itin()->travelSeg())
  {
    // only 1st booking code, do need cross of loraine
    ts->setBookingCode(BookingCode(paxTypeFare->bookingCode()[0]));
  }

  if (!processOWRTForFT(trx, *fp, *paxTypeFare))
  {
    LOG4CXX_DEBUG(logger,
                  "FareDisplayTax:populateFarePath::Error populating return seg and pricing unit");
    return false;
  }

  LOG4CXX_DEBUG(logger,
                "populateFarePath:: fareClass: "
                    << paxTypeFare->fareClass() << ", owrt: " << paxTypeFare->owrt()
                    << ", FarePath::totalNUCAmount = " << fp->getTotalNUCAmount());

  return true;
}
}
}
