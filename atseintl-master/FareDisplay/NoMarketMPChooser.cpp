#include "FareDisplay/NoMarketMPChooser.h"

#include "FareDisplay/NoMarketMPDataBuilderImpl.h"
#include "FareDisplay/NoMarketMPMileageAdapterImpl.h"

namespace tse
{
MileageAdapterImpl&
NoMarketMPChooser::getMileageAdapterImpl() const
{
  return Singleton<NoMarketMPMileageAdapterImpl>::instance();
}

MPDataBuilderImpl&
NoMarketMPChooser::getMPDataBuilderImpl() const
{
  return Singleton<NoMarketMPDataBuilderImpl>::instance();
}

NoMarketMPChooser::~NoMarketMPChooser() { unregisterChooser(NO_MARKET_MP); }

MPChooser&
NoMarketMPChooser::proxy()
{
  return Singleton<NoMarketMPChooser>::instance();
}
}
