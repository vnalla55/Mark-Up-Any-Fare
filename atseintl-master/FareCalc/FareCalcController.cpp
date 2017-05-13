//-------------------------------------------------------------------
//
//  File:        FareCalcController.cpp
//  Created:     June 24, 2004
//  Authors:     Binh Tran
//
//  Description:
//
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

#include "FareCalc/FareCalcController.h"

#include "Common/Config/ConfigMan.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/Logger.h"
#include "Common/MultiTicketUtil.h"
#include "Common/Thread/ThreadPoolFactory.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/Itin.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag203Collector.h"
#include "Diagnostic/Diag804Collector.h"
#include "Diagnostic/Diag853Collector.h"
#include "Diagnostic/Diag854Collector.h"
#include "Diagnostic/Diag856Collector.h"
#include "Diagnostic/Diag864Collector.h"
#include "Diagnostic/Diag894Collector.h"
#include "Diagnostic/Diag970Collector.h"
#include "Diagnostic/Diag980Collector.h"
#include "Diagnostic/Diag983Collector.h"
#include "Diagnostic/DiagManager.h"
#include "FareCalc/AirlineShoppingFCUtil.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/FareCalcItinerary.h"
#include "FareCalc/SoloCarnivalFCUtil.h"
#include "Taxes/Common/TaxSplitter.h"

#include <map>
#include <string>
#include <vector>

namespace tse
{
FALLBACK_DECL(fallbackHalfRTPricingForIbf);

static Logger
logger("atseintl.FareCalc.FareCalcController");

bool
FareCalcController::process()
{
  LOG4CXX_INFO(logger, "Entering FareCalcController::process()");

  if (!initFareConfig())
    return false;

  if (_trx.diagnostic().diagnosticType() == Diagnostic203)
    return performDiagnostic203();

  if (_trx.diagnostic().diagnosticType() == Diagnostic853)
    return performDiagnostic853();

  if (_trx.diagnostic().diagnosticType() == Diagnostic856)
    return performDiagnostic856();

  if ((_trx.getRequest()->owPricingRTTaxProcess()) &&
      (_trx.getRequest()->processingDirection() != ProcessingDirection::ONEWAY))
  {
    AirlineShoppingFCUtil::copyTaxResponsesAndSwapItinsVec(_trx);
  }

  if (TrxUtil::taxEnhancementActivated(DateTime::fromMilitaryTime(1200)))
  {
    if (_trx.getRequest()->originBasedRTPricing())
    {
      if(_trx.getRequest()->isParityBrandsPath() && !fallback::fallbackHalfRTPricingForIbf(&_trx))
        _trx.computeBrandOrder();

      WnSnapUtil::splitItinsByDirection(_trx, true);
      WnSnapUtil::createFarePathsForOutIn(_trx, true);

      if (_trx.diagnostic().diagnosticType() == Diagnostic983)
        performDiagnostic983();

      bool const isFakeInbound = _trx.outboundDepartureDate().isEmptyDate();

      if (isFakeInbound)
      {
        _trx.itin().swap(_trx.subItinVecOutbound());

        std::vector<Itin*>::iterator itinIter = _trx.itin().begin();
        std::vector<Itin*>::iterator itinIterEnd = _trx.itin().end();

        for (; itinIter != itinIterEnd; ++itinIter)
        {
          Itin* itin = (*itinIter);
          itin->travelSeg().back()->stopOver() = false;
        }
      }
      else
      {
        _trx.itin().swap(_trx.subItinVecInbound());
      }
    }
  }

  bool isMultiTicketSolutionFound =
      (_trx.getRequest()->multiTicketActive() && MultiTicketUtil::isMultiTicketSolutionFound(_trx));
  processItinFareCalc();

  if (_trx.isRfbListOfSolutionEnabled() && _trx.getRequest()->isTicketEntry())
    throw ErrorResponseException(
            ErrorResponseException::SFB_LIST_OF_SOLUTION_NOT_ALLOWED_FOR_TICKETING);

  assignPriceToSolItinGroupsMapItems();
  if (isMultiTicketSolutionFound)
  {
    MultiTicketUtil::processMultiTicketItins(_trx);
  }

  if ((_trx.getRequest()->owPricingRTTaxProcess()) &&
      (_trx.getRequest()->processingDirection() != ProcessingDirection::ONEWAY))
  {
    TaxSplitter taxSplitter(static_cast<ShoppingTrx&>(_trx));
    AirlineShoppingFCUtil::updateTaxResponse(_trx, taxSplitter);

    performDiagnostic804();
  }

  if (TrxUtil::taxEnhancementActivated(DateTime::fromMilitaryTime(1200)))
  {
    if (_trx.getRequest()->originBasedRTPricing())
    {
      TaxSplitter taxSplitter(static_cast<ShoppingTrx&>(_trx));
      AirlineShoppingFCUtil::updateTaxResponse(_trx, taxSplitter);

      if (_trx.diagnostic().diagnosticType() == LegacyTaxDiagnostic24)
        performDiagnostic804();
    }
  }

  if (_trx.diagnostic().diagnosticType() == Diagnostic980)
    return performDiagnostic980();

  if (_trx.diagnostic().diagnosticType() == Diagnostic970)
    return performDiagnostic970();

  if (_trx.diagnostic().diagnosticType() == Diagnostic894)
    return performDiagnostic894();

  if (_trx.diagnostic().diagnosticType() == Diagnostic864)
    return performDiagnostic864();

  return true;
}

bool
FareCalcController::structuredRuleProcess()
{
  LOG4CXX_INFO(logger, "Entering FareCalcController::structuredRuleProcess()");

  if (!initFareConfig())
    return false;

  processItinFareCalc();

  return true;
}

void
FareCalcController::performDiagnostic804()
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector& diag = *(factory->create(_trx));
  diag.trx() = &_trx;

