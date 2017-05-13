//----------------------------------------------------------------------------
//  File:        FareCollectorOrchestratorESV.h
//  Created:     2008-07-01
//
//  Description: ESV fare collector
//
//  Updates:
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Fares/FareCollectorOrchestratorESV.h"

#include "Common/FallbackUtil.h"
#include "Common/GoverningCarrier.h"
#include "Common/ItinUtil.h"
#include "Common/TSEAlgorithms.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/ShoppingTrx.h"
#include "Fares/CarrierFareController.h"
#include "Fares/FareCollectorOrchestrator.h"
#include "Fares/FlightTracker.h"
#include "Fares/SalesRestrictionByNation.h"
#include "Server/TseServer.h"

namespace tse
{

Logger
FareCollectorOrchestratorESV::_logger("atseintl.Fares.FareCollectorOrchestratorESV");

FareCollectorOrchestratorESV::FareCollectorOrchestratorESV(ShoppingTrx& trx,
                                                           TseServer& server,
                                                           TseThreadingConst::TaskId taskId)
  : _trx(&trx), _tseServer(&server), _taskId(taskId)
{
}

FareCollectorOrchestratorESV::~FareCollectorOrchestratorESV() {}

bool
FareCollectorOrchestratorESV::process()
{
  TSELatencyData metrics((*_trx), "FCO PROCESS");

  LOG4CXX_INFO(_logger, "FareCollectorOrchestratorESV::process()");

  PricingTrx& pricingTrx = dynamic_cast<PricingTrx&>((*_trx));
  Diagnostic& trxDiag = pricingTrx.diagnostic();
  bool resetDiag = ((!trxDiag.isActive()) && (trxDiag.diagnosticType() != DiagnosticNone));

  if (resetDiag)
  {
    trxDiag.deActivate();
  }

  Itin& journeyItin = *(_trx->journeyItin());
  journeyItin.fareMarket().clear();

  // Go thorough all fare markets
  std::vector<FareMarket*>::iterator fareMarketIter;

  for (fareMarketIter = _trx->fareMarket().begin(); fareMarketIter != _trx->fareMarket().end();
       fareMarketIter++)
  {
    FareMarket* fareMarket = (*fareMarketIter);

    if (nullptr == fareMarket)
    {
      LOG4CXX_ERROR(_logger, "FareCollectorOrchestratorESV::process - Fare market object is NULL.");
      continue;
    }

    // Set the global direction to ZZ
    fareMarket->setGlobalDirection(GlobalDirection::ZZ);

    // Add fare market to journey itin
    journeyItin.fareMarket().push_back(fareMarket);
  }

  // Setup fare markets
  invoke_foreach_valid_fareMarket(*_trx, journeyItin, setupFareMarket);

  // Read all fares
  exec_foreach_valid_fareMarket(_taskId, *_trx, journeyItin, allFareMarketStepsESV);

  // Sort fares by money amount
  invoke_foreach_valid_fareMarket(*_trx, journeyItin, sortStep);

  // Remove duplicated fares
  invoke_foreach_valid_fareMarket(*_trx, journeyItin, removeDuplicateFares);

  if (resetDiag)
  {
    trxDiag.activate();
  }

  return true;
}

void
FareCollectorOrchestratorESV::setupFareMarket(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  TSELatencyData metrics(trx, "FCO SETUP FARE MARKET");

  LOG4CXX_DEBUG(_logger,
                "FareCollectorOrchestratorESV::setupFareMarket(PricingTrx&, Itin&, FareMarket&)");

  // Process governing carrier perference
  GoverningCarrier governingCarrier(&trx);

  if (!governingCarrier.processCarrierPreference(fareMarket))
  {
    LOG4CXX_ERROR(_logger,
                  "FareCollectorOrchestratorESV::setupFareMarket - Failed governing carrier "
                  "preferences check for carrier"
                      << fareMarket.governingCarrier());
    fareMarket.failCode() = ErrorResponseException::NEED_PREFERRED_CARRIER;

    return;
  }

  // Check sales restriction
  SalesRestrictionByNation saleRestr;

  if (saleRestr.isRestricted(itin, fareMarket, trx))
  {
    LOG4CXX_INFO(_logger, "FareCollectorOrchestratorESV::setupFareMarket - Pricing and Ticketing "
                          "Restricted By Government.");
    fareMarket.failCode() = ErrorResponseException::PRICING_REST_BY_GOV;

    return;
  }

  // Initialize fare market
  if (!fareMarket.initialize(trx))
  {
    LOG4CXX_ERROR(_logger, "FareCollectorOrchestratorESV::setupFareMarket - Unknown exception "
                           "while initializing fare market.");
    fareMarket.failCode() = ErrorResponseException::UNKNOWN_EXCEPTION;

    return;
  }

  FlightTracker flightTracker(trx);
  flightTracker.process(fareMarket);

  return;
}

void
FareCollectorOrchestratorESV::allFareMarketStepsESV(PricingTrx& trx,
                                                    Itin& itin,
                                                    FareMarket& fareMarket)
{
  TSELatencyData metrics(trx, "FCO ALL FARE MARKET STEPS ESV");

  LOG4CXX_DEBUG(
      _logger,
      "FareCollectorOrchestratorESV::allFareMarketStepsESV(PricingTrx&, Itin&, FareMarket&)");

  // Find published fares
  findPublishedFares(trx, itin, fareMarket);

  if (fareMarket.failCode() != ErrorResponseException::NO_ERROR)
  {
    return;
  }

  // Apply multi currency pricing
  if (!ItinUtil::applyMultiCurrencyPricing(&trx, itin))
  {
    FareCollectorOrchestrator::selectCurrencyStep(trx, itin, fareMarket);
  }
}

void
FareCollectorOrchestratorESV::sortStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  TSELatencyData metrics(trx, "FCO SORT STEP");

