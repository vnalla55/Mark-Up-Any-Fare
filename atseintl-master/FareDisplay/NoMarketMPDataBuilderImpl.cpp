#include "FareDisplay/NoMarketMPDataBuilderImpl.h"

#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "FareDisplay/MPData.h"

namespace tse
{
MPData*
NoMarketMPDataBuilderImpl::buildMPData(FareDisplayTrx& trx) const
{
  MPData* mpData(nullptr);
  trx.dataHandle().get(mpData);
  mpData->initialize("",
                     "",
                     trx.getRequest()->calcFareAmount(),
                     trx.getOptions()->currencyOverride(),
                     trx.travelDate(),
                     trx.ticketingDate());
  return mpData;
}
}
