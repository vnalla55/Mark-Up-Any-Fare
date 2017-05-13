#include "FareDisplay/ShortMPDataBuilderImpl.h"

#include "BookingCode/FareDisplayBookingCode.h"
#include "Common/Logger.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/RoutingUtil.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/MPData.h"
#include "FareDisplay/RDFareGroupingMgr.h"
#include "Routing/RoutingConsts.h"

#include <vector>

using std::vector;
using std::string;

namespace tse
{
static Logger
logger("atseintl.FareDisplay.ShortMPDataBuilderImpl");

MPData*
ShortMPDataBuilderImpl::buildMPData(FareDisplayTrx& trx) const
{
  prepareInput(trx);
  const vector<PaxTypeFare*>& fares(trx.allPaxTypeFare());
  MPData* mpData(nullptr);
  if (!fares.empty())
  {
    const PaxTypeFare* fare(fares.front());
    if (fare != nullptr)
    {
      trx.dataHandle().get(mpData);
      mpData->initialize(trx.boardMultiCity(),
                         trx.offMultiCity(),
                         getAmount(*fare, trx.getOptions()),
                         getCurrency(*fare, trx),
                         trx.travelDate(),
                         trx.ticketingDate());
      mpData->setMPM(fare->globalDirection(), 0);
    }
    else
    {
      LOG4CXX_ERROR(logger, "paxTypeFare empty");
    }
  }
  else
  {
    LOG4CXX_ERROR(logger, "allPaxTypeFare vector empty");
  }
  return mpData;
}

MoneyAmount
ShortMPDataBuilderImpl::getAmount(const PaxTypeFare& fare, const FareDisplayOptions* options)
{
  if (options != nullptr)
  {
    if (options->isOneWay())
    {
      if (fare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
        throw NonFatalErrorResponseException(ErrorResponseException::MP_OW_ON_RT_FARE);
      return fare.convertedFareAmount();
    }
    if (options->isHalfRoundTrip())
    {
      if (fare.owrt() != ROUND_TRIP_MAYNOT_BE_HALVED)
        throw NonFatalErrorResponseException(ErrorResponseException::MP_HR_ON_OW_FARE);
      return fare.convertedFareAmount() / 2.0;
    }
  }
  if (fare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    return fare.convertedFareAmount() / 2.0;
  return fare.convertedFareAmount();
}

CurrencyCode
ShortMPDataBuilderImpl::getCurrency(const PaxTypeFare& fare, const FareDisplayTrx& trx)
{
  if (!trx.itin().empty())
  {
    const Itin* itin(trx.itin().front());
    if (itin != nullptr)
      return itin->calculationCurrency();
  }
  return fare.currency();
}

void
ShortMPDataBuilderImpl::prepareInput(FareDisplayTrx& trx)
{
  if (!trx.allPaxTypeFare().empty())
  {
    PaxTypeFare* fare(trx.allPaxTypeFare().front());
    if (fare != nullptr)
    {
      if ((fare->routingNumber() != CAT25_DOMESTIC) &&
          (fare->routingNumber() != CAT25_INTERNATIONAL) &&
          (trx.getOptions()->routingNumber() != MILEAGE_ROUTING))
      {
        if (RoutingUtil::isRouting(*fare, trx))
          throw NonFatalErrorResponseException(ErrorResponseException::MP_WITH_ROUTING_FARE);
      }

      RDFareGroupingMgr fareGroupMgr;
      fareGroupMgr.groupFares(trx);
      trx.fdResponse()->groupHeaders().push_back(Group::GROUP_BY_GLOBAL_DIR);
      if (fare->bookingCode().empty())
      {
        FareDisplayBookingCode fdbc;
        fdbc.getBookingCode(trx, *fare, fare->bookingCode());
      }
    }
  }
}
}
