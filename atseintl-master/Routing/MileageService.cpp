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

#include "Routing/MileageService.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/StatusTrx.h"
#include "DBAccess/DataHandle.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag452Collector.h"
#include "Routing/Collector.h"
#include "Routing/MileageDisplay.h"
#include "Routing/MileageEqualization.h"
#include "Routing/MileageRoute.h"
#include "Routing/MileageRouteBuilder.h"
#include "Routing/MileageUtil.h"
#include "Routing/MPMCollectorWN.h"
#include "Routing/Retriever.h"
#include "Routing/SouthAtlanticTPMExclusion.h"
#include "Routing/TicketedPointDeduction.h"
#include "Routing/TPMCollectorWN.h"
#include "Routing/TPMCollectorWNfromItin.h"
#include "Routing/TPMRetriever.h"
#include "Routing/TPMRetrieverWN.h"
#include "Server/TseServer.h"

namespace tse
{
static LoadableModuleRegister<Service, MileageService>
_("libRouting.so");

static Logger
logger("atseintl.Routing.MileageService");

MileageService::MileageService(const std::string& name, tse::TseServer& server)
  : Service(name, server), _config(Global::config())
{
}

bool
MileageService::initialize(int argc, char* argv[])
{
  return true;
}

bool
MileageService::process(MileageTrx& trx)
{
  LOG4CXX_INFO(logger, "Entering MileageService::process");

  Diag452Collector* diagPtr = nullptr;
  DCFactory* factory = nullptr;
  if (trx.diagnostic().diagnosticType() == Diagnostic452)
  {
    factory = DCFactory::instance();
    diagPtr = dynamic_cast<Diag452Collector*>(factory->create(trx));
    diagPtr->enable(Diagnostic452);
  }
  MileageDisplay mDisplay(trx);
  MileageRoute mileageRoute;
  MileageRouteBuilder mileageRouteBuilder(trx.isFromItin());
  mileageRouteBuilder.buildMileageRoute(trx, mileageRoute, diagPtr);
  if (trx.isFromItin())
  {
    const Collector<TPMCollectorWNfromItin>& tpmCollector(
        tse::Singleton<Collector<TPMCollectorWNfromItin> >::instance());
    if (!tpmCollector.collectMileage(mileageRoute))
      return true;
  }
  else
  {
    const Collector<TPMCollectorWN>& tpmCollector(
        tse::Singleton<Collector<TPMCollectorWN> >::instance());
    if (!tpmCollector.collectMileage(mileageRoute))
    {
      mDisplay.displayGDPrompt(trx, mileageRoute.gdPrompt());
      return true;
    }
  }
  const Collector<MPMCollectorWN>& mpmCollector(
      tse::Singleton<Collector<MPMCollectorWN> >::instance());

  mDisplay.displayMileageRequest(trx);
  mDisplay.displayHeader(trx);

  if (mpmCollector.collectMileage(mileageRoute))
  {
    mileageRoute.globalDirection() = mileageRoute.mileageRouteItems().back().mpmGlobalDirection();
    // apply South Atlantic TPM Exclusion if applicable
    if (getSouthAtlantic(mileageRoute))
    {
      DataHandle& dataHandle(*mileageRoute.dataHandle());
      MileageRouteItems::iterator itr(mileageRoute.mileageRouteItems().begin());
      MileageRouteItems::iterator end(mileageRoute.mileageRouteItems().end());
      MileageRouteItem tempItem;
      for (; itr != end; ++itr)
      {
        if (itr->southAtlanticExclusion() == true)
        {
          tempItem.city1() = itr->origCityOrAirport();
          // tempItem.city1() = itr->city1();
          // tempItem.multiTransportOrigin() = itr->multiTransportOrigin();
          MileageRouteItems::reverse_iterator ritr(itr);
          MileageRouteItems::reverse_iterator sec =
              std::find_if(mileageRoute.mileageRouteItems().rbegin(),
                           ritr - 1,
                           std::mem_fun_ref<bool>(&MileageRouteItem::southAtlanticExclusion));
          if (sec != ritr - 1)
          {
            tempItem.city2() = sec->destCityOrAirport();
            // tempItem.city2() = sec->city2();
            // tempItem.multiTransportDestination() = sec->multiTransportDestination();
            tempItem.globalDirection(TPM) = GlobalDirection::AT;
            tempItem.globalDirection(MPM) = GlobalDirection::AT;
            tempItem.travelDate() = sec->travelDate();
            itr = sec.base() - 1;
            break;
          }
        }
      }
      if ((trx.isFromItin() && getTPM(tempItem, dataHandle)) ||
          (!trx.isFromItin() && getTPM(tempItem, dataHandle, mileageRoute.gdPrompt())))
      {
        itr->tpm() = tempItem.tpm();
        itr->southAtlanticExclusion() = false;
      }
    }
    if (mileageRoute.mileageRouteItems().size() > 1)
    {
      processTPD(mileageRoute);
    }

    mileageRoute.ems() =
        MileageUtil::getEMS(mileageRoute.mileageRouteTPM(), mileageRoute.mileageRouteMPM());

    if (mileageRoute.globalDirection() == GlobalDirection::WH)
    {
      processEqualization(mileageRoute);
    }
    mDisplay.displayMileageRoute(mileageRoute);
    LOG4CXX_INFO(logger, "Leaving MileageService::process : Successful");
  }
  else
  {
    LOG4CXX_INFO(logger, "Leaving MileageService::process : Failed");
  }
  if (diagPtr)
  {
    diagPtr->flushMsg();
  }
  return true;
}

bool
MileageService::getTPM(MileageRouteItem& item, DataHandle& dataHandle, GDPrompt*& gdPrompt) const
{
  const Retriever<TPMRetrieverWN>& tpmRetriever(
      tse::Singleton<Retriever<TPMRetrieverWN> >::instance());
  return tpmRetriever.retrieve(item, dataHandle, gdPrompt);
}

bool
MileageService::getTPM(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const Retriever<TPMRetriever>& tpmRetriever(tse::Singleton<Retriever<TPMRetriever> >::instance());
  return tpmRetriever.retrieve(item, dataHandle);
}

bool
MileageService::getSouthAtlantic(MileageRoute& route) const
{

  const MileageExclusion& southAtlanticTPMExclusion(
      tse::Singleton<SouthAtlanticTPMExclusion>::instance());
  return southAtlanticTPMExclusion.apply(route);
}

bool
MileageService::getTPD(MileageRoute& route) const
{
  const TicketedPointDeduction& tpd(tse::Singleton<TicketedPointDeduction>::instance());
  MileageRouteBuilder mileageRouteBuilder;
  mileageRouteBuilder.buildWNMileageRoute(route);
  return tpd.apply(route);
}

bool
MileageService::processTPD(MileageRoute& route) const
{
  MileageRoute incRoute;

  incRoute.partialInitialize(route);

  MileageRouteItems::iterator itr(route.mileageRouteItems().begin());
  MileageRouteItems::iterator end(route.mileageRouteItems().end());
  uint16_t cumulativeTPM(0), grandTPD(0);
  for (; itr != end; ++itr)
  {
    MileageRouteItem& item(*itr);
    cumulativeTPM += item.tpm();
    incRoute.mileageRouteItems().push_back(item);
    if (item.mpm() > 0 && cumulativeTPM > item.mpm() && getTPD(incRoute))
    {
      item.tpd() = incRoute.tpd();
      if (incRoute.tpd() > grandTPD)
      {
        grandTPD = incRoute.tpd();
      }
    }
    else if (!incRoute.mileageRouteItems().back().condTpdViaGeoLocs().empty())
    {
      item.condTpdViaGeoLocs() = incRoute.mileageRouteItems().back().condTpdViaGeoLocs();

      /// TPD project - check if any of incRoute.mileageRouteItems failed Direct Service reqirement
      MileageRouteItems::iterator it(incRoute.mileageRouteItems().begin());
      MileageRouteItems::iterator itEnd(incRoute.mileageRouteItems().end());
      for (; it != itEnd; ++it)
      {
        MileageRouteItem& incItem(*it);
        if (incItem.failedDirService())
        {
          item.failedDirService() = true;
          break;
        }
      }
    }
  }
  if (grandTPD > 0)
  {
    route.tpd() = grandTPD;
    route.mileageRouteTPM() -= grandTPD;
    return true;
  }
  return false;
}

bool
MileageService::processEqualization(MileageRoute& mRoute) const
{
  if (mRoute.ems() == 0)
    return false;

  const MileageExclusion& equalization(tse::Singleton<MileageEqualization>::instance());
  return equalization.apply(mRoute);
}
}
