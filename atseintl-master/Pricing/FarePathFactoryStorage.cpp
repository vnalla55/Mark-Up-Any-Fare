#include "Pricing/FarePathFactoryStorage.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/Logger.h"
#include "Common/Memory/Config.h"
#include "Common/Memory/LocalManager.h"
#include "Common/TseObjectPool.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "Pricing/CountingObjectPoolMemoryManager.h"
#include "Pricing/ObjectPoolMemoryManager.h"

#include <cassert>
#include <cstddef>

namespace tse
{
FarePathFactoryStorage::FarePathFactoryStorage()
{
  if (LIKELY(Memory::managerEnabled))
    _memoryManager.reset(new CountingObjectPoolMemoryManager(Memory::fpStorageCheckPeriod));
  else
    _memoryManager.reset(new ObjectPoolMemoryManager());
}

void
FarePathFactoryStorage::initialize(PricingTrx& trx,
                                   Memory::LocalManager* memoryManager,
                                   unsigned puSize)
{
  _trx = &trx;
  _puSize = puSize;
  _isRexTrxAndNewItin = RexPricingTrx::isRexTrxAndNewItin(trx);

  if (memoryManager)
    memoryManager->registerManager(_memoryManager.get());
}

void
FarePathFactoryStorage::releaseFPPQItemDeep(FPPQItem& fppqItem)
{
  if (FarePath* farePath = fppqItem.farePath())
  {
    // do not release FP for REX pricing in some cases
    // see FarePathFactory::reuseResultsForRex and FarePathFactory::_processedAsBookedFPForRex
    const bool canReleaseFarePath =
        !_isRexTrxAndNewItin || (_isRexTrxAndNewItin && farePath->rebookClassesExists());
    if (LIKELY(canReleaseFarePath))
    {
      if (fppqItem.hasCopiedPricingUnit())
      {
        std::vector<PricingUnit*>& pricingUnits = farePath->pricingUnit();
        for (PricingUnit* pricingUnit : pricingUnits)
        {
          std::vector<FareUsage*>& fareUsages = pricingUnit->fareUsage();
          for (FareUsage* fareUsage : fareUsages)
          {
            // TODO destroy surcharges
            releaseFareUsage(*fareUsage);
          }

          releasePricingUnit(*pricingUnit);
        }
      }
      releaseFarePath(*farePath);
      releaseFPPQItem(fppqItem);
    }
  }
}

} // namespace tse
