//----------------------------------------------------------------------------
//  File:        FareValidatorOrchestratorESV.cpp
//  Created:     2008-04-16
//
//  Description: Orchestrator class for ESV fares validation
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

#include "Fares/FareValidatorOrchestratorESV.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"
#include "Common/ShoppingUtil.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/ShoppingTrx.h"
#include "Fares/FareValidatorESV.h"
#include "Rules/FareMarketRuleController.h"
#include "Server/TseServer.h"

#include <deque>

namespace tse
{
Logger
FareValidatorOrchestratorESV::_logger("atseintl.Fares.FareValidatorOrchestratorESV");

namespace
{
ConfigurableValue<ConfigVector<std::string>>
esvValidation("RULECATEGORY",
              "ESV_VALIDATION",
              std::string("1|15S|2|3|5|6|7|11|14|15|8FR|4FR|9FR|2FR|14FR|4Q"));
}

const std::string FareValidatorOrchestratorESV::_default_validation_order =
    "1|15S|2|3|5|6|7|11|14|15|8FR|4FR|9FR|2FR|14FR|4Q";

FareValidatorOrchestratorESV::FareValidatorOrchestratorESV(ShoppingTrx& trx, TseServer& server)
  : _trx(&trx), _tseServer(&server), _rule_validation_order(0)
{
}

FareValidatorOrchestratorESV::~FareValidatorOrchestratorESV()
{
}

bool
FareValidatorOrchestratorESV::process()
{
  TSELatencyData metrics((*_trx), "FVO PROCESS");

  LOG4CXX_INFO(_logger, "FareValidatorOrchestratorESV::process()");

  if ((*_trx).legs().empty())
  {
    LOG4CXX_ERROR(_logger, "FareValidatorOrchestratorESV::process - Legs vector is empty.");
    return false;
  }

  initializeMap();

  for (const auto& op : esvValidation.getValue())
  {
    if (0 == op.compare("1"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_1);
    }
    else if (0 == op.compare("2"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_2);
    }
    else if (0 == op.compare("2FR"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_2FR);
    }
    else if (0 == op.compare("3"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_3);
    }
    else if (0 == op.compare("4FR"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_4FR);
    }
    else if (0 == op.compare("4Q"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_4Q);
    }
    else if (0 == op.compare("5"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_5);
    }
    else if (0 == op.compare("6"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_6);
    }
    else if (0 == op.compare("7"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_7);
    }
    else if (0 == op.compare("8FR"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_8FR);
    }
    else if (0 == op.compare("9FR"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_9FR);
    }
    else if (0 == op.compare("11"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_11);
    }
    else if (0 == op.compare("14"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_14);
    }
    else if (0 == op.compare("14FR"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_14FR);
    }
    else if (0 == op.compare("15S"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_15S);
    }
    else if (0 == op.compare("15"))
    {
      _rule_validation_order.push_back(FareValidatorESV::CAT_15);
    }
    else
    {
      LOG4CXX_ERROR(_logger, "FareValidatorOrchestratorESV::process - Unsupported rule category");
    }
  }

  // Prepare data structures for each thread
  std::deque<GovCxrValidatorESV> tasks;
  std::deque<Itin> journeyItins;
  std::deque<RuleControllers> ruleControllersESV;

  // Go thorough all legs
  std::vector<ShoppingTrx::Leg>::iterator legIter;

  for (legIter = (*_trx).legs().begin(); legIter != (*_trx).legs().end(); legIter++)
  {
    ShoppingTrx::Leg& leg = (*legIter);

    // Go thorough all carriers keys
    ItinIndex::ItinMatrixIterator matrixIter;

    for (matrixIter = leg.carrierIndex().root().begin();
         matrixIter != leg.carrierIndex().root().end();
         matrixIter++)
    {
      ItinIndex::ItinRow& itinRow = matrixIter->second;

      std::vector<uint16_t> PVseq;
      PVseq.push_back(0);

      std::vector<uint16_t> SCVseq;
      SCVseq.push_back(0);

      std::vector<uint16_t> FRVseq;
      FRVseq.push_back(0);

      std::vector<uint16_t> C4Qseq;
      C4Qseq.push_back(0);

      RuleControllers ruleControllerESV(PVseq, SCVseq, FRVseq, C4Qseq);

      ruleControllersESV.push_back(ruleControllerESV);

      // Do copy of some data structures for each thread
      journeyItins.push_back(*((*_trx).journeyItin()));

      // Create task object for specified governing carrier
      GovCxrValidatorESV task((*_trx),
                              (*_tseServer),
                              leg,
                              itinRow,
                              &journeyItins.back(),
                              ruleControllersESV.back(),
                              _rule_validation_order);

      tasks.push_back(task);
    }
  }

  // Run thread for every governing carrier
  return runAllGovCarrierThreads(tasks);
}

bool
FareValidatorOrchestratorESV::runAllGovCarrierThreads(std::deque<GovCxrValidatorESV>& tasks)
{
  TSELatencyData metrics((*_trx), "FVO RUN ALL CARRIER THREADS");

  LOG4CXX_DEBUG(
      _logger,
      "FareValidatorOrchestratorESV::runAllGovCarrierThreads(std::deque<GovCxrValidatorESV>&)");

  // Process each governing carrier in separate thread
  TseRunnableExecutor taskExecutor(TseThreadingConst::SHOPPING_TASK);

  // Go thorough all carrier tasks
  std::deque<GovCxrValidatorESV>::iterator taskIter;

  for (taskIter = tasks.begin(); taskIter != tasks.end(); taskIter++)
  {
    GovCxrValidatorESV& task = *taskIter;

    // It will run processGovCarrierESV method in separate thread for each
    // governing carrier
    taskExecutor.execute(task);
  }

  taskExecutor.wait();

  return true;
}

void
FareValidatorOrchestratorESV::initializeMap()
{
  TSELatencyData metrics((*_trx), "FVO INITIALIZE MAP");

  LOG4CXX_DEBUG(_logger, "FareValidatorOrchestratorESV::initializeMap()");

  // Go thorough all legs
  std::vector<ShoppingTrx::Leg>::iterator legIter;

  for (legIter = (*_trx).legs().begin(); legIter != (*_trx).legs().end(); legIter++)
  {
    ShoppingTrx::Leg& leg = (*legIter);

    std::vector<ShoppingTrx::SchedulingOption>::iterator sopIter;

    uint32_t flightId = 0;

    // Go thorough all scheduling options
    for (sopIter = leg.sop().begin(); sopIter != leg.sop().end(); sopIter++)
    {
      if (true == sopIter->getDummy())
      {
        continue;
      }

      Itin* curItin = sopIter->itin();

      // Go thorough all fare markets
      std::vector<FareMarket*>::iterator fareMarketIter;

      for (fareMarketIter = curItin->fareMarket().begin();
           fareMarketIter != curItin->fareMarket().end();
           fareMarketIter++)
      {
        FareMarket* fareMarket = (*fareMarketIter);

        if (fareMarket->paxTypeCortege().empty())
        {
          continue;
        }

        // Go thorough all fares
        std::vector<PaxTypeFare*>::iterator paxTypeFareIter;

        for (paxTypeFareIter = fareMarket->paxTypeCortege()[0].paxTypeFare().begin();
             paxTypeFareIter != fareMarket->paxTypeCortege()[0].paxTypeFare().end();
             paxTypeFareIter++)
        {
          PaxTypeFare* paxTypeFare = (*paxTypeFareIter);

          paxTypeFare->setFlightInvalidESV(flightId, 0);
        }
      }

      flightId++;
    }
  }
}
} // tse
