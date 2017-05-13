#include "FareDisplay/ShortMPMileageAdapterImpl.h"

namespace tse
{
bool
ShortMPMileageAdapterImpl::getMPM(MPData& mpData) const
{
  return getMPMforGD(mpData);
}
}