  const std::string& modifiedTaxes = diag.rootDiag()->diagParamMapItem("MODIFIED_TAXES");

  if ("Y" == modifiedTaxes)
  {
    std::vector<Itin*>::iterator itinIter = _trx.itin().begin();
    std::vector<Itin*>::iterator itinIterEnd = _trx.itin().end();

    for (; itinIter != itinIterEnd; ++itinIter)
    {
      Itin* itin = (*itinIter);

      if (itin->getTaxResponses().empty())
      {
        diag.enable(LegacyTaxDiagnostic24, Diagnostic24); // 804
        diag << "\n\n T A X E S   N O T   A P P L I C A B L E \n\n";
        diag.flushMsg();

        continue;
      }

      std::vector<TaxResponse*>::const_iterator taxResponseIter = itin->getTaxResponses().begin();
      std::vector<TaxResponse*>::const_iterator taxResponseIterEnd = itin->getTaxResponses().end();

      for (; taxResponseIter != taxResponseIterEnd; ++taxResponseIter)
      {
        if ((_trx.diagnostic().diagnosticType() == LegacyTaxDiagnostic24) ||
            (_trx.diagnostic().diagnosticType() == Diagnostic24))
        {
          diag.enable(LegacyTaxDiagnostic24, Diagnostic24);
          diag << **taxResponseIter;
        }
      }

      diag.flushMsg();
    }
  }
}

void
FareCalcController::performDiagnostic983()
{
  DCFactory* factory = DCFactory::instance();
  Diag983Collector* diagPtr = dynamic_cast<Diag983Collector*>(factory->create(_trx));

  diagPtr->enable(Diagnostic983);
  diagPtr->activate();

  (*diagPtr) << _trx;

  diagPtr->flushMsg();
}

bool
FareCalcController::initFareConfig()
{
  if (_fcConfig == nullptr)
  {
    _fcConfig = FareCalcUtil::getFareCalcConfig(_trx);
  }
  return (_fcConfig != nullptr);
}

bool
FareCalcController::performDiagnostic203()
{
  Diag203Collector diag(_trx.diagnostic());
  diag.enable(Diagnostic203);
  diag.process(_trx);
  diag.flushMsg();
  return true;
}

