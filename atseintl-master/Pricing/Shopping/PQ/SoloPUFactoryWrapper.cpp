#include "Pricing/Shopping/PQ/SoloPUFactoryWrapper.h"

#include "Common/Logger.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/PricingOrchestrator.h"
#include "Pricing/PricingUnitFactoryBucket.h"
#include "Pricing/PUPathMatrix.h"
#include "Pricing/Shopping/PQ/ConxRouteCxrPQItem.h"
#include "Pricing/Shopping/PQ/DiagSoloPQCollector.h"
#include "Pricing/Shopping/PQ/SoloPUPathCollector.h"

namespace tse
{
namespace shpq
{
Logger
SoloPUFactoryWrapper::_logger("atseintl.ShoppingPQ.SoloPUFactoryWrapper");

SoloPUFactoryWrapper::SoloPUFactoryWrapper(ShoppingTrx& trx, PricingOrchestrator& po)
  : _trx(trx), _pricingOrchestrator(po), _puPathCollector(trx)
{
  _pricingOrchestrator.createPricingUnitFactoryBucket(_trx, _puFactoryBucketVec);
}

SoloPUFactoryWrapper::PUStruct
SoloPUFactoryWrapper::getPUStruct(const ConxRouteCxrPQItem* const pqItem, DiagSoloPQCollector& dc)
{
  SoloPUFactoryWrapper::PUStruct puStruct =
      _puPathCollector.buildPUStruct(pqItem, _puFactoryBucketVec);
  dc.displayPUPath(puStruct, pqItem);

  if (puStruct._puPath && puStruct._itin && puStruct.isSinglePax())
    initPUFactory(puStruct._puPath);

  return puStruct;
}

void
SoloPUFactoryWrapper::initPUFactory(const PUPath* const puPath)
{
  DataHandle dataHandle(_trx.ticketingDate());
  TseRunnableExecutor taskExecutor(TseThreadingConst::PRICINGPQ_TASK);

  typedef std::vector<PricingOrchestrator::PUFInitThreadInput*> ThrInputVect;
  typedef std::map<PU*, PricingUnitFactory*> PUFactoryMap;

  ThrInputVect thrInputVect;
  PUFactoryMap& puFactoryBucket = _puFactoryBucketVec.front()->puFactoryBucket();

  PUFactoryMap::const_iterator puMapSearchIt;
  PUFactoryMap::const_iterator puMapEndIt = puFactoryBucket.end();

  for (auto pu : puPath->puPath())
  {
    std::pair<PUSet::iterator, bool> insertResult = _updatedPUFactorySet.insert(pu);
    if (!insertResult.second)
      continue;

    puMapSearchIt = puFactoryBucket.find(pu);
    if (LIKELY(puMapSearchIt != puMapEndIt))
    {
      _pricingOrchestrator.initPricingUnitFactory(
          _trx, puMapSearchIt->second, dataHandle, taskExecutor, thrInputVect);
    }
  }

  taskExecutor.wait();

  for (const auto puf : thrInputVect)
  {
    if (UNLIKELY(puf->errResponseCode != ErrorResponseException::NO_ERROR))
    {
      LOG4CXX_TRACE(_logger,
                    "SoloPUFactoryWrapper::initPUFactory() (Error code: "
                        << puf->errResponseCode << ") " << puf->errResponseMsg);
    }
  }
}
}
} // namespace tse::shpq