  LOG4CXX_DEBUG(_logger, "FareCollectorOrchestratorESV::sortStep(PricingTrx&, Itin&, FareMarket&)");

  if (fareMarket.paxTypeCortege().empty() || fareMarket.paxTypeCortege().size() != 1)
  {
    LOG4CXX_ERROR(_logger, "FareCollectorOrchestratorESV::sortStep - More than one pax type "
                           "cortege or pax type cortege is empty for processing fare market");
    return;
  }

  PaxTypeBucket& paxTypeCortege = fareMarket.paxTypeCortege()[0];

  if (paxTypeCortege.paxTypeFare().empty())
  {
    return;
  }

  PaxTypeFare::FareComparator fareComparator(
      paxTypeCortege, 0, trx.getOptions()->isZeroFareLogic());
  std::sort(
      paxTypeCortege.paxTypeFare().begin(), paxTypeCortege.paxTypeFare().end(), fareComparator);
}

void
FareCollectorOrchestratorESV::removeDuplicateFares(PricingTrx& trx,
                                                   Itin& itin,
                                                   FareMarket& fareMarket)
{
  TSELatencyData metrics(trx, "FCO REMOVE DUPLICATE FARES");

  LOG4CXX_DEBUG(
      _logger,
      "FareCollectorOrchestratorESV::removeDuplicateFares(PricingTrx&, Itin&, FareMarket&)");

  if (fareMarket.paxTypeCortege().empty() || fareMarket.paxTypeCortege().size() != 1)
  {
    LOG4CXX_ERROR(_logger, "FareCollectorOrchestratorESV::removeDuplicateFares - More than one pax "
                           "type cortege or pax type cortege is empty for processing fare market");
    return;
  }

  PaxTypeBucket& paxTypeCortege = fareMarket.paxTypeCortege()[0];

  std::vector<PaxTypeFare*>::iterator ptfI = paxTypeCortege.paxTypeFare().begin();

  while ((!paxTypeCortege.paxTypeFare().empty()) &&
         (ptfI != paxTypeCortege.paxTypeFare().end() - 1))
  {
    PaxTypeFare* paxTypeFare = (*ptfI);
    std::vector<PaxTypeFare*>::iterator ptfINext = ptfI + 1;

    while (ptfINext != paxTypeCortege.paxTypeFare().end())
    {
      PaxTypeFare* paxTypeFareNext = (*ptfINext);

      if (dupFare(*paxTypeFare, *paxTypeFareNext))
      {
        if ((paxTypeFare->footNote1() == paxTypeFareNext->footNote1()) &&
            (paxTypeFare->footNote2() == paxTypeFareNext->footNote2()))
        {
          ptfINext = paxTypeCortege.paxTypeFare().erase(ptfINext);
        }
        else
        {
          std::vector<PaxTypeFare*>::iterator ptfINext2 = ptfINext + 1;

          while (ptfINext2 != paxTypeCortege.paxTypeFare().end())
          {
            const PaxTypeFare* paxTypeFareNext2 = (*ptfINext2);

            if (!dupFare(*paxTypeFare, *paxTypeFareNext2))
            {
              break;
            }

            if ((paxTypeFare->footNote1() == paxTypeFareNext2->footNote1()) &&
                (paxTypeFare->footNote2() == paxTypeFareNext2->footNote2()))
            {
              ptfINext2 = paxTypeCortege.paxTypeFare().erase(ptfINext2);
            }
            else
            {
              ++ptfINext2;
            }
          }

          ptfI = ptfINext;
          break;
        }
      }
      else
      {
        ptfI = ptfINext;
        break;
      }
    }

    if ((ptfI == paxTypeCortege.paxTypeFare().end()) ||
        (ptfI == paxTypeCortege.paxTypeFare().end() - 1))
    {
      break;
    }
  }
}

bool
FareCollectorOrchestratorESV::findPublishedFares(PricingTrx& trx,
                                                 Itin& itin,
                                                 FareMarket& fareMarket)
{
  TSELatencyData metrics(trx, "FCO FIND PUBLISHED FARES");

  LOG4CXX_DEBUG(
      _logger, "FareCollectorOrchestratorESV::findPublishedFares(PricingTrx&, Itin&, FareMarket&)");

  CarrierFareController carrierFareController(trx, itin, fareMarket);

  std::vector<Fare*> publishedFares;

  if (fareMarket.governingCarrier() != INDUSTRY_CARRIER)
  {
    if (!carrierFareController.processESV(publishedFares))
    {
      LOG4CXX_INFO(_logger, "CarrierFareController.process() returned false");
      return false;
    }
  }

  return true;
}

bool
FareCollectorOrchestratorESV::dupFare(const PaxTypeFare& ptf1, const PaxTypeFare& ptf2)
{
  const MoneyAmount diff = abs(ptf2.nucFareAmount() - ptf1.nucFareAmount());

  if ((diff < EPSILON) && (ptf1.fareClass() == ptf2.fareClass()) &&
      (ptf1.vendor() == ptf2.vendor()) && (ptf1.tcrRuleTariff() == ptf2.tcrRuleTariff()) &&
      (ptf1.carrier() == ptf2.carrier()) && (ptf1.ruleNumber() == ptf2.ruleNumber()) &&
      (ptf1.owrt() == ptf2.owrt()) && (ptf1.currency() == ptf2.currency()) &&
      (ptf1.directionality() == ptf2.directionality()) &&
      (ptf1.routingNumber() == ptf2.routingNumber()) &&
      (ptf1.fare()->fareInfo()->getPaxType() == ptf2.fare()->fareInfo()->getPaxType()))
  {
    return true;
  }

  return false;
}
} //tse
