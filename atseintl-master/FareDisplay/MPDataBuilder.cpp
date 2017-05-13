#include "FareDisplay/MPDataBuilder.h"

#include "FareDisplay/MPChooser.h"
#include "FareDisplay/MPDataBuilderImpl.h"

namespace tse
{
MPData*
MPDataBuilder::buildMPData(FareDisplayTrx& trx, MPChooser& chooser) const
{
  return chooser.getMPDataBuilderImpl().buildMPData(trx);
}
}
