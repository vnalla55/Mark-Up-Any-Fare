/*---------------------------------------------------------------------------
 *  File:    NetFareMarketThreadTask.h
 *
 *  Copyright Sabre 2010
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/

#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TseEnums.h"

namespace tse
{

class NetFareMarketThreadTask : public TseCallableTrxTask
{

public:
  NetFareMarketThreadTask(NetRemitFareSelection& netRemitFareSel,
                          PricingTrx& trx,
                          const Loc* loc1,
                          const Loc* loc2,
                          const DateTime& arrivalDT,
                          const DateTime& departureDT,
                          const FareMarket*& fm)
    : _netRemitFareSel(netRemitFareSel),
      _loc1(loc1),
      _loc2(loc2),
      _arrivalDT(arrivalDT),
      _departureDT(departureDT),
      _fm(fm)
  {
    TseCallableTrxTask::trx(&trx);
    TseCallableTrxTask::desc("NET REMIT FARE MARKET TASK");
  }

  void performTask() override
  {
    _netRemitFareSel.buildFareMarketsTask(_loc1, _loc2, _arrivalDT, _departureDT, _fm);
  }

private:
  NetRemitFareSelection& _netRemitFareSel;
  const Loc* _loc1;
  const Loc* _loc2;
  const DateTime& _arrivalDT;
  const DateTime& _departureDT;
  const FareMarket*& _fm;
};
}
