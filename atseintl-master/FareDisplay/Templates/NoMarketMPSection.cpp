#include "FareDisplay/Templates/NoMarketMPSection.h"

#include "Common/Logger.h"
#include "FareDisplay/MPData.h"

namespace tse
{
static Logger
logger("atseintl.FareDisplay.Templates.NoMarketMPSection");

void
NoMarketMPSection::doBuildDisplay()
{
  if (&_trx != nullptr && _trx.mpData() != nullptr)
  {
    Amounts amounts(SurchargeRanges);
    prepareAmounts(amounts);
    displayHeader();
    displayColumnHeaders();
    displaySpace();
    displayLine(_trx.mpData()->getCurrency());
    displayAmounts(amounts);
  }
  else
  {
    LOG4CXX_ERROR(logger, "NoMarketMPSection not initialized or MPData is null");
  }
}
} // tse namespace
