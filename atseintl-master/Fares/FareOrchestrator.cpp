// ----------------------------------------------------------------
//
//   File:        FareOrchestrator.cpp
//   Created:     Dec 4, 2003
//   Authors:     Abu Islam, Mark Kasprowicz, Bruce Melberg,
//                Vadim Nikushin
//
//   Description: Base class for all Fare Orchestrators
//
//   Copyright Sabre 2003
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Fares/FareOrchestrator.h"

#include "Common/Thread/ThreadPoolFactory.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "Server/TseServer.h"

namespace tse
{
FareOrchestrator::FareOrchestrator(const std::string& name,
                                   tse::TseServer& server,
                                   TseThreadingConst::TaskId taskId)
  : Service(name, server), _config(server.config()), _taskId(taskId)
{
}

bool
FareOrchestrator::initialize(int argc, char** argv)
{
  return true;
}

bool
FareOrchestrator::checkIfFailCodeExist(const PricingTrx& trx,
                                       const ErrorResponseException::ErrorResponseCode& failCode)
    const
{
  std::vector<Itin*>::const_iterator itinIter = trx.itin().begin();
  std::vector<Itin*>::const_iterator itinIterEnd = trx.itin().end();

  for (; itinIter != itinIterEnd; ++itinIter)
  {
    const Itin* itin = *itinIter;

    if (itin == nullptr)
      continue;

    std::vector<FareMarket*>::const_iterator fareMarketIter = itin->fareMarket().begin();
    std::vector<FareMarket*>::const_iterator fareMarketIterEnd = itin->fareMarket().end();

    for (; fareMarketIter != fareMarketIterEnd; ++fareMarketIter)
    {
      const FareMarket* fareMarket = *fareMarketIter;

      if (UNLIKELY(fareMarket == nullptr))
        continue;

      if (fareMarket->failCode() == failCode)
        return true;
    }
  }

  return false;
}

uint32_t
FareOrchestrator::getActiveThreads()
{
  if (UNLIKELY(!ThreadPoolFactory::isMetricsEnabled()))
    return 0;

  return ThreadPoolFactory::getNumberActiveThreads(_taskId);
}
} //tse
