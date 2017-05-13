//-------------------------------------------------------------------
//  Description: Add-on Construction Orchestrator
//
//  Copyright Sabre 2004
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

#include "AddonConstruction/AddonConstructionOrchestrator.h"

#include "AddonConstruction/AddOnCacheControl.h"
#include "AddonConstruction/AddonConstruction.h"
#include "AddonConstruction/ConstructedCacheManager.h"
#include "AddonConstruction/ConstructedFareInfoResponse.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/ConstructionVendor.h"
#include "AddonConstruction/DiagRequest.h"
#include "Common/Logger.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "Common/Vendor.h"
#include "DataModel/ExcItin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "Diagnostic/Diag259Collector.h"
#include "Util/Algorithm/Container.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FALLBACK_DECL(removeDynamicCastForAddonConstruction);
FALLBACK_DECL(dffOaFareCreation);
FALLBACK_DECL(createSMFOFaresForALlUsers);

namespace
{
Logger
logger("atseintl.Fares.AddonConstructionOrchestrator");
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
AddonConstructionOrchestrator::ConstructionTask::ConstructionTask(PricingTrx& trx,
                                                                  const DateTime& travelDate,
                                                                  const DateTime& ticketingDate,
                                                                  const LocCode& orig,
                                                                  const LocCode& boardCity,
                                                                  const LocCode& dest,
                                                                  const LocCode& offCity,
                                                                  const GlobalDirection globalDir,
                                                                  const bool singleOverDouble,
                                                                  SpecifiedFareCache* specCache)
  : _cJob(trx,
          travelDate,
          ticketingDate,
          orig,
          boardCity,
          dest,
          offCity,
          globalDir,
          singleOverDouble,
          specCache)
{
  desc("ADDON CONSTRUCTION TASK");

  this->trx(&trx);
}
#else
AddonConstructionOrchestrator::ConstructionTask::ConstructionTask(PricingTrx& trx,
                                                                  const DateTime& travelDate,
                                                                  const DateTime& ticketingDate,
                                                                  const LocCode& orig,
                                                                  const LocCode& boardCity,
                                                                  const LocCode& dest,
                                                                  const LocCode& offCity,
                                                                  const bool singleOverDouble)
  : _cJob(trx, travelDate, ticketingDate, orig, boardCity, dest, offCity, singleOverDouble)
{
  desc("ADDON CONSTRUCTION TASK");

  this->trx(&trx);
}
#endif

AddonConstructionOrchestrator::ConstructionTask::~ConstructionTask()
{
}

void
AddonConstructionOrchestrator::ConstructionTask::performTask()
{
  try
  {
    AddonConstructionOrchestrator::process(_cJob);
  }
  catch (ErrorResponseException& ex)
  {
    LOG4CXX_INFO(logger, "Exception:" << ex.message() << " - AddonConstruction failed");

    throw ex;
  }
  catch (std::exception& e)
  {
    LOG4CXX_INFO(logger, "Exception:" << e.what() << " - AddonConstruction failed");

    throw e;
  }
  catch (...)
  {
    LOG4CXX_INFO(logger, "UNKNOWN EXCEPTION - AddonConstruction failed");

    throw;
  }
}

void
AddonConstructionOrchestrator::classInit()
{
  ConstructedCacheManager::instance();
  AddOnCacheControl::classInit();
}

struct isSameVendor
{
  bool operator()(const AddonConstruction::VCPair& p1, const AddonConstruction::VCPair& p2) const
  {
    return p1.first < p2.first;
  }
};

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
bool
AddonConstructionOrchestrator::process(PricingTrx& trx,
                                       const VendorCode& vendor,
                                       const CarrierCode& carrier,
                                       const LocCode& origin,
                                       const LocCode& boardCity,
                                       const LocCode& destination,
                                       const LocCode& offCity,
                                       const GlobalDirection globalDir,
                                       const bool singleOverDouble,
                                       ConstructedFareInfoResponse& response,
                                       const DateTime& travelDate,
                                       SpecifiedFareCache* specCache)
#else
bool
AddonConstructionOrchestrator::process(PricingTrx& trx,
                                       const VendorCode& vendor,
                                       const CarrierCode& carrier,
                                       const LocCode& origin,
                                       const LocCode& boardCity,
                                       const LocCode& destination,
                                       const LocCode& offCity,
                                       const bool singleOverDouble,
                                       ConstructedFareInfoResponse& response,
                                       const DateTime& travelDate)
#endif
{
  LOG4CXX_TRACE(logger, "Entered AddonConstructionOrchestrator::process(...)");

  TSELatencyData metrics(trx, "FCO ACO PROCESS");

  // setting ticketing date for RexPricingTrx
  DateTime ticketingDate = trx.getRequest()->ticketingDT();
  if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX))
    ticketingDate = trx.dataHandle().ticketDate();

  // define the list of distinct pairs [vendor/carrier]

  AddonConstruction::VCList vcPairs;
  AddonConstruction::defineVendorCarrierPairs(trx.dataHandle(),
                                              carrier,
                                              origin,
                                              boardCity,
                                              destination,
                                              offCity,
                                              vcPairs,
                                              TrxUtil::isDisableYYForExcItin(trx));

  if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX && vendor != Vendor::EMPTY))
  {
    std::pair<AddonConstruction::VCList::iterator, AddonConstruction::VCList::iterator> e_range =
        std::equal_range(
            vcPairs.begin(), vcPairs.end(), std::make_pair(vendor, ""), isSameVendor());
    vcPairs.erase(vcPairs.begin(), e_range.first);
    vcPairs.erase(e_range.second, vcPairs.end());
  }

  const bool shouldFilterSMFOFares =
      fallback::createSMFOFaresForALlUsers(&trx) ||
      (!fallback::dffOaFareCreation(&trx) && !TrxUtil::isRequestFromAS(trx));

  if (shouldFilterSMFOFares)
  {
    alg::erase_if(vcPairs,
                  [](const auto& vendorCarrierPair)
                  { return vendorCarrierPair.first == Vendor::SMFO; });
  }

  if (!vcPairs.empty())
  {
    // Create a "seed" task
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
    ConstructionTask task(trx,
                          travelDate,
                          ticketingDate,
                          origin,
                          boardCity,
                          destination,
                          offCity,
                          globalDir,
                          singleOverDouble,
                          specCache);
#else
    ConstructionTask task(
        trx, travelDate, ticketingDate, origin, boardCity, destination, offCity, singleOverDouble);
#endif

    // Create the list of tasks (vendor and carrier will be reset later)
    ConstructionTaskList constructionTasks(vcPairs.size(), task);

    // create list of construction tasks and execute them
    TseRunnableExecutor taskExecutor(TseThreadingConst::VENDOR_TASK);

    // Process a job for each VC pair

    AddonConstruction::VCListI vc = vcPairs.begin();
    AddonConstruction::VCListI vce = vcPairs.end();

    for (int currTaskIndex = 0; vc != vce; ++currTaskIndex, ++vc)
    {
      constructionTasks[currTaskIndex].setVendorCode((*vc).first);
      constructionTasks[currTaskIndex].carrier() = (*vc).second;
      constructionTasks[currTaskIndex].createDiagCollector();

      // Submit the task to be executed
      taskExecutor.execute(constructionTasks[currTaskIndex]);
    }

    // Wait for the tasks to complete
    taskExecutor.wait();

    // Get the responses

    ConstructionTaskList::iterator i = constructionTasks.begin();
    ConstructionTaskList::iterator ie = constructionTasks.end();

    for (i = constructionTasks.begin(); i != ie; ++i)
    {
      copy((*i).response().responseHashSet().begin(),
           (*i).response().responseHashSet().end(),
           inserter(response.responseHashSet(), response.responseHashSet().end()));
      (*i).reclaimDiagCollector();
    }
  }

  return true;
}

