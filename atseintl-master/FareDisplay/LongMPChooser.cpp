#include "FareDisplay/LongMPChooser.h"

#include "FareDisplay/LongMPDataBuilderImpl.h"
#include "FareDisplay/LongMPMileageAdapterImpl.h"

namespace tse
{
MileageAdapterImpl&
LongMPChooser::getMileageAdapterImpl() const
{
  return Singleton<LongMPMileageAdapterImpl>::instance();
}

MPDataBuilderImpl&
LongMPChooser::getMPDataBuilderImpl() const
{
  return Singleton<LongMPDataBuilderImpl>::instance();
}

LongMPChooser::~LongMPChooser() { unregisterChooser(LONG_MP); }

MPChooser&
LongMPChooser::proxy()
{
  return Singleton<LongMPChooser>::instance();
}
}
