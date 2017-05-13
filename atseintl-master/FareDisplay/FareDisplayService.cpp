//----------------------------------------------------------------------------
//
// Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#include "FareDisplay/FareDisplayService.h"

#include "AddonConstruction/FareDup.h"
#include "BookingCode/FareDisplayBookingCode.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/Money.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PrivateIndicator.h"
#include "FareDisplay/FareDisplayController.h"
#include "FareDisplay/FareDisplayErrorResponse.h"
#include "FareDisplay/FareGroupingMgr.h"
#include "FareDisplay/FBDisplayCat10Controller.h"
#include "FareDisplay/FDAddOnFareController.h"
#include "FareDisplay/FDConsts.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/MergeFares.h"
#include "FareDisplay/MileageAdapter.h"
#include "FareDisplay/MPChooser.h"
#include "FareDisplay/MPData.h"
#include "FareDisplay/MPDataBuilder.h"
#include "FareDisplay/RoutingMgr.h"
#include "FareDisplay/ScheduleCountMgr.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Fares/FBDisplayController.h"
#include "Fares/FDFareCurrencySelection.h"
#include "Fares/RuleCategoryProcessor.h"
#include "RTG/RTGController.h"
#include "Rules/Config.h"
#include "Server/TseServer.h"

namespace tse
{
namespace
{
ConfigurableCategories
ruleDisplayValidationCategories("RULE_DISPLAY_VALIDATION");
}

static Logger
logger("atseintl.FareDisplay.FareDisplayService");

static LoadableModuleRegister<Service, FareDisplayService>
_("libFareDisplay.so");

FareDisplayService::FareDisplayService(const std::string& sname, tse::TseServer& tseServer)
  : Service(sname, tseServer), _config(Global::config())
{
}

bool
FareDisplayService::initialize(int argc, char* argv[])
{
  return true;
}

bool
FareDisplayService::process(FareDisplayTrx& trx)
{
  TSELatencyData metics(trx, MetricsUtil::FARE_DISPLAY_SVC);

  LOG4CXX_INFO(logger, "Entering FareDisplayService::process");

  if (trx.isDiagnosticRequest() && !trx.isFDDiagnosticRequest())
  {
    // Pricing diagnostic number displays have already been taken care of
    return true;
  }

  {
    TSELatencyData tld(trx, "FDS MERGE FARES");

    bool needAllFares =
        (trx.isFDDiagnosticRequest() &&
         trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALLFARES");

    mergeFares(trx, needAllFares);

    // Apply the below method when the request is short RD or RB entry
    if (trx.getOptions()->lineNumber() > 0)
    {
      checkPaxTypes(trx);
      checkPrivateIndicator(trx);
      checkItemNo(trx);
    }
  }

  {
    TSELatencyData tld(trx, "FDS GET TOTAL FARES");
    FareDisplayUtil::getTotalFares(trx);
  }

  FareDisplayService::RequestType _requestType = getRequestType(trx);

  {
    TSELatencyData tld(trx, "FDS PROCESS RQST");
    processRequest(trx, _requestType);
  }
  LOG4CXX_INFO(logger, "Leaving FareDisplayService::process");
  return true;
}

bool
FareDisplayService::applyGroupingAndSorting(FareDisplayTrx& trx)
{
  LOG4CXX_INFO(logger, "Entering FareDisplayService::Apply Grouping and Sorting");
  if (trx.getRequest()->diagnosticNumber() != DIAG_214_ID) // Bypass Grouping
  {
    FareGroupingMgr* mgr = FareGroupingMgr::create(trx);
    if (mgr != nullptr)
    {
      mgr->groupFares(trx);
    }
    LOG4CXX_INFO(logger, "Entering FareDisplayService::Apply Grouping and Sorting");
    return true;
  }
  LOG4CXX_ERROR(logger, "Not Applying Grouping and Sorting Due to Template Override");
  return true;
}

void
FareDisplayService::filterScheduleCount(FareDisplayTrx& trx)
{
  LOG4CXX_INFO(logger, "Entering FareDisplayService::filterScheduleCount() ");

  ScheduleCountMgr::process(trx);

  LOG4CXX_INFO(logger, "Entering FareDisplayService::filterScheduleCount() ");
}

bool
FareDisplayService::processFT(FareDisplayTrx& trx)
{
  return true;
}

bool
FareDisplayService::processRB(FareDisplayTrx& trx)
{
  RTGController rtgController(trx);
  rtgController.getRBText();
  return true;
}

bool
FareDisplayService::processRD(FareDisplayTrx& trx)
{
  applyGroupingAndSorting(trx);

  FBDisplayCat10Controller fbCat10(trx);
  fbCat10.collectRecord2Info();

  FBDisplayController fbDisplayController(trx);
  fbDisplayController.collectRecord2Info();

  if (trx.allPaxTypeFare().size() > 1 || trx.getOptions()->isRoutingDisplay())
  {
    RoutingMgr routingMgr(trx);
    if (!routingMgr.buildTravelRouteAndMap())
    {
      LOG4CXX_ERROR(logger, "buildTravelRouteAndMap failed");
    }
  }
  return true;
}

bool
FareDisplayService::processFQ(FareDisplayTrx& trx)
{
  if (trx.allPaxTypeFare().empty())
  {
    FareDisplayErrorResponse errResponse(trx);
    errResponse.process();
  }

  filterScheduleCount(trx);
  applyGroupingAndSorting(trx);

  // Eliminate duplicate fares
  if (trx.getRequest()->requestedInclusionCode() == ALL_FARES)
  {
    eliminateDuplicateFares(trx);
  }

  if (!trx.allPaxTypeFare().empty() && trx.isSDSOutputType())
  {
    processSDS(trx);
  }
  if (!trx.isShopperRequest())
  {
    RoutingMgr routingMgr(trx);
    if (!routingMgr.buildTravelRouteAndMap())
    {
      LOG4CXX_ERROR(logger, "buildTravelRouteAndMap failed");
    }
  }

  return true;
}

bool
FareDisplayService::processDiagnostic(FareDisplayTrx& trx)
{
  return true;
}

bool
FareDisplayService::processAD(FareDisplayTrx& trx)
{
  setupDisplayCurrency(trx);
  FDAddOnFareController fdAddOnFareController(trx.allFDAddOnFare());
  fdAddOnFareController.process(trx, trx.getRequest()->globalDirection());

  RoutingMgr routingMgr(trx);
  if (!routingMgr.buildAddOnRoutings())
  {
    LOG4CXX_ERROR(logger, "buildTravelRouteAndMap failed");
  }

  return true;
}

bool
FareDisplayService::processMP(FareDisplayTrx& trx)
{
  try
  {
    setupDisplayCurrency(trx);

    MPChooser& chooser = MPChooser::getChooser(FareDisplayUtil::determineMPType(trx));
    trx.mpData() = Singleton<MPDataBuilder>::instance().buildMPData(trx, chooser);
    if (trx.mpData() == nullptr)
    {
      LOG4CXX_ERROR(logger, "Failed to build MP data");
      return false;
    }
    return Singleton<MileageAdapter>::instance().getMPM(*trx.mpData(), chooser);
  }
  catch (MPChooser::ChooserNotRegisteredException&)
  {
    LOG4CXX_ERROR(logger, "No handler registered for given MP type");
    return false;
  }
}

bool
FareDisplayService::processSDS(FareDisplayTrx& trx)
{
  LOG4CXX_INFO(logger, "ENTERING processSDS");
  std::vector<uint16_t> categorySequence = ruleDisplayValidationCategories.read();

  RuleCategoryProcessor::adjustCategoryLists(trx, categorySequence);

  // collect category rule information for each fare
  FBDisplayCat10Controller fbCat10(trx);
  fbCat10.collectRecord2Info();
  FBDisplayController fbDisplayController(trx);
  fbDisplayController.collectRecord2Info();
  LOG4CXX_INFO(logger, "LEAVING processSDS");
  return true;
}

FareDisplayService::RequestType
FareDisplayService::getRequestType(FareDisplayTrx& trx)
{
  if (trx.getRequest()->inclusionCode() == ADDON_FARES)
  {
    return AD_REQUEST;
  }
  else if (trx.getRequest()->diagnosticNumber() == DIAG_200_ID ||
           trx.getRequest()->diagnosticNumber() == DIAG_201_ID ||
           trx.getRequest()->diagnosticNumber() == DIAG_203_ID)
  {
    return DIAGNOSTIC_REQUEST;
  }
  else if (trx.isRD() && trx.itin().size() != 0 && trx.allPaxTypeFare().size() != 0 &&
           trx.allPaxTypeFare().front()->fareMarket())
  {
    return RD_REQUEST;
  }
  else if (trx.getRequest()->requestType() == FARE_BOOKINGCODE_REQUEST)
  {
    return RB_REQUEST;
  }
  else if (trx.getRequest()->requestType() == FARE_TAX_REQUEST)
  {
    return FT_REQUEST;
  }
  else if (trx.getRequest()->requestType() == FARE_MILEAGE_REQUEST)
  {
    return MP_REQUEST;
  }

  return FQ_REQUEST;
}

void
FareDisplayService::processRequest(FareDisplayTrx& trx, FareDisplayService::RequestType& reqType)
{
  bool success(false);
  switch (reqType)
  {
  case AD_REQUEST:
    success = processAD(trx);
    break;
  case RD_REQUEST:
    success = processRD(trx);
    break;
  case RB_REQUEST:
    success = processRB(trx);
    break;
  case FT_REQUEST:
    success = processFT(trx);
    break;
  case MP_REQUEST:
    success = processMP(trx);
    break;
  case DIAGNOSTIC_REQUEST:
    success = processDiagnostic(trx);
    break;

  default:
    success = processFQ(trx);
    break;
  }
  if (success)
  {
    // Get booking code for all paxTypeFares for RD, FT and FQ if it is not already there
    // AD doesn't need bookingCode and for RB, processRB takes care of it
    if (reqType == FQ_REQUEST || reqType == RD_REQUEST || reqType == FT_REQUEST)
    {
      std::vector<PaxTypeFare*>::iterator pftItr = trx.allPaxTypeFare().begin();
      std::vector<PaxTypeFare*>::iterator pftItrEnd = trx.allPaxTypeFare().end();
      FareDisplayBookingCode fdbc;
      for (; pftItr != pftItrEnd; pftItr++)
      {
        if ((*pftItr)->bookingCode().empty())
        {
          fdbc.getBookingCode(trx, **pftItr, (*pftItr)->bookingCode());
        }
      }
    }
    display(trx);
  }
  else
    LOG4CXX_ERROR(logger, "FAILED TO PROCESS REQUEST : IGNORING DISPLAY");
}

void
FareDisplayService::display(FareDisplayTrx& trx)
{
  TSELatencyData tld(trx, "FDS DISPLAY");

  FareDisplayController fareDisplayController;
  fareDisplayController.display(trx);
}

void
FareDisplayService::setupDisplayCurrency(FareDisplayTrx& trx)
{
  // Save the display currencyCode requests in itin
  Itin* itin = trx.itin().front();
  if (!FDFareCurrencySelection::getDisplayCurrency(trx, itin->calculationCurrency()))
  {
    LOG4CXX_ERROR(logger,
                  "getDisplayCurrency failed in FareDisplayService::setupDisplayCurrency()");
  }
}

void
FareDisplayService::checkPaxTypes(FareDisplayTrx& trx)
{
  if (trx.allPaxTypeFare().size() < 2)
    return;

  // For RD, trx only has one requested paxType
  PaxTypeCode& requestedPTC = trx.getRequest()->displayPassengerTypes().front();
  FareDisplayInfo* fdi = nullptr;

  std::vector<PaxTypeFare*>::iterator ptfItr = trx.allPaxTypeFare().begin();
  while (ptfItr != trx.allPaxTypeFare().end())
  {
    // use fdInfo because matching paxType may be in a merged PTF
    fdi = (*ptfItr)->fareDisplayInfo();

    if (fdi != nullptr && !fdi->hasPaxType(requestedPTC))
      removePTF(trx, ptfItr);
    else
      ptfItr++;
  }
}

void
FareDisplayService::eliminateDuplicateFares(FareDisplayTrx& trx)
{
  if (trx.allPaxTypeFare().size() < 2)
  {
    return;
  }

  std::vector<PaxTypeFare*>::iterator currItr = trx.allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::iterator nextItr = currItr;
  nextItr++;

  EqualBy eb(&trx);

  // Eliminate duplicate elements (fares) from vector
  // Note: Deleting an element in the middle of a vector invalidates all
  //       iterators that point to elements following the deletion point.
  //       Therefore, we should always determine the end of the vector in
  //       each iteration.

  while (currItr != trx.allPaxTypeFare().end() && nextItr != trx.allPaxTypeFare().end())
  {
    if (eb.areEqual(*currItr, *nextItr))
    {
      PaxTypeCode currPaxType = "ADT";

      if ((*currItr)->actualPaxType() != nullptr)
      {
        currPaxType = (*currItr)->actualPaxType()->paxType();
      }

      PaxTypeCode nextPaxType = "ADT";

      if ((*nextItr)->actualPaxType() != nullptr)
      {
        nextPaxType = (*nextItr)->actualPaxType()->paxType();
      }

      // Eliminate fare if they are equal and have same pax type
      if (currPaxType == nextPaxType)
      {
        // Get rid of this one
        nextItr = trx.allPaxTypeFare().erase(nextItr);
      }
      else
      {
        currItr++;
        nextItr++;
      }
    }
    else
    {
      currItr++;
      nextItr++;
    }
  }
}

void
FareDisplayService::checkPrivateIndicator(FareDisplayTrx& trx)
{
  LOG4CXX_INFO(logger, " Entered FareDisplayService::checkPrivateIndicator");
  if (trx.allPaxTypeFare().size() <= 1)
    return;

  std::vector<PaxTypeFare*>::iterator ptfItr = trx.allPaxTypeFare().begin();
  std::string pi;
  while (ptfItr != trx.allPaxTypeFare().end())
  {
    PaxTypeFare* ptf = (*ptfItr);
    bool isFQ = true;
    PrivateIndicator::privateIndicatorOld(*ptf, pi, true, isFQ);

    if (trx.getOptions()->privateIndicator() != pi)
    {
      removePTF(trx, ptfItr);
      continue;
    }
    ptfItr++;
  }
}

void
FareDisplayService::checkItemNo(FareDisplayTrx& trx)
{
  LOG4CXX_INFO(logger, " Entered FareDisplayService::checkItemNo");
  if (trx.allPaxTypeFare().size() <= 1)
    return;

  if (!trx.getOptions()->cat25Values().isNonPublishedFare())
    return;

  std::vector<PaxTypeFare*>::iterator ptfItr = trx.allPaxTypeFare().begin();
  while (ptfItr != trx.allPaxTypeFare().end())
  {
    PaxTypeFare* ptf = (*ptfItr);
    if (ptf->isFareByRule())
    {
      if (!matchNPValues(*ptf, trx.getOptions()->cat25Values(), &ptf->fareByRuleInfo()))
      {
        removePTF(trx, ptfItr);
        continue;
      }
      ptfItr++;
    }
    else
    {
      removePTF(trx, ptfItr);
    }
  }
}

void
FareDisplayService::removePTF(FareDisplayTrx& trx, std::vector<PaxTypeFare*>::iterator& ptfItr)
{
  PaxTypeFare* ptf = (*ptfItr);
  if (trx.getRequest()->diagnosticNumber() == DIAG_200_ID)
  {
    ptf->invalidateFare(PaxTypeFare::FD_Fare_Not_Selected_for_RD);
    ptfItr++;
  }
  else
  {
    ptfItr = trx.allPaxTypeFare().erase(ptfItr);
  }
}

bool
FareDisplayService::matchNPValues(PaxTypeFare& ptf,
                                  NonPublishedValues& nonPubValues,
                                  const RuleItemInfo* itemInfo)
{
  if (!itemInfo)
    return false;

  if (itemInfo->vendor() != nonPubValues.vendorCode())
    return false;

  if (nonPubValues.itemNo() != 0 && itemInfo->itemNo() != nonPubValues.itemNo())
    return false;

  return true;
}

void
FareDisplayService::mergeFares(FareDisplayTrx& trx, const bool needAllFares)
{
  Itin* itin = trx.itin().front(); // lint !e578

  if (!itin)
    return;
  if (itin->fareMarket().empty())
    return;

  bool copyAllFares = (trx.getRequest()->diagnosticNumber() == DIAG_200_ID || needAllFares);

  bool isDomestic = (itin->geoTravelType() == GeoTravelType::Domestic || itin->geoTravelType() == GeoTravelType::Transborder);

  MergeFares merge(trx);
  merge.run(itin->fareMarket(), copyAllFares, isDomestic);
}
}