bool
AddonConstructionOrchestrator::process(ConstructionJob& cj)
{
  LOG4CXX_TRACE(logger,
                "Entered AddonConstruction::process(...) for "
                    << cj.od(CP_ORIGIN) << "-" << cj.carrier() << "-" << cj.od(CP_DESTINATION));

  // first to check for diag259 (constructed cache flash)

  if (UNLIKELY(cj.constructionCacheFlush()))
    if (processConstructionCacheFlush(cj))
      return true;

  ConstructedCacheManager& ccm = ConstructedCacheManager::instance();

  bool cacheConstructedFares = (ccm.useCache() && !cj.isHistorical());
  if (UNLIKELY(cacheConstructedFares))
  {
    LOG4CXX_TRACE(logger,
                  "Get " << cj.od(CP_ORIGIN) << "-" << cj.carrier() << "-" << cj.od(CP_DESTINATION)
                         << " fares from Constructed Fare Cache");

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
    CacheKey cjKey(cj.origin(), cj.destination(), cj.carrier(), cj.vendorCode(), cj.getGlobalDirection());
#else
    CacheKey cjKey(cj.origin(), cj.destination(), cj.carrier(), cj.vendorCode());
#endif

    if (cj.forceAddonConstruction())
    {
      // Delete everything for this orig, dest, carrier and vendor

      ccm.aclruCache().invalidate(cjKey);
    }

    // Run AddonConstruction

    std::shared_ptr<ConstructedCacheDataWrapper> cacheData = ccm.aclruCache().get(cjKey, &cj);

    if (cacheData.get() == nullptr)
    {
      LOG4CXX_ERROR(logger, "Internal Error: Invalid cached object");
      return false;
    }

    AddonConstruction::createResponse(cj, cacheData->ccFares());
  }
  else // no cache
  {
    LOG4CXX_TRACE(logger,
                  "Get " << cj.od(CP_ORIGIN) << "-" << cj.carrier() << "-" << cj.od(CP_DESTINATION)
                         << " fares from Local Storage");

    ConstructedCacheDataWrapper localDataWrapper;

    AddonConstruction::runConstructionProcess(cj, localDataWrapper);

    AddonConstruction::createResponse(cj, localDataWrapper.ccFares());
  }

  return true;
}

