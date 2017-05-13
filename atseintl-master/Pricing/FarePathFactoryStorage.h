#pragma once

#include "Common/Memory/Manager.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "Pricing/FPPQItem.h"

#include <ctime>
#include <memory>

namespace tse
{
namespace Memory
{
class LocalManager;
}
class PricingTrx;

class FarePathFactoryStorage
{

public:
  class IMemoryManager : public Memory::Manager
  {
  public:
    virtual FPPQItem& constructFPPQItem() = 0;
    virtual void releaseFPPQItem(FPPQItem& fppqItem) = 0;

    virtual FarePath& constructFarePath() = 0;
    virtual void releaseFarePath(FarePath& farePath) = 0;

    virtual FareUsage& constructFareUsage() = 0;
    virtual void releaseFareUsage(FareUsage& fareUsage) = 0;

    virtual PricingUnit& constructPricingUnit() = 0;
    virtual void releasePricingUnit(PricingUnit& pricingUnit) = 0;

    virtual void trimMemory() = 0;
  };

  explicit FarePathFactoryStorage();

  void initialize(PricingTrx& trx, Memory::LocalManager* memoryManager, unsigned puSize);

  FPPQItem& constructFPPQItem();
  void releaseFPPQItem(FPPQItem& fppqItem);
  void releaseFPPQItemDeep(FPPQItem& fppqItem);

  FarePath& constructFarePath();
  void releaseFarePath(FarePath& farePath);

  PricingUnit& constructPricingUnit();
  void releasePricingUnit(PricingUnit& pricingUnit);

  FareUsage& constructFareUsage();
  void releaseFareUsage(FareUsage& fareUsage);

  void trimMemory();

private:
  FarePathFactoryStorage(const FarePathFactoryStorage&);
  void operator=(const FarePathFactoryStorage&);

  std::unique_ptr<IMemoryManager> _memoryManager;
  PricingTrx* _trx = nullptr;
  unsigned _puSize = 2;
  bool _isRexTrxAndNewItin = false;
};

inline FPPQItem&
FarePathFactoryStorage::constructFPPQItem()
{
  FPPQItem& fppqItem = _memoryManager->constructFPPQItem();
  fppqItem.puIndices().reserve(_puSize);
  fppqItem.pupqItemVect().reserve(_puSize);
  return fppqItem;
}

inline void
FarePathFactoryStorage::releaseFPPQItem(FPPQItem& fppqItem)
{
  _memoryManager->releaseFPPQItem(fppqItem);
}

inline FarePath&
FarePathFactoryStorage::constructFarePath()
{
  FarePath& farePath = _memoryManager->constructFarePath();
  farePath.pricingUnit().reserve(_puSize);
  return farePath;
}

inline void
FarePathFactoryStorage::releaseFarePath(FarePath& farePath)
{
  _memoryManager->releaseFarePath(farePath);
}

inline PricingUnit&
FarePathFactoryStorage::constructPricingUnit()
{
  PricingUnit& pricingUnit = _memoryManager->constructPricingUnit();
  return pricingUnit;
}

inline void
FarePathFactoryStorage::releasePricingUnit(PricingUnit& pricingUnit)
{
  _memoryManager->releasePricingUnit(pricingUnit);
}

inline FareUsage&
FarePathFactoryStorage::constructFareUsage()
{
  FareUsage& fareUsage = _memoryManager->constructFareUsage();
  return fareUsage;
}

inline void
FarePathFactoryStorage::releaseFareUsage(FareUsage& fareUsage)
{
  _memoryManager->releaseFareUsage(fareUsage);
}

inline void
FarePathFactoryStorage::trimMemory()
{
  _memoryManager->trimMemory();
}

} // namespace tse

