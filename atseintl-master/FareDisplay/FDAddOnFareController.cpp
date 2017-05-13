//----------------------------------------------------------------------------
//  File: FDAddOnFareController.cpp
//
//  Author: Partha Kumar Chakraborti
//  Created:      08/09/2005
//  Description:  This takes cares of getting AddOn Fares, group, sort.
//								It acts as a repisotary.
//  Copyright Sabre 2005
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#include "FareDisplay/FDAddOnFareController.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayRequest.h"
#include "Fares/FDFareCurrencySelection.h"

namespace tse
{
static Logger
logger("atseintl.FareDisplay.FDAddOnFareController");

// --------------------------------------
// get :: Call DBAccess to get AddOnFare
// --------------------------------------
const std::vector<AddonFareInfo*>&
FDAddOnFareController::get(DataHandle& dataHandle,
                           const LocCode& origin,
                           const LocCode& destination,
                           const CarrierCode& carrier,
                           const RecordScope& crossRefType,
                           const DateTime& date)

{
  return dataHandle.getAddOnFare(origin, destination, carrier, crossRefType, date);
}

// --------------------------------------
// process: Main entry point
// --------------------------------------
bool
FDAddOnFareController::process(FareDisplayTrx& trx, const GlobalDirection& globalDir)
{

  // Clear existing AddOn Fare vector
  if (!_addOnFaresVec.empty())
    _addOnFaresVec.clear();

  // Do not waste time getting addon fares when a Ticket Designator is requested.
  // There will be no possible match.
  if (!trx.getRequest()->ticketDesignator().empty())
    return false;

  const RecordScope& crossRefType = (trx.isRecordScopeDomestic()) ? DOMESTIC : INTERNATIONAL;

  const LocCode& orig = trx.travelSeg().front()->boardMultiCity();
  const LocCode& dest = trx.travelSeg().front()->offMultiCity();
  const CarrierCode& cxr = trx.requestedCarrier();

  const std::vector<AddonFareInfo*>& tempAddOnFareVec =
      get(getDataHandle(trx), orig, dest, cxr, crossRefType, trx.travelDate());

  // If No AddOn Fare found return
  if (tempAddOnFareVec.empty())
    return false;

  // ------------------------------
  // Save the AddOn Fare List
  // ------------------------------
  if (!populate(trx, tempAddOnFareVec, globalDir))
    return false;

  // -------------------------
  // Generate Routing Seq No
  // -------------------------
  if (!generateRoutingSeqNo(crossRefType))
    return false;

  return true;
}

bool
FDAddOnFareController::isAvaiableInReverseCityPair(FareDisplayTrx& trx,
                                                   std::vector<CurrencyCode>& alternateCurrencyVec)
{

  const RecordScope& crossRefType = (trx.isRecordScopeDomestic()) ? DOMESTIC : INTERNATIONAL;

  const LocCode& orig = trx.travelSeg().front()->boardMultiCity();
  const LocCode& dest = trx.travelSeg().front()->offMultiCity();
  const CarrierCode& cxr = trx.requestedCarrier();

  const std::vector<AddonFareInfo*>& tempAddOnFareVec =
      get(getDataHandle(trx), dest, orig, cxr, crossRefType, trx.travelDate());

  if (tempAddOnFareVec.empty())
    return false;

  // ----------------------------------------------------
  // TODO: Made an arrangement to copy AddonFareInfo to
  // FDAddOnFareInfo untill we create our own
  // DBAccess method.
  // ----------------------------------------------------
  std::vector<FDAddOnFareInfo*> tempFDAddOnFareVec;

  std::vector<AddonFareInfo*>::const_iterator iB = tempAddOnFareVec.begin();
  std::vector<AddonFareInfo*>::const_iterator iE = tempAddOnFareVec.end();

  for (; iB != iE; ++iB)
  {
    FDAddOnFareInfo* fdAddOnFareInfo(nullptr);
    getDataHandle(trx).get(fdAddOnFareInfo);
    (*iB)->clone(*fdAddOnFareInfo);
    tempFDAddOnFareVec.push_back(fdAddOnFareInfo);
  }

  // ----------------------------------
  // Get Alternate Currency List
  // ----------------------------------
  generateAltCurency(trx, tempFDAddOnFareVec, alternateCurrencyVec);
  return true;
}

// -------------------------------------------------------------
// populate : Converts each AddonFareInfo to FDAddOnFareInfo
//            and inserts them in addOnFaresVec
// -----------------------------------------------------------

bool
FDAddOnFareController::populate(FareDisplayTrx& trx,
                                const std::vector<AddonFareInfo*>& tempAddOnFareVec,
                                const GlobalDirection& globalDir)
{

  if (tempAddOnFareVec.empty())
    return false;

  const FareClassCode requestedFareClassCode = trx.getRequest()->fareBasisCode();

  std::vector<AddonFareInfo*>::const_iterator iB = tempAddOnFareVec.begin();
  std::vector<AddonFareInfo*>::const_iterator iE = tempAddOnFareVec.end();

  for (; iB != iE; ++iB)
  {
    // Keep fares thoses matches with requested global
    if (globalDir != GlobalDirection::ZZ && (*iB)->globalDir() != globalDir)
      continue;

    // Keep fares that match the requested Fare Class Code
    if (!requestedFareClassCode.empty() && (*iB)->fareClass() != requestedFareClassCode)
      continue;

    if (trx.isShortRD())
    {
      FareDisplayOptions* options = trx.getOptions();
      if (!options)
      {
        LOG4CXX_DEBUG(logger, "Non Existant FareDisplay Options");
        continue;
      }
      // Keep fares that match requested linkno
      if ((*iB)->linkNo() != options->linkNumber())
      {
        LOG4CXX_DEBUG(logger, "Fare not selected due to linkNumber mismatch");
        continue;
      }

      // Keep fares that match requested sequenceno
      if ((*iB)->seqNo() != options->sequenceNumber())
      {
        LOG4CXX_DEBUG(logger, "Fare not selected due to sequenceNumber mismatch");
        continue;
      }

      // Keep fares that match requested addontariff
      if ((*iB)->addonTariff() != options->fareTariff())
      {
        LOG4CXX_DEBUG(logger, "Fare not selected due to addonTariff mismatch");
        continue;
      }

      // Keep fares that match requested vendor
      if ((*iB)->vendor() != options->vendorCode())
      {
        LOG4CXX_DEBUG(logger, "Fare not selected due to vendor mismatch");
        continue;
      }

      // Keep fares that match requested currency
      if ((*iB)->cur() != options->baseFareCurrency())
      {
        LOG4CXX_DEBUG(logger, "Fare not selected due to currency mismatch");
        continue;
      }

      // Keep fares that match requested createdate
      if ((*iB)->createDate().dateToString(YYYYMMDD, "-") !=
              options->createDate().dateToString(YYYYMMDD, "-")
              //			   || (*iB)->createDate().timeToString(HHMMSS, "") !=
              //options->createDate().timeToString(HHMMSS, ""))
          ||
          (*iB)->createDate().timeToSimpleString() != options->createTime())
      {
        LOG4CXX_DEBUG(logger, "Fare not selected due to createDate mismatch");
        continue;
      }
    }

    FDAddOnFareInfo* fdAddOnFareInfo(nullptr);
    getDataHandle(trx).get(fdAddOnFareInfo);
    (*iB)->clone(*fdAddOnFareInfo);
    _addOnFaresVec.push_back(fdAddOnFareInfo);
  }

  return true;
}

bool
FDAddOnFareController::alternateCurrencyVec(FareDisplayTrx& trx,
                                            std::vector<CurrencyCode>& alternateCurrencyVec)
{
  return generateAltCurency(trx, _addOnFaresVec, alternateCurrencyVec);
}

bool
FDAddOnFareController::groupAndSort()
{
  FDAddOnGroupingStrategy groupStrategy(_addOnFaresVec);
  groupStrategy.apply();
  return true;
}

bool
FDAddOnFareController::generateRoutingSeqNo(const RecordScope& crossRefType)
{
  groupAndSort();

  RoutingSequenceGenerator obj;
  obj.generate(_addOnFaresVec, crossRefType);
  return true;
}

bool
FDAddOnFareController::generateAltCurency(FareDisplayTrx& trx,
                                          const std::vector<FDAddOnFareInfo*>& addOnFaresVec,
                                          std::vector<CurrencyCode>& alternateCurrencyVec)
{
  if (!alternateCurrencyVec.empty())
    alternateCurrencyVec.clear();

  // Get the Currency
  CurrencyCode displayCurrency;
  if (!FDFareCurrencySelection::getDisplayCurrency(trx, displayCurrency))
    return false;

  std::set<CurrencyCode> currencies;
  std::for_each(
      addOnFaresVec.begin(), addOnFaresVec.end(), UniqueCurrency(currencies, displayCurrency));

  if (currencies.empty())
    return false;

  std::set<CurrencyCode>::iterator iB = currencies.begin();
  std::set<CurrencyCode>::iterator iE = currencies.end();

  for (; iB != iE; ++iB)
    alternateCurrencyVec.push_back(*iB);

  return true;
}
}