bool
AddonConstructionOrchestrator::processConstructionCacheFlush(ConstructionJob& cJob)
{
  ConstructedCacheManager& ccm = ConstructedCacheManager::instance();
  if (!ccm.useCache())
    return true;

  Diag259Collector* dc{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&cJob.trx()))
  {
    dc = cJob.diagnostic<Diag259Collector>();
  }
  else
  {
    dc = cJob.diag259();
  }

  TSE_ASSERT(nullptr != dc);

  Diag259Collector& diag259 = *dc;
  DiagRequest& diagRequest = *cJob.diagRequest();

  switch (diagRequest.flashEventType())
  {
  case DiagRequest::FE_NO_FLUSH_EVENT:
    return false;

  case DiagRequest::FE_FLASH_ADDON_FARE:
    ccm.flushAddonFares(diagRequest.flushMarket1(),
                        diagRequest.flushMarket2(),
                        diagRequest.diagVendor(),
                        diagRequest.diagCarrier());

    diag259 << " \nADDON CONSTRUCTION DIAGNOSTIC 259\n"
            << "FLUSH ADDON FARES FOR " << diagRequest.diagVendor() << " CARRIER "
            << diagRequest.diagCarrier() << " MARKET " << diagRequest.flushMarket1() << "-"
            << diagRequest.flushMarket2() << "\n";
    break;

  case DiagRequest::FE_FLASH_SPECIFIED_FARE:
    ccm.flushSpecifiedFares(diagRequest.flushMarket1(),
                            diagRequest.flushMarket2(),
                            diagRequest.diagVendor(),
                            diagRequest.diagCarrier());

    diag259 << " \nADDON CONSTRUCTION DIAGNOSTIC 259\n"
            << "FLUSH SPECIFIED FARES FOR " << diagRequest.diagVendor() << " CARRIER "
            << diagRequest.diagCarrier() << " MARKET " << diagRequest.flushMarket1() << "-"
            << diagRequest.flushMarket2() << "\n";
    break;

  case DiagRequest::FE_FLASH_BY_VENDOR_CARRIER:

    diag259 << " \nADDON CONSTRUCTION DIAGNOSTIC 259\n"
            << "SIZE OF CONSTRUCTED CACHE BEFORE FLUSH: " << ccm.size() << "\n";

    ccm.flushVendorCxrFares(diagRequest.diagVendor(), diagRequest.diagCarrier());

    diag259 << " \nADDON CONSTRUCTION DIAGNOSTIC 259\n"
            << "SIZE OF CONSTRUCTED CACHE AFTER FLUSH: " << ccm.size() << "\n";

    diag259 << "FLUSH ALL CONSTRUCTED FARES FOR " << diagRequest.diagVendor() << " CARRIER "
            << diagRequest.diagCarrier() << "\n";
    break;

  case DiagRequest::FE_FLASH_ALL:
    ccm.flushAll();

    diag259 << " \nADDON CONSTRUCTION DIAGNOSTIC 259\n"
            << "FLUSH ALL CONSTRUCTED FARES FOR ALL VENDORS AND CARRIERS"
            << "\n";
    break;

  default:
    break;
  }

  return true;
}
} // tse
