#include "Routing/MileageValidator.h"

#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/Agent.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "Routing/Collector.h"
#include "Routing/FareCalcVisitor.h"
#include "Routing/GlobalDirectionRetriever.h"
#include "Routing/MileageEqualization.h"
#include "Routing/MileageExclusion.h"
#include "Routing/MileageRoute.h"
#include "Routing/MileageRouteBuilder.h"
#include "Routing/MileageRouteItem.h"
#include "Routing/MileageUtil.h"
#include "Routing/MPMCollectorMV.h"
#include "Routing/PermissibleSpecifiedRouting.h"
#include "Routing/Retriever.h"
#include "Routing/RoutingInfo.h"
#include "Routing/TicketedPointDeduction.h"
#include "Routing/TPMCollectorMV.h"
#include "Routing/TPMRetriever.h"

#include <numeric>

namespace tse
{

static Logger
logger("atseintl.Routing.MileageValidator");

MileageValidator::MileageValidator() {}

MileageValidator::~MileageValidator() {}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// MileageValidator::validate()
//----------------------------------------------------------------------------

void
MileageValidator::validate(PricingTrx& trx,
                           MileageInfo& mInfo,
                           const TravelRoute& travelRoute,
                           PaxTypeFare* ptf,
                           DiagCollector* diag) const
{
  LOG4CXX_DEBUG(logger, "Entered MileageValidator::validate()");

  MileageRoute mileageRoute;

  MileageRouteBuilder mileageRouteBuilder;
  mileageRouteBuilder.buildMileageRoute(
      trx, travelRoute, mileageRoute, trx.dataHandle(), trx.ticketingDate());

  mileageRoute.isOutbound() = true;

  if (ptf)
  {
    mileageRoute.isYYFare() = ptf->fare()->isIndustry();
    mileageRoute.isOutbound() = (ptf->directionality() == FROM);
  }

  mileageRoute.diagnosticHandle() = diag;

  {
    TSELatencyData metics(trx, "FVO MILEAGE PSR");
    mInfo.psrApplies() = getPSR(mileageRoute);
  }
  if (mInfo.psrApplies())
  {
    mInfo.valid() = true;
    fillInDiagnosticInfoForPSR(mInfo, mileageRoute);
    return;
  }

  else
  {
    const Collector<TPMCollectorMV>& tpmCollector(
        tse::Singleton<Collector<TPMCollectorMV> >::instance());
    const Collector<MPMCollectorMV>& mpmCollector(
        tse::Singleton<Collector<MPMCollectorMV> >::instance());
    bool processTPMValidation(true), processMPMValidation(true);
    {
      TSELatencyData metics(trx, "FVO MILEAGE TPM");
      processTPMValidation = tpmCollector.collectMileage(mileageRoute);
    }
    {
      TSELatencyData metics(trx, "FVO MILEAGE MPM");
      processMPMValidation = mpmCollector.collectMileage(mileageRoute);
    }

    if (processTPMValidation && processMPMValidation)
    {
      FareCalcVisitor fareCalcVisitor;
      // calculate and populateMileageInfo for Diagnostic
      {
        TSELatencyData metics(trx, "FVO MILEAGE TPD");
        calculateMileage(mInfo, mileageRoute, fareCalcVisitor, trx);
      }

      fillInDiagnosticInfo(mInfo, mileageRoute, fareCalcVisitor);
      if (mInfo.surchargeAmt() != 0)
      {
        mInfo.mileageEqualizationApplies() = applyMileageEqualization(mileageRoute, mInfo);
      }

      mInfo.valid() = !(mileageRoute.mileageRouteMPM() == 0 || mInfo.surchargeAmt() > 25);
      return;
    }
  }

  mInfo.valid() = false;

  return;
}

//----------------------------------------------------------------------------
// MileageValidator::fillInDiagnosticInfo()
//----------------------------------------------------------------------------

bool
MileageValidator::fillInDiagnosticInfo(MileageInfo& mInfo,
                                       MileageRoute& mRoute,
                                       FareCalcVisitor& fareCalcVisitor) const
{
  // set all exclusion flags
  mInfo.southAtlanticTPMExclusion() = mRoute.southAtlanticExclusion();

  mInfo.totalApplicableTPM() = mRoute.mileageRouteTPM();
  mInfo.totalApplicableMPM() = mRoute.mileageRouteMPM();
  mInfo.tpd() = mRoute.tpd();

  MileageRouteItems::const_iterator itr(mRoute.mileageRouteItems().begin());
  MileageRouteItems::const_iterator end(mRoute.mileageRouteItems().end());

  for (; itr != end; ++itr)
  {
    if (UNLIKELY(itr->tpmSurfaceSectorExempt()))
    {
      Market mkt;
      mkt.first = itr->city1()->loc();
      mkt.second = itr->city2()->loc();
      mInfo.surfaceSectorExemptCities().push_back(mkt);
    }

    if (itr->southAtlanticExclusion())
    {
      Market mkt;
      mkt.first = itr->origCityOrAirport()->loc();
      mkt.second = itr->destCityOrAirport()->loc();
      mInfo.southAtlanticTPDCities().push_back(mkt);
      mInfo.tpmExclusion() = mRoute.tpmExclusion();
    }

    else
    {
      continue;
    }
  }
  mInfo.tpdViaGeoLocs() = fareCalcVisitor.getMatching();

  // this vector contains travel segment numbers where the matched VIA GEO locs belong to
  mInfo.tpdMatchedViaLocs() = fareCalcVisitor.getMatchedTPD();

  return true;
}

//----------------------------------------------------------------------------
// MileageValidator::fillInDiagnosticInfoForPSR()
//----------------------------------------------------------------------------
bool
MileageValidator::fillInDiagnosticInfoForPSR(MileageInfo& mInfo, MileageRoute& mRoute) const
{

  if (mInfo.psrApplies() && mRoute.applicablePSR() != nullptr)
  {
    TpdPsr* psr = mRoute.applicablePSR();
    mInfo.psrHipExempt() = mRoute.hipExempt();

    if (psr->carrier().empty() && psr->thruMktCxrs().empty())
    {
      mInfo.psrGovCxr() = INDUSTRY_CARRIER;
    }
    else
    {
      mInfo.psrGovCxr() = mRoute.governingCarrier();
    }

    //--------------------------------------------------
    // Save geoLocs from applicable psr set for display
    //--------------------------------------------------
    std::vector<TpdPsrViaGeoLoc*>::const_iterator geoLocItr = psr->viaGeoLocs().begin();
    std::vector<TpdPsrViaGeoLoc*>::const_iterator geoLocEnd = psr->viaGeoLocs().end();

    while (geoLocItr != geoLocEnd && static_cast<int>(mRoute.psrSetNumber()) == (*geoLocItr)->setNo())
    {
      mInfo.psrGeoLocs().push_back((**geoLocItr));

      ++geoLocItr;
    }
  }

  return true;
}

struct SumTPM
{
  uint16_t operator()(uint16_t sum, const MileageRouteItem& item) const { return sum + item.tpm(); }
};

//----------------------------------------------------------------------------
// MileageValidator::calculateMileage()
//----------------------------------------------------------------------------

bool
MileageValidator::calculateMileage(MileageInfo& mInfo,
                                   MileageRoute& mRoute,
                                   FareCalcVisitor& fareCalcVisitor,
                                   PricingTrx& trx) const
{
  MileageRouteItems::iterator itr(mRoute.mileageRouteItems().begin());
  MileageRouteItems::const_iterator end(mRoute.mileageRouteItems().end());
  uint16_t totalMileage = 0;
  uint16_t totalActualMileage = 0;
  for (; itr != end; ++itr)
  {
    totalActualMileage = totalActualMileage + itr->tpm();
    if (UNLIKELY(itr->tpmSurfaceSectorExempt()))
    {
      continue;
    }
    else if (itr->southAtlanticExclusion())
    {
      MileageRouteItem mItem;
      mItem.city1() = itr->origCityOrAirport();
      const Retriever<TPMRetriever>& tpmRetriever(
          tse::Singleton<Retriever<TPMRetriever> >::instance());
      MileageRouteItems::reverse_iterator ritr(itr);
      MileageRouteItems::reverse_iterator sec =
          std::find_if(mRoute.mileageRouteItems().rbegin(),
                       ritr - 1,
                       std::mem_fun_ref<bool>(&MileageRouteItem::southAtlanticExclusion));
      if (sec != ritr - 1)
      {
        totalActualMileage = std::accumulate(sec, ritr - 1, totalActualMileage, SumTPM());
        mItem.city2() = sec->destCityOrAirport();
        mItem.travelDate() = sec->travelDate();
        // Set a global direction
        const GlobalDirectionRetriever& globalDirectionRetriever(
            tse::Singleton<GlobalDirectionRetriever>::instance());
        globalDirectionRetriever.retrieve(mItem, *mRoute.dataHandle(), &trx);
        globalDirectionRetriever.retrieve(mItem, *mRoute.dataHandle(), &trx, MPM);

        if (tpmRetriever.retrieve(mItem, *mRoute.dataHandle()))
        {
          totalMileage = totalMileage + mItem.tpm();
        }
        itr = sec.base() - 1;
      }
    }
    else
    {
      totalMileage = totalMileage + itr->tpm();
    }
  }

  // If it SouthAtlanticException (not Exclusion) then store the mileageRouteTPM and
  // corresponding surchargeAmt to apply exception for 'YY' fares later
  mInfo.totalApplicableTPMSAException() = NO_TPM;
  mInfo.surchargeAmtSAException() = 0;
  if (UNLIKELY(mRoute.southAtlanticExceptionApplies()))
  {
    mInfo.totalApplicableTPMSAException() = totalActualMileage - mRoute.tpd();
    mInfo.surchargeAmtSAException() =
        MileageUtil::getEMS(totalActualMileage, mRoute.mileageRouteMPM());
  }

  bool isTPDApplicable =
      totalMileage > mRoute.mileageRouteMPM() ? getTPD(mRoute, fareCalcVisitor) : false;
  mRoute.mileageRouteTPM() = isTPDApplicable ? (totalMileage - mRoute.tpd()) : totalMileage;
  mInfo.totalApplicableTPM() = mRoute.mileageRouteTPM();
  mInfo.tpd() = mRoute.tpd();
  mInfo.surchargeAmt() = MileageUtil::getEMS(mRoute.mileageRouteTPM(), mRoute.mileageRouteMPM());
  mRoute.ems() = mInfo.surchargeAmt();
  return true;
}

bool
MileageValidator::getTPD(MileageRoute& route, FareCalcVisitor& fareCalcVisitor) const
{
  const TicketedPointDeduction& tpd(tse::Singleton<TicketedPointDeduction>::instance());
  return tpd.apply(route, &fareCalcVisitor);
}

bool
MileageValidator::getPSR(MileageRoute& route) const
{
  const MileageExclusion& psr(tse::Singleton<PermissibleSpecifiedRouting>::instance());
  return psr.apply(route);
}

bool
MileageValidator::applyMileageEqualization(MileageRoute& mRoute, MileageInfo& mInfo) const
{
  const MileageExclusion& mileageEqualization(tse::Singleton<MileageEqualization>::instance());
  if (!mileageEqualization.apply(mRoute))
    return false;

  mInfo.equalizationSurcharges().first = mInfo.surchargeAmt();
  mInfo.equalizationSurcharges().second = mRoute.ems();
  mInfo.totalApplicableMPM() = mRoute.mileageRouteMPM();
  mInfo.totalApplicableTPM() = mRoute.mileageRouteTPM();
  mInfo.surchargeAmt() = mRoute.ems(); // updating for the changed value

  return true;
}

}