bool
FareCalcController::performDiagnostic853()
{
  Diag853Collector diag(_trx.diagnostic());
  diag.enable(Diagnostic853);
  diag.process(_trx, _fcConfig);
  diag.flushMsg();
  return true;
}

bool
FareCalcController::performDiagnostic856()
{
  NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(&_trx);
  if (noPNRTrx != nullptr)
  {
    DCFactory* factory = DCFactory::instance();
    Diag856Collector* dc = dynamic_cast<Diag856Collector*>(factory->create(_trx));
    if (dc != nullptr)
    {
      dc->enable(Diagnostic856);
      dc->process(*noPNRTrx);
      dc->flushMsg();
    }
  }
  return true;
}

bool
FareCalcController::performDiagnostic864()
{
  if (!_trx.hasPriceDynamicallyDeviated())
    return true;
  DiagManager diag(_trx, Diagnostic864);
  if (diag.isActive())
  {
    Diag864Collector& dc = static_cast<Diag864Collector&>(diag.collector());
    dc.printPriceDeviationResults(_trx);
  }

  return true;
}

bool
FareCalcController::performDiagnostic970()
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic970)
  {
    DCFactory* factory = DCFactory::instance();
    Diag970Collector* diag970 = dynamic_cast<Diag970Collector*>(factory->create(_trx));
    diag970->enable(Diagnostic970);

    if (diag970->displayHeader(_trx))
    {
      for (Itin* itin : _trx.itin())
      {
        FareCalcCollector* fareCalcCollector =
            FareCalcUtil::getFareCalcCollectorForItin(_trx, itin);

        if (!fareCalcCollector)
          continue;

        diag970->displayOption(itin);

        for (PaxType* paxType : _trx.paxType())
        {
          const FarePath* path = nullptr;

          for (FarePath* fp : itin->farePath())
          {
            if (paxType == fp->paxType())
            {
              path = fp;
              break;
            }
          }

          CalcTotals* totals = nullptr;

          if (path)
            totals = fareCalcCollector->findCalcTotals(path);

          if (!totals)
            continue;

          if (_trx.getOptions()->isSplitTaxesByFareComponent())
          {
            FareCalc::FcTaxInfo::TaxesPerFareUsage taxGrouping;
            totals->getFcTaxInfo().getTaxesSplitByFareUsage(taxGrouping);

            diag970->displayTaxesByFareUsage(taxGrouping, itin, path);
          }

          if (_trx.getOptions()->isSplitTaxesByLeg())
          {
            FareCalc::FcTaxInfo::TaxesPerLeg taxesPerLeg;
            totals->getFcTaxInfo().getTaxesSplitByLeg(taxesPerLeg);

            diag970->displayTaxesByLeg(taxesPerLeg);
          }
        }
      }
    }

    diag970->flushMsg();
  }
  return true;
}

bool
FareCalcController::performDiagnostic980()
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic980)
  {
    DCFactory* factory = DCFactory::instance();
    Diag980Collector* diag980 = dynamic_cast<Diag980Collector*>(factory->create(_trx));
    diag980->enable(Diagnostic980);
    (*diag980) << _trx;
    diag980->flushMsg();
  }
  return true;
}

bool
FareCalcController::performDiagnostic894()
{
  if (_trx.diagnostic().diagnosticType() != Diagnostic894)
    return true;

  bool isValidUse = false;
  if (_trx.getTrxType() == PricingTrx::PRICING_TRX)
  {
    // only EXPEDIA and PBB with brands
    if (_trx.activationFlags().isSearchForBrandsPricing() ||
       (_trx.isPbbRequest() == PBB_RQ_PROCESS_BRANDS))
      isValidUse = true;
  }
  else if (_trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    // only BFA, BFS or IBF (+context, and calendar)
    if (_trx.isBrandsForTnShopping() || _trx.getRequest()->isBrandedFaresRequest())
      isValidUse = true;
  }

  DCFactory* factory = DCFactory::instance();
  Diag894Collector* diag = dynamic_cast<Diag894Collector*>(factory->create(_trx));
  diag->enable(Diagnostic894);

  if (!isValidUse)
  {
    diag->printImproper894Use();
    return true;
  }

  *diag << "***** START FARE USAGE INFO (FARE CALC SERVICE) *****" << std::endl;
  for (Itin* itin : _trx.itin())
  {
    *diag << "** BEGIN ITIN";
    // in pricing there is only one itin with number -1, display this number only in shopping
    if (itin->itinNum() >= 0)
      *diag << ": " << itin->itinNum();
    *diag << " **" << std::endl;
    diag->printItinFareUsageInfo(*itin, _trx);
    *diag << "** END ITIN";
    // in pricing there is only one itin with number -1, display this number only in shopping
    if (itin->itinNum() >= 0)
      *diag << ": " << itin->itinNum();
    *diag << " **" << std::endl;
  }
  *diag << "***** END FARE USAGE INFO (FARE CALC SERVICE) *****" << std::endl;
  diag->printFooter();
  diag->flushMsg();

  return true;
}

