//-------------------------------------------------------------------
//
//  File:        TicketingCxrService.cpp
//  Created:     November 2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "TicketingCxrService/TicketingCxrService.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/ValidatingCxrUtil.h"
#include "DataModel/TicketingCxrTrx.h"
#include "DBAccess/GenSalesAgentInfo.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/Customer.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag191Collector.h"
#include "Server/TseServer.h"

#include <algorithm>
#include <string>

namespace tse
{

static Logger
logger("atseintl.TicketingCxrService.TicketingCxrService");

static LoadableModuleRegister<Service, TicketingCxrService>
_("libTicketingCxrService.so");

TicketingCxrService::TicketingCxrService(const std::string& name, TseServer& srv)
  : Service(name, srv), _config(srv.config())
{
}

bool
TicketingCxrService::initialize(int argc, char* argv[])
{
  return true;
}

bool
TicketingCxrService::process(TicketingCxrTrx& tcsTrx)
{
  LOG4CXX_DEBUG(logger, "Entering TicketingCxrService::process");
  if (tcsTrx.getRequest())
  {
    const PseudoCityCode posPcc = tcsTrx.getRequest()->getPcc();
    const bool isPosUS = "US" == tcsTrx.getRequest()->getPosCountry();
    const bool isSpecifiedCountry =
        !tcsTrx.getRequest()->getSpecifiedCountry().empty();
    const bool checkArcUser = !isPosUS && !isSpecifiedCountry && !posPcc.empty();
    if (checkArcUser)
    {
      const std::vector<Customer*> customerList =
          tcsTrx.dataHandle().getCustomer(posPcc);
      if (!customerList.empty() && customerList.front()->isArcUser())
      {
        tcsTrx.getRequest()->setPosCountry("US");
        tcsTrx.getRequest()->setArcUser(true);
      }
    }

    DCFactory* factory = DCFactory::instance();
    Diag191Collector* diag191 = dynamic_cast<Diag191Collector*>(factory->create(tcsTrx));

    if (diag191)
    {
      diag191->enable(Diagnostic191);
      diag191->print191Header();
      diag191->printHeaderForPlausibilityReq(tcsTrx);
    }

    if (!ValidatingCxrUtil::isNationValid(tcsTrx, tcsTrx.getCountry(), diag191))
    {
      tcsTrx.setValidationResult(vcx::NOT_VALID);
      tcsTrx.setValidationStatus(vcx::INVALID_COUNTRYCODE_ERR);
      if (diag191)
        *diag191 << "INVALID COUNTRY CODE" << std::endl;
    }
    else
    {
      SettlementPlanType sp(tcsTrx.getRequest()->getSettlementPlan());
      CountrySettlementPlanInfo* spInfo = checkCountrySettlementPlan(tcsTrx, diag191, sp);
      if (!spInfo)
      {
        tcsTrx.setValidationResult(vcx::NOT_VALID);
        tcsTrx.setValidationStatus(vcx::INVALID_SETTLEMENTPLAN_ERR);
        if (diag191)
          *diag191 << "INVALID SETTLEMENT PLAN" << std::endl;
      }
      else
      {
        tcsTrx.ctrySettlementInfo() = spInfo;
        if (vcx::SETTLEMENTPLAN_CHECK == tcsTrx.getRequest()->getRequestType())
        {
          if (!processCxrSettlementPlan(tcsTrx, diag191))
            LOG4CXX_DEBUG(logger, "AREX Settlement Check Failed");
        }
        else if (vcx::PLAUSIBILITY_CHECK == tcsTrx.getRequest()->getRequestType())
        {
          if (!processPlausibilityCheck(tcsTrx, diag191))
            LOG4CXX_DEBUG(logger, "Plausibility Check Failed");
        }
        else
        {
          if (diag191)
            *diag191 << "SYSTEM ERROR" << std::endl;
          throw ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
        }
      }
    }

    if (diag191)
    {
      diag191->printPlausibilityResult(tcsTrx);
      diag191->print191Footer();
      diag191->flushMsg();
    }
  }
  LOG4CXX_DEBUG(logger, "Leaving TicketingCxrService::process : Successful");
  return true;
}

bool
TicketingCxrService::processCxrSettlementPlan(TicketingCxrTrx& tcsTrx,
    Diag191Collector* diag191) const
{
  LOG4CXX_DEBUG(logger, "Entering TicketingCxrService::processCxrSettlementPlan");
  vcx::ValidationStatus vstatus =
    ValidatingCxrUtil::getAirlineCountrySettlementPlanInfo(
        *tcsTrx.ctrySettlementInfo(),
        diag191,
        tcsTrx.getRequest()->getValidatingCxr(),
        tcsTrx.getRequest()->getMultiHost(),
        tcsTrx.dataHandle()) ?
    vcx::VALID_MSG :
    vcx::CXR_DOESNOT_PARTICIPATES_IN_SETTLEMENTPLAN;

  tcsTrx.setValidationStatus(vstatus);
  tcsTrx.setValidationResult(vcx::VALID_MSG==vstatus ? vcx::VALID : vcx::NOT_VALID);
  if (vcx::VALID==tcsTrx.getValidationResult())
    tcsTrx.validatingCxrs().push_back(tcsTrx.getRequest()->getValidatingCxr());
  return tcsTrx.isResultValid();
}

// This method process standalone/plausibility request
bool
TicketingCxrService::processPlausibilityCheck(TicketingCxrTrx& tcsTrx, Diag191Collector* diag191) const
{
  LOG4CXX_DEBUG(logger, "Entering TicketingCxrService::processPlausibilityCheck");

  vcx::ValidationStatus vstatus = vcx::NO_MSG;
  vcx::Pos pos(tcsTrx.getCountry(), tcsTrx.getRequest()->getMultiHost());

  tcsTrx.vcxrData().participatingCxrs = tcsTrx.getRequest()->participatingCxrs();
  tcsTrx.vcxrData().ticketType = vcx::ETKT_PREF; // default
  const CarrierCode valCxr(tcsTrx.getRequest()->getValidatingCxr());

  vstatus = checkForValCxr(tcsTrx, diag191, pos, valCxr);
  if (vcx::CXR_DOESNOT_PARTICIPATES_IN_SETTLEMENTPLAN == vstatus)
  {
    vstatus = checkForValCxrAmongGSA(tcsTrx, diag191, pos);
    if (vcx::CXR_DOESNOT_PARTICIPATES_IN_SETTLEMENTPLAN == vstatus)
    {
      tcsTrx.setGTCCarrier(
          ValidatingCxrUtil::isGTCCarrier(
            tcsTrx.dataHandle(),
            diag191,
            tcsTrx.getCountry(),
            pos.primeHost,
            valCxr)
          );
    }
  }

  tcsTrx.setValidationResult(tcsTrx.getResultType(vstatus));
  tcsTrx.setValidationStatus(vstatus);
  return tcsTrx.isResultValid();
}

// Check whether a settlement plan exist in the country of POS
CountrySettlementPlanInfo*
TicketingCxrService::checkCountrySettlementPlan(TicketingCxrTrx& tcsTrx,
                                                Diag191Collector* diag191,
                                                SettlementPlanType& sp) const
{
  LOG4CXX_DEBUG(logger, "Entering TicketingCxrService::checkCountrySettlementPlan");
  CountrySettlementPlanInfo* spInfo = nullptr;
  try { spInfo = ValidatingCxrUtil::determineCountrySettlementPlan(tcsTrx, diag191, &sp); }
  catch (...)
  {
    LOG4CXX_DEBUG(logger,
                  "EXCEPTION: " << sp << " DOEST EXISTS IN COUNTRY " << tcsTrx.getCountry());
  }

  if (!spInfo)
  {
    if (diag191)
    {
      *diag191 << "\nSETTLEMENT PLAN INFO\n";
      *diag191 << sp << " DOEST EXISTS IN COUNTRY " << tcsTrx.getCountry()
        << std::endl;
    }
  }
  return spInfo;
}

/*
 * Check participation in STLMNT PLAN and interline agreement
 * A carrier is a validating cxr if it participates in the settlement
 * of POS's country and has interline agreement with participating carriers.
 */
vcx::ValidationStatus
TicketingCxrService::checkForValCxr(TicketingCxrTrx& tcsTrx,
                                    Diag191Collector* diag191,
                                    const vcx::Pos& pos,
                                    const CarrierCode& cxr) const
{
  LOG4CXX_DEBUG(logger, "Entering TicketingCxrService::checkForValCxr");
  vcx::ValidationStatus vstatus = vcx::CXR_DOESNOT_PARTICIPATES_IN_SETTLEMENTPLAN;

  AirlineCountrySettlementPlanInfo* cxrSpInfo =
    ValidatingCxrUtil::getAirlineCountrySettlementPlanInfo(
        *tcsTrx.ctrySettlementInfo(),
        diag191,
        cxr,
        pos.primeHost,
        tcsTrx.dataHandle());

  if (cxrSpInfo) //cxr participates in stlm plan
  {
    tcsTrx.vcxrData().ticketType =
      ValidatingCxrUtil::getTicketingMethod(*tcsTrx.ctrySettlementInfo(), cxrSpInfo);

    if (ValidatingCxrUtil::isPaperTicketOverrideConflict(tcsTrx.vcxrData().ticketType,
          tcsTrx.getRequest()->getTicketType()))
    {
      // agent wants paper but etkt required
      vstatus = vcx::PAPER_TKT_OVERRIDE_ERR;
      if (diag191)
        diag191->printPaperConflict(tcsTrx);
    }
    else if (tcsTrx.isSkipInterlineCheck() /* skip self check */ ||
        ValidatingCxrUtil::validateInterlineAgreements(tcsTrx.dataHandle(),
          diag191,
          pos,
          cxr,
          tcsTrx.vcxrData(),
          isCxrInItin(cxr, tcsTrx.vcxrData().participatingCxrs),
          tcsTrx.getRequest()->getTicketType()))
    {
      vstatus = vcx::VALID_OVERRIDE;
      tcsTrx.setTicketType(tcsTrx.vcxrData().ticketType);
      tcsTrx.validatingCxrs().push_back(cxr);
    }
    else
    {
      // no agmt found
      vstatus = tcsTrx.vcxrData().interlineStatusCode;
    }
  }
  return vstatus;
}

/*
 * Check GSA (Gen Sales Agents), its participation in STLMNT PLAN and interline agreements
 * General Sales Agents are carrier who represent another carrier in a
 * settlement method. For exampel KL is GSA of DL in BSP settlement method.
 */
vcx::ValidationStatus
TicketingCxrService::checkForValCxrAmongGSA(TicketingCxrTrx& tcsTrx,
                                            Diag191Collector* diag191,
                                            const vcx::Pos& pos) const
{
  LOG4CXX_DEBUG(logger, "Entering TicketingCxrService::checkForValCxrAmongGSA");
  // const CarrierCode& cxr = tcsTrx.getRequest()->getValidatingCxr();       unused variable
  vcx::ValidationStatus vstatus = vcx::CXR_DOESNOT_PARTICIPATES_IN_SETTLEMENTPLAN;
  std::set<CarrierCode> gsaList;
  ValidatingCxrUtil::getGenSalesAgents(tcsTrx.dataHandle(),
      diag191,
      tcsTrx.getRequest()->getValidatingCxr(),
      pos.country,
      pos.primeHost,
      tcsTrx.ctrySettlementInfo()->getSettlementPlanTypeCode(),
      gsaList);

  if (!gsaList.empty())
  {
    std::vector<vcx::ValidationStatus> statusList;
    statusList.reserve(gsaList.size());

    for (const CarrierCode& gsa : gsaList)
    {
      vstatus = checkForValCxr(tcsTrx, diag191, pos, gsa);
      statusList.push_back(vstatus);
      if (vcx::VALID_OVERRIDE != vstatus)
        continue; //next GSA
      tcsTrx.setGsaSwap(true);
    }

    vstatus = determineValidationStatus(tcsTrx, vstatus, statusList);
  }
  return vstatus;
}

bool
TicketingCxrService::isCxrInItin(const CarrierCode& valCxr,
    const std::vector<vcx::ParticipatingCxr>& cxrs) const
{
  for (const vcx::ParticipatingCxr& cxr : cxrs)
  {
    if (strncmp(valCxr.c_str(), cxr.cxrName.c_str(), 2) == 0)
      return true;
  }
  return false;
}

// In case of  multiple GSAs, one may fail due to paper conflict but other
// fails due to other reasons. When user requeted paper and there is paper conflict,
// we should return paper error.
vcx::ValidationStatus
TicketingCxrService::determineValidationStatus(const TicketingCxrTrx& tcsTrx,
    const vcx::ValidationStatus& vstatus,
    const std::vector<vcx::ValidationStatus>& slist) const
{
  if (!tcsTrx.validatingCxrs().empty())
    return vcx::VALID_MSG;

  if (std::find(slist.begin(), slist.end(), vcx::PAPER_TKT_OVERRIDE_ERR) != slist.end())
    return vcx::PAPER_TKT_OVERRIDE_ERR;

  return vcx::NO_VALID_TKT_AGMT_FOUND;
}
}
