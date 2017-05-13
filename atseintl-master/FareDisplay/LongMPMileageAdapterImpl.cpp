#include "FareDisplay/LongMPMileageAdapterImpl.h"

namespace tse
{
bool
LongMPMileageAdapterImpl::getMPM(MPData& mpData) const
{
  return getMPMforGD(mpData);
}
}