void
FareCalcController::processItinFareCalc()
{
  try
  {
    DataHandle dataHandle(_trx.ticketingDate());

    TseRunnableExecutor pooledExecutor(TseThreadingConst::FARE_CALC_TASK);
    TseRunnableExecutor synchronousExecutor(TseThreadingConst::SYNCHRONOUS_TASK);

    for (std::vector<Itin*>::iterator itinI = _trx.itin().begin(), itinEnd = _trx.itin().end();
         itinI != itinEnd;
         ++itinI)
    {
      if ((*itinI) == nullptr || ((*itinI)->errResponseCode() != ErrorResponseException::NO_ERROR) ||
          ((*itinI)->farePath().empty()))
        continue;

      FareCalcItinerary* fareCalcItinerary = nullptr;
      dataHandle.get(fareCalcItinerary);

      // Create the FareCalcCollector here to avoid the unneccessary locking in
      // FareCalcItinerary:
      FareCalcCollector* fcCollector = getFareCalcCollector(*itinI);

      if (LIKELY(fareCalcItinerary != nullptr && fcCollector != nullptr))
      {
        fareCalcItinerary->initialize(_trx, **itinI, fcCollector, _fcConfig);

        TseRunnableExecutor& taskExecutor =
            (std::distance(itinI, itinEnd) > 1 ? pooledExecutor : synchronousExecutor);
        taskExecutor.execute(*fareCalcItinerary);
      }
    }
    pooledExecutor.wait();
  }
  catch (boost::thread_interrupted&)
  {
    LOG4CXX_ERROR(logger, "thread_interrupted has Been thrown");
    throw;
  }
}

FareCalcCollector*
FareCalcController::getFareCalcCollector(const Itin* itin)
{
  std::map<const Itin*, FareCalcCollector*>::iterator iter;
  iter = _trx.fareCalcCollectorMap().find(itin);
  if (LIKELY(iter == _trx.fareCalcCollectorMap().end()))
  {
    FareCalcCollector* fcCollector = createFareCalcCollector();

    if (LIKELY(fcCollector != nullptr))
    {
      _trx.fareCalcCollectorMap().insert(std::make_pair(itin, fcCollector));

      // FIXME: todo - remove this when done away with the old vector
      _trx.fareCalcCollector().push_back(fcCollector);
    }
    return fcCollector;
  }
  return iter->second;
}

FareCalcCollector*
FareCalcController::createFareCalcCollector()
{
  FareCalcCollector* fcCollector = nullptr;
  _trx.dataHandle().get(fcCollector);

  return fcCollector;
}

uint32_t
FareCalcController::getActiveThreads()
{
  if (!ThreadPoolFactory::isMetricsEnabled())
    return 0;

  return ThreadPoolFactory::getNumberActiveThreads(TseThreadingConst::FARE_CALC_TASK);
}

void
FareCalcController::assignPriceToSolItinGroupsMapItems()
{
  if (_trx.getTrxType() != PricingTrx::MIP_TRX || !_trx.getOptions()->isCarnivalSumOfLocal())
  {
    return;
  }

  SoloCarnivalFCUtil scFCUtil(_trx);
  scFCUtil.assignPriceToSolItinGroupsMapItems();
}
} // tse namespace
