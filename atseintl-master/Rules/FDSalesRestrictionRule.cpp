//-------------------------------------------------------------------
//
//  File:        FDSalesRestrictionRule.cpp
//
//  Authors:     Marco cartolano, Lipika Bardalai
//  Created:     March 10, 2005
//  Description: Validate method for Fare Display
//
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
//------------------------------------------------------------------

#include "Rules/FDSalesRestrictionRule.h"

#include "Common/DiagMonitor.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/SalesRestriction.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag315Collector.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

namespace tse
{

static Logger
logger("atseintl.Rules.FDSalesRestrictionRule");

//----------------------------------------------------------------------------
// validate()     Fare component scope
//----------------------------------------------------------------------------
Record3ReturnTypes
FDSalesRestrictionRule::validate(PricingTrx& trx,
                                 Itin& itin,
                                 FareUsage* fU,
                                 PaxTypeFare& paxTypeFare,
                                 const CategoryRuleInfo& cri,
                                 const CategoryRuleItemInfo* rule,
                                 const SalesRestriction* salesRestrictionRule,
                                 bool isQualifiedCategory,
                                 bool& isCat15Security,
                                 bool skipCat15Security)
{
  LOG4CXX_INFO(logger, " Entered FDSalesRestrictionRule::validate()");

  Record3ReturnTypes retCode = PASS;
  Cat15FailReasons failReason = SALES_RESTR_NO_ERROR;

  //--------------------------------------------------------------
  // Get a Fare Display Transaction from the Pricing Transaction
  //--------------------------------------------------------------
  FareDisplayUtil fdUtil;
  FareDisplayTrx* fdTrx;

  if (!fdUtil.getFareDisplayTrx(&trx, fdTrx))
  {
    LOG4CXX_DEBUG(logger, "Unable to get FareDisplayTrx");
    LOG4CXX_INFO(logger, " Leaving FDSalesRestrictionRule::validate() - FAIL");

    return FAIL;
  }

  //-------------------------------------------------------------------
  // Get Fare Display Info object
  //-------------------------------------------------------------------
  FareDisplayInfo* fdInfo = paxTypeFare.fareDisplayInfo();

  if (!fdInfo)
  {
    LOG4CXX_ERROR(logger, "Unable to get FareDisplayInfo object");
    LOG4CXX_INFO(logger, " Leaving FDSalesRestrictionRule::validate() - FAIL");

    return FAIL;
  }

  //--------------------------------------------------------------
  // Check if data is unavailable
  //--------------------------------------------------------------
  if (salesRestrictionRule->unavailTag() == RuleConst::DATA_UNAVAILABLE)
  {
    //-------------------------------------------------------------------
    // Update FareDisplayInfo object: Unavailable rule data
    //-------------------------------------------------------------------
    LOG4CXX_INFO(logger, " Updating FareDisplayInfo object");

    fdInfo->setUnavailableR3Rule(RuleConst::SALE_RESTRICTIONS_RULE);

    LOG4CXX_INFO(logger, " Leaving FDAdvanceResTkt::validate() - NOTPROCESSED");

    retCode = NOTPROCESSED;
    failReason = SALES_RESTR_DATA_UNAVAILABLE;
    updateDiagnostic315(
        trx, paxTypeFare, cri, rule, salesRestrictionRule, retCode, failReason, false);
    return retCode;
  }

  //-------------------------------------------------------------------
  // Update FareDisplayInfo object with Sales Restrictions information
  //-------------------------------------------------------------------
  if (!isQualifiedCategory)
  {
    LOG4CXX_INFO(logger, " Updating FareDisplayInfo object");
    updateFareDisplayInfo(salesRestrictionRule, fdInfo, TrxUtil::getTicketingDT(trx));
  }

  //---------------------------------------------------------------------
  // invoke base class protected methods
  //---------------------------------------------------------------------
  bool failedSabre = false;

  // The security portion of category 15 will not be validated when both Category 35 and Category 15
  //(Fare Rule, General Rule, and/or Footnote) are present as a Main categories.
  // Base fare of Cat 25 fare with DISC 'L/C/T' will have skipCat15Security indicator on.

  bool skipCat15SecurityCheck = false;

  if ((paxTypeFare.isNegotiated() && !isQualifiedCategory) || // Cat 35 fare with Main 15  OR
      (skipCat15Security && !isQualifiedCategory)) // Base fare of Cat 25/Cat 35 fare with Main 15
  {
    skipCat15SecurityCheck = true;
  }

  // For Cat 25 fares - when Keywords are set in PSS - bypass Security check
  if (paxTypeFare.isFareByRule() && fdTrx->getOptions()->isViewPrivateFares())
  {
    skipCat15SecurityCheck = true;
  }

  if (salesRestrictionRule->unavailTag() == RuleConst::TEXT_ONLY)
  {
    retCode = SKIP;
    failReason = SALES_RESTR_TEXT_ONLY;
    updateDiagnostic315(trx,
                        paxTypeFare,
                        cri,
                        rule,
                        salesRestrictionRule,
                        retCode,
                        failReason,
                        skipCat15SecurityCheck);
    return retCode;
  }
  else if (!skipCat15SecurityCheck)
  {
    if (!isPropperGDS(salesRestrictionRule))
    {
      // Check if this is not a qualified cat 15
      //  (cat 15 off another category - if relation)
      // if (( isQualifiedCategory ) || (paxTypeFare.fare()->isCat15GeneralRuleProcess())) -- change
      // for SPR 97177
      if (paxTypeFare.fare()->isCat15GeneralRuleProcess())
      {
        retCode = SKIP;
        failReason = SALES_RESTR_SKIP; // skip this cat15
        updateDiagnostic315(trx,
                            paxTypeFare,
                            cri,
                            rule,
                            salesRestrictionRule,
                            retCode,
                            failReason,
                            skipCat15SecurityCheck);
        return retCode;
      }
      else
      {
        retCode = FAIL; // fail this cat15
        failReason = SALES_RESTR_CRS_OTHER_CARRIER;
        updateDiagnostic315(trx,
                            paxTypeFare,
                            cri,
                            rule,
                            salesRestrictionRule,
                            retCode,
                            failReason,
                            skipCat15SecurityCheck);
        return retCode;
      }
    }
  }

  // check carrier restrictions for the private tariff
  // skip this check for Cat 35 fare when these fields are in main Cat 15.

  if (paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF // Private fare
      && // and
      !skipCat15SecurityCheck // Not skip Cat 15 security portion
      && // and
      rule->relationalInd() != CategoryRuleItemInfo::AND) // Should be THEN/OR string
  {
    // Check for security data
    if (salesRestrictionRule->carrierCrsInd() == RuleConst::BLANK &&
        salesRestrictionRule->tvlAgentSaleInd() == RuleConst::BLANK &&
        salesRestrictionRule->locales().size() == 0)
    {
      if (typeid(cri) != typeid(FootNoteCtrlInfo) && !isQualifiedCategory)
      {
        if (!paxTypeFare.fare()->isCat15GeneralRuleProcess() ||
            !paxTypeFare.fare()->cat15HasSecurity())
        {
          retCode = FAIL;
          failReason = SALES_RESTR_PRIVATE_SECURITY;
          updateDiagnostic315(trx,
                              paxTypeFare,
                              cri,
                              rule,
                              salesRestrictionRule,
                              retCode,
                              failReason,
                              skipCat15SecurityCheck);
          return retCode;
        }
      }
    }
    else
    {
      isCat15Security = !isQualifiedCategory;
    }
  }

  // Check for security data in Main Cat 15
  if (!isQualifiedCategory && (salesRestrictionRule->carrierCrsInd() != RuleConst::BLANK ||
                               salesRestrictionRule->tvlAgentSaleInd() != RuleConst::BLANK ||
                               salesRestrictionRule->locales().size() > 0))
  {
    paxTypeFare.fare()->setCat15HasSecurity(true);
    // save SecurityInCat15 for 'REUSE of cat15 Footnote/fareRule/genRule'
    if (typeid(cri) == typeid(FootNoteCtrlInfo))
      paxTypeFare.fare()->setSecurityInCat15Fn();
    else if (paxTypeFare.fare()->isCat15GeneralRuleProcess())
      paxTypeFare.fare()->setSecurityInCat15Gr();
    else
      paxTypeFare.fare()->setSecurityInCat15Fr();
  }

  if (salesRestrictionRule->overrideDateTblItemNo() != 0)
  {
    DiagMonitor diagMonitor(trx, Diagnostic315);

    if (!RuleUtil::validateDateOverrideRuleItem(trx,
                                                salesRestrictionRule->overrideDateTblItemNo(),
                                                paxTypeFare.vendor(),
                                                trx.travelDate(),
                                                trx.getRequest()->ticketingDT(),
                                                trx.bookingDate(),
                                                &diagMonitor.diag(),
                                                Diagnostic315))
    {
      retCode = SKIP;
      failReason = SALES_RESTR_DATE_OVERRIDE;
      updateDiagnostic315(trx,
                          paxTypeFare,
                          cri,
                          rule,
                          salesRestrictionRule,
                          retCode,
                          failReason,
                          skipCat15SecurityCheck);
      return retCode;
    }
  }

  //  if (!checkCountryRestriction(trx, itin, fU, paxTypeFare, salesRestrictionRule, softPass))

  if (paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF &&
      !checkCountryRestriction(trx, fU, paxTypeFare, salesRestrictionRule))
  {
    retCode = FAIL;
    failReason = SALES_RESTR_COUNTRY;
    updateDiagnostic315(trx,
                        paxTypeFare,
                        cri,
                        rule,
                        salesRestrictionRule,
                        retCode,
                        failReason,
                        skipCat15SecurityCheck);
    return retCode;
  }

  if (!checkSalesDate(trx, paxTypeFare, salesRestrictionRule))
  {
    retCode = FAIL;
    LOG4CXX_INFO(logger, "Leaving SalesRestrictionRule::checkSalesDate() - FAIL");
    failReason = SALES_RESTR_SALES_DATE;
    updateDiagnostic315(trx,
                        paxTypeFare,
                        cri,
                        rule,
                        salesRestrictionRule,
                        retCode,
                        failReason,
                        skipCat15SecurityCheck);
    return retCode;
  }

  if (!checkFormOfPayment(trx, paxTypeFare, salesRestrictionRule, fU))
  {
    retCode = FAIL;
    failReason = SALES_RESTR_FORM_OF_PAYMENT;
    updateDiagnostic315(trx,
                        paxTypeFare,
                        cri,
                        rule,
                        salesRestrictionRule,
                        retCode,
                        failReason,
                        skipCat15SecurityCheck);
    return retCode;
  }

  // Bypass checking currency restriction whenever #HR is requested
  // The #HR qualifier forces to NUC currency
  if (!fdTrx->getOptions()->halfRoundTripFare())
  {
    if (!checkCurrencyRestriction(trx, fU, paxTypeFare, salesRestrictionRule))
    {
      retCode = FAIL;
      failReason = SALES_RESTR_CURRENCY;
      updateDiagnostic315(trx,
                          paxTypeFare,
                          cri,
                          rule,
                          salesRestrictionRule,
                          retCode,
                          failReason,
                          skipCat15SecurityCheck);
      return retCode;
    }
  }

  if (!tse::checkTicketRestriction(*this, trx, nullptr, paxTypeFare, salesRestrictionRule, cri))
  {
    retCode = FAIL;
    failReason = SALES_RESTR_TICKET;
    updateDiagnostic315(trx,
                        paxTypeFare,
                        cri,
                        rule,
                        salesRestrictionRule,
                        retCode,
                        failReason,
                        skipCat15SecurityCheck);
    return retCode;
  }

  bool validateLocale = false;

  bool isPrivateTariff = (paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF);
  if (!skipCat15SecurityCheck && paxTypeFare.tcrTariffCat() != RuleConst::PRIVATE_TARIFF &&
      paxTypeFare.cat25Fare() &&
      paxTypeFare.cat25Fare()->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
    isPrivateTariff = true;

  if (isPrivateTariff || cri.categoryNumber() == RuleConst::SURCHARGE_RULE ||
      (fdTrx->getOptions()->validateLocaleForPublFares() == 'Y'))
  {
    validateLocale = true;
  }

  // Skip Cat 15 Security portion check in main Cat 15 for Cat 35 fare

  if (!skipCat15SecurityCheck)
  {
    // Check Locale part first
    if (validateLocale)
    {
      if (!tse::checkLocaleItems(*this, trx, fU, paxTypeFare, cri, salesRestrictionRule, false))
      {
        retCode = FAIL;
        failReason = SALES_RESTR_LOCALE;
        updateDiagnostic315(trx,
                            paxTypeFare,
                            cri,
                            rule,
                            salesRestrictionRule,
                            retCode,
                            failReason,
                            skipCat15SecurityCheck);
        paxTypeFare.setCat15SecurityFail();
        return retCode;
      }
      validateLocale = false;
    }

    if (!checkSecurityRestriction(*this,
                                  trx,
                                  fU,
                                  paxTypeFare,
                                  cri,
                                  rule,
                                  salesRestrictionRule,
                                  failedSabre,
                                  true,
                                  isQualifiedCategory,
                                  validateLocale))
    {
      // change for SPR 97177 - qualified If 15 not 1S fail instead of skip
      if (failedSabre)
      {
        failReason = SALES_RESTR_CRS_OTHER_CARRIER; // SPR 97177 - changed the fail reason
      }
      else
      {
        failReason = SALES_RESTR_SECURITY; // fail it
        paxTypeFare.setCat15SecurityFail();
      }

      retCode = FAIL;
      updateDiagnostic315(trx,
                          paxTypeFare,
                          cri,
                          rule,
                          salesRestrictionRule,
                          retCode,
                          failReason,
                          skipCat15SecurityCheck);

      return retCode;
    }
  }

  updateDiagnostic315(trx,
                      paxTypeFare,
                      cri,
                      rule,
                      salesRestrictionRule,
                      retCode,
                      failReason,
                      skipCat15SecurityCheck);

  LOG4CXX_INFO(logger, " Leaving FDSalesRestrictionRule::validate() - PASS ");

  return retCode;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    FDSalesRestrictionRule::updateDiagnostic315()
//
// updateFareDisplayInfo()  update Diagnostic 315 object with Sales
//                      Restrictions information
//
//  @param PaxTypeFare&        - reference to Pax type fare

//  @param CategoryRuleInfo&   - reference to Category Rule Info object
//  @param CategoryRuleItemInfo*- pointer to Category rule Item info
//  @param SalesRestriction*- pointer to Sales Restriction class
//  @param Record3ReturnTypes - return type (status of the rule)
//  @param uint16_t           - fail code
//
// </PRE>
//-------------------------------------------------------------------
void
FDSalesRestrictionRule::updateDiagnostic315(PricingTrx& trx,
                                            const PaxTypeFare& paxTypeFare,
                                            const CategoryRuleInfo& cri,
                                            const CategoryRuleItemInfo* rule,
                                            const SalesRestriction* salesRestrictionRule,
                                            Record3ReturnTypes retCode,
                                            Cat15FailReasons failReason,
                                            bool skipCat15SecurityCheck)
{
  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic315))
  {
    const std::string& diagFareClass =
        trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);
    if (diagFareClass.empty() ||
        RuleUtil::matchFareClass(diagFareClass.c_str(), paxTypeFare.fareClass().c_str()))
    {

      if (DiagnosticUtil::isvalidForCarrierDiagReq(trx, paxTypeFare) ||
          (paxTypeFare.fare()->cat15HasSecurity() && retCode != FAIL && !skipCat15SecurityCheck))
      {
        DCFactory* factory = DCFactory::instance();
        Diag315Collector* diag = dynamic_cast<Diag315Collector*>(factory->create(trx));
        diag->enable(Diagnostic315);
        diag->diag315Collector<CategoryRuleInfo, FDSalesRestrictionRule>(paxTypeFare, cri, rule, salesRestrictionRule, retCode, failReason);

        diag->flushMsg();
      }
    }
  }
  return;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    FDSalesRestrictionRule::updateFareDisplayInfo()
//
// updateFareDisplayInfo()  update FareDisplayInfo object with Sales
//                      Restrictions information
//
//  @param SalesRestriction*&  - reference to Sales Restrictions rule ptr
//  @param FareDisplayInfo*&   - reference to Fare Display Info ptr
//  @param DateTime&           - reference to ticketing date
//
// </PRE>
//-------------------------------------------------------------------
void
FDSalesRestrictionRule::updateFareDisplayInfo(const SalesRestriction*& salesRestrictionRule,
                                              FareDisplayInfo*& fdInfo,
                                              const DateTime& ticketingDate) const
{
  // Do not update the FareDisplayInfo object with past dates
  DateTime earliestTktDate = (salesRestrictionRule->earliestTktDate().date() < ticketingDate.date())
                                 ? DateTime::emptyDate()
                                 : salesRestrictionRule->earliestTktDate();

  DateTime latestTktDate = (salesRestrictionRule->latestTktDate().date() < ticketingDate.date())
                               ? DateTime::emptyDate()
                               : salesRestrictionRule->latestTktDate();

  // Check for valid dates
  if (earliestTktDate.isValid() || latestTktDate.isValid())
  {
    fdInfo->addTicketInfo(earliestTktDate, latestTktDate);
  }
}

// FD doesn't hide fares based on XETR in request
bool
FDSalesRestrictionRule::checkTicketElectronic(bool reqEtkt, Indicator canEtkt) const
{
  return true;
}
}
