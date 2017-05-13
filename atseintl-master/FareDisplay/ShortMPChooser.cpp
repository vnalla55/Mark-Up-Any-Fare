#include "FareDisplay/ShortMPChooser.h"

#include "FareDisplay/ShortMPDataBuilderImpl.h"
#include "FareDisplay/ShortMPMileageAdapterImpl.h"

namespace tse
{
MileageAdapterImpl&
ShortMPChooser::getMileageAdapterImpl() const
{
  return Singleton<ShortMPMileageAdapterImpl>::instance();
}

MPDataBuilderImpl&
ShortMPChooser::getMPDataBuilderImpl() const
{
  return Singleton<ShortMPDataBuilderImpl>::instance();
}

ShortMPChooser::~ShortMPChooser() { unregisterChooser(SHORT_MP); }

MPChooser&
ShortMPChooser::proxy()
{
  return Singleton<ShortMPChooser>::instance();
}
}
