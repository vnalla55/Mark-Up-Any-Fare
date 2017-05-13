//-------------------------------------------------------------------
//
//  File:        FareDisplayController
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

#include "FareDisplay/FareDisplayController.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/TSELatencyData.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "FareDisplay/FBDisplayCat10Controller.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/Templates/DisplayDocumentFactory.h"
#include "FareDisplay/Templates/MXCombinabilityDisplay.h"
#include "RTG/RTGController.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <istream>
#include <string>
#include <vector>

namespace tse
{
namespace
{
Logger
logger("atseintl.FareDisplay.FareDisplayController");
ConfigurableValue<bool>
bundleRtgRequest("FAREDISPLAY_SVC", "BUNDLE_RTG_REQUESTS", false);
}

void
FareDisplayController::display(FareDisplayTrx& trx)
{
  displayFareInfo(trx);
}

//----------------------------------------------------------------
// check if rule category requested in the input request options
//----------------------------------------------------------------
bool
FareDisplayController::isRuleCategoryRequested(FareDisplayOptions& fdo, CatNumber cat)
{
  LOG4CXX_DEBUG(logger, "Cat # checked " << cat);
  // Categories and text
  const bool isMatch = std::any_of(fdo.ruleCategories().cbegin(),
                                   fdo.ruleCategories().cend(),
                                   [cat](const CatNumber requestedCatNumber)
                                   { return requestedCatNumber == cat; });

  if (isMatch)
    LOG4CXX_DEBUG(logger, "Cat # matched " << cat);

  return isMatch;
}

// ----------------------------------------------------------------------------
// DisplayStuff based on factory templates
// ----------------------------------------------------------------------------
bool
FareDisplayController::displayFareInfo(FareDisplayTrx& trx)
{
  bool isShopping = trx.isShopperRequest();
  std::vector<FareDispTemplate*> templateRecs;
  std::vector<FareDispTemplateSeg*> templateSegRecs;
  FareDispTemplate* templateRec = nullptr;
  int16_t templateNumber = 0;
  //*ACT TemplateType templateType = SINGLE_CARRIER;

  // interim solution - ultimately long and currency MP should have their own transaction type
  // currency MP does not need user preference
  if (trx.getRequest()->requestType() == MP_REQUEST &&
      FareDisplayUtil::determineMPType(trx) == NO_MARKET_MP)
  {
    DisplayDocumentFactory& factory(tse::Singleton<DisplayDocumentFactory>::instance());
    Document& document = factory.getDisplayDocument(trx);
    document.buildDisplay();
  }
  else
  {
    TemplateType templateType =
        trx.getOptions()->templateType() == 'S' ? SINGLE_CARRIER : MULTI_CARRIER;

    if (templateType == MULTI_CARRIER)
    {
      isShopping = true;
    }
    else
    {
      if (isShopping)
        templateType = MULTI_CARRIER;
    }

    const FareDisplayPref* prefs = trx.getOptions()->fareDisplayPref();
    if (prefs == nullptr)
    {
      trx.response() << "NO USER PREFERENCE FOUND FOR DISPLAY " << std::endl;
      return false;
    }

    bool isFL = trx.getOptions()->displayBaseTaxTotalAmounts() == TRUE_INDICATOR;
    int16_t templateOverride = trx.getOptions()->templateOverride();
    if (templateOverride != 0 && trx.getRequest()->inclusionCode() != ADDON_FARES && !isFL)
    {
      // Pass on diagnostic requests
      if (templateOverride < DIAG_200_ID)
        templateNumber = templateOverride;
    }
    else
    {
      if (trx.getRequest()->inclusionCode() == ADDON_FARES)
      {
        templateNumber = atoi(prefs->addOnTemplateId().c_str());
        templateType = ADDON;
      }
      else if (isFL)
      { // FL Display - Template 19
        templateNumber = atoi(prefs->taxTemplateId().c_str());
        templateType = TAX;
      }
      else if (isShopping)
      {
        templateNumber = atoi(prefs->multiCxrTemplateId().c_str());
        templateType = MULTI_CARRIER;
      }
      else
        templateNumber = atoi(prefs->singleCxrTemplateId().c_str());
      LOG4CXX_DEBUG(logger,
                    "Template number: " << templateNumber << ", Template type: " << templateType);
    }

    if (templateNumber < DIAG_200_ID)
    {
      templateRecs = trx.dataHandle().getFareDispTemplate(templateNumber, templateType);
      if (templateRecs.empty())
      {
        LOG4CXX_DEBUG(
            logger, "Template number: " << templateNumber << ", not found, using DEFAULT_TEMPLATE");
        if (trx.getRequest()->inclusionCode() == ADDON_FARES)
          templateNumber = DEFAULT_ADDON_TEMPLATE;
        else
          templateNumber = DEFAULT_TEMPLATE;
        templateRecs = trx.dataHandle().getFareDispTemplate(templateNumber, templateType);
      }
      if (templateRecs.empty())
      {
        trx.response() << "DEFAULT TEMPLATE NOT FOUND" << std::endl;
        return false;
      }
      templateRec = templateRecs.front();

      templateSegRecs = trx.dataHandle().getFareDispTemplateSeg(templateNumber, templateType);
      LOG4CXX_DEBUG(logger,
                    "Template number: " << templateNumber << ", Template type: " << templateType);
    }

    // For rules displays
    if (isRuleDisplay(trx) && isRTGNeeded(trx))
    {
      TSELatencyData metrics(trx, "GET RULE TEXT");
      getRuleText(trx);
    }

    if (trx.isERDFromSWS() && !trx.isFDDiagnosticRequest())
    {
      if (trx.allPaxTypeFare().empty())
        throw NonFatalErrorResponseException(ErrorResponseException::PRCRD_FARE_BASIS_NOT_FOUND,
                                             "$FARE BASIS NOT FOUND FOR CITYPAIR/DATE$");
    }
    else
    {
      DisplayDocumentFactory& factory(tse::Singleton<DisplayDocumentFactory>::instance());
      Document& document = factory.getDisplayDocument(trx, *prefs, *templateRec, templateSegRecs);

      document.buildDisplay();
    }
  }
  // Move everything to diagnostic object for FD diagnostic requests
  // to be consistent with Pricing diagnostic
  if (trx.isFDDiagnosticRequest())
  {
    trx.diagnostic().insertDiagMsg(trx.response().str());
    trx.response().str("");
  }

  return true;
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  FareDisplayController::isRuleDisplay()
//
// This method determines if a transaction is for a RD request and
// returns a true or false result.  It also returns, via
// parameter, the number of valid fares.
//
//
// @param   field   - a reference to a FareDisplayTrx.
//
// @return  void.
//
// </PRE>
// -------------------------------------------------------------------------
bool
FareDisplayController::isRuleDisplay(FareDisplayTrx& trx)
{
  int16_t numFares = 0;

  numFares = validFares(trx);
  if (trx.isRD())
  {
    if (numFares == 1)
      return true;

    else if (numFares > 1)
    {
      // At this point we have a request type of short RD.  However, we
      // also have more than 1 one-way fare.  So now we need
      // eliminate fares from the allPaxTypeFare vector except ONE
      // and display the one-way fare in a Rule Display.  Otherwise, we
      // just show the paxTypeFares in a regular Fare Display.

      if (trx.isShortRD())
      {
        trx.allPaxTypeFare().erase(trx.allPaxTypeFare().begin() + 1, trx.allPaxTypeFare().end());
        return true;
      }
      // OK, at this point we have a request type of RD.  However, we
      // also have more than 1 fare.  So now we need to determine if we
      // have a single one-way fare in the bunch.  If we do, we need to
      // eliminate the round-trip fares from the allPaxTypeFare vector
      // and display the one-way fare in a Rule Display.  Otherwise, we
      // just show the paxTypeFares in a regular Fare Display.

      // Iterate through the collection of fares

      const size_t numOneWayFares =
          std::count_if(trx.allPaxTypeFare().cbegin(),
                        trx.allPaxTypeFare().cend(),
                        [](const PaxTypeFare* const paxTypeFare)
                        { return paxTypeFare->owrt() != ROUND_TRIP_MAYNOT_BE_HALVED; });

      // Check the number of one way fares.  If it's 1 and only one,
      // we need to remove all other fares.  Otherwise we just
      // go on as usual.

      if (numOneWayFares == 1)
      {
        auto isRoundTripMayNotBeHalved = [](const PaxTypeFare* const paxTypeFare) -> bool
        { return paxTypeFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED; };

        trx.allPaxTypeFare().erase(std::remove_if(trx.allPaxTypeFare().begin(),
                                                  trx.allPaxTypeFare().end(),
                                                  isRoundTripMayNotBeHalved),
                                   trx.allPaxTypeFare().end());

        return true;
      }
    }
  }
  return false;
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  FareDisplayController::validFares()
//
// This method returns the number of valid fares.
// It requires a reference to the Fare Display Transaction.
//
// @param   field   - a reference to a FareDisplayTrx.
//
// @return  void.
//
// </PRE>
// -------------------------------------------------------------------------
int16_t
FareDisplayController::validFares(FareDisplayTrx& trx)
{
  return trx.allPaxTypeFare().size();
}

// -------------------------------------------------------------------
//
// @MethodName  FareDisplayController::getRuleText()
//
// Collect rule text for all mapped categories
//
// @param trx - a reference to a FareDisplayTrx.
//
// @return void
//
// -------------------------------------------------------------------------
void
FareDisplayController::getRuleText(FareDisplayTrx& trx)
{
  if (trx.allPaxTypeFare().empty())
  {
    LOG4CXX_WARN(logger, "No pax type fares!");
    return;
  }

  PaxTypeFare* paxTypeFare = trx.allPaxTypeFare().front();

  if (paxTypeFare->fareDisplayInfo() == nullptr)
  {
    LOG4CXX_ERROR(logger, "No Fare Display Info record!");
    return;
  }

  callForRuleText(trx,
                  *(trx.fdResponse()),
                  paxTypeFare->fareDisplayInfo()->fbDisplay(),
                  paxTypeFare->fareDisplayInfo()->myObjId(),
                  *paxTypeFare);
}

// -------------------------------------------------------------------
//
// @MethodName  FareDisplayController::callForRuleText()
//
// Collect rule text for all mapped categories
//
// @param response  - a reference to a FareDisplayResponse object.
// @param fbDisplay - a reference to a FBDisplay object.
//
// @return void
//
// -------------------------------------------------------------------------
void
FareDisplayController::callForRuleText(FareDisplayTrx& trx,
                                       const FareDisplayResponse& response,
                                       FBDisplay& fbDisplay,
                                       const int32_t objId,
                                       PaxTypeFare& ptf)
{
  // Go through the category rule record map processing each entry
  std::map<CatNumber, FBCategoryRuleRecord*>::const_iterator ruleRecIter =
      fbDisplay.fbCategoryRuleRecordMap().begin();
  std::map<CatNumber, FBCategoryRuleRecord*>::const_iterator ruleRecIterEnd =
      fbDisplay.fbCategoryRuleRecordMap().end();
  std::map<CatNumber, FBCategoryRuleRecord*>::const_iterator ruleRecIterLast =
      fbDisplay.fbCategoryRuleRecordMap().end();

  if (!fbDisplay.fbCategoryRuleRecordMap().empty())
  {
    ruleRecIterLast--;
  }

  RTGController rtgController(trx);
  std::string repository;

  FareDisplayInfo* fdi = ptf.fareDisplayInfo();
  bool isMinMaxFare = (fdi) ? fdi->isMinMaxFare() : false;

  bool haveSubCategories = !response.subCategoryNumbers().empty();
  bool haveSubCats = true;
  bool hasRule = false;

  FareDisplayOptions& fdo = *trx.getOptions();
  bool optionsRequested = fdo.ruleCategories().size() < RuleConst::MAXIMUM_RULE_CATEGORY_COUNT;
  bool showMX = fdo.isCombScoreboardDisplay();
  // Check if RTG requests should be bundled
  if (bundleRtgRequest.getValue())
  {
    rtgController.initializeBundledRequest(trx, ptf);
  }

  for (; ruleRecIter != ruleRecIterEnd; ruleRecIter++)
  {
    if (!optionsRequested && ruleRecIter->first == VOLUNTARY_CHANGES)
    {
      continue; // Skip CAT-31 - Voluntary Changes for RDn
    }

    if (optionsRequested && !isRuleCategoryRequested(fdo, ruleRecIter->first))
    {
      if (!showMX || ruleRecIter->first != COMBINABILITY_RULE_CATEGORY)
        continue;
    }

    repository.erase();

    if (showMX)
    {
      //      if(   (ruleRecIter->first >= COMBINABILITY_RULE_CATEGORY)
      if ((ruleRecIter->first == COMBINABILITY_RULE_CATEGORY) || (ruleRecIter == ruleRecIterLast))
      {
        MXCombinabilityDisplay* mxd = nullptr;
        trx.dataHandle().get(mxd);
        if (!mxd)
        {
          LOG4CXX_ERROR(logger, "Unable to instance MXCombinabilityDisplay...");
        }
        else
        {
          repository.erase();
          mxd->getMXInfo(trx, ptf, false, repository);
          fbDisplay.mxCombinabilityInfo() = repository;
          repository.erase();
        }
        showMX = false;
      }
    }

    if (ruleRecIter->second->isAllEmpty() && !ptf.isFareByRule())
    {
      LOG4CXX_INFO(logger, "Skipping, No rule data found for category: " << ruleRecIter->first);
      continue;
    }

    std::string& ruleText = fbDisplay.ruleTextMap()[ruleRecIter->first];

    // Get the rule text for this category

    if (ruleRecIter->first == PENALTY_RULE_CATEGORY && ptf.isFareByRule() &&
        hasCat16fromBaseFare(ptf))
    {
      ruleText += "   CONTACT CARRIER FOR DETAILS.";
      continue;
    }

    if (ruleRecIter->first == VOLUNTARY_REFUNDS)
    {
      ruleText += "   CHECK CATEGORY 16 OR CONTACT CARRIER FOR DETAILS.";
      continue;
    }

    if (ruleRecIter->first == COMBINABILITY_RULE_CATEGORY)
    {
      if (haveSubCategories)
      {
        // get rule text for the sub-categories
        // First, check the rec 2 to see if there aren't any sub-cats
        //

        FBDisplayCat10Controller fbd(trx);
        const CombinabilityRuleInfo* pCat10 =
            fbd.collectCombinabilityScoreboardRec2Info(*fdi, ptf, RuleConst::COMBINABILITY_RULE);

        if (pCat10)
        {
          if (pCat10->segCnt() == 0)
          {
            haveSubCats = false;
          }
        }

        for (const auto catNumbber : response.subCategoryNumbers())
        {
          repository.erase();

          if (optionsRequested && !isRuleCategoryRequested(fdo, ruleRecIter->first))
          {
            continue;
          }

          // Check if RTG requests should be bundled
          if (bundleRtgRequest.getValue())
          {
            rtgController.buildBundledRequest(ruleRecIter->second,
                                              ptf,
                                              ruleRecIter->first,
                                              isMinMaxFare,
                                              hasRule,
                                              fbDisplay,
                                              catNumbber);

            if (!haveSubCats)
              break;
          }
          else
          {
            if (rtgController.getRuleText(ruleRecIter->second,
                                          ptf,
                                          *trx.getRequest()->ticketingAgent(),
                                          *trx.billing(),
                                          repository,
                                          ruleRecIter->first,
                                          isMinMaxFare,
                                          catNumbber))
            {
              if (!haveSubCats)
              {
                repository += "\n ";
                repository += "\n    THIS CAT 10 RECORD 2 HAS NO SUB-CATEGORIES";
              }

              LOG4CXX_DEBUG(logger,
                            "Found rule text for category: " << catNumbber << " with ID=" << objId);
              LOG4CXX_DEBUG(logger, "repository=" << repository);
              ruleText += repository;

              if (!haveSubCats)
                break;
            }
            else
            {
              LOG4CXX_WARN(logger,
                           "Rule Text generation failed for category: " << ruleRecIter->first);
              ruleText += "RULE TEXT GENERATOR AT PEAK USE - RETRY 8 SECS";
            }
          }
        }
        continue;
      }
    }

    // Check if RTG requests should be bundled
    if (bundleRtgRequest.getValue())
    {
      rtgController.buildBundledRequest(
          ruleRecIter->second, ptf, ruleRecIter->first, isMinMaxFare, hasRule, fbDisplay);
    }
    else
    {
      if (rtgController.getRuleText(ruleRecIter->second,
                                    ptf,
                                    *trx.getRequest()->ticketingAgent(),
                                    *trx.billing(),
                                    repository,
                                    ruleRecIter->first,
                                    isMinMaxFare))
      {
        LOG4CXX_DEBUG(
            logger, "Found rule text for category: " << ruleRecIter->first << " with ID=" << objId);
        ruleText += repository;
      }
      else
      {
        LOG4CXX_INFO(logger, "Rule Text generation failed for category: " << ruleRecIter->first);
        ruleText += "RULE TEXT GENERATOR AT PEAK USE - RETRY 8 SECS";
      }
    }
  }

  // Check if RTG requests should be bundled
  if (bundleRtgRequest.getValue() && hasRule)
  {
    rtgController.finalizeBundledRequest();

    // Send request to RTG server and get response
    rtgController.sendBundledRequest(repository);
    rtgController.updateFareDisplayRuleTextMap(fbDisplay, haveSubCats, ptf.isFareByRule());
  }

  // For Cat 12, add sector surcharge text if any
  if (!fdi->secSurchargeText().empty() && isRuleCategoryRequested(fdo, 12))
  {
    // If there is cat 12, append to it, otherwise make this the text
    std::map<CatNumber, std::string>::iterator ruleText = fdi->fbDisplay().ruleTextMap().find(12);

    if (ruleText != fdi->fbDisplay().ruleTextMap().end())
    {
      ruleText->second += "\n" + fdi->secSurchargeText();
    }
    else
    {
      // insert this category into the map
      fdi->fbDisplay().ruleTextMap().insert(std::make_pair(12, fdi->secSurchargeText()));
    }
  }
}

bool
FareDisplayController::isRTGNeeded(FareDisplayTrx& trx)
{
  FareDisplayOptions& fdo = *trx.getOptions();
  int catSize = fdo.ruleCategories().size();
  // No need to call RTG when options M, H, RTG or IC are requested
  if ((fdo.ruleMenuDisplay() == TRUE_INDICATOR) || (fdo.headerDisplay() == TRUE_INDICATOR) ||
      (fdo.isRoutingDisplay()) ||
      ((fdo.IntlConstructionDisplay() == TRUE_INDICATOR) && catSize == 1))
    return false;
  else
    return true;
}

bool
FareDisplayController::hasCat16fromBaseFare(const PaxTypeFare& ptf) const try
{
  return (ptf.fareByRuleInfo().ovrdcat16() == 'B' && ptf.fareByRuleInfo().ovrdcat33() == 'X');
}
catch (...)
{
  return false;
}
}
