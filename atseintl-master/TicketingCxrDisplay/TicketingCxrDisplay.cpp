#include "TicketingCxrDisplay/TicketingCxrDisplay.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/ValidatingCxrUtil.h"
#include "DataModel/TicketingCxrDisplayRequest.h"
#include "DataModel/TicketingCxrDisplayTrx.h"
#include "DBAccess/AirlineInterlineAgreementInfo.h"
#include "DBAccess/DataHandle.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag191Collector.h"
#include "Server/TseServer.h"

namespace tse
{
static Logger
logger("atseintl.TicketingCxrDisplay.TicketingCxrDisplay");

static LoadableModuleRegister<Service, TicketingCxrDisplay>
_("libTicketingCxrDisplayService.so");

TicketingCxrDisplay::TicketingCxrDisplay(const std::string& name, TseServer& srv)
  : Service(name, srv), _config(srv.config())
{
}

bool
TicketingCxrDisplay::initialize(int argc, char* argv[])
{
  return true;
}

bool
TicketingCxrDisplay::process(TicketingCxrDisplayTrx& tcdTrx)
{
  LOG4CXX_DEBUG(logger, "Entering TicketingCxrDisplay::process");
  if (tcdTrx.getRequest())
  {
    DCFactory* factory = DCFactory::instance();
    Diag191Collector* diag191 = dynamic_cast<Diag191Collector*>(factory->create(tcdTrx));
    if (diag191)
    {
      diag191->enable(Diagnostic191);
      diag191->print191Header();
      diag191->printHeaderForDisplayReq(tcdTrx);
    }

    if (!ValidatingCxrUtil::isNationValid(tcdTrx, tcdTrx.getCountry(), diag191))
    {
      tcdTrx.setValidationStatus(vcx::INVALID_COUNTRYCODE_ERR);
    }
    else
    {
      if (vcx::DISPLAY_INTERLINE_AGMT == tcdTrx.getRequest()->getRequestType())
      {
        if (!processDisplayInterline(tcdTrx, diag191))
          tcdTrx.setValidationStatus(vcx::NO_VALID_TKT_AGMT_FOUND);
      }
      else if (vcx::DISPLAY_VCXR == tcdTrx.getRequest()->getRequestType())
      {
        if (!processDisplayValCxr(tcdTrx, diag191))
          tcdTrx.setValidationStatus(vcx::INVALID_SETTLEMENTPLAN_ERR);
      }
      else
      {
        LOG4CXX_ERROR(logger, "TicketingCxrDisplay::process UNNOWN DISPLAY COMMAND");
      }
    }

    if (diag191)
    {
      diag191->print191Footer();
      diag191->flushMsg();
    }
  }
  LOG4CXX_DEBUG(logger, "Leaving TicketingCxrDisplay::process : Successful");
  return true;
}

bool
TicketingCxrDisplay::processDisplayInterline(TicketingCxrDisplayTrx& tcdTrx, Diag191Collector* diag191) const
{
  std::vector<AirlineInterlineAgreementInfo*> interlineAgmts;
  const std::vector<AirlineCountrySettlementPlanInfo*>& acspList =
      tcdTrx.dataHandle().getAirlineCountrySettlementPlans(
          tcdTrx.getPrimeHost(),
          tcdTrx.getCountry(),
          tcdTrx.getValidatingCxr());

  if ( !acspList.empty() ) // The given airline participates for the given POS
  {
    ValidatingCxrUtil::getAirlineInterlineAgreements(
        tcdTrx.dataHandle(),
        tcdTrx.getValidatingCxr(),
        tcdTrx.getCountry(),
        tcdTrx.getPrimeHost(),
        interlineAgmts);
  }

  if (interlineAgmts.empty())
  {
    if (diag191)
      *diag191 << "\nNO INTERLINE AGMTS FOUND IN DATABASE\n";
    return false;
  }
  else
  {
    TicketingCxrDisplayResponse& resp = tcdTrx.getResponse();
    resp.primeHost()=tcdTrx.getPrimeHost();
    resp.country()=tcdTrx.getCountry();
    resp.validatingCxr()=tcdTrx.getValidatingCxr();
    resp.setInterlineAgreements(interlineAgmts);
  }

  return true;
}

bool
TicketingCxrDisplay::processDisplayValCxr(TicketingCxrDisplayTrx& tcdTrx, Diag191Collector* diag191) const
{
  if (tcdTrx.getRequest())
  {
    TicketingCxrDisplayResponse& resp = tcdTrx.getResponse();
    resp.primeHost() = tcdTrx.getPrimeHost();
    resp.country() = tcdTrx.getCountry();
    const std::vector<CountrySettlementPlanInfo*>& cspList =
      tcdTrx.dataHandle().getCountrySettlementPlans(tcdTrx.getCountry());

    if (cspList.empty())
    {
      if (diag191) *diag191<<"NO SETTLEMENT PLAN FOR COUNTRY "<< tcdTrx.getCountry();
      return false;
    }

    const SettlementPlanType requestedSp = tcdTrx.getRequest()->settlementPlan();
    if (!requestedSp.empty())
    {
      const CountrySettlementPlanInfo* csp =
        getRequestedSettlementPlanInfo(requestedSp, cspList);
      if (!csp)
      {
        if (diag191)
          *diag191<<requestedSp<<" NOT FOUND IN THE COUNTRY "<<tcdTrx.getCountry();
        return false;
      }

      processDisplayValCxr(tcdTrx, diag191, csp);
    }
    else
    {
      // Display validating carriers for all settlement plan in a specified country
      for (CountrySettlementPlanInfo* csp : cspList)
      {
        processDisplayValCxr(tcdTrx, diag191, csp);
      }
    }
  }
  return true;
}

void
TicketingCxrDisplay::processDisplayValCxr(TicketingCxrDisplayTrx& tcdTrx,
    Diag191Collector* diag191,
    const CountrySettlementPlanInfo* csp) const
{
  processDisplayValidatingCxrList(tcdTrx, csp, diag191);
  processDisplayGeneralSalesAgent(tcdTrx, csp->getSettlementPlanTypeCode(), diag191);
  processDisplayNeutralValCxr(tcdTrx, csp->getSettlementPlanTypeCode(), diag191);
}

CountrySettlementPlanInfo*
TicketingCxrDisplay::getRequestedSettlementPlanInfo(const SettlementPlanType& reqSp,
    const std::vector<CountrySettlementPlanInfo*>& cspList) const
{
  for (CountrySettlementPlanInfo* csp : cspList)
  {
    if (reqSp == csp->getSettlementPlanTypeCode())
      return csp;
  }
  return nullptr;;
}

bool
TicketingCxrDisplay::processDisplayValidatingCxrList(TicketingCxrDisplayTrx& tcdTrx,
                                                     const CountrySettlementPlanInfo* csp,
                                                     Diag191Collector* diag191) const
{
  const SettlementPlanType spType = csp->getSettlementPlanTypeCode();

  TicketingCxrDisplayResponse& resp = tcdTrx.getResponse();
  const std::vector<AirlineCountrySettlementPlanInfo*>& acspList =
      tcdTrx.dataHandle().getAirlineCountrySettlementPlans(tcdTrx.getCountry(),
                                                           tcdTrx.getPrimeHost(),
                                                           spType);
  if (acspList.empty())
  {
    if (diag191)
    {
      *diag191 << "\n NO VALIDATING CARRIER FOUND FOR SETTLEMENT PLAN: ";
      *diag191 << spType.c_str();
      *diag191 << " \n";
    }
    return false;
  }
  else
  {
    resp.validatingCxrDisplayMap()[spType].setValidatingCxrMap(acspList, csp);
  }
  return true;
}


bool
TicketingCxrDisplay::processDisplayGeneralSalesAgent( TicketingCxrDisplayTrx& tcdTrx,
                                                      const SettlementPlanType& spType,
                                                      Diag191Collector* diag191) const
{
    TicketingCxrDisplayResponse& resp = tcdTrx.getResponse();
    const std::vector<GenSalesAgentInfo*>& gsa =
    tcdTrx.dataHandle().getGenSalesAgents(tcdTrx.getPrimeHost(),
                                          tcdTrx.getCountry(),
                                          spType);
    if (gsa.empty())
    {
      if (diag191)
      {
        *diag191 << "\n NO GENERAL SALES AGENT FOUND FOR GDS/COUNTRY/SETTLEMENT PLAN: ";
        *diag191 << tcdTrx.getPrimeHost() << "/" << tcdTrx.getCountry() << "/" << spType;
        *diag191 << " \n";
      }
      return false;
    }
    else
    {
      resp.validatingCxrDisplayMap()[spType].setGeneralSalesAgentMap(gsa);
    }
    return true;
}

bool
TicketingCxrDisplay::processDisplayNeutralValCxr(TicketingCxrDisplayTrx& tcdTrx, const SettlementPlanType& spType, Diag191Collector* diag191) const
{

    TicketingCxrDisplayResponse& resp = tcdTrx.getResponse();
    const std::vector<NeutralValidatingAirlineInfo*>&  nv =
    tcdTrx.dataHandle().getNeutralValidatingAirlines( tcdTrx.getCountry(),
                                                      tcdTrx.getPrimeHost(),
                                                      spType);
    if (nv.empty())
    {
      if (diag191)
       {
        *diag191 << "\n NO NEUTRAL VALIDATING CARRIER FOUND FOR SETTLEMENT PLAN: ";
        *diag191 << spType.c_str();
        *diag191 << " \n";
      }
      return false;
    }
    else
    {
      resp.validatingCxrDisplayMap()[spType].setNeutralValidatingCxr(nv);
    }
    return true;
}
}
