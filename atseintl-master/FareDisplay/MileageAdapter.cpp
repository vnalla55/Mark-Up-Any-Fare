#include "FareDisplay/MileageAdapter.h"

#include "FareDisplay/MileageAdapterImpl.h"
#include "FareDisplay/MPChooser.h"

namespace tse
{
bool
MileageAdapter::getMPM(MPData& mpData, MPChooser& chooser) const
{
  return chooser.getMileageAdapterImpl().getMPM(mpData);
}
}
